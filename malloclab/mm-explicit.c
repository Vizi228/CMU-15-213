/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

// Methods for working with heap
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t size);
static void place(void *bp, size_t size);

static void free_push(void *bp);
static void free_swap(void *bp);
static void free_clear(void *bp);

static void check_heap();

// single word size, double word size and chunk size for heap extending
#define WSIZE 4
#define DSIZE 8
#define MIN_BLOCKSIZE (DSIZE * 2)
#define CHUNKSIZE (1<<12)
/* single word (4) or double word (8) alignment */
#define ALIGNMENT DSIZE

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define MAX(x, y) ((x) > (y)? (x) : (y))

// pack size of block and allocation flag to one word
#define PACK(size, alloc) ((size) | (alloc))

// get and put value from/to pointer
#define GET(p) (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

// get size of block and allocation flag from word
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// calculate address of header and edge of block
#define HDRP(p) ((char *)(p) - WSIZE)
#define FTRP(p) ((char *)(p) + GET_SIZE(HDRP(p)) - DSIZE)

// calculate address of next and previous block
#define NEXT_BLKP(p) ((char *)(p) + GET_SIZE((char *)(p) - WSIZE))
#define PREV_BLKP(p) ((char *)(p) - GET_SIZE((char *)(p) - DSIZE))

// get and put pointer to next and prev pointer in free blocks
#define FREE_PREV(p) ((char *)(p))
#define FREE_NEXT(p) ((char *)(p) + WSIZE)
#define GET_PREV(p) (GET(FREE_PREV(p)))
#define GET_NEXT(p) (GET(FREE_NEXT(p)))
#define PUT_PREV(p, val) (PUT(FREE_PREV(p), val))
#define PUT_NEXT(p, val) (PUT(FREE_NEXT(p), val))

static char *heap_list;
static char *head;
static char *tail;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char *brk;
    if ((brk = mem_sbrk((MIN_BLOCKSIZE * 2) + DSIZE)) == (void *)-1) {
        return -1;
    }

    // head
    PUT(brk + (0*WSIZE), 0);
    PUT(brk + (1*WSIZE), PACK(MIN_BLOCKSIZE, 1));
    PUT(brk + (2*WSIZE), NULL);
    PUT(brk + (3*WSIZE), NULL);
    PUT(brk + (4*WSIZE), PACK(MIN_BLOCKSIZE, 1));

    // tail
    PUT(brk + (5*WSIZE), PACK(MIN_BLOCKSIZE, 1));
    PUT(brk + (6*WSIZE), NULL);
    PUT(brk + (7*WSIZE), NULL);
    PUT(brk + (8*WSIZE), PACK(MIN_BLOCKSIZE, 1));

    PUT(brk + (9*WSIZE), PACK(0, 1));
    heap_list = brk + (2*WSIZE);
    head = brk + (2*WSIZE);
    tail = brk + (6*WSIZE);
    PUT_PREV(tail, head);
    PUT_NEXT(head, tail);

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    return 0;
}

/*
 * extend_heap - Reserve more space in heap.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    bp = coalesce(bp);
    free_push(bp);
    return bp;
}

/*
 * free_push - Push block to head.
 */
static void free_push(void *bp) {
    PUT_PREV(bp, head);
    PUT_NEXT(bp, GET_NEXT(head));
    PUT_PREV(GET_NEXT(bp), bp);
    PUT_NEXT(head, bp);
}

/*
 * free_swap - Swap pointers in the block.
 */
static void free_swap(void *bp) {
    PUT_NEXT(GET_PREV(bp), GET_NEXT(bp));
    PUT_PREV(GET_NEXT(bp), GET_PREV(bp));
}

/*
 * free_clear - Remove pointers.
 */
static void free_clear(void *bp) {
    PUT_NEXT(bp, NULL);
    PUT_PREV(bp, NULL);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    char *bp;
    size_t asize = ALIGN(MIN_BLOCKSIZE + size);

    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    size_t extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * find_fit - Find block that fit to size.
 */
static void *find_fit(size_t size) {
    void *bp;
    for(bp = GET(FREE_NEXT(head)); bp != NULL; bp = GET(FREE_NEXT(bp))) {
        if (size <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    return NULL;
}

/*
 * place - Place data to block and slice it if can.
 */
static void place(void *bp, size_t asize) {
    size_t block_size = GET_SIZE(HDRP(bp));
    if ((block_size - asize) > MIN_BLOCKSIZE) {
        void *prev = GET(FREE_PREV(bp));
        void *next = GET(FREE_NEXT(bp));
        
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        free_clear(bp);

        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(block_size - asize, 0));
        PUT(FREE_PREV(bp), prev);
        PUT(FREE_NEXT(bp), next);
        PUT(FTRP(bp), PACK(block_size - asize, 0));

        PUT(FREE_NEXT(prev), bp);
        // it`s possible that we don`t have next element in the head
        if (next != NULL) {
            PUT(FREE_PREV(next), bp);
        }
    } else {
        free_swap(bp);
        PUT(HDRP(bp), PACK(block_size, 1));
        PUT(FTRP(bp), PACK(block_size, 1));
        free_clear(bp);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    ptr = coalesce(ptr);
    free_push(ptr);
}

/*
 * coalesce - Coaliscing for blocks that can be coalesced.
 */
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        return bp;
    } else if (!prev_alloc && next_alloc) {
        void *predecessor = PREV_BLKP(bp);
        free_swap(predecessor);

        size += GET_SIZE(HDRP(predecessor));
        PUT(HDRP(predecessor), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = predecessor;
    } else if (prev_alloc && !next_alloc) {
        void *successor = NEXT_BLKP(bp);
        free_swap(successor);

        size += GET_SIZE(HDRP(successor));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } else {
        void *predecessor = PREV_BLKP(bp);
        void *successor = NEXT_BLKP(bp);
        free_swap(predecessor);
        free_swap(successor);

        size += GET_SIZE(HDRP(predecessor)) + GET_SIZE(FTRP(successor));
        PUT(HDRP(predecessor), PACK(size, 0));
        PUT(FTRP(successor), PACK(size, 0));
        bp = predecessor;
    }

    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    if (ptr == NULL) {
        newptr = mm_malloc(size);
        mm_free(oldptr);
    }

    copySize = GET_SIZE(HDRP(ptr));

    if (copySize == size) {
        return ptr;
    }
    newptr = mm_malloc(size);

    if (copySize > size) {
        copySize = size;
    }
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);

    return newptr;
}

// Check consistency of the heap
static void check_heap() {
    unsigned int *bp;
    for(bp = GET(FREE_NEXT(head)); bp != NULL; bp = GET(FREE_NEXT(bp))) {
        if (bp != head && bp != tail) {
            if (GET_ALLOC(HDRP(bp))) {
                printf("Heap inconsistent!\nAllocated block(%x) in head!\n", bp);
                exit(1);
            }

            if (GET_NEXT(bp) == NULL || GET_PREV(bp) == NULL) {
                printf("Heap inconsistent!\nInvalid pointers in block(%x)!\n", bp);

            }
        }
    }
}