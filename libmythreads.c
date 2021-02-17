#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
#define array_size 5000

typedef struct library{
    ucontext_t thread_context;
    bool active;
}library;

library thread_lib[array_size];

extern void threadInit(){
    for(int i = 0; i < array_size; ++i){
        thread_lib[i].active = false;
    }
    getcontext(&(thread_lib[0].thread_context));
    thread_lib[0].active = true;
}

extern int threadCreate(thFuncPtr funcPtr, void *argPtr){
    
    ucontext_t newcontext;

    getcontext(&newcontext);
    newcontext.uc_stack.ss_sp = malloc ( STACK_SIZE ) ;
    newcontext.uc_stack.ss_size = STACK_SIZE ;
    newcontext.uc_stack.ss_flags = 0;

    thread_lib[1].thread_context = newcontext;
    thread_lib[1].active = true;

    makecontext(&newcontext, ( void (*) ( void ))funcPtr, 1, argPtr);

    printf("swap to function\n");
    swapcontext( &(thread_lib[0].thread_context), &newcontext);
    


    printf("swapped backed to threadcreate\n");
    return 0;
    
}

extern void threadYield(){
    swapcontext(&(thread_lib[1].thread_context), &(thread_lib[0].thread_context));
}