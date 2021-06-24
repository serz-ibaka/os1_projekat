#ifndef _IVTENTRY_H_
#define _IVTENTRY_H_

#define PREPAREENTRY(numEntry, callOld)\
void interrupt inter##numEntry(...);\
IVTEntry newEntry##numEntry(numEntry, inter##numEntry);\
void interrupt inter##numEntry(...) {\
newEntry##numEntry.signal();\
if (callOld == 1) newEntry##numEntry.callOldRoutine();\
}

class KernelEv;

typedef void interrupt (*pInterrupt)(...);

class IVTEntry {

public:
	IVTEntry(IVTNo, pInterrupt);
	~IVTEntry();

	int ivtNo;

	pInterrupt oldRoutine;

	static IVTEntry* routines[256];

	void callOldRoutine() { (*oldRoutine)(); }

	void signal();

	Event* event;
};

#endif
