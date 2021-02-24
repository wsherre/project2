#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
int num = 0;
void *t(void* arg){
    threadLock(0);
    threadYield();
    num++;
    threadUnlock(0);
    return NULL;
}
void *t1(void* arg){
    threadLock(0);
    num--;
    threadUnlock(0);
    return NULL;
}
void *t2(void* arg){
  printf("sup : %d\n", *((int *)arg));
  return NULL;
}
int main(){


    int id1, id2;
  int p1;
  int p2;

  int array[5001];

  p1 = 0;
  p2 = 0;

  int *result1, *result2;

  // initialize the threading library. DON'T call this more than once!!!
  threadInit();

  for (int i = 0; i < 1024; ++i){
      array[i] = threadCreate(t2, (void*)&i);
  }

  for(int i = 1; i < 1025; ++i){
    looping:
    threadJoin(array[i], (void *)&result1);
    printf("%d\n", i);
  }
  /*id1 = threadCreate(t, NULL);
  id2 = threadCreate(t1, NULL);
  threadJoin(id1, (void *)&result1);
  printf("thread1: %d\n", num);
  threadJoin(id2, (void *)&result1);
  threadYield();
  printf("thread2: %d\n", num);*/
}