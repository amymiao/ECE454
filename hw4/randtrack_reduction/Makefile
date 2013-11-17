
CC = g++-4.7
CFLAGS = -O3 -pthread

all: randtrack 

randtrack: list.h hash.h defs.h randtrack_reduction.cc
	$(CC) $(CFLAGS) randtrack_reduction.cc -o randtrack


clean:
	rm -f *.o randtrack randtrack_reduction
