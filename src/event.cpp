#include "headers.h"


Event::Event(IVTNo ivtNo) {
	this->myImpl = new KernelEv(ivtNo);
	IVTEntry::routines[ivtNo]->event = this;
}

Event::~Event() {
	delete this->myImpl;
	this->myImpl = 0;
}

void Event::wait() { this->myImpl->wait(); }

void Event::signal() { this->myImpl->signal(); }
