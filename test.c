#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>

void *t(void* arg){
    int param = *((int*)arg);
    printf("inside t1: %d\n", param);
    int *result = malloc(sizeof(int));
    *result = param++;
    //threadYield();
    return result;
}
int main(){


    int p = 1;
    int s = 2;
    threadInit();
    printf("t1\n\n");
    threadCreate(t, (void*)&p);
    printf("t2\n\n");
    threadCreate(t, (void*)&s);
    printf("end of main\n\n");
}