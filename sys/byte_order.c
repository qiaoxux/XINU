#include <stdio.h>

int little_endian(){
	int i = 1;
    return *(char*)&i;
}
int big_endian(){
    return !little_endian();
}

int main(){
    if(big_endian())
        printf("Big Endian\n");
    if(little_endian())
        printf("Little Endian\n");

    return 0;
}