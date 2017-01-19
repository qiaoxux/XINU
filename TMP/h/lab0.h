// Task 1
extern long zfunction(long);

// Task 2
extern void printsegaddress(void);

// Task 3
extern void printtos(void);

// Task 4
extern void printprocstks(int);

// Task 5
extern int sys_trace;

extern unsigned long ctr1000;
extern int sys_call_time[50][27];
extern int sys_call_nums[50][27];
extern char sys_call_names[27][16];	

extern void syscallsummary_start();
extern void syscallsummary_stop();
extern void printsyscallsummary();