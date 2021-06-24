#include "headers.h"

ID PCB::currId = 0;

PCB::PCBList* PCB::threads = 0;
PCB* PCB::parentCopyStack = 0;
PCB* PCB::childCopyStack = 0;

PCB::PCB(StackSize stackSize, Time timeSlice, Thread* thread) {
	this->timeSlice = timeSlice;
	timePassed = this->timeToWait = 0;
	myThread = thread;

	status = NEW;
	deblock = SIGNAL;

	if(stackSize > 65536) stackSize = 65536;
	if(stackSize < 1024) stackSize = 1024;
	stackSize /= sizeof(unsigned);
	this->stackSize = stackSize;
	stack = new unsigned[stackSize];

#ifndef BCC_BLOCK_IGNORE
	stack[stackSize-1] = 0; // za forkanje da dodje bp poslednji na ovo
	stack[stackSize-2] = 0x200;
	stack[stackSize-3] = FP_SEG(PCB::wrapper);
	stack[stackSize-4] = FP_OFF(PCB::wrapper);
	stack[stackSize-13] = FP_OFF(stack + stackSize - 1);
	ss = FP_SEG(stack + stackSize - 13);
	sp = FP_OFF(stack + stackSize - 13);
	bp = sp;
#endif

	this->id = currId++;

	this->waitingToComplete = 0;

	PCBList* node = new PCBList();
	node->pcb = this;
	node->next = PCB::threads;
	PCB::threads = node;

	// za fork
	childrenCount = 0;
	parent = 0;
	jasonKidd = 0;

}

PCB::PCB() { // samo za main nit
	myThread = 0;
	timeSlice = 2;
	timePassed = timeToWait = 0;
	id = currId++;
	status = READY;
	deblock = SIGNAL;
	waitingToComplete = 0;

	PCBList* node = new PCBList();
	node->pcb = this;
	node->next = PCB::threads;
	PCB::threads = node;

	// za fork
	childrenCount = 0;
	parent = 0;
	jasonKidd = 0;

	stack = 0;
}

PCB::~PCB() {
	// budjenje niti koje su cekale
	PCBList* node = this->waitingToComplete;
	while(node) {
		node->pcb->status = READY;
		Scheduler::put(node->pcb);
		PCBList* temp = node;
		node = node->next;
		delete temp;
	}

	delete[] this->stack;
}

void PCB::wrapper() {
	Kernel::running->myThread->run();
	Kernel::running->status = FINISHED;
	// deblokiranje i dealokacija niti koje su cekale
	PCBList* node = Kernel::running->waitingToComplete;
	while(node) {
		node->pcb->status = READY;
		Scheduler::put(node->pcb);
		PCBList* temp = node;
		node = node->next;
		delete temp;
	}
	Kernel::running->waitingToComplete = 0;

	// aktiviranje roditelja
	if(Kernel::running->parent != 0) {
		Kernel::running->parent->childrenCount--;
		if(Kernel::running->parent->childrenCount == 0 && Kernel::running->parent->status == BLOCKED) {
			Kernel::running->parent->status = READY;
			Scheduler::put(Kernel::running->parent);
		}
	}
	Kernel::running->parent = 0;

	Kernel::dispatch();
}

void PCB::start() {
	if(this->status == NEW) {
		Scheduler::put(this);
		this->status = READY;
	}
}

void PCB::waitToComplete() {
	if(Kernel::running == this || this->status == FINISHED) return;

	Kernel::running->status = BLOCKED;
	PCBList* node = new PCBList();
	node->pcb = (PCB*)Kernel::running;
	node->next = this->waitingToComplete;
	this->waitingToComplete = node;
	Kernel::dispatch();
}

int PCB::getId() {
	return this->id;
}

int PCB::getRunningId() {
	return Kernel::running->id;
}

Thread* PCB::getThreadById(int id) {
	PCBList* node = PCB::threads;
	while(node != 0) {
		if(node->pcb->id == id) return node->pcb->myThread;
		node = node->next;
	}
	return 0;
}

void PCB::exit() {
	Kernel::running->status = FINISHED;

	// deblokiranje i dealokacija niti koje su cekale
	PCBList* node = Kernel::running->waitingToComplete;
	while(node) {
		node->pcb->status = READY;
		Scheduler::put(node->pcb);
		PCBList* temp = node;
		node = node->next;
		delete temp;
	}
	Kernel::running->waitingToComplete = 0;

	// aktiviranje roditelja
	if(Kernel::running->parent != 0) {
		Kernel::running->parent->childrenCount--;
		if(Kernel::running->parent->childrenCount == 0 && Kernel::running->parent->status == BLOCKED) {
			Kernel::running->parent->status = READY;
			Scheduler::put(Kernel::running->parent);
		}
	}
	Kernel::running->parent = 0;

	Kernel::dispatch();
}

void PCB::waitForForkChildren() {
	if(Kernel::running->childrenCount > 0) {
		Kernel::running->status = BLOCKED;
		Kernel::dispatch();
	}
}

void interrupt copyStack() {
	for(StackSize i = 0; i < PCB::parentCopyStack->stackSize; i++) {
		PCB::childCopyStack->stack[i] = PCB::parentCopyStack->stack[i];
	}
#ifndef BCC_BLOCK_IGNORE
	PCB::childCopyStack->ss = FP_SEG(PCB::childCopyStack->stack);
#endif
	PCB::childCopyStack->bp = (unsigned)(PCB::childCopyStack->stack + ((unsigned*)PCB::parentCopyStack->bp - PCB::parentCopyStack->stack));

	while(PCB::childCopyStack->bp != 0) { // dok se ne dodje do podmetnutog bp
#ifndef BCC_BLOCK_IGNORE
		unsigned novi = *((unsigned*)(MK_FP(PCB::childCopyStack->ss, PCB::childCopyStack->bp)));
		PCB::childCopyStack->stack[(unsigned*)PCB::childCopyStack->bp - PCB::childCopyStack->stack] = novi;
		PCB::childCopyStack->bp = novi;
#endif
	}

	PCB::childCopyStack->sp = (unsigned)(PCB::childCopyStack->stack + ((unsigned*)PCB::parentCopyStack->sp - PCB::parentCopyStack->stack));
	PCB::childCopyStack->bp = (unsigned)(PCB::childCopyStack->stack + ((unsigned*)PCB::parentCopyStack->bp - PCB::parentCopyStack->stack));
}

ID PCB::fork() {
	Kernel::lockCnt++;
	Thread* child = Kernel::running->myThread->clone();
	if(child == 0 || child->myPCB == 0 || child->myPCB->stack == 0) return -1;
	if(child->getId() != PCB::parentCopyStack->id) {
		PCB::parentCopyStack = 0;
		return 0;
	}
	PCB::parentCopyStack = (PCB*)Kernel::running;
	PCB::childCopyStack = child->myPCB;

	copyStack();

	if(child->getId() == getRunningId()) {
		return 0;
	} else {
		PCB::childCopyStack = 0;
		Kernel::running->childrenCount++;
		child->myPCB->parent = (PCB*)Kernel::running;
		child->start();
		Kernel::lockCnt--;
		return child->getId();
	}
}




