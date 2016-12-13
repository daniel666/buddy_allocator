# buddy_allocator
This is a buddy allocator implemented in C

##memalloc.c
This is the file that implements malloc function by brutal force. Each
allocated block freatures a preamble meta information that describes the size
of the block and a pointer that points to the next block. The allocation scheme
is simply to iterate the free list to find the first block with a size no less
than that required. Freeing a block is simply to add it its two neighboring
free block by address.

## mem_buddy_alloc.c/h
This is the real buddy allocator. In contrast to the naive memalloc, the
following things are achieved. 

+ blocks of same size are chained by pointer and stored in a slot indexed by
level value. Level 0 maintains the largest sized block, while the
<em>MAX_LEVEL</em>
macro defines the maximal available level which features block of size
<em>BASE_SIZE</em>. Growing from level 0, block size at each level reduces by half.

+ the next pointer is stored in the free block (NOT as a preamble), which occupies sizof(void *)
bytes. If on a 64 bit system, the pointer is 8 bytes, and the first 8 bytes of
a block stores a pointer to the next available free block of same size. 
