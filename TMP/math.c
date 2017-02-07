/*-----------------------------------------------------------------------
  * math.c -- some fundemantal math functions
  *------------------------------------------------------------------------
  */

#include <kernel.h>
#include <math.h>

double expdev(double lambda) {
    double dummy;
    do {
        dummy = (double) rand() / RAND_MAX;
    } while (dummy == 0.0);
    return -log(dummy) / lambda;
}

double log(double x) {
    if (x <= 0.0f || x >= 2.0f) {
        kprintf("Invalid Parameter\n");
        return(SYSERR);
    }
    
    int n;
    double sum = 0.0f;
    for (n = 1; n <= 20; n++) {
        sum += (1.0f/(double) n) * pow(x - 1.0f, n) * ((n % 2 == 0) ? -1 : 1);
    }

    return sum;
}

double pow(double x, int y) {
    if (y == 0) return 0;
    
    int i;
    double power = 1.0f;
    for (i = 0; i < y; i++) {
        power = power * x;
    }
    return power;
}