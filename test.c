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


    int id1, id2;
  int p1;
  int p2;

  p1 = 23;
  p2 = 2;

  int *result1, *result2;

  // initialize the threading library. DON'T call this more than once!!!
  threadInit();

  id1 = threadCreate(t, (void *)&p1);
  printf("created thread 1.     %d\n", id1);

  id2 = threadCreate(t, (void *)&p2);
  printf("created thread 2.     %d\n", id2);

  threadJoin(id1, (void *)&result1);
  printf("joined #1 --> %d.\n", *result1);

  threadJoin(id2, (void *)&result2);
  printf("joined #2 --> %d.\n", *result2);
}