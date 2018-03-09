#include <unistd.h>
#include <stdio.h>

int
main()
{
	pid_t pid;
	int rand = 1;

	printf("Test call for fork stub\n");
	pid = fork();
	printf("pid: %d\n", pid);

	printf("Test call for getpid stub\n");
	pid = getpid();
	printf("pid: %d\n", pid);

	printf("Test call for waitpid stub\n");
	pid = waitpid(pid, &rand, 0);
	printf("pid: %d\n", pid);

	_exit(0);
}
