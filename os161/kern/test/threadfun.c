//Sam George
//Threadfun threadtest (FTT in Test Menu).
//Execution: FTT [thread count] [increment]
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <spinlock.h>
static volatile int corruptCounter;
static struct semaphore *tsem = NULL;
static struct lock *lock = NULL;
static struct spinlock sl;

//Initialize the semaphore "tsem".
static
void
init_sem(void)
{
	if (tsem==NULL) {
		tsem = sem_create("tsem", 0);
		if (tsem == NULL) {
			panic("threadtest: sem_create failed\n");
		}
	}

	if(lock == NULL){
		lock = lock_create("lock");
		if(lock == NULL){ kprintf("Lock creation failed!"); }
	}
	spinlock_init(&sl);
}

//Takes a function pointer and unsigned long as parameters.
//Junk is suppressed for now, num is is printed by the thread, then tsem is incremented.
static
void
funthread(void *junk, unsigned long num){
	(void)junk;
	kprintf("%d", (int)num);
	V(tsem);
}

//increment shared counter in an unsafe manner (no mutex, corruption).
static
void
unsafethreads(void *junk, unsigned long num){
	(void)junk;
	for(int i = 0; i < (int)num; i++){
		corruptCounter++;
	}
	V(tsem);
}

//increment shared counter with locks implemented (no corruption).
static
void
lockthreads(void *junk, unsigned long num){
	(void)junk;
	for(int i = 0; i < (int)num; i++){
		lock_acquire(lock);
		corruptCounter++;
		lock_release(lock);
	}

	V(tsem);
}

//increment shared counter with spinlocks implemented (no corruption).
static
void
spinlockthreads(void *junk, unsigned long num){
	(void)junk;
	for(int i = 0; i <(int)num; i++){
		spinlock_acquire(&sl);
		corruptCounter++;
		spinlock_release(&sl);
	}

	V(tsem);
}

static
void
runthreads(int threadCount, int increment, int testType){
	int result;
	char name[16];
	void(*tType)();
	
	corruptCounter = 0;

	switch(testType){
		case 0:
			tType = funthread;
			break;
		case 1:
			tType = unsafethreads;
			break;
		case 2:
			tType = lockthreads;
			break;
		case 3:
			tType = spinlockthreads;
		default:
			break;
	}

	init_sem();
	for(int i = 0; i < threadCount; i++){
		result = thread_fork(name, NULL, tType, NULL, (testType != 0) ? increment : i);
		if(result){ panic("Thread_fork failed %s\n", strerror(result)); }
		if(testType == 0){
			for(int o = 0; o < 2000; o++){}
		}
	}

	for(int i = 0; i < threadCount; i++){
		P(tsem);
	}
}
//Test for lab 5. takes argument count (nargs) and an array of arguments as parameters.
//args[1] contains the number of threads to print from (args[0] contains the command).
//sets variable "count" to integer cast of argument[1].
//calls runthreads with count as a parameter.
int funtest(int nargs, char **args){
	(void)nargs;
	(void)args;
	int count = atoi(args[1]);
	if(atoi(args[1]) && atoi(args[2])){
		kprintf("\nStarting fun thread test...\n");
		runthreads(count, atoi(args[2]), 0);
		kprintf("\nFun thread test complete\n\n");
	}
	else{
		kprintf("\nUsage:\'ftt [integer: thread count] [integer: increment]\'\n\n");
	}
	return 0;
}

int
unsafethreadcounter(int nargs, char **args){
	(void)nargs;
	int count = atoi(args[1]);
	int increment = atoi(args[2]);

	if(atoi(args[1]) && atoi(args[2])){
		kprintf("Expected result: %d\n ", (count * increment));
	}

	runthreads(count, increment, 1);
	kprintf("Actual result: %d\n", corruptCounter);
	return 0;
}

int
lockthreadcounter(int nargs, char **args){
	(void)nargs;
	int count = atoi(args[1]);
	int increment = atoi(args[2]);

	if(atoi(args[1]) && atoi(args[2])){
		kprintf("Expected result: %d\n", (count * increment));
	}

	runthreads(count, increment, 2);
	kprintf("Actual result: %d\n", corruptCounter);
	return 0;
}

int
spinlockthreadcounter(int nargs, char **args){
	(void)nargs;
	int count = atoi(args[1]);
	int increment = atoi(args[2]);

	if(atoi(args[1]) && atoi(args[2])){
		kprintf("Expected result: %d\n", (count * increment));
	}

	runthreads(count, increment, 3);
	kprintf("Actual result: %d\n", corruptCounter);
	return 0;
}
