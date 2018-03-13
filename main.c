#include <stdio.h>
#include <stdlib.h>
#include "binary_heap.h"
#include "set.h"
#include "array.h"

typedef struct _edge{
    int from, to, w;
} edge;

int edgecmp(const void *ap, const void *bp)
{
    edge a = * (edge *) ap;
    edge b = * (edge *) bp;

    if (a.w < b.w)
            return -1;
    if (a.w > b.w)
            return 1;
    if (a.from < b.from)
            return -1;
    if (a.from > b.from)
            return 1;
    if (a.to < b.to)
            return -1;
    if (a.to > b.to)
            return 1;
    return 0;
}

int edgecmp2(const void *ap, const void *bp)
{
    edge a = * (edge *) ap;
    edge b = * (edge *) bp;
    
    if (a.from < b.from)
            return -1;
    if (a.from > b.from)
            return 1;
    if (a.to < b.to)
            return -1;
    if (a.to > b.to)
            return 1;
    return 0;
}

int main(){

    int n_str = 0, max_dist = 0, n_edges = 0;
    int distance;
    char** v_str;
    edge* edges;
    size_t str_size = 0;
    int i, j, k;

    scanf("%d\n", &n_str);

    v_str = (char**) malloc(n_str * sizeof(char*));

    array* a = ArrayMake();
    char c;
    while((c = getchar()) != '\n') {
        ArrayPush(a, c);
    }
    ArrayPush(a, '\0');

    str_size = a->size;

    v_str[0] = a->p;
    a->p = NULL;
    a->max_size = 0;
    a->size = 0;
    ArrayFree(a);

    for (i = 1; i < n_str; i++){
        v_str[i] = (char*) malloc(str_size * sizeof(char));
    }

    for (int i = 1; i < n_str; i++){
        scanf("%s", v_str[i]);
    }

    //for (int i = 0; i < n_str; i++){
    //    printf("%s\n", v_str[i]);
    //}

    scanf("%d", &max_dist);

    edges = (edge*) malloc(n_str*(n_str-1)/2* sizeof(edge));

    for (i = 0; i < n_str; i++){
        for (j = i+1; j < n_str; j++){
            distance = 0;
            
            for (k = 0; v_str[i][k] != '\0'; k++) {
                if (v_str[i][k] != v_str[j][k])
                    ++distance;
                if (distance > max_dist)
                    break;
            }
            
            if (distance <= max_dist) {
                edges[n_edges].from = i;
                edges[n_edges].to = j;
                edges[n_edges++].w = distance;
            }
        } 
    }

    //for (int i = 0; i < n_edges; i++) {
    //    printf("%d %d %d \n", edges[i].from, edges[i].to, edges[i].w);
    //}

    binheap *q;

    q = BINHEAP_INIT(sizeof(edge), edges, n_edges, edgecmp);
    BINHEAP_MAKE(q, n_edges);

    set* s = SetInit(n_str);

    for (i = 0; i < n_str; i++)
        SetMake(s, i);

    edge* v = (edge*) malloc(sizeof(edge) * n_str);
    i = 0;
    //printf("--\n");
    while (! BINHEAP_EMPTY(q)) {
        edge top = edges[BINHEAP_TOP(q)];
        if (SetFind(s, top.from) != SetFind(s, top.to)){
            v[i++] = edges[BINHEAP_TOP(q)];
            //printf("%d %d\n", edges[BINHEAP_TOP(q)].from+1,
            //                edges[BINHEAP_TOP(q)].to+1);
            SetUnion(s, top.from, top.to);
        }
        //printf("--%d %d\n", edges[BINHEAP_TOP(q)].from+1,
        //                    edges[BINHEAP_TOP(q)].to+1);

        BINHEAP_POP(q);
    }

    qsort(v, i, sizeof(edge), edgecmp2);

    for (j = 0; j < i; j++) {
        printf("%d %d\n", v[j].from+1,
                          v[j].to+1);
    }

    // free stuff
    BINHEAP_FREE(q);
    free(v);
    free(edges);
    for (i = 0; i < n_str; i++)
        free(v_str[i]);
    free(v_str);
    SetFree(s);

    return 0;
}