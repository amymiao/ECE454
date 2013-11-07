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

typedef struct free_block {
    struct free_block* next;
    struct free_block* prev;
} free_block;

free_block* free_list;

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(OVERHEAD, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(OVERHEAD, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += DSIZE;

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
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
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
    //return coalesce(bp);
    //FIXME
    return bp;  //cannot coalesce for now - we will break our free list
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    // Traverse the free list and find the first fitting block
    // If there is excess space in the free block we found, we
    // place the remainder back into the free list as a smaller free block
    if (free_list == NULL) {
        // No free blocks in the free list
        return NULL;
    }
    free_block* temp = free_list;
    do {
        int size = GET_SIZE(HDRP(temp));
        if (size >= asize && size < (asize + 2*DSIZE)) {
            // We found a block that can fit, but cannot be split into an excess free block
            // This is easy to resolve, simply remove it from the free list
            if (temp != temp->next) {
                temp->prev->next = temp->next; // Make previous free block point to next free block
                temp->next->prev = temp->prev; // Make next free block point to previous free block
                if (free_list == temp) {
                    // If we are removing the head pointer, we must set a new head pointer
                    free_list = temp->next;
                }
            } else {
                free_list = NULL;
            }
            return (void*)temp;
        } else if (size >= (asize + 2*DSIZE)) {
            // The block found has excess size and we can split the excess
            // into a free block
            // Example: Requested size = 4
            //          Found free block size = 10
            // [H1][ ][ ][ ][ ][ ][ ][ ][ ][F1]
            // newSize = 6
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
            printf("Doing free block splitting");
            mm_check();
            return userPtr;
        } 
        temp = temp->next;
    } while (temp != free_list);
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
    if(bp == NULL){
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));

    if (GET_ALLOC(HDRP(bp)) == 0) {
        // Block was already freed
        return;
    }
    
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    DPRINTF("RECEIVED FREE (0x%x), size=%d\n",bp,size);

    free_block* temp = (free_block*)bp;

    if (free_list == NULL) {
        // The free list is empty, this will be the first free block. Set head to point to it.
        free_list = temp;
        free_list->next = temp;
        free_list->prev = temp;
    } else {
        // Insert the newly freed block into the start of the free list
        // It will become the new head
        temp->next = free_list;
        temp->prev = free_list->prev;
        temp->prev->next = temp;
        temp->next->prev = temp;
    }

    
    mm_check();

    // FIXME: Uncomment
    //coalesce(bp);
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

    /* Search the free list for a fit */
    
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        DPRINTF("find fit - SERVICED MALLOC (0x%x), size=%d\n",bp,asize);
        mm_check();
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    DPRINTF("extendsize - SERVICED MALLOC (0x%x), size=%d\n",bp,asize);
    mm_check();
    
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    
    // If size == 0 then this is just free, and we return NULL. 
    if (size == 0){
        mm_free(ptr);
        return NULL;
    }

    /// If old ptr is NULL, then this is just malloc. 
    if (ptr == NULL)
        return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    // Copy the old data. 
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;

}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void)
{
    #ifdef DEBUG
    void* start = heap_listp;

    DPRINTF("\n\nHEAP STATS:\n");
    while (GET_SIZE(HDRP(start)) != 0) {
        DPRINTF("Address: 0x%x\tSize: %d\tAllocated: %d\n",start,GET_SIZE(HDRP(start)),GET_ALLOC(HDRP(start)));
        start = NEXT_BLKP(start); 
    }
    DPRINTF("\n");


    //Free list consistency check - check to see if all blocks in the free list are indeed free
    free_block* traverse = free_list;
    int size = -1;
    int free_status = -1;
    DPRINTF("\n\nFREE LIST STATS:\n");
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
            traverse = traverse->next;
        }
        while (traverse != free_list);  //termination condition of a circular list
    }
    DPRINTF("\n");
    #endif

    return 1;
}
