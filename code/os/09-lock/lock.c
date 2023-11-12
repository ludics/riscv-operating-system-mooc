#include "os.h"

void initlock(struct spinlock *lk, char *name)
{
	lk->locked = 0;
	lk->name = name;
}

void acquire(struct spinlock *lk)
{
	while(__sync_lock_test_and_set(&lk->locked, 1))
		;
}

void release(struct spinlock *lk)
{
	__sync_lock_release(&lk->locked);
}

int spin_lock()
{
	w_mstatus(r_mstatus() & ~MSTATUS_MIE);
	return 0;
}

int spin_unlock()
{
	w_mstatus(r_mstatus() | MSTATUS_MIE);
	return 0;
}
