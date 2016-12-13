#include<unistd.h>
#include<stdbool.h>


typedef struct _header{
    struct _header *next;
    size_t size;
    bool isfree;
    int magic;
} header_t;

header_t *blocklist = NULL;
header_t *tail = NULL;


void *get_next_free(header_t *blocklist, size_t size, header_t *prev){
    prev = NULL;
    while(blocklist && (blocklist->size < size)){
        prev = blocklist;
        blocklist = blocklist->next;

    }
    return blocklist;
}

void *m_malloc(size_t size){

    if ( NULL == blocklist ){
        void *n_ptr = sbrk(size + sizeof(header_t));
        if(n_ptr == (void *) -1) 
            return NULL;
        header_t *blocklist = (header_t *) n_ptr;
        blocklist->size = size;
        blocklist->isfree = false;
        blocklist->next = NULL;
        blocklist->magic = 0x12345678;

        tail = blocklist;

        return blocklist + 1;
    }

    header_t prev_free;
    void *n_free = get_next_free(blocklist, size, &prev_free);
    header_t *block = NULL;

    if( NULL == n_free ){
        void *n_ptr = sbrk(size + sizeof(header_t));
        if(n_ptr == (void *) -1) 
            return NULL;

        block = (header_t *) n_ptr;
        block->size = size;
        block->next = blocklist;
        block->isfree = false;
        block->magic = 0x12345678;
        tail = block;
        blocklist = block;
        return block + 1;
    }

    //split the free block 
    int oldsize = 0;
    block = (header_t *) n_free;
    block->isfree = false;
    block->magic = 0x77777777;
    oldsize = block->size;
    block->size = size;

    header_t *nblock = NULL;
    
    nblock = (char *) block + block->size + sizeof( header_t );
    nblock->size = oldsize - block->size - sizeof(header_t);
    nblock->isfree = true;
    block->magic = 0x88888888;
    //let previous free block point to the newly splitted free block
    prev_free.next = nblock;
    //let the newly splitted free block point to the next block of the original
    //block
    nblock->next = block->next;

    return block + 1;
}


//insert into two neighboring free block
int free(void *ptr){
    if(ptr == NULL || blocklist == NULL) return -1;

    header_t *hptr = (header_t *) ptr;
    hptr->isfree = 1;
    hptr->size = 0;
    hptr->magic = 0x55555555;


    header_t *cur = blocklist;
    header_t *prev = NULL;

    while(cur && cur < hptr){
        prev = cur;
        cur = cur->next;
    }

    prev->next = hptr;
    hptr->next = prev->next;


    return 0;
}
