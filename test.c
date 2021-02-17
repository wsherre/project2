#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>

void *t(void* arg){
    int param = *((int*)arg);
    printf("inside t1: %d\n", param);
    int *result = malloc(sizeof(int));
    *result = param++;
    threadYield();
    return result;
}
int main(){


    int p = 1;
    int s = 2;
    threadCreate(t, (void*)&p);
    threadCreate(t, (void*)&s);
    printf("if here then it works and it should break\n\n");
    print_lib();

}