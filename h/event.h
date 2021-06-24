#ifndef _event_h_
#define _event_h_

typedef unsigned char IVTNo;
class KernelEv;

class Event {
public:
	Event (IVTNo ivtNo);
	~Event();

	void wait();

protected:
	friend class KernelEv;
	friend class IVTEntry;
	void signal();

private:
	KernelEv* myImpl;

};

#endif
