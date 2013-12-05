/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <pthread.h>

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

 
 //Imported helper functions from lifeseq.c
 /**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])
 
 
//Implement main game loop here
void *
threaded_game_of_life (void * inputs) 
{
    //recast to proper input argument structure
    thread_input* input_args = (thread_input*) inputs;
    
    //just leaving LDA the same as before except now passing from thread arguments
    const int LDA = input_args->nrows;
    int curgen, i, j;
    
    //specify thread specific boundaries and parameters
    //breakdown into segments using thread id and columns
    int column_start = ((input_args->ncols)/NUM_THREADS) * (input_args->tid);
    int column_end   = ((input_args->ncols)/NUM_THREADS) * (input_args->tid + 1);
    
    //normal outer loop
    for (curgen = 0; curgen < input_args->gens_max; curgen++)
    {
        //flipped the iterators so it strides nicely for increased cache hits
        for (j=0; j<(input_args->nrows); j++)
        {
            //This is the main segmented computation done on a thread by thread basis
            for (i=column_start; i<column_end; i++)
            {
                const int inorth = mod (i-1, input_args->nrows);
                const int isouth = mod (i+1, input_args->nrows);
                const int jwest = mod (j-1,  input_args->ncols);
                const int jeast = mod (j+1,  input_args->ncols);
                
                const char neighbor_count = 
                    BOARD (input_args->inboard, inorth, jwest) + 
                    BOARD (input_args->inboard, inorth, j) + 
                    BOARD (input_args->inboard, inorth, jeast) + 
                    BOARD (input_args->inboard, i, jwest) +
                    BOARD (input_args->inboard, i, jeast) + 
                    BOARD (input_args->inboard, isouth, jwest) +
                    BOARD (input_args->inboard, isouth, j) + 
                    BOARD (input_args->inboard, isouth, jeast);
                
                BOARD(input_args->outboard, i, j) = alivep (neighbor_count, BOARD (input_args->inboard, i, j));
            }
        }
        SWAP_BOARDS(input_args->outboard, input_args->inboard);
        //synchronize threads so they can begin the next iteration together
        pthread_barrier_wait(input_args->barr);
    }
    pthread_exit(NULL);
}
 
 
/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
  
  //lower than the minimum size - do this in case someone runs N < 32
  if (nrows < 32)
    return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
  
  //Setup pthread and synchronization primitives
  pthread_t tid[NUM_THREADS];
  pthread_barrier_t barr;
  
  //Setup thread argument structure
  thread_input t_input[NUM_THREADS];
  int i;
  for(i=0;i<NUM_THREADS;i++)
  {
    t_input[i].tid       =       i;
    t_input[i].barr      =       &barr;
    t_input[i].inboard   =       inboard;
    t_input[i].outboard  =       outboard;
    t_input[i].ncols     =       ncols;
    t_input[i].nrows     =       nrows;
    t_input[i].gens_max  =       gens_max;
  }
  
  /*
   * Initialize barrier with the number of threads 
   * that it will synchronize
   */
  pthread_barrier_init(&barr, NULL, NUM_THREADS);
  
  /*
   * Initialize threads
   */
  for(i=0;i<NUM_THREADS;i++)
  {
    pthread_create(&tid[i], NULL, threaded_game_of_life, (void*) &t_input[i]);
  }
  
  for(i=0;i<NUM_THREADS;i++)
  {
    pthread_join(tid[i], NULL);
  }
  
  return t_input[0].inboard;
}
