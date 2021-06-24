#include "headers.h"

// KernelEv* KernelEv::events[256];

KernelEv::KernelEv(IVTNo ivtNo) {
	this->val = 0;
	this->ivtNo = ivtNo;
	this->thread = (PCB*)Kernel::running;
	// KernelEv::events[ivtNo] = this;
}

KernelEv::~KernelEv() {
	if(this->thread->status == BLOCKED) {
		this->thread->status = READY;
		Scheduler::put(this->thread);
	}
	this->thread = 0;
}

void KernelEv::wait() {
	if (Kernel::running == this->thread) {
		if (this->val == 0) {
			this->thread->status = BLOCKED;
			this->val = 1;
			Kernel::dispatch();
		} else this->val = 0;
	}
}

void KernelEv::signal() {
	if (this->thread->status == BLOCKED) {
		this->thread->status = READY;
		Scheduler::put(this->thread);
		this->val = 0;
	}
	else this->val = 1;
	Kernel::dispatch();
}
