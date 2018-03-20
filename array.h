#include <stdlib.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

typedef struct _array {
    char* p;
    size_t size, max_size;
} array;

array* ArrayMake();
void ArrayPush(array* a, char x);
void ArrayFree(array* a);
