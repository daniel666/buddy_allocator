#ifndef _MEM_BUDDY_ALLOC_H
#define _MEM_BUDDY_ALLOC_H

#include<unistd.h>

//base size of allocation, note this is at least the size of pointer, because
//next pointer is stored in free block itself
#define BASE_SIZE 8

//max level of the split, note the total level is this value plust 1
#define MAX_LEVEL 3

typedef enum _state{
    UNINITIALIZED,
    FREE,
    USED,
    SPLIT,
} state_t; 

typedef struct _free_block{
    struct _free_block *next;
    void *addr;
} free_block_t;

void visualize();
void buddy_alloc_init();
void *buddy_malloc(size_t size);
int buddy_free(void *ptr);


#endif
