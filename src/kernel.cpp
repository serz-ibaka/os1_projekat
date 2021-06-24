#include "headers.h"

volatile DispatchFlag Kernel::dispatchFlag = OFF;
volatile PCB* Kernel::running = 0;
static Idle idleThread;
volatile PCB* Kernel::idle = idleThread.myPCB;
volatile Time Kernel::currentTimeSlice = 2;
volatile int Kernel::lockCnt = 0;
volatile int Kernel::flag = 0;

// stara prekidna rutina
unsigned oldTimerSEG;
unsigned oldTimerOFF;
// postavlja novi tajmer na ulaz 0x08
void Kernel::setCustomTimer() {
#ifndef BCC_BLOCK_IGNORE
	asm{
		cli
		push es
		push ax
		mov ax,0
		mov es,ax // es = 0

		// pamti staru rutinu
		mov ax, word ptr es:0022h
		mov word ptr oldTimerSEG, ax
		mov ax, word ptr es:0020h
		mov word ptr oldTimerOFF, ax

		// postavlja novu rutinu
		mov word ptr es:0022h, seg timer
		mov word ptr es:0020h, offset timer

		// postavlja staru rutinu na int 60h
		mov ax, oldTimerSEG
		mov word ptr es:0182h, ax
		mov ax, oldTimerOFF
		mov word ptr es:0180h, ax

		pop ax
		pop es
		sti
	}
#endif
}

void Kernel::initialization() {
	Kernel::setCustomTimer();
}

void Kernel::restoreTimer() {
#ifndef BCC_BLOCK_IGNORE
	asm{
		cli
		push es
		push ax

		mov ax,0
		mov es,ax
		mov ax, word ptr oldTimerSEG
		mov word ptr es:0022h, ax

		mov ax, word ptr oldTimerOFF
		mov word ptr es:0020h, ax

		pop ax
		pop es
		sti
	}
#endif
}

void Kernel::restore() {
	Kernel::restoreTimer();
}

unsigned tsp;
unsigned tss;
unsigned tbp;
void tick();
void interrupt timer() {
	if(Kernel::dispatchFlag == OFF) Kernel::flag = 0;
	if(Kernel::flag == 0) {
		tick();
		KernelSem::semaphorTimer();
		if(Kernel::currentTimeSlice > 0) Kernel::currentTimeSlice--;
#ifndef BCC_BLOCK_IGNORE
		asm int 60h;
#endif
	}

	if(Kernel::dispatchFlag == ON || (Kernel::currentTimeSlice == 0 && Kernel::running->timeSlice > 0)) {
		if(Kernel::lockCnt == 0 || Kernel::dispatchFlag == ON) {
			Kernel::flag = 0;
			Kernel::dispatchFlag = OFF;
#ifndef BCC_BLOCK_IGNORE
			asm{
				mov tss, ss
				mov tsp, sp
				mov tbp, bp
			}
#endif
			Kernel::running->ss = tss;
			Kernel::running->sp = tsp;
			Kernel::running->bp = tbp;

			if((PCB*)Kernel::running != Kernel::idle && Kernel::running->status == READY) Scheduler::put((PCB*)Kernel::running);

			syncPrintf("Promena konteksta: %d -> ", PCB::getRunningId());
			Kernel::running = Scheduler::get();
			if(Kernel::running == 0) Kernel::running = Kernel::idle;
			syncPrintf("%d\n", PCB::getRunningId());

			tsp = Kernel::running->sp;
			tss = Kernel::running->ss;
			tbp = Kernel::running->bp;
			Kernel::currentTimeSlice = Kernel::running->timeSlice;
#ifndef BCC_BLOCK_IGNORE
			asm{
				mov sp, tsp
				mov ss, tss
				mov bp, tbp
			}
#endif
		} else Kernel::dispatchFlag = ON;
	}
}

void Kernel::dispatch() {
	Kernel::lockCnt++;
	if(Kernel::dispatchFlag == ON) Kernel::flag = 0;
	else Kernel::flag = 1;
	Kernel::dispatchFlag = ON;
	timer();
	Kernel::lockCnt--;
}

int syncPrintf(const char* format, ...) {
	Kernel::lockCnt++;
	int res;
	va_list args;
	va_start(args, format);
	res = vprintf(format, args);
	va_end(args);
	Kernel::lockCnt--;
	return res;
}
