#include <unistd.h>
#include <stdio.h>

int main(){
	pid_t pid;
   pid = getpid();
	printf("Calling getpid test stub\n");
	printf("PID: %d\n", pid);

	return 0;
}
