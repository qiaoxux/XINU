#define EXPDISTSCHED 1
#define LINUXSCHED 2

extern int SCHED_CLASS;

extern void setschedclass(int sched_class);
extern int getschedclass();