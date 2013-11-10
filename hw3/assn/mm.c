/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Sil.The.Bird",
    /* First member's full name */
    "Calvin Fernandes",
    /* First member's email address */
    "calvin.fernandes@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "Everard Francis",
    /* Second member's email address (leave blank if none) */
    "everard.francis@mail.utoronto.ca"
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
 *************************************************************************/
#define WSIZE       sizeof(void *)         /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */
#define OVERHEAD    DSIZE     /* overhead of header and footer (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

#ifdef DEBUG
#define DPRINTF(fmt, ...) \
    printf(fmt, ## __VA_ARGS__)
#else
#define DPRINTF(fmt, ...)
#endif

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* alignment */
#define ALIGNMENT 16
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0xf)

int mm_check();

void* heap_listp = NULL;

/* Data structures for free list management BEGIN */
typedef struct free_block {
    struct free_block* next;
    struct free_block* prev;
} free_block;

#define NUM_FREE_LISTS 8
#define MIN_BLOCK_SIZE 32
#define MIN_BLOCK_PWR 5
free_block* free_list_array[NUM_FREE_LISTS];


/* Data structures for free list management END */

int hash_function(int size) {
    int counter = 0;
    assert(size >= 2*DSIZE);
    size--;
    while (size != 0) {
        size = size >> 1;
        counter++;
    }
    assert(counter >= MIN_BLOCK_PWR);
    counter = counter - MIN_BLOCK_PWR;
    return counter >= NUM_FREE_LISTS ? NUM_FREE_LISTS-1 : counter;
}

/* This function removes a free block from the free_list
 * It also adjusts the free_list pointer if needed
 */
void remove_from_list(free_block* block) {
    DPRINTF("REMOVE_FROM_FREE_LIST: REMOVING 0x%x\n", block);

    size_t size = GET_SIZE(HDRP(block));
    int free_list_index = hash_function(size);

    if (block == NULL) {
        return;
    }
    if (block != block->next) {
        block->prev->next = block->next; // Make previous free block point to next free block
        block->next->prev = block->prev; // Make next free block point to previous free block
        if (free_list_array[free_list_index] == block) {
            // If we are removing the head pointer, we must set a new head pointer
            free_list_array[free_list_index] = block->next;
        }
    } else  {
        free_list_array[free_list_index] = NULL;
    }
}

/* This function adds a free block to the free_list
 */
void add_to_list(free_block* temp)
{
    size_t size = GET_SIZE(HDRP(temp));
    int free_list_index = hash_function(size);

    DPRINTF("ADD_TO_LIST: ADDING 0x%x\n", temp);
    if (temp == NULL)
        return;

    if (free_list_array[free_list_index] == NULL)
    {
        // The free list is empty, this will be the first free block. Set head to point to it.
        free_list_array[free_list_index] = temp;
        free_list_array[free_list_index]->next = temp;
        free_list_array[free_list_index]->prev = temp;
    }

    else
    {
        // Insert the newly freed block into the start of the free list
        // It will become the new head
        temp->next = free_list_array[free_list_index];
        temp->prev = free_list_array[free_list_index]->prev;
        temp->prev->next = temp;
        temp->next->prev = temp;
    }
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    DPRINTF("MM_INIT:\n");
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(OVERHEAD, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(OVERHEAD, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += DSIZE;
    // Initialize the free list
    int i;
    for (i=0; i < NUM_FREE_LISTS; i++) {
        free_list_array[i] = NULL;
    }

    return 0;
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp)
{
    DPRINTF("COALESCING\n");
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        // Remove the next free block from free_list
        free_block* temp = (free_block*)NEXT_BLKP(bp);
        remove_from_list(temp);

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        // Remove the prev free block from free_list
        free_block* temp = (free_block*)PREV_BLKP(bp);
        remove_from_list(temp);

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
        // Remove both the next and prev free blocks from free list
        free_block* temp = (free_block*)PREV_BLKP(bp);
        remove_from_list(temp);
        temp = (free_block*)NEXT_BLKP(bp);
        remove_from_list(temp);

        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
                GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        return (PREV_BLKP(bp));
    }
}


/**********************************************************
 * deferred_coalesce
 * Look for blocks that can be coalesced together so an
 * sbrk can be prevented
 * Calls coalesce to do its heavy lifting
 **********************************************************/
void *deferred_coalesce(size_t size)
{
 	DPRINTF("DEFERRED COALESCING\n");
 	DPRINTF("FIND_FIT HAS RETURNED NULL SO ATTEMPT COALESCING\n");
 	//We can't really be too smart about this because find_fit has already
 	//searched the list where it will hope to find the index
 	//This means we need to start out at the list that holds the smallest blocks
 	free_block* traverse = NULL;

 	int i;
 //	for (i=0; i < NUM_FREE_LISTS; i++)
 	for (i=NUM_FREE_LISTS-1; i >= 0; i--)
 	{
 		traverse = free_list_array[i];
 		if (free_list_array[i] != NULL)	//list should not be empty
 		{
 			do
 			{

 			   /* block is already in the free list so we don't need to do
 				* anything besides removing it from the free list before we
 				* coalesce. We have to be careful because when we reinsert 
 				* the block back into the free list - there is a very high
 				* chance that we will not end up in the same free list we 
 				* started in
 				*/

 				//keep track of the next position in the current free block
 				//Reasoning: If traverse coalesces but the coalesce still 
 				//was not enough then it will get placed in another free list
 				//thus breaking our traversal in this free list
 				//we need to ensure we stay in the current free list

 				remove_from_list(traverse);		//remove block from the free list
 				
 				//this can remove multiple blocks from the free list
 				size_t old_size = GET_SIZE(HDRP(traverse));
 				traverse = coalesce(traverse);
 				size_t new_size = GET_SIZE(HDRP(traverse));
 				if (GET_SIZE(HDRP(traverse)) >= size)
 				{
 					//mark allocated otherwise user will have the complication of getting the size and then doing it if they choose to do it
 					PUT(HDRP(traverse), PACK(new_size, 1));
 					PUT(FTRP(traverse), PACK(new_size, 1));

 					return (void *)traverse;
 				}
 				else	//coalesced size < size requested
 				{
 					//put back in free list - this won't go back in the same free list
 					//sanity and caution go hand-in-hand
 					PUT(HDRP(traverse), PACK(new_size, 0));
 					PUT(FTRP(traverse), PACK(new_size, 0));
 					add_to_list(traverse);
 				}

 				if (old_size == new_size)
 				{
 					//still in the same list
 					traverse = traverse->next;
 				}

 				else 
 				{
 					traverse = free_list_array[i];	//reset the pointer and start at the beginning
 				}
 			}
 			while (traverse != free_list_array[i]);
 		}
 	}
 	return NULL;	//could not find anything to give user - use sbrk
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    DPRINTF("EXTENDING HEAP\n");
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == (void *)-1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    /* Coalesce if the previous block was free */
    // FIXME: This cannot be done right now
    // because realloc marks a block as "free" - which becomes a problem as coalese
    // will try to merge that block with this one, and therefore remove the old one from the list
    // since it was never in the list, it will seg fault
    //return coalesce(bp);
    return bp;
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    DPRINTF("FINDING FIT\n");

    int free_list_index = hash_function(asize);

    mm_check();
    // Traverse the free list and find the first fitting block
    // If there is excess space in the free block we found, we
    // place the remainder back into the free list as a smaller free block
    int i;
    for (i=free_list_index; i < NUM_FREE_LISTS; i++) {
        if (free_list_array[i] != NULL) {
            break;
        }
    }

    if (i == NUM_FREE_LISTS) {
        return NULL;
    }

    free_block* temp = free_list_array[i];

    do {
        int size = GET_SIZE(HDRP(temp));
        if (size >= asize && size < (asize + 2*DSIZE)) {
            // We found a block that can fit, but cannot be split into an excess free block
            // This is easy to resolve, simply remove it from the free list
            remove_from_list(temp);
            return (void*)temp;
        } else if (size >= (asize + 2*DSIZE)) {
            // The block found has excess size and we can split the excess
            // into a free block
            // Example: Requested size = 4
            //          Found free block size = 10
            // [H1][ ][ ][ ][ ][ ][ ][ ][ ][F1]
            // newSize = 6
            remove_from_list(temp);
            int newSize = size - asize;

            // Point to the ending portion of the free block, so that
            // we can assign this portion to the user
            void* userPtr = (void*)temp + newSize;

            // Set the header/footer of the user required block with their requested size
            // Example (cont): [H1][ ][ ][ ][ ][ ][H2][ ][ ][F2]
            PUT(HDRP(userPtr), PACK(asize,0));
            PUT(FTRP(userPtr), PACK(asize,0));

            // Adjust the size of the free block
            // Example (cont): [H1][ ][ ][ ][ ][F1][H2][ ][ ][F2]
            PUT(HDRP(temp), PACK(newSize,0));
            PUT(FTRP(temp), PACK(newSize,0));

            add_to_list(temp);
            DPRINTF("AFTER FREE BLOCK SPLITTING");
            mm_check();
            return userPtr;
        }
        temp = temp->next;
    } while (temp != free_list_array[free_list_index]);
    /*void *bp;

      for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
      {
      if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
      {
      return bp;
      }
      }*/
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
    DPRINTF("PLACE - setting allocated bit\n");
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(bsize, 1));
    PUT(FTRP(bp), PACK(bsize, 1));
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 * This function frees a block and inserts it into the free_list
 * There is no additional overhead, as the space we freed up is used for the entry and is
 * invisible to the user.
 **********************************************************/
void mm_free(void *bp)
{
    if(bp == NULL) {
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    DPRINTF("RECEIVED FREE (0x%x), size=%d\n",bp,size);

    if (GET_ALLOC(HDRP(bp)) == 0) {
        // Block was already freed
        DPRINTF("BLOCK ALREADY FREE\n");
        return;
    }

    //mark allocated bit 0
    mm_check();
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));

    // After coalescing, we can add the new (possibly bigger) free block to the free list
    //add_to_list((free_block*)coalesce(bp));
    add_to_list((free_block*)(bp));		//DO DEFERRED COALESCING SO DON'T COALESCE AFTER A FREE

    DPRINTF("AFTER FREE:\n");
    mm_check();

}

/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{

    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = DSIZE + OVERHEAD;
    else
        asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1))/ DSIZE);

    DPRINTF("RECEIVED MALLOC: size=%d\n",asize);

    /* Search the free list for a fit */

    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        DPRINTF("FIND FIT - SERVICED MALLOC (0x%x), size=%d\n",bp,asize);
        mm_check();
        return bp;
    }

    /* If we cannot find a fit in the free list - then we need to coalesce */
    if ((bp = deferred_coalesce(asize)) != NULL)
    {
    	//already marks allocated
    	DPRINTF("DEFERRED COALESCING - SERVICED MALLOC (0x%x)\n",bp);
    	mm_check();
    	return bp;
    }

    /* No fit and no coalesce can be done. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    DPRINTF("EXTENDSIZE - SERVICED MALLOC (0x%x), size=%d\n",bp,asize);
    mm_check();

    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    DPRINTF("RECEIVED REALLOC: (0x%x), size=%d\n",ptr,size);
    // If size == 0 then this is just free, and we return NULL.
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    /// If old ptr is NULL, then this is just malloc.
    if (ptr == NULL)
        return (mm_malloc(size));

    mm_check();
    void *oldptr = ptr;
    void *newptr;

    size_t old_size = GET_SIZE(HDRP(oldptr));
    size_t padded_size;

    if (size <= DSIZE)
        padded_size = DSIZE + OVERHEAD;
    else
        padded_size = DSIZE * ((size + (OVERHEAD) + (DSIZE-1))/ DSIZE);

    // Handle shrink case
    if (padded_size <= old_size) 
    {
        // CASE 1 - User wants to shrink the allocation 
    	//old_size = padded_size + excess 
    	//check if excess is >= MIN_BLOCK_SIZE for tearing
    	size_t excess = old_size - padded_size;

        // FIXME right now its faster to not bother tearing. Doing this return gets us +2 utilization points
        return oldptr;
    	
    	//tearing possible
    	if (excess >= 2*DSIZE)
    	{
    		//Fix the 2nd part of the block (the tear) - send this to the free list
    		newptr = (void *)oldptr+padded_size;	//get a pointer to the payload of the torn block
    		PUT(HDRP(newptr), PACK(excess,0));		//set the proper size for the header of the torn block - deallocate markation
            PUT(FTRP(newptr), PACK(excess,0));		//set the proper size for the footer of the torn block - deallocate markation
            add_to_list((free_block*)newptr);

            //Fix the 1st part of the block - the part we send to the user
            //The part of the relevant data is already sitting in the payload so we can return this once fixed
            PUT(HDRP(oldptr), PACK(padded_size,1));		//set the proper size for the header of the torn block - set allocated as we give to user
            PUT(FTRP(oldptr), PACK(padded_size,1));		//^ for footer
            //DPRINTF("TORN CASE HAPPENED!\n");
            return oldptr;	//give this to the user
    	}

    	//tearing impossible - excess data can not be torn
    	if (excess < 2*DSIZE)
    	{
    		//DPRINTF("NO-TEAR CASE HAPPENED!\n");
    		return oldptr;
    	}


        //memcpy(ptr, oldptr, size);
        //return ptr;
    } 

    //Handle expand case
    //User wants more data
    else 
    {
    	
    	//Attempt a coalesce 
    	//use ptr to do the coalescing and oldptr will point to the payload

    	//Mark as free so coalesce will not complain and pray to God this doesn't mess up - it did so I didn't pray hard enough
    	//old_size is what the block originally was
    	PUT(HDRP(ptr), PACK(old_size,0));
    	PUT(FTRP(ptr), PACK(old_size,0));
    	ptr = coalesce(ptr);
        mm_check();
    	
    	size_t coalesced_size = GET_SIZE(HDRP(ptr));
    	if (coalesced_size >= padded_size)
    	{
    		//coalesce worked properly
    		//remove the overhead so we can memcpy properly
    		size_t payload_size = old_size-OVERHEAD;
            // We have to use memmove because of potential overlap. Memcopy does not handle overlap
    		memmove(ptr, oldptr, payload_size);	//shift the payload if necessary

    		//fix the allocated bits - give to user
    		PUT(HDRP(ptr), PACK(coalesced_size,1));
    		PUT(FTRP(ptr), PACK(coalesced_size,1));
    		return ptr;
    	}

        // Worst case scenario - This means an extend_heap will most likely be done
        newptr = mm_malloc(size);
        if (newptr == NULL) {
            printf("WTF?\n");
            return NULL;
        }

        //remove the overhead that is calculated using the SIZE macro and only copy what is in the payload
        //this should avoid problems
        size_t payload_size = old_size-OVERHEAD;
        memcpy(newptr, oldptr, payload_size);

        // The old location was coalesed and is now free. Add it to the free list
        add_to_list((free_block*)ptr);

        return newptr;
    }

    //printf("WTF\n");
    return NULL;
    /* assert(1==0);

     newptr = mm_malloc(size);
     if (newptr == NULL)
         return NULL;

     // Copy the old data.
    printf("copySize (includes HDR+FTR) before removing OVERHEAD:%d\n", (int)copySize);
     copySize = GET_SIZE(HDRP(oldptr)) - OVERHEAD;
     printf("copySize without OVERHEAD:%d\n", (int)copySize);
     if (size < copySize)
         copySize = size;
     memcpy(newptr, oldptr, copySize);
     mm_free(oldptr);
     return newptr;
    */
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
#ifndef DEBUG
inline
#endif
int mm_check(void)
{
#ifdef DEBUG
    void* start = heap_listp;

    DPRINTF("\n\nHEAP STATS:\n");
    while (GET_SIZE(HDRP(start)) != 0) {
        DPRINTF("Address: 0x%x\tSize: %d\tAllocated: %d\n",start,GET_SIZE(HDRP(start)),GET_ALLOC(HDRP(start)));
        start = NEXT_BLKP(start);
    }
    DPRINTF("Address: 0x%x\tSize: %d\tAllocated: %d [HEAP END]\n",start,GET_SIZE(HDRP(start)),GET_ALLOC(HDRP(start)));
    DPRINTF("\n");


    //Free list consistency check - check to see if all blocks in the free list are indeed free

    int i;
    int size = -1;
    int free_status = -1;
    for (i=0; i < NUM_FREE_LISTS; i++) {
        free_block* traverse = free_list_array[i];
        DPRINTF("\nFREE LIST STATS (Range %d-%d):\n",MIN_BLOCK_SIZE << i,MIN_BLOCK_SIZE << (i+1));
        if (traverse != NULL)
        {
            do
            {
                size = GET_SIZE(HDRP(traverse));
                free_status = GET_ALLOC(HDRP(traverse));
                DPRINTF("Address in Free-List: 0x%x\tSize: %d\tAllocated: %d\n", traverse, size, free_status);
                if (free_status == 1)
                {
                    DPRINTF("ERROR: BLOCKS IN FREE LIST ARE ALLOCATED!\n\n\n");
                    break;
                }

                //checking if pointers in the free list are within the heap
                if ((void*)traverse > mem_heap_hi() || (void*)traverse < heap_listp )
                {
                    DPRINTF("ERROR: 0x%x in the free list is not found in the heap\n\n\n", traverse);
                    break;
                }

                traverse = traverse->next;
            }
            while (traverse != free_list_array[i]);  //termination condition of a circular list
        }
    }
    //Check if all blocks are at least the minimum block size and if they are aligned
    start = heap_listp;
    start = NEXT_BLKP(start); //skip the prologue block
    DPRINTF("\n\nBLOCK INFO STATS:\n");
    size = -1;
    while (GET_SIZE(HDRP(start)) != 0)
    {
        size = GET_SIZE(HDRP(start));
        //DPRINTF("Address: 0x%x\tSize: %d\n",start,GET_SIZE(HDRP(start)));

        //Minimum size check
        if (size < (DSIZE+OVERHEAD) || ALIGN(size) != size )
        {
            DPRINTF("\nERROR: Address: 0x%x contains a block that is less than the minimum size\n", start);
        }

        //DSIZE alignment check
        if (size % DSIZE)
        {
            DPRINTF("\nERROR: Address: 0x%x contains a block that is not aligned\n", start);
        }

        //Header footer mismatch check
        if ( GET_SIZE(HDRP(start)) != GET_SIZE(FTRP(start)) )
        {
            DPRINTF("\nERROR: Address: 0x%x has a header that does not match its footer\n", start);
        }

        //Coalesce check for contiguous free blocks (whether before or after)
        //This check should only be done before program termination otherwise extraneous errors will result when block splitting occurs
        //if ( (GET_ALLOC(HDRP(start)) == 0) && (GET_ALLOC(HDRP(NEXT_BLKP(start))) == 0) && start != heap_listp && NEXT_BLKP(start) != mem_heap_hi() )
        //{
        //	DPRINTF("\nERROR: Address: 0x%x has an adjacent block 0x%x that are not coalesced\n", start, NEXT_BLKP(start));
        //}

        start = NEXT_BLKP(start);
    }

#endif

    return 1;
}
