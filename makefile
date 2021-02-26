CC=clang
CFLAGS=-Wall -g

all: libmythreads.a

a:  libmythreads.a t c p

libmythreads.o: libmythreads.c
	clang -Wall -c -g libmythreads.c
libmythreads.a: libmythreads.o
	ar -crvs libmythreads.a libmythreads.o
t: libmythreads.a test.c
	clang -g -o t test.c libmythreads.a
c: libmythreads.a cooperative_test.c
	clang -g -o c cooperative_test.c libmythreads.a
p: libmythreads.a preemptive_test.c
	clang -g -o p preemptive_test.c libmythreads.a

clean:
	rm -rf *.o *.dSYM *.a
