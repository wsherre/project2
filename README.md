# project2

will sherrer project 2

known problems{
    it passed the cooperative_test.c test and runs it i belive correctly
    however the preemptive test gets stuck in an infinite while loop.
    every time its about to call threadCreate the timer fires and it interrupts. if i set
    interruptsAreSisable to 1 int threadInit it doesnt matter cause it will still keep firing the timer
    please help me

    also i have not solved the mystery of handling threadExit when it is called by main

    this is a disclaimer that there may be many more problems that are unknown.
    any problems discovered that are not stated are not insured or under warranty.
    the creator of this program holds no responsibility for the inability of the code.
    however if this code actually works then the creator will hold responsibility.
}