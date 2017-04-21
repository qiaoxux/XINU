/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

void halt();

#define TPASSED    1
#define TFAILED    0

#define MYVADDR1   0x40000000
#define MYVPNO1    0x40000
#define MYVADDR2   0x80000000
#define MYVPNO2    0x80000
#define MYBS1      1
#define MAX_BSTORE 8

#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1

      
void test1()
{
  kprintf("\nTest 1: Testing xmmap.\n");
  char *addr1 = (char*)0x40000000; 
  int i = ((unsigned long)addr1) >> 12; 
  
  get_bs(MYBS1, 100);
  if (xmmap(i, MYBS1, 100) == SYSERR) {
    kprintf("xmmap call failed\n");
    kprintf("\t\tFAILED!\n");
    return;
  }

  for (i = 0; i < 26; i++) {
    *addr1 = 'A'+i;   //trigger page fault for every iteration
    addr1 += NBPG;    //increment by one page each time
  }

  addr1 = (char*)0x40000000;
  for (i = 0; i < 26; i++) {
    if (*addr1 != 'A'+i) {
      kprintf("\t\tFAILED!\n");
      return;
    }
    addr1 += NBPG;    //increment by one page each time
  }

  xmunmap(0x40000000>>12);

  release_bs(MYBS1);
  
  kprintf("\t\tPASSED!\n");
  return;
}
/*----------------------------------------------------------------*/
void proc_test2(int i,int j,int* ret,int s) {
  char *addr;
  int bsize;
  int r;
  bsize = get_bs(i, j);
  if (bsize != 50)
    *ret = TFAILED;
  r = xmmap(MYVPNO1, i, j);

  if (j<=50 && r == SYSERR){
    *ret = TFAILED;
  }
  if (j> 50 && r != SYSERR){
    *ret = TFAILED;
  }
  sleep(s);
  if (r != SYSERR)
    xmunmap(MYVPNO1);
  release_bs(i);
  return;
}
void test2() {
  int pids[16];
  int mypid;
  int i,j;

  int ret = TPASSED;
  kprintf("\nTest 2: Testing backing store operations\n");

  int bsize = get_bs(1, 100);
  if (bsize != 100)
    ret = TFAILED;
  release_bs(1);
  bsize = get_bs(1, 260);
  if (bsize != SYSERR)
    ret = TFAILED;
  bsize = get_bs(1, 0);
  if (bsize != SYSERR)
    ret = TFAILED;

  mypid = create(proc_test2, 2000, 20, "proc_test2", 4, 1,
                 50, &ret, 4);

  resume(mypid);
  sleep(2);
  
  for(i=1;i<=5;i++){
    pids[i] = create(proc_test2, 2000, 20, "proc_test2", 4, 1,
                     i*20, &ret, 0);
    resume(pids[i]);
  }
  sleep(3);
  kill(mypid);
  for(i=1;i<=5;i++){
    kill(pids[i]);
  }
  if (ret != TPASSED)
    kprintf("\t\tFAILED!\n");
  else
    kprintf("\t\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/
void proc1_test3(int i,int* ret) {
  char *addr;
  int bsize;
  int r;

  get_bs(i, 100);
  
  if (xmmap(MYVPNO1, i, 100) == SYSERR) {
    *ret = TFAILED;
    return 0;
  }
  sleep(4);
  xmunmap(MYVPNO1);
  release_bs(i);
  return;
}
void proc2_test3() {
  /*do nothing*/
  sleep(1);
  return;
}
void test3() {
  int pids[8];
  int mypid;
  int i,j;

  int ret = TPASSED;
  kprintf("\nTest 3: Private heap is exclusive\n");

  for(i=0;i<=7;i++){
    pids[i] = create(proc1_test3, 2000, 20, "proc1_test3", 2, i,&ret);
    if (pids[i] == SYSERR){
      ret = TFAILED;
    }else{
      resume(pids[i]);
    }
  }
  sleep(1);
  mypid = vcreate(proc2_test3, 2000, 100, 20, "proc2_test3", 0, NULL);
  if (mypid != SYSERR)
    ret = TFAILED;

  for(i=0;i<=7;i++){
    kill(pids[i]);
  }
  if (ret != TPASSED)
    kprintf("\t\tFAILED!\n");
  else
    kprintf("\t\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/

void proc1_test4(int* ret) {
  char *addr;
  int i;

  get_bs(MYBS1, 100);

  if (xmmap(MYVPNO1, MYBS1, 100) == SYSERR) {
    kprintf("xmmap call failed\n");
    *ret = TFAILED;
    sleep(3);
    return;
  }

  addr = (char*) MYVADDR1;
  for (i = 0; i < 26; i++) {
    *(addr + i * NBPG) = 'A' + i;
  }
  sleep(6);

  /*Shoud see what proc 2 updated*/
  for (i = 0; i < 26; i++) {
    /*expected output is abcde.....*/
    if (*(addr + i * NBPG) != 'a'+i){
      kprintf("%c\n", *(addr + i * NBPG));
      *ret = TFAILED;
      break;    
    }
  }
  kprintf("2ret %d\n", *ret);

  xmunmap(MYVPNO1);
  release_bs(MYBS1);
  return;
}
void proc2_test4(int *ret) {
  char *addr;
  int i;

  get_bs(MYBS1, 100);

  if (xmmap(MYVPNO2, MYBS1, 100) == SYSERR) {
    kprintf("xmmap call failed\n");
    *ret = TFAILED;
    sleep(3);
    return;
  }

  addr = (char*) MYVADDR2;

  /*Shoud see what proc 1 updated*/
  for (i = 0; i < 26; i++) {
    /*expected output is ABCDEF.....*/
    if (*(addr + i * NBPG) != 'A'+i){
      *ret = TFAILED;
      break;
    }
  }
  kprintf("0ret %d\n", *ret);
  /*Update the content, proc1 should see it*/
  for (i = 0; i < 26; i++) {
    *(addr + i * NBPG) = 'a' + i;
  }
  kprintf("1ret %d\n", *ret);

  xmunmap(MYVPNO2);
  release_bs(MYBS1);
  return;
}
void test4() {
  int pid1;
  int pid2;
  int ret = TPASSED;
  kprintf("\nTest 4: Shared backing store\n");

  pid1 = create(proc1_test4, 2000, 20, "proc1_test4", 1, &ret);
  pid2 = create(proc2_test4, 2000, 20, "proc2_test4", 1, &ret);

  resume(pid1);
  sleep(3);
  resume(pid2);

  sleep(10);
  
  kill(pid1);
  kill(pid2);
  
  if (ret != TPASSED)
    kprintf("\t\tFAILED!\n");
  else
    kprintf("\t\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/
void proc1_test5(int* ret) {
  int *x;
  int *y;
  int *z;

  kprintf("ready to allocate heap space\n");
  x = vgetmem(1024);
  if ((x ==SYSERR) || (x < 0x1000000)
      || (x > 0x1000000 + 256 * NBPG - 1024)) {
    *ret = TFAILED;
  }
  kprintf("ret 1 %d\n", ret);
  if (x == SYSERR)
    return;

  *x = 100;
  *(x + 1) = 200;

  if ((*x != 100) || (*(x+1) != 200)) {
    *ret = TFAILED;
  }
  kprintf("ret 2 %d\n", ret);
  vfreemem(x, 1024);

  x = vgetmem(257*NBPG); //try to acquire a space that is bigger than size of one backing store
  if (x != SYSERR) {
    *ret = TFAILED;
  }
  kprintf("ret 3 %d\n", ret);

  x = vgetmem(50*NBPG);
  y = vgetmem(50*NBPG);
  z = vgetmem(50*NBPG);
  if ((x == SYSERR) || (y == SYSERR) || (z != SYSERR)){
    *ret = TFAILED;
    // kprintf("x y z %d %d %d\n", x, y, z);
    if (x != SYSERR) vfreemem(x, 50*NBPG);
    if (y != SYSERR) vfreemem(y, 50*NBPG);
    if (z != SYSERR) vfreemem(z, 50*NBPG);
    return;
  }
  kprintf("ret 4 %d\n", ret);
  vfreemem(y, 50*NBPG);
  z = vgetmem(50*NBPG);
  if (z == NULL){
    *ret = TFAILED;
  }
  kprintf("ret 5 %d\n", ret);
  if (x != NULL) vfreemem(x, 50*NBPG);
  if (z != NULL) vfreemem(z, 50*NBPG);
  return;


}
void test5() {
  int pid1;
  int ret = TPASSED;

  kprintf("\nTest 5: vgetmem/vfreemem\n");
  pid1 = vcreate(proc1_test5, 2000, 100, 20, "proc1_test5", 1, &ret);

  kprintf("pid %d has private heap\n", pid1);
  resume(pid1);
  sleep(3);
  kill(pid1);
  if (ret != TPASSED)
    kprintf("\t\tFAILED!\n");
  else
    kprintf("\t\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/
void proc1_test6(int *ret) {

  char *vaddr;
  int i, j;
  int tempaddr;
  int vaddr_beg = 0x40000000;
  int vpno;
  
  for(i = 0; i < 8; i++){
    tempaddr = vaddr_beg + 100 * NBPG * i; 
    vaddr = (char *) tempaddr;
    vpno = tempaddr >> 12;
    get_bs(i, 100);
    if (xmmap(vpno, i, 100) == SYSERR) {
      *ret = TFAILED;
      kprintf("xmmap call failed\n");
      sleep(3);
      return;
    }

    for (j = 0; j < 100; j++) {
      *(vaddr + j * NBPG) = 'A' + i;
    }
    
    for (j = 0; j < 100; j++) {
      if (*(vaddr + j * NBPG) != 'A'+i){
        *ret = TFAILED;
        break;
      }
    }    
    xmunmap(vpno);
    release_bs(i);    
  }

  return;
}

void test6(){
  int pid1;
  int ret = TPASSED;
  kprintf("\nTest 6: Stress testing\n");

  pid1 = create(proc1_test6, 2000, 50, "proc1_test6",1,&ret);

  resume(pid1);
  sleep(4);
  kill(pid1);
  if (ret != TPASSED)
    kprintf("\t\tFAILED!\n");
  else
    kprintf("\t\tPASSED!\n");
}
 
void proc1_test7(int *ret) {
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);

	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}

	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 26; i++) {
		*(addr + i * NBPG) = 'A' + i;
	}

	sleep(6);

	for (i = 0; i < 26; i++) {
		if(*(addr + i * NBPG) != 'A' + i) {
			*ret = TFAILED;
		}
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	xmunmap(PROC1_VPNO);
	return;
}

void test7() {
	int ret = TPASSED;
	kprintf("\nTest 7: shared memory\n");
	int pid1 = create(proc1_test7, 2000, 20, "proc1_test7", 1, &ret);
	resume(pid1);
	sleep(10);
	kill(pid1);
	if (ret != TPASSED)
    	kprintf("\t\tFAILED!\n");
  	else
    	kprintf("\t\tPASSED!\n");
}

void proc1_test8(int *ret) {
	int *x;

	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	if (x != 16777216) {
		*ret = TFAILED;
	}
	kprintf("heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;

	kprintf("heap variable: %d %d\n", *x, *(x + 1));
	if (*x != 100) {
		*ret = TFAILED;
	}
	if (*(x + 1) != 200) {
		*ret = TFAILED;
	}
	vfreemem(x, 1024);
}

void test8() {
	int ret = TPASSED;
	kprintf("\nTest 8: vgetmem/vfreemem\n");
	int pid1 = vcreate(proc1_test8, 2000, 100, 20, "proc1_test8", 1, &ret);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);
	kill(pid1);
	if (ret != TPASSED)
    	kprintf("\t\tFAILED!\n");
  	else
    	kprintf("\t\tPASSED!\n");
}

void proc1_test9(int *ret) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 1024; i++) {
		*(addr + i * NBPG) = 'B';
	}

	for (i = 0; i < 9; i++) {
		if(*(addr + i * NBPG) != 'B') {
			*ret = TFAILED;
		}
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}
	return;
}

void test9() {
	int ret = TPASSED;
	kprintf("\nTest 9: Frame test\n");
	int pid1 = create(proc1_test9, 2000, 20, "proc1_test9", 1, &ret);
	resume(pid1);
	sleep(3);
	// kill(pid1);
	if (ret != TPASSED)
    	kprintf("\t\tFAILED!\n");
  	else
    	kprintf("\t\tPASSED!\n");
}

void testSC_func()
{
	int PAGE0 = 0x40000;
	int i,j,temp,ret;
	unsigned long addrs[1500];
	int cnt = 0; 
	//can go up to  (NFRAMES - 5 frames for null prc - 1pd for main - 1pd + 1pt frames for this proc)
	//frame for pages will be from 1032-2047
	int maxpage = (NFRAMES - (5 + 1 + 1 + 1));

	kprintf("are you finished0?\n");
	for (i=0;i<=maxpage/128;i++){
		if(get_bs(i,128) == SYSERR)
		{
			kprintf("get_bs call failed \n");
			return;
		}
		if ((ret = xmmap(PAGE0+i*128, i, 128)) == SYSERR) {
			kprintf("xmmap call failed, virtpage=0x%08x, source=%d, npages=%d, ret=%d\n", PAGE0+i*128, i, 128, ret);
			return;
		}
		for(j=0;j < 128;j++)
		{  
			//store the virtual addresses
			addrs[cnt++] = (unsigned long)(PAGE0+(i*128) + j) << 12;
		}			
	}
	kprintf("are you finished1?\n");
	// kprintf("cnt=%d\n", cnt);
	// for(i=0; i < 8*128; i++) {  
	// 	kprintf("vaddr[%d]=0x%08x\n", i, addrs[i]);
	// }

	/* all of these should generate page fault, no page replacement yet
	   acquire all free frames, starting from 1032 to 2047, lower frames are acquired first
	   */
	
	char *zero_addr = (char*) 0x0;
	for(i=0; i < maxpage; i++) {  
	    *((unsigned long *)addrs[i]) = i + 1;
	    if (i + 1 != *((unsigned long *)(zero_addr + (i + 1032) * NBPG))) {
			kprintf("\t\tFAILED!\n");
			return;
	    }
	}
	kprintf("are you finished2?\n");

	//trigger page replacement, this should clear all access bits of all pages
	//expected output: frame 1032 will be swapped out
	*((unsigned long *)addrs[maxpage]) = maxpage + 1;
	kprintf("are you finished3?\n");
	temp = *((unsigned long *)addrs[maxpage]);
	kprintf("temp=%d\n", temp);
	if (temp != *((unsigned long *)(zero_addr + 1032 * NBPG))) {
		kprintf("\t\tFAILED!\n");
		return;
	}

	kprintf("Reset access bits of all pages except 508\n");
	// for(i=1; i <=maxpage; i++) {
	// 	if (i != 500)  //reset access bits of all pages except this
	// 		*((unsigned long *)addrs[i])= i + 1;
	// }
	for(i=1; i <= maxpage; i++) {
		if (i == 500) continue;
		*((unsigned long *)addrs[i])= i + 1;
	}


	//Expected page to be swapped: 1032+500 = 1532
	*((unsigned long *)addrs[maxpage+1]) = maxpage+2;

	temp = *((unsigned long *)addrs[maxpage+1]);
	kprintf("temp=%d\n", temp);
	if (temp != *((unsigned long *)(zero_addr + 1518 * NBPG))) {
		kprintf("\t\tFAILED!\n");
		return;
	}

	for (i=0;i<=maxpage/256;i++){
		xmunmap(PAGE0+(i*256));
		release_bs(i);			
	}


}
void testSC(){
	int pid1;

	srpolicy(SC);

	kprintf("\nTest SC page replacement policy\n");

	pid1 = create(testSC_func, 2000, 40, "testSC_func", 0, NULL);

	resume(pid1);
	sleep(5);
	kill(pid1);
}

int main() {
	// test1();
	// test2();
	// test3();
	// test4();
	// test5();
	// test6();
	test7();
	// test8();
	// test9();
	// testSC();


	shutdown();
	return 0;
}
