#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
int num = 0;
void *t(void* arg){
    threadLock(lock[0]);
    threadYield();
    num++;
    threadUnlock(lock[0]);
}
int main(){


    int id1, id2;
  int p1;
  int p2;

  p1 = 0;
  p2 = 0;

  int *result1, *result2;

  // initialize the threading library. DON'T call this more than once!!!
  threadInit();

  
  id1 = threadCreate(t, NULL);
  id2 = threadCreate(t, NULL);
  threadJoin(id1, (void *)&result1);
  printf("thread1: %d\n", *result1);
  threadJoin(id2, (void *)&result1);
  printf("thread2: %d\n", *result1);
}