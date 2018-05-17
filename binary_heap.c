#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "binary_heap.h"


//#define LEFT(x)    ((((x) + 1) << 1) - 1)
//#define RIGHT(x)   (((x) + 1) << 1)
//#define PARENT(x)  ((((x) + 1) >> 1) - 1)

size_t LEFT(size_t x) {
    return (x + 1) * 2 - 1;
}

size_t RIGHT(size_t x) {
    return (x + 1) * 2;
}

size_t PARENT(size_t x) {
    if (x != 0)
        return (x - 1) / 2;
    else
        return SIZE_MAX;
}

//////////
binheap* BINHEAP_INIT(size_t type_size, void* data, size_t max_size,
                      int (*cmp_func)(const void*, const void*)) {
    /* initialiaze the heap
     * 
     * get n space for heap, queue and select key
     * set max_size
     * set cmp_func
     */

    binheap* h = (binheap*) malloc(sizeof(binheap));
    h->map  = (size_t*) malloc(sizeof(size_t) * max_size);
    h->heap = (size_t*) malloc(sizeof(size_t) * max_size);
    
    memset(h->map , 0xFF, max_size * sizeof(size_t));
    memset(h->heap, 0xFF, max_size * sizeof(size_t));

    h->key  = data;
    h->key_size = type_size;
    h->max_size = max_size;
    h->size = 0;
    h->cmp = cmp_func;

    return h;
}

void BINHEAP_MAKE(binheap* h, size_t size) {
    /* set size for heap
     *
     * organize the data structure
     * 
     */
    int i;

    h->size = size;
    
    for (i = 0; i < (int) size; i++)
        h->map[i] = h->heap[i] = i;
    
    // nodes without children are ignored
    // if size is 0 this is a big problem, as parent(0) = MAX_SIZE
    for (i = PARENT(size); i >= 0; i--)
        binheap_siftdown(h, i);
}

void binheap_exchange(binheap *h, size_t i, size_t j) {
    /* exchange 2 keys
     * 
     * first change the map to the new locations
     * then change the heap
     */

    h->map[h->heap[i]] = j;
    h->map[h->heap[j]] = i;

    size_t temp = h->heap[i];
    h->heap[i] = h->heap[j];
    h->heap[j] = temp;
}

void binheap_siftdown(binheap* h, size_t i) {
    /* check if node i needs to be swaped in q
     * 
     * get left and right children
     * smallest = i
     * if left < size and left < smallest
     *     smallest = left
     * if right < size and right < smallest
     *     smallest = right
     * if smallest != i
     *     swap and call again for smallest
     */

    size_t left = LEFT(i), right = RIGHT(i), smallest = i;

    if ((left < h->size) && 
        (h->cmp(h->key + h->key_size * h->heap[left], 
                h->key + h->key_size * h->heap[smallest]) < 0)) {
        smallest = left;
    }

    if ((right < h->size) && 
        (h->cmp(h->key + h->key_size * h->heap[right], 
                h->key + h->key_size * h->heap[smallest]) < 0)) {
        smallest = right;
    }

    if (smallest != i){
        // exchange changes location in heap but not the indexes
        // therefor, smallest was the index of the smallest before the func
        // and after it is location of the node that might be bad
        binheap_exchange(h, i, smallest);
        binheap_siftdown(h, smallest);
    }

}

void binheap_siftup(binheap* h, size_t i) {
    /* 
     * 
     * if parent does not exist or child is larger than parent swap them
     * be carefull with parent of 0
     * call again on parent
     */
    
    size_t p = PARENT(i);

    if ((p != SIZE_MAX) && 
        (h->cmp(h->key + h->key_size * h->heap[i], 
                h->key + h->key_size * h->heap[p]) < 0)) {
        binheap_exchange(h, p, i);
        binheap_siftup(h, p);
    }
}


void BINHEAP_PUSH(binheap* h, size_t id) { // = BINHEAP_UPDATE
    /* add element to heap
     * 
     * if it is not in queue, add to queue (check map)
     * set map, add element to end, increase size
     * 
     * fix up and fix down
     */

    if (h->map[id] == SIZE_MAX) {
        //id is not in heap
        h->map[id] = h->size;
        h->heap[h->size] = id;
        h->size++;
    }

    binheap_siftup(h, h->map[id]);
    binheap_siftdown(h, h->map[id]);
}

void BINHEAP_DEL(binheap* h, size_t id) {
    /* delete element of the heap
     * 
     * remove element (set to NULL)
     * fix up, fix down
     */

    size_t aux = h->map[id];
    h->size--;
    if (aux != h->size) {
        binheap_exchange(h, h->size, aux);
        binheap_siftdown(h, aux);
        binheap_siftup(h, aux);
    }
    h->map[id] = SIZE_MAX;
    h->heap[h->size] = SIZE_MAX;
    // else the element deleted was already in the bottom

}

void BINHEAP_POP(binheap* h) {
    /* Remove top element
     * 
     * same as BINHEAP_DEL(h, h->heap[0])
     */

    BINHEAP_DEL(h, h->heap[0]);
}

void BINHEAP_FREE(binheap* h) {
    free(h->map);
    free(h->heap);
    free(h);
}

int BINHEAP_EMPTY(binheap* h){
    return !(h->size);
}

size_t BINHEAP_TOP(binheap* h){
    return h->heap[0];
}
    