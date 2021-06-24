#ifndef _KERNELEV_H_
#define _KERNELEV_H_

typedef unsigned char IVTNo;

class KernelEv {
public:

	int val;
	int ivtNo;
	PCB* thread;

	// static KernelEv* events[256];

	KernelEv(IVTNo ivtNo);
	~KernelEv();

	void wait();
	void signal();


};

#endif
