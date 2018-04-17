#include <stdlib.h>
#include <limits.h>

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

typedef struct _group{
    struct _group* parent;
    size_t rank;
    struct _group** children;
    size_t value;
} group;

typedef struct _RRHeap{
    // pointer to the groups with the lowest value
    group* smallest_value;
    // log of max_size, used to get group idx from value
    size_t log_n;
    // function used to compare 2 groups
    int (*cmp)(const void*, const void*);
    // size of each element
    size_t key_size;

    size_t max_size;
    // root of the tree. Doesn't hold any elements
    group root;
    // vector with data of the heap
    group* index_to_group;
    // vector with pointers to active nodes
    group** A;
    // vector with actual data
    void * key;
    // information on whether id is in heap or not
    // used as a bitset
    char* in;
} RRHeap;

RRHeap* RRHeapMake(size_t type_size, void* data, size_t max_size,
                   int (*cmp_func)(const void*, const void*));

void RRHeapFree(RRHeap* h);
void RRHeapInsert(RRHeap* h, size_t id);
void RRHeapUpdate(RRHeap* h, size_t id);
void RRHeapPop(RRHeap* h);
size_t RRHeapTop(RRHeap *h);
int RRHeapTopEmpty(RRHeap *h);

//other
size_t log_base_2(size_t n);
