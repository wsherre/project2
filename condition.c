#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
int num = 0;
void *t(void* arg){
    threadLock(0);
    threadWait(0, 5);
    *((int *)arg) += 2;
    threadUnlock(0);
    return arg;
}
void *t2(void* arg){
    threadLock(5);
    threadWait(5, 8);
    *((int *)arg) += 2;
    threadUnlock(5);
    return arg;
}
void *t1(void*arg){
    //threadYield();
    threadSignal(0, 5);
    threadSignal(0,5);
    //threadYield();
    threadSignal(5, 8);
    threadSignal(5, 8);threadSignal(5, 9);
    return NULL;
}

int main(){


  int id1, id2, id3, id4, id5;
  int p1, p2, p3, p4;

  int array[3000];

  p1 = 0;
  p2 = 5; 
  p3 = 10;
  p4 = 15;


  int *result1, *result2;

  // initialize the threading library. DON'T call this more than once!!!
  threadInit();

  id1 = threadCreate(t, (void*)&p1);
  id2 = threadCreate(t, (void*)&p2);
  id3 = threadCreate(t2, (void*)&p3);
  id4 = threadCreate(t2, (void*)&p4);
  id5 = threadCreate(t1, (void*)&p1);

  threadJoin(id1, (void *)&result1);
  printf("joined #1 --> %d.\n", *result1);

  threadJoin(id2, (void *)&result1);
  printf("joined #2 --> %d.\n", *result1);

  threadJoin(id4, (void *)&result1);
  printf("joined #4 --> %d.\n", *result1);

  threadJoin(id3, (void *)&result1);
  printf("joined #3 --> %d.\n", *result1);

  threadJoin(id5, (void *)&result1);
  printf("joined #4 --> %d.\n", *result1);
  
}