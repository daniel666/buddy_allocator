#include"mem_buddy_alloc.h"
#include<string.h>
#include<stdio.h>


static void *buddy[MAX_LEVEL + 1] = {NULL};
static void *base_addr = NULL;

/*0 uninitialized, 1 free, 2 used, 3 split*/
static state_t status[1 << (MAX_LEVEL+1)];

static inline char status2str(state_t s){
    switch(s){
        case UNINITIALIZED: return 'U'; break;
        case FREE: return 'F'; break;
        case USED: return 'U'; break;
        case SPLIT: return 'S'; break;
    }
}

static inline int get_blk_size_level(int level){
    return (1 << (MAX_LEVEL - level)) * BASE_SIZE;
}


static int get_level_status_ptr(void *ptr){
    int idx_bottom_level = (ptr - base_addr) / BASE_SIZE;
    return status[idx_bottom_level];
}

static int get_blk_idx_level(void *ptr, int level){
    int blksize = get_blk_size_level(level);    
    return (ptr-base_addr)/blksize;
}

static int get_blk_idx(void *ptr, int level){
    int blk_idx_level = get_blk_idx_level(ptr, level);
    return (1 << level) - 1 + blk_idx_level;
}

static void mark_status(void *ptr, int n, state_t s){
    int blk_idx = get_blk_idx(ptr, n);
    status[blk_idx] = s;
}

static int get_level_used_ptr(void *ptr){
    int level = MAX_LEVEL;
    while(level > 0){
        int blk_idx = get_blk_idx(ptr, level-1);
        if(status[blk_idx] == SPLIT)
            break;
        level--;
    }
    return level;
}

static int remove_list(void *ptr, int level){
    void *cur = buddy[level];
    if(cur == ptr){
        buddy[level] = *(void **)buddy[level];
        return 0;
    }
    while(cur && *(void **)cur != ptr){
        cur = *(void **)cur;
    }
    if(cur){
        *(void **)cur = *(void **) ptr;
        return 0;
    }
    return -1;
}


static int buddy_insert_parts(void *blk1, void *blk2, int level){
    //blk1 next point to blk2 
    *(void **) blk1 = blk2;
    *(void **) blk2 = buddy[level];
    buddy[level] = blk1;
    return 0;
}


static int buddy_split(int level){
    if(level < 0) return -1;
    int ret = 0;
    if(!buddy[level]){
        ret = buddy_split(level-1);
        if(ret)
            return ret;
    }
    void *split_blk = buddy[level];
    int  blk_size_level = get_blk_size_level(level);
    buddy[level] = *(void **) split_blk;
    void *part1 = split_blk;
    void *part2 = (char *)split_blk + blk_size_level/2;

    mark_status(split_blk, level, SPLIT);
    mark_status(part1, level+1, FREE);
    mark_status(part2, level+1, FREE);
    buddy_insert_parts(part1, part2, level+1);
    return ret;
}



static void visualize_level(int level){
    void *cur = buddy[level];
    int width = 1 << (MAX_LEVEL - level);

    printf("level %d: ", level);
    while(cur){
        int blk_idx = get_blk_idx(cur, level);
        printf("|addr:%p, blk_idx:%3d, status:%c| ---> ", cur, blk_idx, status2str(status[blk_idx]));
        cur = *(void **) cur;
    }
    printf("NULL\n");
}

static int get_level(int size){
    int level = MAX_LEVEL;
    int cur_size = BASE_SIZE;
    while(cur_size < size ){
        level--;
        cur_size <<= 1;
    }
    return level;
}

void buddy_alloc_init(){
    /*void *naddr = sbrk((1 << MAX_LEVEL) * BASE_SIZE + sizeof(free_block_t));*/
    void *naddr = sbrk((1 << MAX_LEVEL) * BASE_SIZE );
    if(naddr == (void *) -1)
        return ;
    *(void **)naddr = NULL;
    buddy[0] = naddr;
    base_addr = naddr;
    memset(status, 0, sizeof(status));
    status[0] = FREE;
    /*free_block_t *blk = (free_block_t *) naddr; */
    /*blk->next = NULL;*/
    /*blk->addr = blk + 1;*/
    /*pages[0] = blk;*/
}

/*0: 512*/
/*1: 256 | 256*/
/*2: 128 | 128 | 128 | 128*/
void *buddy_malloc(size_t size){
    unsigned int n = get_level(size);
    int ret = -1;
    if(n > MAX_LEVEL)
        return (void *) -1;
    size_t size_level_n =  (1 << (MAX_LEVEL - n)) * BASE_SIZE;
    unsigned int num_blk_level_n = 1 << n;

    if(!buddy[n]){
        ret = buddy_split(n-1);
        if( ret == -1 )
            return (void *) -1;
    }
    void *cur = buddy[n];
    buddy[n] = *(void **)buddy[n];
    mark_status(cur, n, USED);
    return cur;
}

int buddy_free(void *ptr){
    int level = get_level_used_ptr(ptr);
    if(level == 0){
        status[0] = FREE;
        buddy[0] = ptr;
        return 0;
    }
    int size_level = get_blk_size_level(level);
    int blk_idx = get_blk_idx(ptr, level);
    int buddy_blk_idx = 0 ;
    int parent_blk_idx = (blk_idx - 1) / 2;
    void *buddy_ptr = NULL;
    void *parent_ptr = NULL;
    if(blk_idx % 2 == 1){
        buddy_blk_idx = blk_idx + 1;
        buddy_ptr = (char *) ptr + size_level;
        parent_ptr = ptr;
    }
    else{
        buddy_blk_idx = blk_idx - 1;
        buddy_ptr = (char *) ptr - size_level;
        parent_ptr = buddy_ptr;
    }

    status[blk_idx] = FREE;
    if(status[buddy_blk_idx] != FREE){
        *(void **) ptr = buddy[level];
        buddy[level] = ptr;
        return 0;
    }
    remove_list(buddy_ptr, level);
    status[blk_idx] = UNINITIALIZED;
    status[buddy_blk_idx] = UNINITIALIZED;
    status[parent_blk_idx] = USED;

    return buddy_free(parent_ptr);
}

void visualize(){
    printf("---------------------VISUALIZE--------------------\n");
    for(int i = 0; i <= MAX_LEVEL; i++){
        visualize_level(i);
    }
}
