#include <types.h>
#include <lib.h>
#include <test.h>

void Hello(){
	extern const int buildversion;
	extern const char buildconfig[];

	kprintf("\n\nWelcome to %s: v%d.\n\n", buildconfig, buildversion);
}
