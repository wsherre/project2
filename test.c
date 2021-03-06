#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
int num = 0;
void *t(void* arg){
    threadLock(1);
    *((int *)arg) += 2;
    threadYield();
    threadUnlock(1);
    return arg;
}
void *t1(void* arg){
    threadLock(1);
    *((int *)arg) -= 1;
    threadYield();
    threadUnlock(1);
    return arg;
}
void *t2(void* arg){
  printf("sup : %d\n", *((int *)arg));
  //int *result = malloc(sizeof(int));
  //*result = *((int *)arg);
  //threadExit(&num);
  return NULL;
}
int main(){


  int id1, id2;
  int p1;
  int p2;

  int array[400];

  p1 = 0;
  p2 = 0; 

  int *result1, *result2;

  // initialize the threading library. DON'T call this more than once!!!
  threadInit();

  for (int i = 0; i < 100; ++i){
      array[i] = threadCreate(t, (void*)&num);
  }
  for (int i = 100; i < 200; ++i){
      array[i] = threadCreate(t1, (void*)&num);
  }
  //threadExit(result1);

  for(int i = 1; i < 201; ++i){
    threadJoin(i, (void *)&result1);
    printf("%d %d\n", *result1, i);

  }
  //num should be 0
  printf("num %d\n", num);
  //test already exited thread
  threadExit((void*)&result2);
  
}