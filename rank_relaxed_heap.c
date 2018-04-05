#include "rank_relaxed_heap.h"
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// gives position of largest bit = 1
// copied from boost c++, is it ok?
size_t log_base_2(size_t n)
{
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

// attention: max_size = 0 might be a problem
RRHeap* RRHeapMake(size_t type_size, void* data, size_t max_size,
                   int (*cmp_func)(const void*, const void*)) {
    
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

    h->in = (int*) malloc( sizeof(int) * max_size);
    memset(h->in, 0, sizeof(int) * max_size);

    // this is where I allocate the space for the groups
    h->index_to_group = (group*) malloc( sizeof(group) * g);

    for(size_t i = 0; i < g; i++) {
        h->index_to_group[i].value = SIZE_MAX;
        // TODO: they are their own parent for some reason
        h->index_to_group[i].parent = &h->index_to_group[i];
        // largest_key means that they the group is empty
        // i could use SIZE_MAX for value, should work as well
        h->index_to_group[i].kind = largest_key;
    }


    h->A = (group**) malloc(sizeof(group*) * (r+1));
    memset(h->A, 0, sizeof(group*) * (r+1));
    h->root.rank = r+1;
    h->root.parent = NULL;
    h->root.kind = smallest_key;

    // dont get the (log_g + 1) * (g + 1)
    // yes I do: max_rank * num_groups
    h->root.children = (group**) malloc(sizeof(group*) * (log_g+1) * (g+1));
    
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

size_t build_tree(RRHeap* h,  group* parent, size_t idx,
                  size_t r,  size_t max_rank){
    
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
    h->in[id] = 1;
    RRHeapUpdate(h, id);
}

void RRHeapUpdate(RRHeap* h, size_t id) {
    group* a = &h->index_to_group[id / h->log_n];
    if (a->value == SIZE_MAX
            || a->kind == largest_key // not
            || a->value == id
            || h->cmp(h->key + h->key_size * id, 
                      h->key + h->key_size * a->value) < 0) {
        
        // stuff
        if (a != h->smallest_value)
            h->smallest_value = NULL;
        a->kind = stored_key;
        a->value = id;
        RRHeapPromote(h, a);
    }
}

void RRHeapPromote(RRHeap* h, group* a) {
    assert(a != NULL);
    size_t r = a->rank;
    group* p = a->parent;
    assert(p != NULL);
    
    if (!(RRHeapGroupCompare(h, a, p) < 0)){
        if(h->A[r] == a)
            h->A[r] = NULL;
    }
    else {
        // s is the rank + 1 sibling
        group* s = p->rank > r + 1? p->children[r + 1] : NULL;

        // If a is the last child of p
        if (r == p->rank - 1) {
            if (h->A[r] == NULL)
                h->A[r] = a;
            else if (h->A[r] != a)
                RRHeapPairTransform(h, a);
        } else {
            assert(s != NULL);
            if (h->A[r + 1] == s)
                RRHeapActiveSiblingTransform(h, a, s);
            else
                RRHeapGoodSiblingTransform(h, a, s);
        }
    }
}

// I don't have to change this compartion function!
int RRHeapGroupCompare(RRHeap* h, group* x, group* y)
{
    if (x->kind < y->kind
        || (x->kind == y->kind
            && x->kind == stored_key
            && h->cmp(h->key + h->key_size * x->value, 
                      h->key + h->key_size * y->value) < 0))
        return -1;
    else if (x->kind > y->kind
        || (x->kind == y->kind
            && x->kind == stored_key
            && h->cmp(h->key + h->key_size * x->value, 
                      h->key + h->key_size * y->value) > 0))
        return 1;
    
    return 0;
}

void RRHeapFindSmallest(RRHeap* h) {
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
                    || RRHeapGroupCompare(h, h->A[i], h->smallest_value) < 0)){
                h->smallest_value = h->A[i];
            }
        }
    }
}

static void do_swap(group** x, group** y)
{
    group* tmp = *x;
    *x = *y;
    *y = tmp;
}

// somewhat diferent from book, try to understand how
// ok
void RRHeapPairTransform(RRHeap* h, group* a) {
    size_t r = a->rank;

    group* pa = a->parent;
    assert(pa != NULL);

    group* ga = pa->parent;
    assert(ga != NULL);

    assert(h->A[r] != NULL);
    group* b = h->A[r];
    assert(b != NULL);

    h->A[r] = NULL;

    group* pb = b->parent;
    assert(pb != NULL);

    group* gb = pb->parent;
    assert(gb != NULL);

    // Remove a and b from their parents
    assert(b == pb->children[pb->rank-1]); // Guaranteed because ap is active
    --pb->rank;

    // Guaranteed by caller
    assert(a == pa->children[pa->rank-1]);
    --pa->rank;

    // Note: a, pa, b, pb all have rank r
    if (RRHeapGroupCompare(h, pb, pa) < 0) {
        do_swap(&a, &b);
        do_swap(&pa, &pb);
        do_swap(&ga, &gb);
    }

    // Assuming k(pa) <= k(pb)
    // make pb the rank r child of pa
    assert(r == pa->rank);
    pa->children[pa->rank++] = pb;
    pb->parent = pa;

    // Combine a, b into a rank r+1 group c
    group* c = RRHeapCombine(h, a, b);

    // make c the rank r+1 child of gb
    assert(gb->rank > r+1);
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

// understand what this is
void RRHeapActiveSiblingTransform(RRHeap* h, group* a, group* s)
{
    group* p = a->parent;
    group* g = p->parent;

    // remove a, s from their parents
    assert(s->parent == p);
    assert(p->children[p->rank-1] == s);
    --p->rank;
    assert(p->children[p->rank-1] == a);
    --p->rank;

    size_t r = a->rank;
    h->A[r+1] = 0;
    a = RRHeapCombine(h, p, a);
    group* c = RRHeapCombine(h, a, s);

    // make c the rank r+2 child of g
    assert(g->children[r+2] == p);
    g->children[r+2] = c;
    c->parent = g;
    
    // this part is different from paper
    if (h->A[r+2] == p)
        h->A[r+2] = NULL;
    
    RRHeapPromote(h, c);
}

// void RRHeapGoodSiblingTransform(RRHeap* h, group* a, group* s){
//     size_t r = a->rank;
//     group* pa = a->parent;
//     group* ga = pa->parent;
//     group* c = s->children[s->rank-1];
//     assert(c->rank == r);
//     if (h->A[r] == c) {
//         RRHeapPairTransform(h, a);
//     } else {
//         group* p = a->parent;
//         s->children[r] = a;
//         a->parent = s;
//         p->children[r] = c;
//         c->parent = p;

//         RRHeapPromote(h, a);
//         //RRHeapClean(h, pa);
//     }
//     RRHeapPromote(h, a);
// }

//very diferent from paper, wtf?
void RRHeapGoodSiblingTransform(RRHeap* h, group* a, group* s)
{
    size_t r = a->rank;
    group* c = s->children[s->rank-1];
    assert(c->rank == r);
    if (h->A[r] == c) {
        h->A[r] = NULL;
        group* p = a->parent;

        // Remove c from its parent
        --s->rank;

        // Make s the rank r child of p
        s->parent = p;
        p->children[r] = s;

        // combine a, c and let the result be the rank r+1 child of p
        assert(p->rank > r+1);
        group* x = RRHeapCombine(h, a, c);
        x->parent = p;
        p->children[r+1] = x;

        // this part is very confusion
        // s should not be active garantied by the caller, right?
        if (h->A[r+1] == s)
            h->A[r+1] = x;
        else
            RRHeapPromote(h, x);

      //      pair_transform(a);
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


// ok
group* RRHeapCombine(RRHeap* h, group* a, group* b)
{
    assert(a->rank == b->rank);
    if (RRHeapGroupCompare(h, b, a) < 0)
        do_swap(&a, &b);
    a->children[a->rank++] = b;
    b->parent = a;
    RRHeapClean(h, a);
    return a;
}

// ok
void RRHeapClean(RRHeap*h, group* q)
{
    if (q->rank < 2)
        return;
    group* qp = q->children[q->rank-1];
    size_t s = q->rank - 2;
    group* x = q->children[s];
    group* xp = qp->children[s];
    assert(s == x->rank);

    // If x is active, swap x and xp
    if (h->A[s] == x) {
        q->children[s] = xp;
        xp->parent = q;
        qp->children[s] = x;
        x->parent = qp;
    }
}

////////////////

void RRHeapRemove(RRHeap* h, size_t id)
{
    group* a = &h->index_to_group[id / h->log_n];
    assert(h->in[id]);
    a->value = id;
    a->kind = smallest_key;
    RRHeapPromote(h, a);
    h->smallest_value = a;
    RRHeapPop(h);
}

size_t RRHeapTop(RRHeap *h)
{
    RRHeapFindSmallest(h);
    return h->smallest_value->value;
}

int RRHeapTopEmpty(RRHeap *h)
{
    RRHeapFindSmallest(h);
    // both mean the same thing, is there a casa in which one is true \
    and the other is not?
    return h->smallest_value->value == SIZE_MAX 
        || (h->smallest_value->kind == largest_key);
}

void RRHeapPop(RRHeap* h)
{
    // Fill in smallest_value. This is the group x.
    RRHeapFindSmallest(h);
    group* x = h->smallest_value;
    h->smallest_value = NULL;

    // Make x a leaf, giving it the smallest value within its group
    size_t r = x->rank;
    group* p = x->parent;
    
    assert(x->value != SIZE_MAX);

    // Find x's group
    size_t start = x->value - (x->value % h->log_n);
    size_t end = start + h->log_n;
    if (end > h->max_size)
        end = h->max_size;

    // Remove the smallest value from the group, and find the new
    // smallest value.
    //groups[get(id, *x->value)].reset();
    h->in[x->value] = 0;
    x->value = SIZE_MAX;
    x->kind = largest_key;
    for (size_t i = start; i < end; ++i) {
        if (h->in[i] && (x->value == SIZE_MAX || 
            h->cmp(h->key + h->key_size * i, 
                   h->key + h->key_size * x->value) < 0)) {
            x->kind = stored_key;
            x->value = i;
        }
    }

    x->rank = 0;

    // Combine prior children of x with x
    group* y = x;
    for (size_t c = 0; c < r; ++c) {
        group* child = x->children[c];
        if (h->A[c] == child)
            h->A[c] = NULL;
        y = RRHeapCombine(h, y, child);
    }

    // If we got back something other than x, let y take x's place
    if (y != x) {
        y->parent = p;
        p->children[r] = y;

        assert(r == y->rank);
        if (h->A[y->rank] == x)
            h->A[y->rank] = (RRHeapGroupCompare(h, y, p) < 0)? y : NULL;
    }
}
