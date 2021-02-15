CC=clang
CFLAGS=-Wall -g
BINS=test#contextfun contexts funcptr makecontext_example #cooperative_test preemptive_test


all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^ 

libmythreads.a: 
	ar -crvs libmythreads.a libmythreads.o

clean:
	rm -rf $(BINS) *.o *.dSYM *.a
