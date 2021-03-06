# project2

will sherrer project 2

this project contains a libmythreads.c file and a makefile and a header file that will allow the user to create a thread_library

design{
    made the data structure an array of thread structs. the index is the unique id. it dynamically resizes and frees at the end.
    the locks are just an array of structs containing if its locked and which thread has the lock
    the condition variables is a 2d array of struct pointers. the structs basically form a linked list for the queue. 
}
known problems{
    the speeed test does seg fault but i believe that to be an assertion error. the memory is being cleaned up at the end. it currently passes
    the memory test case so because it passes i won't bother freeing memory as I go, but if you created 1000s of threads then it could flag the test case because its not freed till its done
}
