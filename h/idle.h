#ifndef _IDLE_H_
#define _IDLE_H_

#include "thread.h"

class Idle : public Thread {
public:
	Idle();
	void run();
};

#endif
