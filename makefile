CC=clang
CFLAGS=-Wall -g

all: libmythreads.a

libmythreads.o:
	clang -Wall -c -g libmythreads.c
libmythreads.a: libmythreads.o
	ar -crvs libmythreads.a libmythreads.o
test:
	clang -g -o test test.c libmythreads.a
coop:
	clang -g -o coop cooperative_test.c libmythreads.a
pre:
	clang -g -o pre preemptive_test.c libmythreads.a

clean:
	rm -rf *.o *.dSYM *.a
