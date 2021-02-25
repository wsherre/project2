#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
int num;
void *t(void* arg){
    threadYield();
    threadLock(0);
    num += 2;
    threadYield();
    threadUnlock(0);
    return &num;
}
void *t1(void* arg){
    threadLock(0);
    num -= 1;
    threadUnlock(0);
    return &num;
}
void *t2(void* arg){
  printf("sup : %d\n", *((int *)arg));
  int *result = malloc(sizeof(int));
  *result = *((int *)arg);
  return result;
}
int main(){


    int id1, id2;
  int p1;
  int p2;

  int array[201];

  p1 = 0;
  p2 = 0;

  int *result1, *result2;

  // initialize the threading library. DON'T call this more than once!!!
  threadInit();

  for (int i = 0; i < 100; ++i){
      array[i] = threadCreate(t, (void*)&i);
  }for (int i = 100; i < 200; ++i){
      array[i] = threadCreate(t1, (void*)&i);
  }

  for(int i = 1; i < 201; ++i){
    threadJoin(i, (void *)&result1);
  }
  printf("num %d\n", num);
  /*id1 = threadCreate(t, NULL);
  id2 = threadCreate(t1, NULL);
  threadJoin(id1, (void *)&result1);
  printf("thread1: %d\n", num);
  threadJoin(id2, (void *)&result1);
  threadYield();
  printf("thread2: %d\n", num);*/
}