#include "headers.h"

Idle::Idle() : Thread(4096, 1) {
	this->myPCB->status = READY;
}

void Idle::run() {
	while(1);
}
