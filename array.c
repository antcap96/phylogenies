#include "array.h"

array* ArrayMake() {
    array* a = (array*) malloc(sizeof(array));
    a->p = NULL;
    a->max_size = 0;
    a->size = 0;

    return a;
}

void ArrayPush(array* a, char x) {
    char* temp;
    if (a->size == a->max_size)
    {
        temp = a->p;
        a->p = (char*) malloc(sizeof(char) * MAX(a->max_size * 2, 1));
        a->max_size = MAX(a->max_size * 2, 1);

        for (size_t i = 0; i < a->size; i++)
            a->p[i] = temp[i];
        if (temp != NULL)
            free(temp);
    }
    a->p[a->size] = x;
    a->size++;
}

void ArrayFree(array* a) {
    if(a->p != NULL)
        free(a->p);
    free(a);
}
