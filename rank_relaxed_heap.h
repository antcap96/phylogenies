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
    group* smallest_value;
    size_t log_n;
    int (*cmp)(const void*, const void*);
    //size_t map;
    size_t key_size;
    //size_t size;
    size_t max_size;
    group root;
    group* index_to_group;
    group** A;
    size_t A_size;
    void * key;
    // information on whether id is in heap or not
    int* in;
} RRHeap;

RRHeap* RRHeapMake(size_t type_size, void* data, size_t max_size,
                   int (*cmp_func)(const void*, const void*));

void RRHeapFree(RRHeap* h);
void RRHeapInsert(RRHeap* h, size_t id);
void RRHeapUpdate(RRHeap* h, size_t id);
int RRHeapGroupCompare(RRHeap* h, group* x, group* y);
//RRHeapRemoveMin()
//RRHeapDecreaseKey()
void RRHeapPromote(RRHeap* h, group* a);
void RRHeapPairTransform(RRHeap* h,group* a);
void RRHeapGoodSiblingTransform(RRHeap* h,group* a, group* s);
void RRHeapActiveSiblingTransform(RRHeap* h,group* a, group* s);
void RRHeapFindSmallest(RRHeap* h);
group* RRHeapCombine(RRHeap* h, group* a1, group* a2);
void RRHeapClean(RRHeap*h, group* q);
void RRHeapPop(RRHeap* h);
size_t RRHeapTop(RRHeap *h);
int RRHeapTopEmpty(RRHeap *h);

//other
size_t log_base_2(size_t n);

size_t build_tree(RRHeap* h, group* root, size_t idx,
                  size_t r, size_t max_rank);