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
    int curgen, i, j;
    
    char *inboard = input_args->inboard;
    char *outboard = input_args->outboard;
    const int LDA = input_args->LDA;
    const int nrows = input_args->nrows;
    const int ncols = input_args->ncols;
    const int start_index = input_args->start_index;
    const int end_index = input_args->end_index;
    const int gens_max = input_args->gens_max;
    
    for(curgen=0; curgen<gens_max; curgen++) 
    {
        for(i=start_index; i<=end_index; i++) 
        {
            const int inorth = mod (i-1, nrows);
            const int isouth = mod (i+1, nrows);
            for(j=0; j<ncols; j++)
            {
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
                
                const char neighbor_count = 
                    BOARD (input_args->inboard, inorth, jwest) + 
                    BOARD (input_args->inboard, inorth, j) + 
                    BOARD (input_args->inboard, inorth, jeast) + 
                    BOARD (input_args->inboard, i, jwest) +
                    BOARD (input_args->inboard, i, jeast) + 
                    BOARD (input_args->inboard, isouth, jwest) +
                    BOARD (input_args->inboard, isouth, j) + 
                    BOARD (input_args->inboard, isouth, jeast);
                
                BOARD(input_args->outboard, i, j) = alivep(neighbor_count, BOARD (input_args->inboard, i, j));
            }
        }
        pthread_barrier_wait(input_args->barr);
        SWAP_BOARDS(input_args->outboard, input_args->inboard);
    }
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
  
  //initial setup
  const int LDA = nrows;
  
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
    t_input[i].tid          =       i;
    t_input[i].barr         =       &barr;
    t_input[i].inboard      =       inboard;
    t_input[i].outboard     =       outboard;
    t_input[i].ncols        =       ncols;
    t_input[i].nrows        =       nrows;
    t_input[i].gens_max     =       gens_max;
    t_input[i].LDA          =       LDA;
    t_input[i].start_index  =       i*nrows/4;
    t_input[i].end_index    =       ((i+1)*nrows/4)-1;
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
