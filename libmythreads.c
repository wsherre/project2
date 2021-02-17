#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#define array_size 5000



extern void threadInit(){
    //nothing
}

extern int threadCreate(thFuncPtr funcPtr, void *argPtr){
    
    ucontext_t newcontext;

    getcontext(&newcontext);
    newcontext.uc_stack.ss_sp = malloc ( STACK_SIZE ) ;
    newcontext.uc_stack.ss_size = STACK_SIZE ;
    newcontext.uc_stack.ss_flags = 0;
    makecontext(&newcontext, funcPtr, 1, argPtr);
    setcontext(&newcontext);
    


    printf("hey\n");
    return 0;
    
}