#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include <mips/trapframe.h>

/* this implementation of sys__exit does not do anything with the exit code */
/* this needs to be fixed to get exit() and waitpid() working properly */
void uproc_thread(void *temp_tr, unsigned long k);
void sys__exit(int exitcode) {

	struct addrspace *as;
	struct proc *p = curproc;
	/* for now, just include this to keep the compiler from complaining about
		an unused variable */
	(void)exitcode;

	DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

	KASSERT(curproc->p_addrspace != NULL);
	as_deactivate();
	/*
	 * clear p_addrspace before calling as_destroy. Otherwise if
	 * as_destroy sleeps (which is quite possible) when we
	 * come back we'll be calling as_activate on a
	 * half-destroyed address space. This tends to be
	 * messily fatal.
	 */
	as = curproc_setas(NULL);
	as_destroy(as);

	/* detach this thread from its process */
	/* note: curproc cannot be used after this call */
	proc_remthread(curthread);

	/* if this is the last user process in the system, proc_destroy()
		will wake up the kernel menu thread */
	proc_destroy(p);

	thread_exit();
	/* thread_exit() does not return, so we should never get here */
	panic("return from thread_exit in sys_exit\n");
}

/* stub handler for uproc_thread 							  */
void
uproc_thread(void *temp_tr, unsigned long k){
	as_activate();	
	struct trapframe c_tf = *((struct trapframe*)temp_tr);

	c_tf.tf_epc += 4;
	c_tf.tf_v0 = k;
	c_tf.tf_a3 = 0;
	kfree(temp_tr);

	mips_usermode(&c_tf);	
	KASSERT(curproc->p_addrspace != NULL);

	thread_exit();
}

/* stub handler for fork() system call						  */
int
sys_fork(struct trapframe *tf, pid_t *retval){
	struct trapframe * temp_tf;
	struct proc *child_thread;
	struct addrspace *c_memspace = NULL;
	int result;

	KASSERT(c_memspace==NULL);
	as_copy(curproc->p_addrspace, &c_memspace);
	if(c_memspace == NULL){ kprintf("Sys_fork failed @ as_copy %s\n", strerror(ENOMEM)); return ENOMEM; }

	child_thread = proc_create_fork("forkedProc");
	DEBUG(DB_SYSCALL,"Syscall: sys_fork()\n");
	child_thread->p_addrspace = c_memspace;
	child_thread->pid = 2;

	temp_tf = kmalloc(sizeof(struct trapframe));
	if(temp_tf == NULL){ return ENOMEM; }
	*temp_tf = *tf;

	result = thread_fork(curthread->t_name, child_thread, uproc_thread, temp_tf, *retval);
	if(result){ return result; }
	child_thread->pid += 1;
	*retval = child_thread->pid;
	return(0);
}

/* stub handler for getpid() system call                */
	int
sys_getpid(pid_t *retval)
{
	*retval = curproc->pid;
	return(0);
}

/* stub handler for waitpid() system call                */
	int
sys_waitpid(pid_t pid,
		userptr_t status,
		int options,
		pid_t *retval)
{
	int exitstatus;
	int result;

	/* this is just a stub implementation that always reports an
		exit status of 0, regardless of the actual exit status of
		the specified process.   
		In fact, this will return 0 even if the specified process
		is still running, and even if it never existed in the first place.

		Fix this!
		*/

	if (options != 0) {
		return(EINVAL);
	}
	/* for now, just pretend the exitstatus is 0 */
	exitstatus = 0;
	result = copyout((void *)&exitstatus,status,sizeof(int));
	if (result) {
		return(result);
	}
	*retval = pid;
	return(0);
}

