#include "headers.h"

IVTEntry* IVTEntry::routines[256];

IVTEntry::IVTEntry(IVTNo numEntry, pInterrupt interruptRoutine) {
	this->ivtNo = numEntry;
#ifndef BCC_BLOCK_IGNORE
	this->oldRoutine = getvect(numEntry);
	setvect(numEntry, interruptRoutine);
#endif
	IVTEntry::routines[numEntry] = this;
	this->event = 0;
}

IVTEntry::~IVTEntry() {
#ifndef BCC_BLOCK_IGNORE
	setvect(this->ivtNo, this->oldRoutine);
#endif
	IVTEntry::routines[this->ivtNo] = 0;
	this->event = 0;
}

void IVTEntry::signal() {
	this->event->signal();
}
