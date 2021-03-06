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
    int curgen, i;
    
    char *inboard = input_args->inboard;
    char *outboard = input_args->outboard;
    const int nrows = input_args->nrows;
    const int ncols = input_args->ncols;
    const int start_index = input_args->start_index;
    const int end_index = input_args->end_index;
    const int gens_max = input_args->gens_max;
    
    
    /* Each thread will work on a horizontal segment of the board
     * This segment is defined by the rows start_index and end_index
     * At the end of a generation, a thread must wait for all other threads to complete
     * We use a barrier to do this. This makes sure that all threads read the same board data
     */

    // Optimization Note: Flip the loops for caches. Traverse horizontally on each row.
    for(curgen=0; curgen<gens_max; curgen++) 
    {
        for(i=start_index; i <= end_index; i+=1)
        {
            // Optimization Note: Simplify mod calculation
            const int inorth = i ? (i-1) : nrows-1;
            const int isouth = (i != nrows-1) ? i+1 : 0;
            
            //LDA setup needed to replace BOARD macro, this computation is LICM
            const unsigned int LDA_inorth = nrows * inorth;
            const unsigned int LDA_isouth = nrows * isouth;
            const unsigned int LDA_i = nrows*i;

            /* As we move across the board, there is overlap when counting neighbours:
             * Neighbour of X:
             * OAA
             * OXY
             * OAA
             *
             * Neighbours of Y:
             * AAO
             * XYO
             * AAO
             *
             * Notice neighbours, "A", overlap. We can reduce the computation by storing their sum
             */
            char prev = 
                inboard[LDA_inorth + ncols-1] +
                inboard[LDA_isouth + ncols-1];
            char cur = 
                inboard[LDA_inorth] +
                inboard[LDA_isouth]; 

            unsigned int j;
            for(j=0; j<ncols; j++) 
            {
                const int jwest = j ? j-1 : ncols-1;
                const int jeast = (j != ncols-1) ? j+1 : 0;

                const char next = 
                    inboard[LDA_inorth + jeast] +
                    inboard[LDA_isouth + jeast];

                // Optimization Note : Reducing pointer usage with prev/cur
                const char neighbor_count =
                    prev +
                    cur +
                    next +
                    inboard[LDA_i + jwest] +
                    inboard[LDA_i + jeast];

                prev = cur;
                cur = next;
                    
                // Optimization Note : Replace alivep algorithm
                unsigned char alivep_result = 0;
                if(neighbor_count == 3)
                        alivep_result = 1;
                else if (neighbor_count == 2 && inboard[LDA_i+j])
                        alivep_result = 1;
                
                outboard[LDA_i+j] = alivep_result;
            }
        }
        
        // Wait till all threads are done before switching the boards and continuing,
        // for consistency
        pthread_barrier_wait(input_args->barr);
        SWAP_BOARDS(outboard, inboard);
    }
    //safely return
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
  //Guard against corner cases as asked by the assignment (N=32 and N=10000)
  if (nrows < 32)
  {
    // Lab says we do not need to parallelize less than 32
    return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
  }
  
  //Setup pthread and synchronization primitives
  pthread_t tid[NUM_THREADS];
  pthread_barrier_t barr;
  
  /* Setup thread argument structure
   * We divide the board into NUM_THREADS horizontal slices
   * Each thread gets one slice to work on
   */
  thread_input t_input[NUM_THREADS];
  int i;
  for(i=0;i<NUM_THREADS;i++)
  {
    t_input[i].tid          =       i;
    t_input[i].barr         =       &barr;
    t_input[i].inboard      =       inboard; // All threads opperate on the same board
    t_input[i].outboard     =       outboard;
    t_input[i].ncols        =       ncols;   // All threads use the same overall read bounds
    t_input[i].nrows        =       nrows;
    t_input[i].gens_max     =       gens_max;
    t_input[i].start_index  =       i*nrows/NUM_THREADS; // Defines Start segment
    t_input[i].end_index    =       ((i+1)*nrows/NUM_THREADS)-1; // Defines End segment
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
  
  /*
   *  Wait for all threads to join before returning a board
   */
  for(i=0;i<NUM_THREADS;i++)
  {
    pthread_join(tid[i], NULL);
  }
  
  return t_input[0].inboard;
}
