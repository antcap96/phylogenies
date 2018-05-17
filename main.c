#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include "binary_heap.h"
#include "rank_relaxed_heap.h"
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

// sort based on index (undirected), treating 'from' and 'to' equaly
int undirectedcmp(const void *ap, const void *bp)
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

// sort based on indexes (directed), 'from' takes precedence to 'to'
int directedcmp(const void *ap, const void *bp)
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
              int (*cmp_func)(const void*, const void*), int* size) {

    // create the queue
    binheap *q;
    q = BINHEAP_INIT(sizeof(edge), edges, n_edges, cmp_func);
    
    // fill the queue with all the edges
    BINHEAP_MAKE(q, n_edges);

    set* s = SetInit(n_nodes);

    // vector of edges of the MST
    edge* v = (edge*) malloc(sizeof(edge) * n_nodes);
    *size = 0;

    while (! BINHEAP_EMPTY(q)) {
        edge top = edges[BINHEAP_TOP(q)];
        if (SetFind(s, top.from) != SetFind(s, top.to)){
            v[(*size)++] = edges[BINHEAP_TOP(q)];
            SetUnion(s, top.from, top.to);
        }
        BINHEAP_POP(q);
    }

    BINHEAP_FREE(q);
    SetFree(s);

    return v;
}

edge* Kruskal2(edge* edges, int n_edges, int n_nodes, 
               int (*cmp_func)(const void*, const void*), int* size) {
    
    
    // sort edges
    qsort(edges, n_edges, sizeof(edge), cmp_func);

    set* s = SetInit(n_nodes);

    // vector of edges of the MST
    edge* v = (edge*) malloc(sizeof(edge) * n_nodes);
    *size = 0;

    for (int i = 0; i < n_edges; i++) {
        edge top = edges[i];
        if (SetFind(s, top.from) != SetFind(s, top.to)){
            v[(*size)++] = edges[i];
            SetUnion(s, top.from, top.to);
        }
    }

    SetFree(s);

    return v;
}

// the vector output of prim includes some xFF elements that are to be ignored
edge* Prim(edge* edges, int n_edges, int n_nodes, 
           int (*cmp_func)(const void*, const void*), int* size){
    
    // vector of idx to mimic adj matrix
    // vertex_idx[i] will be the index of the begining of edges from i
    // vertex_idx[i+1] will be the index after the end

    // O(n)
    size_t* vertex_idx;
    vertex_idx = (size_t*) malloc((n_nodes+1) * sizeof(size_t));

    // O(m log(m))
    qsort(edges, n_edges, sizeof(edge), directedcmp);

    // Fill the vertex_idx O(m)
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

    clock_t t1 = clock();
    
    binheap *q;

    // vector of edges that will be the output as well as the one used
    // in the priority queue
    // filled with SIZE_MAX so that the edges are allways smaller than 
    // any comparison
    // fixed[i] says if i node is already in the MST

    edge* v = (edge*) malloc(sizeof(edge) * n_nodes);
    memset(v , 0xFF, n_nodes * sizeof(edge));
    signed char* fixed = (signed char*) malloc( sizeof(signed char) * n_nodes);
    memset(fixed , 0x00, n_nodes * sizeof(signed char));

    int vertex;

    q = BINHEAP_INIT(sizeof(edge), v, n_nodes, cmp_func);
    // inicialyze the queue empty
    BINHEAP_MAKE(q, 0);

    for (int j = 0; j < n_nodes; j++){
        if (v[j].w == SIZE_MAX) {
            vertex = j;
            fixed[j] = 1;
        }
        else
            continue;
        while(1) {
            // iterate over the edges of the new node
            for (size_t i = vertex_idx[vertex]; i < vertex_idx[vertex+1]; i++){
                if (!fixed[edges[i].to] && \
                    cmp_func(&edges[i], &v[edges[i].to]) < 0){
                    // update entries in the heap if better edge is found
                    v[edges[i].to] = edges[i];
                    BINHEAP_UPDATE(q, edges[i].to); //same as push
                }
            }

            if (BINHEAP_EMPTY(q))
                break;

            vertex = BINHEAP_TOP(q);
            BINHEAP_POP(q);
            fixed[vertex] = 1;
        }
    }
    
    *size = n_nodes;

    BINHEAP_FREE(q);
    free(vertex_idx);
    free(fixed);

    printf("BinHeapPrim:%lg\n",(clock()-t1)/(double) CLOCKS_PER_SEC);

    return v;
}

// the vector output of prim includes some xFF elements that are to be ignored
edge* Prim2(edge* edges, int n_edges, int n_nodes, 
            int (*cmp_func)(const void*, const void*), int* size){
    
    // vector of idx to mimic adj matrix
    // vertex_idx[i] will be the index of the begining of edges from i
    // vertex_idx[i+1] will be the index after the end

    clock_t startSort = clock();

    // O(n)
    size_t* vertex_idx;
    vertex_idx = (size_t*) malloc((n_nodes+1) * sizeof(size_t));

    // O(m log(m))
    qsort(edges, n_edges, sizeof(edge), directedcmp);

    // Fill the vertex_idx O(m)
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

    printf("Sort:%lf\n",(clock()-startSort)/(double) CLOCKS_PER_SEC);

    clock_t startRRHeap = clock();

    RRHeap *q;

    // vector of edges that will be the output as well as the one used
    // in the priority queue
    // filled with SIZE_MAX so that the edges are allways smaller than 
    // any comparison
    // fixed[i] says if i node is already in the MST

    edge* v = (edge*) malloc(sizeof(edge) * n_nodes);
    memset(v , 0xFF, n_nodes * sizeof(edge));
    signed char* fixed = (signed char*) malloc( sizeof(signed char) * n_nodes);
    memset(fixed , 0x00, n_nodes * sizeof(signed char));

    int vertex;

    q = RRHeapMake(sizeof(edge), v, n_nodes, cmp_func);
    // inicialyze the queue empty

    for (int j = 0; j < n_nodes; j++){
        if (v[j].w == SIZE_MAX) {
            vertex = j;
            fixed[j] = 1;
        }
        else
            continue;
        while(1) {
            // iterate over the edges of the new node
            for (size_t i = vertex_idx[vertex]; i < vertex_idx[vertex+1]; i++){
                if (!fixed[edges[i].to] && \
                    cmp_func(&edges[i], &v[edges[i].to]) < 0){
                    // update entries in the heap if better edge is found
                    v[edges[i].to] = edges[i];
                    RRHeapInsert(q, edges[i].to);
                }
            }

            if (RRHeapTopEmpty(q))
                break;

            vertex = RRHeapTop(q);
            RRHeapPop(q);
            fixed[vertex] = 1;
        }
    }
    
    *size = n_nodes;

    RRHeapFree(q);
    free(fixed);
    printf("RRHeapPrim:%lg\n",(clock()-startRRHeap)/(double) CLOCKS_PER_SEC);

    free(vertex_idx);

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

    clock_t startRead = clock();

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

    printf("FirstRead:%lg\n",(clock()-startRead)/(double) CLOCKS_PER_SEC);

    for (i = 1; i < n_str; i++){
        v_str[i] = (char*) malloc(str_size * sizeof(char));
    }

    for (i = 1; i < n_str; i++){
        scanf("%s", v_str[i]);
    }

    scanf("%d", &max_dist);

    printf("AllRead:%lg\n",(clock()-startRead)/(double) CLOCKS_PER_SEC);

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

    printf("Distances:%lg\n",(clock()-startRead)/(double) CLOCKS_PER_SEC);

    edge* v = Prim2(edges, n_edges, n_str, edgecmp, &i);

    qsort(v, i, sizeof(edge), undirectedcmp);

    for (j = 0; j < i; j++) {
        if(v[j].from == v[j].to)
            continue;
        //printf("%lu %lu\n", MIN(v[j].from, v[j].to)+1,
        //                    MAX(v[j].to, v[j].from)+1);
    }

    free(v);
    free(edges);

    return 0;
}