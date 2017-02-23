/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

void donothing(char *msg) {
	kprintf ("  %s: to do nothing\n", msg);
	unsigned long i = 0;
	while (i++ < 10000);
	kprintf ("  %s: but sleep for a while\n", msg);
}

void lwriter (char *msg, int lck) {
    lock (lck, WRITE, DEFAULT_LOCK_PRIO);
    kprintf ("  %s: acquired lock, sleep for a while\n", msg);
    unsigned long i = 0;
	while (i++ < 1000000000);
    kprintf ("  %s: to release lock\n", msg);
    releaseall (1, lck);
}

void swriter (char *msg, int sem) {
    wait (sem);
    kprintf ("  %s: acquired sem, sleep for a while\n", msg);
    unsigned long i = 0;
	while (i++ < 1000000000);
	kprintf ("  %s: to release sem\n", msg);
    signal(sem);
}

void testlock() {
	int     lck;
    int     low, medium, high;

    kprintf("\ntestlock start\n");
    lck  = lcreate ();
    if (lck == SYSERR) {
    	kprintf("\nlcreate failed\n"); 
    	return;
    }

    low    = create(lwriter, 2000, 10, "writer 1", 2, "writer 1", lck);
    medium = create(donothing, 2000, 20, "nothing", 1, "nothing");
    high   = create(lwriter, 2000, 30, "writer 2", 2, "writer 2", lck);

    resume(low);
    sleep(1);
    resume(high);
    sleep(1);
    resume(medium);
    sleep(1);
}

void testsemaphore() {
	int     sem;
    int     low, medium, high;

    kprintf("\ntestsemaphore start\n");
    sem  = screate(1);
    if (sem == SYSERR) {
    	kprintf("\nscreate failed\n"); 
    	return;
    }

    low    = create(swriter, 2000, 10, "writer 1", 2, "writer 1", sem);
    medium = create(donothing, 2000, 20, "nothing", 1, "nothing");
    high   = create(swriter, 2000, 30, "writer 2", 2, "writer 2", sem);

    resume(low);
    sleep(1);
    resume(high);
    sleep(1);
    resume(medium);
    sleep(1);
}

int main ()
{
    testlock();
    sleep(8);
    testsemaphore();

    shutdown();
}