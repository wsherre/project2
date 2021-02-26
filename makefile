CC=clang
CFLAGS=-Wall -g

all: libmythreads.a

libmythreads.o:
	clang -Wall -c -g libmythreads.c
libmythreads.a: libmythreads.o
	ar -crvs libmythreads.a libmythreads.o
t:
	clang -g -o t test.c libmythreads.a
c:
	clang -g -o c cooperative_test.c libmythreads.a
p:
	clang -g -o  preemptive_test.c libmythreads.a

clean:
	rm -rf *.o *.dSYM *.a
