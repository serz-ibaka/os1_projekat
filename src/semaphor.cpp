#include "headers.h"

Semaphore::Semaphore (int init) {
	this->myImpl = new KernelSem(init);
}

Semaphore::~Semaphore () {
	delete this->myImpl;
}

int Semaphore::wait (Time maxTimeToWait) {
	return this->myImpl->wait(maxTimeToWait);
}

void Semaphore::signal() {
	this->myImpl->signal();
}

int Semaphore::val () const { return this->myImpl->val; }
