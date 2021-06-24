#ifndef _USERTHREAD_H_
#define _USERTHREAD_H_

#include "thread.h"

int userMain(int argc, char** argv);
class UserThread : public Thread {
public:
	UserThread(int argc, char** argv) : Thread(), argc(argc), argv(argv), res(0) {}
	~UserThread() { waitToComplete(); }

	int argc;
	char** argv;
	int res;

	void run() {
		res = userMain(argc, argv);
	}

	Thread* clone() const {
		return new UserThread(argc, argv);
	}

};

#endif
