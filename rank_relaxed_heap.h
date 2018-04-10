#include <stdlib.h>

enum group_key_kind { smallest_key, stored_key, largest_key };

typedef struct _group{
    struct _group* parent;
    size_t rank;
    struct _group** children;
    size_t value;
    enum group_key_kind kind;
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
    // TODO: make this a bitset maybe
    int* in;
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
