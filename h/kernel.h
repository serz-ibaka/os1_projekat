#ifndef _KERNEL_H_
#define _KERNEL_H_

class PCB;

typedef unsigned int Time;


typedef enum DispatchFlag {ON, OFF} DispatchFlag;

class Kernel {
public:
	static void setCustomTimer();
	static void initialization();
	static void restoreTimer();
	static void restore();
	static void dispatch();

	volatile static PCB* running;
	volatile static PCB* idle;

	volatile static DispatchFlag dispatchFlag;
	volatile static Time currentTimeSlice;
	volatile static int lockCnt;
	volatile static int flag;

};

void interrupt timer();

int syncPrintf(const char* format, ...);

#endif
