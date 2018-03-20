#include <limits.h>
#include <stdint.h>
#include <string.h>
#include "set.h"


set* SetInit(size_t size) {
    set* s = (set*) malloc (sizeof(set));
    s->m = (member*) malloc (size * sizeof(member));
    
    for (size_t i = 0; i < size; i++)
    {
        s->m[i].parent = i;
        s->m[i].rank = 0;
    }
    //memset(s->m , 0xFF, size * sizeof(member));
    s->size = size;
    return s;
}

// void SetMake(set* s, size_t x) {
//     if (x > s->size); //shit

//     if (s->m[x].parent != SIZE_MAX); //already in

//     s->m[x].parent = x;
//     s->m[x].rank = 0;
// }

size_t SetFind(set* s, size_t x) {
    //if (s->m[x].parent != x)
    //    s->m[x].parent = SetFind(s, s->m[x].parent);
    
    for (; x != s->m[x].parent; x = s->m[x].parent)
        s->m[x].parent = s->m[s->m[x].parent].parent;

    return s->m[x].parent;
}

void SetFree(set* s) {
    free(s->m);
    free(s);
}

void SetUnion(set* s, size_t x, size_t y) {
    size_t xRoot = SetFind(s, x);//s->m[x].parent;
    size_t yRoot = SetFind(s, y);//s->m[y].parent;

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