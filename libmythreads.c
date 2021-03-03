#include "mythreads.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
#include <assert.h>
void __attribute__((destructor)) lib_destroy();

//legit i dont know i  just wanted to start small and work my way up
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

//struct to hold the info for every loc
typedef struct lock_info{
    bool isLocked;
    int thread_id;
}lock_info;

//an array of locks
lock_info lock[NUM_LOCKS];

//linked list struct for the queue in condition variables
typedef struct queue{
    int thread_id;
    bool is_signalled;
    struct queue* next;
}queue;

//array of queue pointers for the condition variables. basically an array of linked lists
queue *condition[NUM_LOCKS][CONDITIONS_PER_LOCK];

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
bool is_in_queue(int, int, int);
bool condition_signalled(int, int, int);
void enqueue(int, int, int);
void dequeue(int, int);
bool first_in_queue(int, int );
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

        
    }
    //initialize locks and condition variables
    for(int i = 0; i < NUM_LOCKS; ++i){
        lock[i].isLocked = false;
        lock[i].thread_id = -1;
        for(int k = 0; k < CONDITIONS_PER_LOCK; ++k){
            condition[i][k] = malloc(sizeof(queue));
            condition[i][k]->thread_id = -1;
            condition[i][k]->is_signalled = false;
            condition[i][k]->next = NULL;
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

//if our library fills up then double its size and initialize
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

//destructor running at the end of the library. free all of the data
void lib_destroy(){

    for(int i = 0; i < array_size; ++i){
            free(thread_lib[i].thread_context.uc_stack.ss_sp);
    }

    for(int i = 0; i < NUM_LOCKS; ++i){
        for(int k = 0; k < CONDITIONS_PER_LOCK; ++k){
            free(condition[i][k]);
        }
    }

    free(thread_lib);
    free(exited_lib);
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


    //if i messed up there's no going back now
    swapcontext( &(thread_lib[temp].thread_context), &newcontext);

    //allow interrupts to happen again
    interruptEnable();
    
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


        swapcontext(&(thread_lib[current].thread_context), &(thread_lib[current_running_tid].thread_context));
        
        //allow us to be interrupted again and pray the next line runs before another interrupt
        interruptEnable();
    }
}

//join a thread yayyy
extern void threadJoin(int thread_id, void **result){

    if(thread_id < 1 || thread_id >= thread_lib_size)
        return;
    if(thread_lib[thread_id].isExited == true)
        return;
    int temp = current_running_tid;
    //int current;
    //if the thread is active we gotta finish it
    while(thread_lib[thread_id].active == true){

        //yield until our process isn't active anymore
        threadYield(); 
        
    }
    interruptDisable();
    current_running_tid = temp;
    //if the value wasn't null then return the result of the thread
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

    
    
    //i actually have no idea what i should do when a thread exits so for now
    // im just going back to the main thread
    swapcontext(&(thread_lib[current].thread_context), &(thread_lib[current_running_tid].thread_context));
    //i wanna stop - not ozzy osbourne
    interruptEnable();
}

//should lock this thread
extern void threadLock(int lockNum){
    int current;
    if(!interruptsAreDisabled)interruptDisable();
    
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

            swapcontext(&(thread_lib[current].thread_context), &(thread_lib[current_running_tid].thread_context));

            interruptEnable();
            
        }
        interruptDisable();
        //grab the lock
        lock[lockNum].isLocked = true;
        lock[lockNum].thread_id = current_running_tid;
        interruptEnable();
    }
}

//should unlock 
extern void threadUnlock(int lockNum){
    if(!interruptsAreDisabled)interruptDisable();
    //if the current running thread holds the lock, unlock it
    if(lock[lockNum].thread_id == current_running_tid){
        lock[lockNum].isLocked = false;
        lock[lockNum].thread_id = -1;
    }
    interruptEnable();
}
//if it works it will do what it the write up tells it to do
extern void threadWait(int lockNum, int conditionNum){
    interruptDisable();
    //must have lock or else bad
    if(!lock[lockNum].isLocked){
        printf("Error thread: %d called threadWait without having the lock", current_running_tid);
        exit(1);
    }else{
        interruptEnable();
        threadUnlock(lockNum);
        interruptDisable();
        //if that condition isnt true then wait until it is
        enqueue(current_running_tid, lockNum, conditionNum);
        
        //this function checks to see if the current thread has been signalled to run
        while(!condition_signalled(current_running_tid, lockNum, conditionNum)){
            //if not then yield to another thread
            interruptEnable();
            threadYield();
            interruptDisable();
        }
        /*if multiple threads are called then this checks to make sure the current thread is 
        the first in the queue*/
        while(!first_in_queue(lockNum, conditionNum)){
            interruptEnable();
            threadYield();
            interruptDisable();
        }
        //once it's been signalled, lock and dequeue the thread from the queue
        threadLock(lockNum);
        interruptDisable();
        dequeue(lockNum, conditionNum);
        interruptEnable();
    }
}
//sets the condition to true for that lock
extern void threadSignal(int lockNum, int conditionNum){
    queue* q = condition[lockNum][conditionNum];
    //loop through the queue and signal the next thread that hasn't been signalled
    //will calmly exit if no threads are in the queue
    while(q->next != NULL){
        q = q->next;
        if(q->is_signalled == false){
            q->is_signalled = true;
            break;
        }
    }
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

/* make a wrapper function
so that the func when finished will have a path to keep going down after its done
then it will call thread exit which take care of carefully and completely disembowling everything 
that thread used to be and throwing it away*/
void wrapper_function(thFuncPtr func, void* parameter){

    interruptEnable();
    void * result = func(parameter);
    interruptDisable();
    if(result != NULL){
        exited_lib[current_running_tid] = result;
    }
    thread_lib[current_running_tid].active = false;
    
    interruptEnable();

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

//checks to see if the thread_id has been signalled to run
bool condition_signalled(int thread_id, int lock, int conditional){
    queue *q = condition[lock][conditional];
    while(q != NULL){
        if(q->thread_id == current_running_tid)
            return q->is_signalled;
        q = q->next;
    }
    return false;
}

//a basic enqueue function for the linked list
void enqueue(int thread_id, int lock, int conditional){
    queue *q = condition[lock][conditional];
    queue *new_node = malloc(sizeof(queue));
    new_node->thread_id = thread_id;
    new_node->is_signalled = false;
    new_node->next = NULL;
    while(q->next != NULL){
        q = q->next;
    }
    q->next = new_node;
}

//basic dequeue
void dequeue(int lock, int conditional){
    queue *q = condition[lock][conditional];
    if(q->next == NULL){
        printf("dequing an empty queue\n lock %d\ncondition %d\n", lock, conditional);
        exit(0);
    }
    queue *next = q->next;
    q->next = next->next;
    free(next);
}

//returns if the current_running_tid is the first thread in the queue and ready to be dequeued
bool first_in_queue(int lockNum, int conditional){
    queue *q = condition[lockNum][conditional];
    return q->next->thread_id == current_running_tid;
}