#ifndef _KERNSEM_H_
#define _KERNSEM_H_

class PCB;

typedef unsigned int Time; // time, x 55ms
typedef int ID;

class KernelSem {
public:
	KernelSem(int init = 1);
	~KernelSem();

	volatile int val;

	typedef struct PCBList {
		PCB* pcb;
		struct PCBList* prev, *next;
	} PCBList;
	PCBList* head, * tail;

	int wait (Time maxTimeToWait);
	void signal();

	static ID currId;
	ID id;

	// lista semafora
	typedef struct SemList {
		KernelSem* sem;
		struct SemList* next;
	} SemList;
	static SemList* semaphores;

	static void semaphorTimer();

};

#endif
