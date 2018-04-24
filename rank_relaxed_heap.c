#include "rank_relaxed_heap.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>

static size_t build_tree(RRHeap* h, group* root, size_t idx,
                         size_t r, size_t max_rank);
static void RRHeapGoodSiblingTransform(RRHeap* h,group* a, group* s);
static void RRHeapActiveSiblingTransform(RRHeap* h,group* a, group* s);
static void RRHeapPairTransform(RRHeap* h,group* a);
static void RRHeapFindSmallest(RRHeap* h);
static group* RRHeapCombine(RRHeap* h, group* a1, group* a2);
static int RRHeapGroupCompare(RRHeap* h, group* x, group* y);
static void RRHeapClean(RRHeap*h, group* q);
static void RRHeapPromote(RRHeap* h, group* a);

// gives position of largest bit = 1
size_t log_base_2(size_t n) {
    assert(n != 0);
    size_t leading_zeroes = 0;
    while(1) {
        size_t next = n << 1;
        if (n == (next >> 1)) {
            ++leading_zeroes;
            n = next;
        }
        else 
            break;
    }
    return sizeof(size_t) * CHAR_BIT - leading_zeroes - 1;
}

// initialize the heap
// elements must be added after
RRHeap* RRHeapMake(size_t type_size, void* data, size_t max_size,
                   int (*cmp_func)(const void*, const void*)) {
    
    assert(max_size != 0);

    RRHeap* h = (RRHeap*) malloc(sizeof(RRHeap));

    h->key_size = type_size;
    h->key = data;
    h->max_size = max_size;
    h->smallest_value = NULL;
    h->cmp = cmp_func;
    h->log_n = log_base_2(max_size);
    if (h->log_n == 0)
        h->log_n = 1;
    size_t g = max_size / h->log_n;
    if (max_size % h->log_n > 0)
        ++g;
    size_t log_g = log_base_2(g);
    size_t r = log_g;

    h->in = (char*) malloc(sizeof(char) * BITNSLOTS(max_size));
    memset(h->in, 0, BITNSLOTS(max_size));

    // this is where I allocate the space for the groups
    h->index_to_group = (group*) malloc( sizeof(group) * g);

    for(size_t i = 0; i < g; i++) {
        // value of SIZE_MAX means it is empty
        h->index_to_group[i].value = SIZE_MAX;
    }


    h->root.rank = r+1;
    h->root.parent = NULL;
    h->A = (group**) malloc(sizeof(group*) * (h->root.rank));
    memset(h->A, 0, sizeof(group*) * (h->root.rank));

    // yes I do: max_rank * (num_groups + root)
    h->root.children = (group**) malloc(sizeof(group*) * h->root.rank * (g+1));
    
    // set the B-trees to NULL, fill only the necessary ones
    for (size_t i = 0; i < r+1; ++i)
        h->root.children[i] = NULL;

    size_t idx = 0;
    while (idx < g) {
        h->root.children[r] = &h->index_to_group[idx];
        idx = build_tree(h, &h->root, idx, r, log_g + 1);
        if (idx != g)
            r = (log_base_2(g-idx));
    }

    return h;
}


// internal function
// max_rank is always log_g + 1
// sets children and parent for the tree
static size_t build_tree(RRHeap* h,  group* parent, size_t idx,
                         size_t r,  size_t max_rank) {
    
    group* this_group = &h->index_to_group[idx];
    this_group->parent = parent;
    idx++;

    this_group->children = h->root.children + (idx * max_rank);
    this_group->rank = r;

    for (size_t i = 0; i < r; ++i) {
        this_group->children[i] = &h->index_to_group[idx];
        idx = build_tree(h, this_group, idx, i, max_rank);
    }

    return idx;
}

void RRHeapFree(RRHeap* h){
    free(h->in);
    free(h->A);
    free(h->index_to_group);
    free(h->root.children);
    free(h);
}

void RRHeapInsert(RRHeap* h, size_t id){
    BITSET(h->in, id);
    RRHeapUpdate(h, id);
}

void RRHeapUpdate(RRHeap* h, size_t id) {
    group* a = &h->index_to_group[id / h->log_n];
    if (a->value == SIZE_MAX
          || a->value == id
          || h->cmp(h->key + h->key_size * id, 
                    h->key + h->key_size * a->value) < 0) {
        if (a != h->smallest_value)
            h->smallest_value = NULL;
        a->value = id;
        RRHeapPromote(h, a);
    }
}

static void RRHeapPromote(RRHeap* h, group* a) {
    size_t r = a->rank;
    group* p = a->parent;
    
    if (!(RRHeapGroupCompare(h, a, p) < 0)){
        if(h->A[r] == a)
            h->A[r] = NULL;
    }
    else {
        // If a is the last child of p
        if (r == p->rank - 1) {
            if (h->A[r] == NULL)
                h->A[r] = a;
            else if (h->A[r] != a)
                RRHeapPairTransform(h, a);
        } else {
            // s is the rank + 1 sibling
            group* s = p->children[r + 1];
            
            if (h->A[r + 1] == s)
                RRHeapActiveSiblingTransform(h, a, s);
            else
                RRHeapGoodSiblingTransform(h, a, s);
        }
    }
}

// compare two groups, output 1 if x > y, -1 if x < y, 0 if equal
static int RRHeapGroupCompare(RRHeap* h, group* x, group* y) {

    if (y == &h->root)
        return 1;
    if (x == &h->root)
        return -1;
    if (y->value == x->value)
        return 0;
    if (y->value == SIZE_MAX)
        return -1;
    if (x->value == SIZE_MAX)
        return 1;
    return h->cmp(h->key + h->key_size * x->value,
                  h->key + h->key_size * y->value);
}

// internal funtion that sets h->smallest_value if not set
// looks though children of root and active nodes
// O(log_g)
static void RRHeapFindSmallest(RRHeap* h) {
    group** roots = h->root.children;

    if (h->smallest_value == NULL) {
        size_t i;
        for (i = 0; i < h->root.rank; ++i) {
            if (roots[i] != NULL &&
                  (h->smallest_value == NULL 
                  || RRHeapGroupCompare(h, roots[i], h->smallest_value) < 0)) {
                h->smallest_value = roots[i];
            }
        }
        for (i = 0; i < h->root.rank; ++i) {
            if (h->A[i] != NULL && (h->smallest_value == NULL 
                  || RRHeapGroupCompare(h, h->A[i], h->smallest_value) < 0)) {
                h->smallest_value = h->A[i];
            }
        }
    }
}

static void swap_groups(group** x, group** y) {
    group* tmp = *x;
    *x = *y;
    *y = tmp;
}

static void RRHeapPairTransform(RRHeap* h, group* a) {
    size_t r = a->rank;

    group* pa = a->parent;
    group* ga = pa->parent;

    group* b = h->A[r];

    h->A[r] = NULL;

    group* pb = b->parent;
    group* gb = pb->parent;

    --pb->rank;
    --pa->rank;
    
    // Note: a, pa, b, pb all have rank r
    if (RRHeapGroupCompare(h, pb, pa) < 0) {
        swap_groups(&a, &b);
        swap_groups(&pa, &pb);
        swap_groups(&ga, &gb);
    }

    // now k(pa) <= k(pb)
    // make pb the rank r child of pa
    pa->children[pa->rank++] = pb;
    pb->parent = pa;

    // Combine a, b into a rank r+1 group c
    group* c = RRHeapCombine(h, a, b);

    // make c the rank r+1 child of gb
    gb->children[r+1] = c;
    c->parent = gb;

    if (h->A[r+1] == pb)
        if (RRHeapGroupCompare(h, c, gb) < 0)
            h->A[r+1] = c;
        else
            h->A[r+1] = NULL;
    else
        RRHeapPromote(h, c);
}

static void RRHeapActiveSiblingTransform(RRHeap* h, group* a, group* s) {
    group* p = a->parent;
    group* g = p->parent;

    --p->rank; // remove s from p
    --p->rank; // remove a form p

    size_t r = a->rank;
    h->A[r+1] = NULL;
    a = RRHeapCombine(h, p, a);
    group* c = RRHeapCombine(h, a, s);

    // make c the rank right most child of g
    g->children[r+2] = c;
    c->parent = g;

    if (h->A[r+2] == p)
        h->A[r+2] = NULL;
    
    RRHeapPromote(h, c);
}


static void RRHeapGoodSiblingTransform(RRHeap* h, group* a, group* s) {
    size_t r = a->rank;
    group* c = s->children[s->rank-1];
    if (h->A[r] == c) {
        h->A[r] = NULL;
        group* p = a->parent;

        // remove c from s
        --s->rank;

        // make s the rank r child of p
        s->parent = p;
        p->children[r] = s;

        // combine a, c and let the result be the right most child of p
        group* x = RRHeapCombine(h, a, c);
        x->parent = p;
        p->children[r+1] = x;

        // x will always be active, as both a and c are smaller than p
        RRHeapPromote(h, x);
    } else {
        // Clean operation
        group* p = a->parent;
        s->children[r] = a;
        a->parent = s;
        p->children[r] = c;
        c->parent = p;

        RRHeapPromote(h, a);
    }
}


static group* RRHeapCombine(RRHeap* h, group* a, group* b) {
    if (RRHeapGroupCompare(h, b, a) < 0)
        swap_groups(&a, &b);
    a->children[a->rank++] = b;
    b->parent = a;
    RRHeapClean(h, a);
    return a;
}


static void RRHeapClean(RRHeap*h, group* q) {
    if (q->rank < 2)
        return;
    
    // q is parent of x and grandparent of y
    //  q
    //  \ \                               .
    //   x qp
    //      \                             .
    //       y

    group* qp = q->children[q->rank-1];
    size_t s = q->rank - 2;
    group* x = q->children[s];
    group* y = qp->children[s];

    // If x is active, swap x and y
    if (h->A[s] == x) {
        q->children[s] = y;
        y->parent = q;
        qp->children[s] = x;
        x->parent = qp;
    }
}

size_t RRHeapTop(RRHeap *h) {
    RRHeapFindSmallest(h);
    return h->smallest_value->value;
}

int RRHeapTopEmpty(RRHeap *h) {
    RRHeapFindSmallest(h);
    return (h->smallest_value->value == SIZE_MAX);
}

void RRHeapPop(RRHeap* h) {
    RRHeapFindSmallest(h);
    group* x = h->smallest_value;
    h->smallest_value = NULL;

    size_t r = x->rank;
    group* p = x->parent;
    
    // assert heap is not empty
    assert(x->value != SIZE_MAX);

    // find x's group
    size_t start = x->value - (x->value % h->log_n);
    size_t end = start + h->log_n;
    if (end > h->max_size)
        end = h->max_size;

    // remove the smallest value and look for the new smallest
    BITCLEAR(h->in, x->value);
    x->value = SIZE_MAX;
    for (size_t i = start; i < end; ++i) {
        if (BITTEST(h->in, i) && (x->value == SIZE_MAX || 
            h->cmp(h->key + h->key_size * i, 
                   h->key + h->key_size * x->value) < 0)) {
            x->value = i;
        }
    }

    x->rank = 0;

    // create group y that is the result of combining prior
    // children of x with x
    // also if any child is active then remove it from active nodes
    group* y = x;
    for (size_t c = 0; c < r; ++c) {
        group* child = x->children[c];
        if (h->A[c] == child)
            h->A[c] = NULL;
        y = RRHeapCombine(h, y, child);
    }

    // if the resulting root is diferent form x then set that as the new root
    if (y != x) {
        y->parent = p;
        p->children[r] = y;

        if (h->A[y->rank] == x)
            h->A[y->rank] = (RRHeapGroupCompare(h, y, p) < 0)? y : NULL;
    }
}
