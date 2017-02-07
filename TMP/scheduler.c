/*-----------------------------------------------------------------------
  * scheduler.c -- set and get scheduling method
  *------------------------------------------------------------------------
  */

#include <sched.h>

void setschedclass(int s_class) {
	SCHED_CLASS = s_class;
}

int getschedclass() {
	return SCHED_CLASS;
}