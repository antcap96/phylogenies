#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

typedef struct _binheap {
    size_t* map; // from key get location in q
    size_t* heap; // order queue (heap)
    void* key; //here we have the data not orderd
    size_t max_size;
    size_t size;
    size_t key_size;
    int (*cmp)(const void*, const void*);
} binheap;

void binheap_siftdown(binheap* h, size_t i);
void binheap_siftup(binheap* h, size_t i);

binheap* BINHEAP_INIT(size_t type_size, void* data, size_t max_size,
                      int (*cmp_func)(const void*, const void*));
void BINHEAP_MAKE(binheap* h, size_t size);

#define BINHEAP_UPDATE BINHEAP_PUSH
void BINHEAP_PUSH(binheap* h, size_t id);
void BINHEAP_DEL(binheap* h, size_t id);

void BINHEAP_POP(binheap* h);
void BINHEAP_FREE(binheap* h);
int BINHEAP_EMPTY(binheap* h);
size_t BINHEAP_TOP(binheap* h);
