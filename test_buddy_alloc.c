#include<stdio.h>
#include "mem_buddy_alloc.h"

int main(){
    buddy_alloc_init();
    void *addr1 = buddy_malloc(8);
    visualize();
    void *addr2 = buddy_malloc(8);
    visualize();
    void *addr3 = buddy_malloc(8);
    visualize();
    buddy_free(addr1);
    visualize();
    buddy_free(addr2);
    visualize();
    buddy_free(addr3);
    visualize();
}
