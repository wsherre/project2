#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
#include <assert.h>
void __attribute__((destructor)) lib_destroy();

//legit idk just wanted to start here
int array_size = 1;

//struct that will hold contents of my library
//thread_context: context of the thread
//active - if the thread is active and has a workable context
//isExited - if the thread has already been used and finished it's tasks
typedef struct library{
    ucontext_t thread_context;
    bool active;
    bool isExited;
}library;

//make an array of structs to hold all the threads information
//the threads unique id will be the index of the array, main's id will be 0
library *thread_lib;
//array of void*. this is for threadJoin to find the results of exited threads
void** exited_lib;

typedef struct lock_info{
    bool isLocked;
    int thread_id;
}lock_info;

lock_info *lock;

bool **condition;

//making globals
int thread_lib_size = 0;
const int main_thread = 0;
int active_threads = 0;
int test = 1;
//keeps track of which thread is running
int current_running_tid = 0;
int interruptsAreDisabled;

//declaring helper functions
int next_thread();
void wrapper_function(thFuncPtr, void*);
static void interruptDisable ();
static void interruptEnable ();

//initialie the arrays and set the context of the main thread
//also allow interrupts
extern void threadInit(){
    //initialize some stuff
    thread_lib = malloc(array_size * sizeof(library));
    exited_lib = malloc(array_size * sizeof(void *));
    for(int i = 0; i < array_size; ++i){
        thread_lib[i].active = false;
        thread_lib[i].isExited = false;
        exited_lib[i] = NULL;

        if(i < NUM_LOCKS){
            
        }
    }
    lock = (lock_info*)malloc(NUM_LOCKS * sizeof(lock_info));
    condition = (bool**)malloc(NUM_LOCKS * sizeof(bool));
    for(int i = 0; i < NUM_LOCKS; ++i){
        condition[i] = (bool*)malloc(CONDITIONS_PER_LOCK * sizeof(bool));
    }
    for(int i = 0; i < NUM_LOCKS; ++i){
        lock[i].isLocked = false;
        lock[i].thread_id = -1;
        for(int k = 0; k < CONDITIONS_PER_LOCK; ++k){
            condition[i][k] = false;
        }
    }

    
    //activated the main thread and increase the size
    thread_lib[main_thread].active = true;
    thread_lib_size++;
    active_threads++;

    ucontext_t new;
    new.uc_stack.ss_sp = malloc ( STACK_SIZE ) ;
    new.uc_stack.ss_size = STACK_SIZE ;
    new.uc_stack.ss_flags = 0;
    getcontext(&new);

    //save the main threads context
    thread_lib[main_thread].thread_context = new;
    current_running_tid = main_thread;

    //allow the program to be rude and interrupt us
    interruptsAreDisabled = 0; 
}

void library_resize(){
    array_size *= 2;
    thread_lib = realloc(thread_lib, array_size * sizeof(library));
    exited_lib = realloc(exited_lib, array_size * sizeof(void *));
    for(int i = thread_lib_size; i < array_size; ++i){
        
        thread_lib[i].active = false;
        thread_lib[i].isExited = false;
        exited_lib[i] = NULL;
    }
}

void lib_destroy(){
    //library main = thread_lib[0];
    //array_size = 1;

    for(int i = 0; i < array_size; ++i){
            free(thread_lib[i].thread_context.uc_stack.ss_sp);
    }

    free(lock);
    free(condition);

    free(thread_lib);
    free(exited_lib);
    //thread_lib = malloc(array_size * sizeof(library));
    //exited_lib = malloc(array_size * sizeof(void *));
    //thread_lib[0] = main;
    //thread_lib_size = 1;
}

//create a thread yayyy
extern int threadCreate(thFuncPtr funcPtr, void *argPtr){
    
    //disable interrupts so this is a smooth process
    interruptDisable();

    if(thread_lib_size == array_size) library_resize();
    ucontext_t newcontext;

    //i think this is redundant (cause we save in swap context) but so is my life so im gonna make sure we're both running
    getcontext(&newcontext);

    //set the stack size of the context
    newcontext.uc_stack.ss_sp = malloc ( STACK_SIZE ) ;
    newcontext.uc_stack.ss_size = STACK_SIZE ;

    //spent 30 minutes trying to look up what this means and still have no clue
    newcontext.uc_stack.ss_flags = 0;

    //create the temporary id that is the current running id
    //this is because the running id can change later so this remembers what the current context # is in the array
    int temp = current_running_tid;

    //activate
    thread_lib[thread_lib_size].thread_context = newcontext;
    thread_lib[thread_lib_size].active = true;
    thread_lib_size++; 
    active_threads++;

    //make a context for this thread. call the wrapper function and pass this function in and its argument
    //details of the wrapper function are way below
    makecontext(&newcontext, ( void (*) ( void ))wrapper_function, 2, funcPtr, argPtr);
    
    //right now im not reusing thread ids so the running thread will be the end of the array
    current_running_tid = thread_lib_size - 1;

    //save thread id so when we return the function can return the proper value
    int thread_id = current_running_tid;

    //allow interrupts to happen again
    interruptEnable();

    //if i messed up there's no going back now
    swapcontext( &(thread_lib[temp].thread_context), &newcontext);
    
    //the current running thread id could be up to like 5000 but luckily we saved it beforehand so we're good
    return thread_id;
    
}

//change a thread yayyy
extern void threadYield(){
    
    //make sure we're allowed to interrupt becuase we don't want to be rude
    if(!interruptsAreDisabled){
        //make sure we aren't interrupted during a yield which would be extremely rude
        interruptDisable();
        //save the current thread number
        int current = current_running_tid;

        //get the next thread in our array, if we're at the end then we go back to 0
        current_running_tid = next_thread();

        //allow us to be interrupted again and pray the next line runs before another interrupt
        interruptEnable();

        swapcontext(&(thread_lib[current].thread_context), &(thread_lib[current_running_tid].thread_context));
        //commenting under this line so a timer doesn't interrupt in the middle of a comment and swapcontext doesn't get called
    }
}

//join a thread yayyy
extern void threadJoin(int thread_id, void **result){

    if(thread_id < 1 || thread_id >= thread_lib_size)
        return;
    if(thread_lib[thread_id].isExited == true)
        return;
    int temp = current_running_tid;
    int current;
    //if the thread is active we gotta finish it
    while(thread_lib[thread_id].active == true){

        //keep swapping threads until the thread has finished and isn't active anymore
        //threadYield();
        interruptDisable();
        //save the current thread number
        current = current_running_tid;
        //get the next thread in our array, if we're at the end then we go back to 0
        current_running_tid = thread_id;

        //allow us to be interrupted again and pray the next line runs before another interrupt
        interruptEnable();

        //swap to that thread so that it finishes faster
        swapcontext(&(thread_lib[current].thread_context), &(thread_lib[current_running_tid].thread_context));
        
    }
    interruptDisable();
    current_running_tid = temp;
    //if the thread has exited then this value will be true. 
    //if false then the thread never existed. just like my work ethic in an engl class
        //free the threads context cause we done with it
    //free(thread_lib[thread_id].thread_context.uc_stack.ss_sp);
    //save the results
    if(exited_lib[thread_id] != NULL)
        *result = exited_lib[thread_id];
    thread_lib[thread_id].isExited = true;
    active_threads--;
    //if we have joined every thread except main then we don't need the lib anymore
    //call lib free to re initialize it
    interruptEnable();
}

//exit a thread yayyy
extern void threadExit(void *result){

    if(current_running_tid == main_thread){
        //lib_destroy();
        exit(0);
    }

    //i dont wann stop - ozzy osbourne
    interruptDisable();

    /*the value passed into this function will be set as the result of the thread
    if a thread normally returns it will still call thread exit so regardless of
    what happpens to a thread it will go through this function*/
    if(result != NULL)
        exited_lib[current_running_tid] = result;
    
    //unactivate and set isExited to true for reasons mentioned in threadJoin
    thread_lib[current_running_tid].active = false;
    //thread_lib[current_running_tid].isExited = true;
    int current = current_running_tid;
    current_running_tid = main_thread;

    //i wanna stop - not ozzy osbourne
    interruptEnable();
    
    //i actually have no idea what i should do when a thread exits so for now
    // im just going back to the main thread
    swapcontext(&(thread_lib[current].thread_context), &(thread_lib[current_running_tid].thread_context));
}

//should lock this thread
extern void threadLock(int lockNum){
    int current;
    interruptDisable();
    //printf("in thread lock; %d\n", current_running_tid);
    //if not lock. lock it.
    if(!lock[lockNum].isLocked){
        lock[lockNum].isLocked = true;
        lock[lockNum].thread_id = current_running_tid;
        interruptEnable();
    }else{
        interruptEnable();
        //while its locked switch into the context that currently holds the lock
        while(lock[lockNum].isLocked){
            interruptDisable();

            current = current_running_tid;
            
            current_running_tid = lock[lockNum].thread_id;

            interruptEnable();

            swapcontext(&(thread_lib[current].thread_context), &(thread_lib[current_running_tid].thread_context));
            
        }
        interruptDisable();
        //grab it
        lock[lockNum].isLocked = true;
        lock[lockNum].thread_id = current_running_tid;
        interruptEnable();
    }
}
//should unlock
extern void threadUnlock(int lockNum){
    interruptDisable();
    //if we hold the lock, unlock it
    if(lock[lockNum].thread_id == current_running_tid){
        lock[lockNum].isLocked = false;
        lock[lockNum].thread_id = -1;
    }
    interruptEnable();
}
//if it works it will do what it the write up tells it to do
extern void threadWait(int lockNum, int conditionNum){
    //must have lock or else bad
    if(!lock[lockNum].isLocked){
        printf("Error thread: %d called threadWait without having the lock", current_running_tid);
        exit(1);
    }else{
        threadUnlock(lockNum);
        //if that condition isnt true then wait until it is
        while(!condition[lockNum][conditionNum]){
            threadYield();
        }
        interruptDisable();
        threadLock(lockNum);
        condition[lockNum][conditionNum] = false;
        interruptEnable();
    }
}
//sets the condition to true for that lock
extern void threadSignal(int lockNum, int conditionNum){
    condition[lockNum][conditionNum] = true;
}

//this function just returns the index of the next active thread to run
int next_thread(){
    int i = current_running_tid;

    do{
        i++;
        if(i == thread_lib_size)
            i = 0;
        if(i == current_running_tid)
            break;
    }while(thread_lib[i].active == false);
    return i;
}

/* okay so apparently i get a bunch of seg faults when i pass a function into make context and run it
because the computer has no idea what to do after it finishes. so i decided to make a wrapper function
so that the func when finished will have a path to keep going down after its done
then it will call thread exit which take care of carefully and completely disembowling everything 
that thread used to be and throwing it away*/
void wrapper_function(thFuncPtr func, void* parameter){
    void * result = func(parameter);
    interruptDisable();
    if(result != NULL){
        exited_lib[current_running_tid] = result;
    }
    thread_lib[current_running_tid].active = false;
    //int current = current_running_tid;
    //current_running_tid = main_thread;
    //thread_lib[current_running_tid].isExited = true;
    interruptEnable();

    //swapcontext(&(thread_lib[current].thread_context), &(thread_lib[current_running_tid].thread_context));
    threadYield();
}

//i dont wanna stop - ozzy osbourne
static void interruptDisable () {
    assert (! interruptsAreDisabled ) ;
    interruptsAreDisabled = 1;
}

//i wanna stop - not ozzy osbourne
static void interruptEnable () {
    assert ( interruptsAreDisabled ) ;
    interruptsAreDisabled = 0;
}