#ifndef _life_h
#define _life_h

#include <pthread.h>
#define NUM_THREADS 4

/**
 * Given the initial board state in inboard and the board dimensions
 * nrows by ncols, evolve the board state gens_max times by alternating
 * ("ping-ponging") between outboard and inboard.  Return a pointer to 
 * the final board; that pointer will be equal either to outboard or to
 * inboard (but you don't know which).
 */
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max);

/**
 * Same output as game_of_life() above, except this is not
 * parallelized.  Useful for checking output.
 */
char*
sequential_game_of_life (char* outboard, 
			 char* inboard,
			 const int nrows,
			 const int ncols,
			 const int gens_max);

/**
 * Parallel game of life implements a threaded version of the single-threaded
 * sequential game_of_life()
 */
void*
threaded_game_of_life (void * inputs); 

typedef struct thread_input
{
    //THREAD SPECIFIC VARIABLES
    int tid;    //thread id
    pthread_barrier_t * barr;
    //NORMAL VARIABLES
    char* outboard;
    char* inboard;
    int nrows;
    int ncols;
    int gens_max;
    int start_index;
    int end_index;
    int LDA;
    
} thread_input;

#endif /* _life_h */
