#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

typedef struct _member{
    size_t parent;
    int rank;
} member;

typedef struct _set{
    member* m;
    size_t size;
} set;


// init
// - create space of size n
// - parent and rank
// makeSet
// find
// union

set* set_init(size_t size) {
    set* s = (set*) malloc (sizeof(set));
    s->m = (member*) malloc (size * sizeof(member));
    memset(s->m , 0xFF, size * sizeof(member));
    s->size = size;
    return s;
}

void makeSet(set* s, size_t x) {
    if (x > s->size); //shit

    if (s->m[x].parent != SIZE_MAX); //already in

    s->m[x].parent = x;
    s->m[x].rank = 0;
}

size_t find(set* s, size_t x) {
    if (s->m[x].parent != x)
        s->m[x].parent = find(s, s->m[x].parent);
    return s->m[x].parent;
}

void Union(set* s, size_t x, size_t y) {
    size_t xRoot = s->m[x].parent;
    size_t yRoot = s->m[y].parent;

    if (xRoot == yRoot)
        return;
    
    if (s->m[xRoot].rank < s->m[yRoot].rank)
        s->m[xRoot].parent = yRoot;
    else if (s->m[xRoot].rank > s->m[yRoot].rank)
        s->m[yRoot].parent = xRoot;
    else {
        s->m[xRoot].parent = yRoot;
        ++(s->m[yRoot].rank);
    }
}