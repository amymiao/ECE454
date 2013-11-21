
CC = g++-4.7
CFLAGS = -O3 

all: randtrack randtrack_global_lock randtrack_tm randtrack_list_lock randtrack_element_lock randtrack_reduction

randtrack: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) randtrack.cc -o randtrack

randtrack_global_lock: list.h hash.h defs.h randtrack_global_lock.cc
	$(CC) $(CFLAGS) randtrack_global_lock.cc -pthread -o randtrack_global_lock

randtrack_tm: list.h hash.h defs.h randtrack_tm.cc
	$(CC) $(CFLAGS) randtrack_tm.cc -pthread -fgnu-tm -o randtrack_tm

randtrack_list_lock: list.h hash.h defs.h randtrack_list_lock.cc
	$(CC) $(CFLAGS) -D LIST_LOCK randtrack_list_lock.cc -pthread -o randtrack_list_lock

randtrack_element_lock: list.h hash.h defs.h randtrack_element_lock.cc
	$(CC) $(CFLAGS) -D ELEMENT_LOCK randtrack_element_lock.cc -pthread -o randtrack_element_lock

randtrack_reduction: list.h hash.h defs.h randtrack_reduction.cc
	$(CC) $(CFLAGS) randtrack_reduction.cc -pthread -o randtrack_reduction

clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock randtrack_element_lock randtrack_reduction rt*.out*
