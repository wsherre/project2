#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>

void *t(void* arg){
    int param = *((int*)arg);
    printf("inside t1\n");
    printf("%d\n", param);
    int *result = malloc(sizeof(int));
    *result = param++;
    return result;
}
int main(){


    int p = 1;

    threadCreate(t, (void*)&p);
}