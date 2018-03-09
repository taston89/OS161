#include <test.h>
#include <types.h>
#include <lib.h>

int messagetest(int nargs,char **args){
	(void)nargs;
	(void)args;
	
	kprintf("Arguments were: \n");
	for(int i = 1; i < nargs; i++){
		kprintf(args[i]);
		kprintf("\n");
	}
	return 0;
}
