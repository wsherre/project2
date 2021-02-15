CC=clang
CFLAGS=-Wall -g
BINS=libmythreads.a#contextfun contexts funcptr makecontext_example #cooperative_test preemptive_test


all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^ 
libmythreads.o:
	clang -Wall -c libmythreads.c
libmythreads.a: libmythreads.o
	ar -crvs libmythreads.a libmythreads.o

clean:
	rm -rf $(BINS) *.o *.dSYM *.a
