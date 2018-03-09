#include <types.h>
#include <lib.h>
#include <spinlock.h>
#include <wchan.h>
#include <thread.h>
#include <current.h>
#include <synch.h>

	struct semaphore *
sem_create(const char *name, int initial_count)
{
	struct semaphore *sem;

	KASSERT(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->sem_name = kstrdup(name);
	if (sem->sem_name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->sem_wchan = wchan_create(sem->sem_name);
	if (sem->sem_wchan == NULL) {
		kfree(sem->sem_name);
		kfree(sem);
		return NULL;
	}

	spinlock_init(&sem->sem_lock);
	sem->sem_count = initial_count;

	return sem;
}

	void
sem_destroy(struct semaphore *sem)
{
	KASSERT(sem != NULL);


	spinlock_cleanup(&sem->sem_lock);
	wchan_destroy(sem->sem_wchan);
	kfree(sem->sem_name);
	kfree(sem);
}

	void 
P(struct semaphore *sem)
{
	KASSERT(sem != NULL);


	KASSERT(curthread->t_in_interrupt == false);

	spinlock_acquire(&sem->sem_lock);
	while (sem->sem_count == 0) {

		wchan_lock(sem->sem_wchan);
		spinlock_release(&sem->sem_lock);
		wchan_sleep(sem->sem_wchan);

		spinlock_acquire(&sem->sem_lock);
	}
	KASSERT(sem->sem_count > 0);
	sem->sem_count--;
	spinlock_release(&sem->sem_lock);
}

	void
V(struct semaphore *sem)
{
	KASSERT(sem != NULL);

	spinlock_acquire(&sem->sem_lock);

	sem->sem_count++;
	KASSERT(sem->sem_count > 0);
	wchan_wakeone(sem->sem_wchan);

	spinlock_release(&sem->sem_lock);
}



	struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->lk_name = kstrdup(name);
	if (lock->lk_name == NULL) {
		kfree(lock);
		return NULL;
	}


	lock->lk_wchan = wchan_create(lock->lk_name);
	if(lock->lk_wchan == NULL) {
		kfree(lock->lk_name);
		kfree(lock);
		return NULL;
	}
	spinlock_init(&lock->lk_lock);
	lock->held = false;
	lock->holder = NULL;


	return lock;
}

	void
lock_destroy(struct lock *lock)
{
	KASSERT(lock != NULL);



	spinlock_cleanup(&lock->lk_lock);
	wchan_destroy(lock->lk_wchan);
	lock->holder = NULL;
}

	void
lock_acquire(struct lock *lock)
{

	KASSERT(lock != NULL);
	KASSERT(curthread->t_in_interrupt == false);
	spinlock_acquire(&lock->lk_lock);

	while(lock->held){
		wchan_lock(lock->lk_wchan);
		spinlock_release(&lock->lk_lock);
		wchan_sleep(lock->lk_wchan);
		spinlock_acquire(&lock->lk_lock);
	}
	KASSERT(!lock->held);
	lock->holder = curthread;
	lock->held = true;
	spinlock_release(&lock->lk_lock);


}

	void
lock_release(struct lock *lock)
{


	KASSERT(lock != NULL);
	spinlock_acquire(&lock->lk_lock);
	if(lock_do_i_hold(lock)){
		lock->held = false;
		wchan_wakeone(lock->lk_wchan);
	}
	spinlock_release(&lock->lk_lock);

}

	bool
lock_do_i_hold(struct lock *lock)
{

	if(lock->holder == curthread) { return true; }
	else { return false; }
}



	struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->cv_name = kstrdup(name);
	if (cv->cv_name==NULL) {
		kfree(cv);
		return NULL;
	}



	return cv;
}

	void
cv_destroy(struct cv *cv)
{
	KASSERT(cv != NULL);


	kfree(cv->cv_name);
	kfree(cv);
}

	void
cv_wait(struct cv *cv, struct lock *lock)
{

	(void)cv; 
	(void)lock;
}

	void
cv_signal(struct cv *cv, struct lock *lock)
{

	(void)cv;   
	(void)lock; 

	}

void cv_broadcast(struct cv *cv, struct lock *lock)
		{

			(void)cv;
			(void)lock;
		}

