#include "headers.h"

int main(int argc, char* argv[]) {
	Kernel::running = new PCB();

	Kernel::initialization();
	UserThread user(argc, argv);
	user.start();
	int ret = user.res;
	user.waitToComplete();
	Kernel::restoreTimer();

	return ret;
}
