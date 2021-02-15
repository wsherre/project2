#include "mythreads.h"
#include <pthread.h>
#include <ucontext.h>
#define array_size 5000



extern void threadInit(){

}

extern int threadCreate(thFuncPtr funcPtr, void *argPtr){
    pthread_t thread;
    pthread_attr_t *attr;

    pthread_attr_init(attr);
    pthread_attr_setstacksize (attr, STACK_SIZE);

    if(pthread_create(&thread, attr, funcPtr, argPtr) == 0){
        return 0;
    }else{
        return -1;
    }
    
}