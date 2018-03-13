#include <stdio.h>
#include <stdlib.h>
#include "binary_heap.h"
#include "set.h"

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

    scanf("%d", &n_str);

    v_str = (char**) malloc(n_str * sizeof(char*));

    for (int i=0; i < n_str; i++){
        v_str[i] = (char*) malloc(100 * sizeof(char));
    }

    for (int i=0; i < n_str; i++){
        scanf("%s", v_str[i]);
    }

    for (int i=0; i < n_str; i++){
        printf("%s\n", v_str[i]);
    }

    scanf("%d", &max_dist);

    edges = (edge*) malloc(n_str*(n_str-1)/2* sizeof(edge));

    for (int i = 0; i < n_str; i++){
        for (int j = i+1; j < n_str; j++){
            distance = 0;
            
            for (int k = 0; v_str[i][k] != '\0'; k++) {
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

    //qsort(edges, n_edges, sizeof(edge), edgecmp);

    binheap *q;

    q = BINHEAP_INIT(sizeof(edge), edges, n_edges, edgecmp);
    BINHEAP_MAKE(q, n_edges);

    set* s = set_init(n_str);

    for (size_t i = 0; i < n_str; i++){
        makeSet(s, i);
    }

    edge* v = (edge*) malloc(sizeof(edge) * n_str);
    int i = 0;
    //printf("--\n");
    while (! BINHEAP_EMPTY(q)) {
        edge top = edges[BINHEAP_TOP(q)];
        if (find(s, top.from) != find(s, top.to)){
            v[i++] = edges[BINHEAP_TOP(q)];
            //printf("%d %d\n", edges[BINHEAP_TOP(q)].from+1,
            //                edges[BINHEAP_TOP(q)].to+1);
            Union(s, top.from, top.to);
        }

        //printf("--%d %d\n", edges[BINHEAP_TOP(q)].from+1,
        //                    edges[BINHEAP_TOP(q)].to+1);

        BINHEAP_POP(q);
    }

    qsort(v, i, sizeof(edge), edgecmp2);

    for (int j = 0; j < i; j++) {
        printf("%d %d\n", v[j].from+1,
                          v[j].to+1);
    }

    // free stuff

    return 0;
}