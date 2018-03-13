#include <stdlib.h>

typedef struct _member{
    size_t parent;
    int rank;
} member;

typedef struct _set{
    member* m;
    size_t size;
} set;

set* SetInit(size_t size);
void SetMake(set* s, size_t x);
size_t SetFind(set* s, size_t x);
void SetFree(set* s);
void SetUnion(set* s, size_t x, size_t y);
