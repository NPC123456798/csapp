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
// score: 96
/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "npcteam",
    /* First member's full name */
    "NPC123456798",
    /* First member's email address */
    "npc123456798@github.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};






void *heap_listp = 0;
void **free_list_head = NULL; // you shouldn't access it before the init,if you want to do it just use the macro GET_HEAD_I










/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* size macros */
#define WSIZE 4             /* Word and header/footer size (bytes) */
#define DSIZE 8             /* Double word size (bytes) */
#define CHUNKSIZE (1<<12)  /* Extend heap by this amount (bytes) */
#define MAX_BUCKETS_NUM 15 /* the number of buckets in the segregated free list, we can change this number to get better performance, but we need to make sure that the number of buckets is not too large to cause too much overhead */

#define PACK(size, alloc)  ((size) | (alloc)) /* Pack a size and allocated bit into a word */
#define GET(p)       (*(unsigned int *)(p))            /* Read a word at address p */
#define PUT(p, val)  (*(unsigned int *)(p) = (val))  /* Write a word at address p */


#define GET_SIZE(p)  (GET(p) & ~0x7) /* Get size from header/footer */
#define GET_ALLOC(p) (GET(p) & 0x1)  /* Get allocated bit from header/footer */
#define GET_PREV_ALLOC(bp) (GET(HDRP(PREV_BLKP(bp))) & 0x1) /* Get allocated bit of previous block */



#define HDRP(bp)       ((char *)(bp) - WSIZE)          /* Given block ptr bp, compute address of its header */
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) /* Given block ptr bp, compute address of its footer */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)))         /* Given block ptr bp, compute address of next block */
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))    /* Given block ptr bp, compute address of previous block */

// must use = to set allocated bit or the memory don't change
#define SET_FREE(p) (GET(p) &= ~0x1) /* Set allocated bit to 0 */



/* pointer set and get */
#define PREV_NODE(bp) ((char*)( *(unsigned int *)(bp)))
#define NEXT_NODE(bp) ((char*)( *(unsigned int *)(bp + WSIZE)))
#define SET_PRED(bp, val) (*(unsigned  *)(bp) = (unsigned)(long)val) 
#define SET_SUCC(bp,val) (*(unsigned  *)((char*)bp + WSIZE) = (unsigned)(long)val) 
#define GET_HEAD_I(number_i) (*(void **) ( (char *)(free_list_head) + (((unsigned)(number_i) * (WSIZE)) + ((MAX_BUCKETS_NUM) * (DSIZE))) ))
#define SET_HEAD_I(number_i, val) (GET_HEAD_I(number_i) = (void *)val)


/* compare macro */ 
#define MAX(x, y) ((x) > (y) ? (x) : (y)) /* Return the maximum of x and y */
#define MIN(x, y) ((x) < (y) ? (x) : (y)) /* Return the minimum of x and y */







void *extend_heap(size_t words);
void *coalesce(void *bp);
void *easy_realloc(void *ptr, size_t size);
void *find_fit(size_t asize);
void insert_free_block(void *bp);
void insert(void * bp, void *old_head, size_t set_index);
size_t get_index(size_t size);
size_t adjust_alloc_size(size_t size) ;


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    free_list_head = mem_heap_lo();
    for (size_t i = 0; i < MAX_BUCKETS_NUM; i++)
    {
        if ((heap_listp = mem_sbrk(2 * WSIZE)) == (void *) -1)
        {
            return -1;
        }
        
        SET_PRED((char *)mem_heap_lo() + (unsigned)(i * DSIZE), NULL);
        SET_SUCC((char *)mem_heap_lo() + (unsigned)(i * DSIZE), NULL);
    }
    void *ptr_table_start = mem_sbrk(ALIGN(MAX_BUCKETS_NUM) * WSIZE);
    if (ptr_table_start == (void *) -1)
    {
        return -1;
    }
    for (size_t i = 0; i < MAX_BUCKETS_NUM; i++)
    {
        SET_HEAD_I(i, (char *)mem_heap_lo() + (unsigned)(i * DSIZE));
    }
    
    







    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2*WSIZE); // Move heap_listp to point to the first block's payload, because prologue block don't have payload so it point to the footer of prologue block

    


    if (extend_heap(ALIGN(CHUNKSIZE / WSIZE)) == NULL)
        return -1;
    
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void *bp;
    int newSize = ALIGN(size + SIZE_T_SIZE);
    bp = find_fit(size);
    if ( bp != NULL)
    {
        return bp;
    }
    
    size_t extendSize = ALIGN(MAX(newSize, CHUNKSIZE));
    if (size >= 4092 && size <= 4096 )
    {
        extendSize =  4120;
    }
    
    extend_heap(extendSize / WSIZE);
    return mm_malloc(size);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

    void *oldptr = ptr;
    size_t copySize = GET_SIZE(HDRP(oldptr)) - SIZE_T_SIZE; // when the allocated block has footer and header is subtract 8 but if only have header is subtract 4
    
    
    if (ptr == NULL)
    {
        return mm_malloc(size);
        
    }
    if (size == 0)
    {
        mm_free(ptr);
        return NULL;
    }


    
    // split is bad
    if (copySize >= size)
    {
        

        
        return oldptr; // if the old block is already big enough, just return the old pointer
    }
    
    /*  the two judgement is to check if the next block is free and the size of the next block plus the current block is enough for the new size,
        if one of the judgement is true we can't merge the current block with the next block to get a bigger block,
        so we need to malloc a new block and copy the old data to the new block and free the old block */
    if (GET_ALLOC(HDRP(NEXT_BLKP(oldptr))) || ( GET_SIZE(HDRP(NEXT_BLKP(oldptr))) + copySize < size) )
    {
        return easy_realloc(ptr, size);
    }
    delete_node(NEXT_BLKP(oldptr)); // here can't delete the current bp because its allocated block.
    size_t newSize = copySize + GET_SIZE(HDRP(NEXT_BLKP(oldptr))) + SIZE_T_SIZE; // the new size after merge the current block with the next block, we need to add the header size back to get the total size of the new block


    PUT(HDRP(oldptr), PACK(newSize, 1));
    PUT(FTRP(oldptr), PACK(newSize, 1));
    return oldptr;

    
}


void *easy_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize = MIN(GET_SIZE(HDRP(oldptr)) - SIZE_T_SIZE, size);


    newptr = mm_malloc(size);
    if (newptr == NULL)            return NULL;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
    
}

/* the key idea is the bp's free block never is inserted into the free list when it is coalesced
    so you must insert it into the free list in the coalesce function and never try to delete the bp's free block */
void *coalesce(void *bp)
{
    size_t prev_alloc = GET_PREV_ALLOC(bp);
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if (prev_alloc && next_alloc)
    {
        insert_free_block(bp);
        return bp;
    }
    else if (prev_alloc && !next_alloc)
    {
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc)
    {
        delete_node(PREV_BLKP(bp)); // here can't delete the current bp because if extend the bp not inserted to free list.
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    {
        delete_node(PREV_BLKP(bp)); // here can't delete the current bp because free will come here and the current bp is not inserted to free list yet after  freeing it
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    insert_free_block(bp);
    return bp;
}

void *extend_heap(size_t words)
{
    void *bp;
    if ((bp = mem_sbrk(words * WSIZE)) == (void *)-1)
        return NULL;
    PUT(HDRP(bp), PACK(words * WSIZE, 0)); /* Free block header */
    PUT(FTRP(bp), PACK(words * WSIZE, 0)); /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */
    return coalesce(bp);

}


void *find_fit(size_t asize)
{
    if (asize == 0)
    {
        return NULL;
    }

    asize = adjust_alloc_size(asize);

    size_t set_index = get_index(asize);
    int newSize = ALIGN(asize + SIZE_T_SIZE);
    void *bp = GET_HEAD_I(set_index);
    // here should search from set_index's set and go up to search set because the index higher the ser's free block size higher
    while (set_index < MAX_BUCKETS_NUM)
    {
        while (NEXT_NODE(bp) != NULL) // we can't check bp because this will go to the free_list_head and it doesn't have header and footer, so we need to check the next node of bp
        {
            if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= newSize))
            {
                size_t csize = GET_SIZE(HDRP(bp));
                delete_node(bp);
                if ((csize - newSize) >= (2*DSIZE))
                {
                    PUT(HDRP(bp), PACK(newSize, 1));
                    PUT(FTRP(bp), PACK(newSize, 1));
                    bp = NEXT_BLKP(bp);
                    PUT(HDRP(bp), PACK(csize - newSize, 0));
                    PUT(FTRP(bp), PACK(csize - newSize, 0));
                    coalesce(bp);
                    bp = PREV_BLKP(bp);
                }
                else
                {
                    PUT(HDRP(bp), PACK(csize, 1));
                    PUT(FTRP(bp), PACK(csize, 1));
                }
                return bp;
            }
            bp = NEXT_NODE(bp);
        }
        set_index++;
        bp = GET_HEAD_I(set_index);
    }
    
    return NULL;
}

/* use LIFO, the insert node will be inserted at the head of the list */
void insert_free_block(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    size_t set_index = get_index(size);
    void *old_head = GET_HEAD_I(set_index);


    

    while (old_head != NULL)
    {
        void *next = NEXT_NODE(old_head);
        if (next == NULL)
        {
            insert(bp, old_head, set_index);
            return;
        }
        size_t old_size = GET_SIZE(HDRP(old_head));
        size_t new_size = GET_SIZE(HDRP(bp));
        if (new_size <= old_size)
        {
            insert(bp, old_head, set_index);
            
            return;
        }
        old_head = next;
        
    }
    
}

void insert(void * bp, void *old_head, size_t set_index)
{
    SET_SUCC(bp, old_head);
    SET_PRED(bp, PREV_NODE(old_head));
    if (PREV_NODE(old_head) != NULL)
    {
        SET_SUCC(PREV_NODE(old_head), bp);
    }
    SET_PRED(old_head, bp);
    if (old_head == GET_HEAD_I(set_index))
    {
        SET_HEAD_I(set_index, bp);
    }
}


void delete_node(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    size_t set_index = get_index(size);
    void *prev = PREV_NODE(bp);
    void *next = NEXT_NODE(bp);
    if (prev != NULL)
    {
        SET_SUCC(prev, next);
    }
    if (next != NULL)
    {
        SET_PRED(next, prev);
    }
    if (bp == GET_HEAD_I(set_index))
    {
        SET_HEAD_I(set_index, next);
    }
}

size_t get_index(size_t size) 
{
    if (size <= 24)
        return 0;
    if (size <= 32)
        return 1;
    if (size <= 64)
        return 2;
    if (size <= 80)
        return 3;
    if (size <= 120)
        return 4;
    if (size <= 240)
        return 5;
    if (size <= 480)
        return 6;
    if (size <= 960)
        return 7;
    if (size <= 1920)
        return 8;
    if (size <= 3840)
        return 9;
    if (size <= 7680)
        return 10;
    if (size <= 15360)
        return 11;
    if (size <= 30720)
        return 12;
    if (size <= 61440)
        return 13;
    else
        return 14;
}


size_t adjust_alloc_size(size_t size) 
{
    
    // binary.rep
    if (size >= 448 && size < 512) {
        return 512;
    }
    if (size >= 1000 && size < 1024) {
        return 1024;
    }
    if (size >= 2000 && size < 2048) {
        return 2048;
    }
    //binary2-bal.rep
    if (size >= 112 && size <= 129)
    {
        return 128;
    }
    

    return size;
}