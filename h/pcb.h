#ifndef _PCH_H_
#define _PCB_H_

class Thread;

typedef unsigned long StackSize;
typedef unsigned int Time; // time, x 55ms
typedef int ID;

typedef enum Status {NEW, READY, BLOCKED, FINISHED} Status;
typedef enum SemaphoreDeblock { SIGNAL, TIMEPASSED } SemaphoreDeblock;

class PCB {
public:

	Thread* myThread;

	StackSize stackSize;
	unsigned* stack;
	unsigned ss;
	unsigned sp;
	unsigned bp;

	Status status;
	SemaphoreDeblock deblock;

	Time timeSlice;
	Time timePassed, timeToWait;

	static ID currId;
	ID id;

	// lista PCB-ova koji cekaju na zavrsetak this
	typedef struct PCBList {
		PCB* pcb;
		PCBList* next;
	} PCBList;

	PCBList* waitingToComplete;

	static PCBList* threads;

	PCB(StackSize stackSize, Time timeSlice, Thread* thread);
	PCB(); // za main nit
	~PCB();

	static void wrapper();

	void start();
	void waitToComplete();

	int getId();
	static int getRunningId();
	static Thread* getThreadById(int id);

	// ######### FORK

	PCB* parent;
	int childrenCount;
	PCB* jasonKidd;

	static void exit();
	static void waitForForkChildren();
	static ID fork();

	static PCB* parentCopyStack;
	static PCB* childCopyStack;

};

void interrupt copyStack();

#endif
