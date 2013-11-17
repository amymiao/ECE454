
CC = g++-4.7
CFLAGS = -O3 -pthread -fgnu-tm

all: randtrack 

randtrack: list.h hash.h defs.h randtrack_tm.cc
	$(CC) $(CFLAGS) randtrack_tm.cc -o randtrack


clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock
