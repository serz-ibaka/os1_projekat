#include "headers.h"

Thread::Thread(StackSize stackSize, Time timeSlice) {
	this->myPCB = new PCB(stackSize, timeSlice, this);
}

Thread::~Thread() {
	delete this->myPCB;
}

void Thread::start() {
	this->myPCB->start();
}

void Thread::waitToComplete() {
	this->myPCB->waitToComplete();
}

ID Thread::getId() {
	return this->myPCB->getId();
}

ID Thread::getRunningId() {
	return PCB::getRunningId();
}

Thread* Thread::getThreadById(ID id) {
	return PCB::getThreadById(id);
}

void dispatch() {
	Kernel::dispatch();
}

ID Thread::fork() {
	return PCB::fork();
}

void Thread::exit() {
	PCB::exit();
}

void Thread::waitForForkChildren() {
	PCB::waitForForkChildren();
}

Thread* Thread::clone() const {
	return new Thread(this->myPCB->stackSize * sizeof(unsigned), this->myPCB->timeSlice);
}

