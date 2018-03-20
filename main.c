#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include "binary_heap.h"
#include "set.h"
#include "array.h"

typedef struct _edge{
    size_t from, to, w;
} edge;

// sort as aked by the project
int edgecmp(const void *ap, const void *bp)
{
    edge a = * (edge *) ap;
    edge b = * (edge *) bp;

    if (a.w < b.w)
        return -1;
    if (a.w > b.w)
        return 1;

    size_t atemp = MIN(a.from, a.to);
    size_t btemp = MIN(b.from, b.to);    
    if (atemp < btemp)
        return -1;
    if (atemp > btemp)
        return 1;
    
    atemp = MAX(a.from, a.to);
    btemp = MAX(b.from, b.to);

    if (atemp < btemp)
        return -1;
    if (atemp > btemp)
        return 1;
    return 0;
}

// sort based on index (undirected), treating from and to equaly
int edgecmp2(const void *ap, const void *bp)
{
    edge a = * (edge *) ap;
    edge b = * (edge *) bp;

    size_t atemp = MIN(a.from, a.to);
    size_t btemp = MIN(b.from, b.to);    
    if (atemp < btemp)
        return -1;
    if (atemp > btemp)
        return 1;
    
    atemp = MAX(a.from, a.to);
    btemp = MAX(b.from, b.to);

    if (atemp < btemp)
        return -1;
    if (atemp > btemp)
        return 1;
    return 0;
}

// sort based on indexes (directed), from takes precedence to to
int edgecmp3(const void *ap, const void *bp)
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


edge* Kruskal(edge* edges, int n_edges, int n_nodes, 
              int (*cmp_func)(const void*, const void*), int*size){

    binheap *q;

    q = BINHEAP_INIT(sizeof(edge), edges, n_edges, cmp_func);
    BINHEAP_MAKE(q, n_edges);

    set* s = SetInit(n_nodes);

    edge* v = (edge*) malloc(sizeof(edge) * n_nodes);
    *size = 0;
    //printf("--\n");
    while (! BINHEAP_EMPTY(q)) {
        edge top = edges[BINHEAP_TOP(q)];
        if (SetFind(s, top.from) != SetFind(s, top.to)){
            v[(*size)++] = edges[BINHEAP_TOP(q)];
            //printf("%d %d\n", edges[BINHEAP_TOP(q)].from+1,
            //                edges[BINHEAP_TOP(q)].to+1);
            SetUnion(s, top.from, top.to);
        }
        //printf("--%d %d\n", edges[BINHEAP_TOP(q)].from+1,
        //                    edges[BINHEAP_TOP(q)].to+1);

        BINHEAP_POP(q);
    }

    BINHEAP_FREE(q);
    SetFree(s);

    return v;
}

// the vector output of prim includes some xFF elements that are to be ignored
edge* Prim(edge* edges, int n_edges, int n_nodes, 
           int (*cmp_func)(const void*, const void*), int* size){

    //mimic adj from sorted edge list

    size_t* vertex_idx;
    vertex_idx = (size_t*) malloc((n_nodes+1) * sizeof(size_t));

    //sort
    qsort(edges, n_edges, sizeof(edge), edgecmp3);

    vertex_idx[0] = 0;
    size_t j = 0;
    for (int i = 0; i < n_edges; i++) {
        //printf("%d\n", edges[i].from);
        if (edges[i].from == j)
            continue;
        
        for (; j < edges[i].from; j++)
            vertex_idx[j+1] = i;
    }
    for (int i = j+1; i < n_nodes+1; i++)
        vertex_idx[i] = n_edges;

    //for (int i = 0; i < n_nodes+1; i++)
    //    printf("%d ", vertex_idx[i]);
    //putchar('\n');

    binheap *q;

    edge* v = (edge*) malloc(sizeof(edge) * n_nodes);
    memset(v , 0xFF, n_nodes * sizeof(edge));
    signed char* fixed = (signed char*) malloc( sizeof(signed char) * n_nodes);
    memset(fixed , 0x00, n_nodes * sizeof(signed char));

    int vertex;

    q = BINHEAP_INIT(sizeof(edge), v, n_nodes, cmp_func);
    BINHEAP_MAKE(q, 0);

    for (int j = 0; j < n_nodes; j++){
        if (v[j].w == SIZE_MAX) {
            vertex = j;
            fixed[j] = 1;
        }
        else
            continue;
        while(1) {
            // update entries in the heap
            for (size_t i = vertex_idx[vertex]; i < vertex_idx[vertex+1]; i++){
                if (!fixed[edges[i].to] && cmp_func(&edges[i], &v[edges[i].to]) < 0){
                    v[edges[i].to] = edges[i];
                    BINHEAP_UPDATE(q, edges[i].to);
                }
            }

            if (BINHEAP_EMPTY(q))
                break;

            vertex = BINHEAP_TOP(q);
            BINHEAP_POP(q);
            fixed[vertex] = 1;
        }
    }
    
    // take lowest

    *size = n_nodes;

    BINHEAP_FREE(q);
    free(vertex_idx);
    //free(fixed);
    return v;

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

    for (i = 1; i < n_str; i++){
        scanf("%s", v_str[i]);
    }

    //for (int i = 0; i < n_str; i++){
    //    printf("%s\n", v_str[i]);
    //}

    scanf("%d", &max_dist);

    edges = (edge*) malloc(n_str*(n_str-1)* sizeof(edge));

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
                edges[n_edges].from = j;
                edges[n_edges].to = i;
                edges[n_edges++].w = distance;
            }
        } 
    }

    // Free strings
    for (i = 0; i < n_str; i++)
        free(v_str[i]);
    free(v_str);


    edge* v = Prim(edges, n_edges, n_str, edgecmp, &i);

    //for (j = 0; j < i; j++) {
    //    printf("--%lu %lu\n", v[j].from+1,
    //                        v[j].to+1);
    //}

    qsort(v, i, sizeof(edge), edgecmp2);

    for (j = 0; j < i; j++) {
        if(v[j].from == v[j].to)
            continue;
        printf("%lu %lu\n", MIN(v[j].from, v[j].to)+1,
                            MAX(v[j].to, v[j].from)+1);
    }

    free(v);
    free(edges);

    return 0;
}