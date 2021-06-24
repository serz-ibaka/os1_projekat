#include "headers.h"

ID KernelSem::currId = 0;

KernelSem::SemList* KernelSem::semaphores = 0;

KernelSem::KernelSem(int init) {
	val = init;
	head = 0;
	tail = 0;
	id = currId++;
	// dodavanje semafora u listu semafora
	SemList* node = new SemList();
	node->sem = this;
	node->next = KernelSem::semaphores;
	KernelSem::semaphores = node;
}

KernelSem::~KernelSem() {
	// aktiviranje niti koje cekaju na semaforu
	/*PCBList* head = this->head;
	while(head) {
		head->pcb->status = READY;
		// Scheduler::put(head->pcb);
		PCBList* temp = head;
		head = head->next;
		delete temp;
	}*/
}

int KernelSem::wait(Time maxTimeToWait) {
	Kernel::lockCnt++;
	int rez = 1;

	Kernel::running->deblock = SIGNAL;
	if(--this->val < 0) {
		Kernel::running->timeToWait = maxTimeToWait;
		Kernel::running->timePassed = 0;
		PCBList* node = new PCBList();
		node->pcb = (PCB*)Kernel::running;
		node->pcb->status = BLOCKED;
		if(this->head == 0) {
			this->head = node;
			node->prev = 0;
		} else  {
			this->tail->next = node;
			node->prev = this->tail;
		}
		node->next = 0;
		this->tail = node;

		Kernel::lockCnt--;
		Kernel::dispatch();
		Kernel::lockCnt++;
	}
	if(Kernel::running->deblock == TIMEPASSED) rez = 0;
	Kernel::lockCnt--;
	return rez;
}

void KernelSem::signal() {
	Kernel::lockCnt++;
	if(++this->val <= 0) {
		PCBList* node = this->head;
		PCB* pcb = node->pcb;
		pcb->status = READY;
		pcb->deblock = SIGNAL;
		Scheduler::put(pcb);
		if(this->head == this->tail) {
			this->head = this->tail = 0;
		} else {
			node->next->prev = 0;
			this->head = node->next;
		}
		delete node;
	}
	Kernel::lockCnt--;
}

void KernelSem::semaphorTimer() {
	SemList* semNode = KernelSem::semaphores;
	while (semNode != 0) {
		PCBList* node = semNode->sem->head;
		while(node != 0) {
			PCBList* next = node->next;
			if(node->pcb->timeToWait != 0) {
				node->pcb->timePassed++;
				if(node->pcb->timePassed == node->pcb->timeToWait) {
					node->pcb->status = READY;
					node->pcb->deblock = TIMEPASSED;
					node->pcb->timeToWait = node->pcb->timePassed = 0;
					Scheduler::put(node->pcb);

					if(node->prev == 0) semNode->sem->head = node->next;
					else node->prev->next = node->next;
					if(node->next == 0) semNode->sem->tail = node->prev;
					else node->next->prev = node->prev;

					delete node;
					semNode->sem->val++;
				}
			}
			node = next;
		}
		semNode = semNode->next;
	}
}
