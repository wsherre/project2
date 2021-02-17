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
int thread_lib_size = 0;
int main_thread = 0;
int current_running_tid = 0;

int next_thread();
void wrapper_function(thFuncPtr, void*);

extern void threadInit(){
    for(int i = 0; i < array_size; ++i){
        thread_lib[i].active = false;
    }
    thread_lib[main_thread].active = true;
    thread_lib_size++;
    getcontext(&(thread_lib[main_thread].thread_context));
    current_running_tid = main_thread;
}

extern int threadCreate(thFuncPtr funcPtr, void *argPtr){
    
    ucontext_t newcontext;

    getcontext(&newcontext);
    newcontext.uc_stack.ss_sp = malloc ( STACK_SIZE ) ;
    newcontext.uc_stack.ss_size = STACK_SIZE ;
    newcontext.uc_stack.ss_flags = 0;

    int temp = current_running_tid;
    thread_lib[thread_lib_size].thread_context = newcontext;
    thread_lib[thread_lib_size].active = true;
    thread_lib_size++;
    

    makecontext(&newcontext, ( void (*) ( void ))wrapper_function, 2, funcPtr, argPtr);
    printf("swap to function\n");
    current_running_tid++;
    swapcontext( &(thread_lib[temp].thread_context), &newcontext);
    


    printf("swapped backed to threadcreate\n");
    return 0;
    
}

extern void threadYield(){
    printf("c thread: %d         next active thread: %d\n", current_running_tid, next_thread());
    int c = current_running_tid;
    current_running_tid = next_thread();
    swapcontext(&(thread_lib[c].thread_context), &(thread_lib[current_running_tid].thread_context));
}

extern void threadExit(void *result){
    printf("in thread exit, thread id: %d     result: %d\n", current_running_tid, *(int*)result);
    threadYield();
    //swapcontext(&(thread_lib[current_running_tid].thread_context), &(thread_lib[main_thread].thread_context));
}

int next_thread(){
    int i = current_running_tid;

    do{
        i++;
        if(i == thread_lib_size)
            i = 0;
        if(i == array_size)
            i = 0;
    }while(thread_lib[i].active == false);
    return i;
}

void wrapper_function(thFuncPtr func, void* parameter){
    void *result;
    result = func(parameter);
    threadExit(result);
}