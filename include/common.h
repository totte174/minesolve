#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "minesolve.h"

// ----------- HARDCODED LIMITATIONS

#define MAX_SQUARES     MS_MAX_SQUARES   /* 1024 */
#define MAX_SEARCH      MS_MAX_DEPTH     /* 8    */
#define MAX_MINE_C_DIFF 18
#define MAX_EDGE_SIZE   (3 * 64)
#define MASK_PARTS      (MAX_EDGE_SIZE / 64)
#define MAX_BUF         (MAX_SQUARES * 2)
//#define TRANSPOSITION_TABLE

// ----------- INTERNAL STRUCTS

typedef struct Edge {
    int32_t edge[MAX_EDGE_SIZE];
    int32_t edge_c;
    int32_t splits_length[MAX_EDGE_SIZE];
    int32_t splits_start[MAX_EDGE_SIZE];
    int32_t split_c;

    int32_t exterior_c;

    int32_t edge_solved[MAX_EDGE_SIZE];
    int32_t edge_solved_c;
} Edge;

typedef struct ProbabilityMap {
    double comb_total;
    double p_edge[MAX_EDGE_SIZE];
    double p_solved[MAX_EDGE_SIZE];
    double p_exterior;
} ProbabilityMap;

typedef struct Mask {
    uint64_t v[MASK_PARTS];
} Mask;


// ----------- MACRO FUNCTIONS

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#define insert_sorted(a, len, elem)         \
({                                          \
    for (int32_t i = 0; i < len; i++) {     \
        if (a[i] > elem) {                  \
            __typeof__ (elem) temp = elem;  \
            elem = a[i];                    \
            a[i] = temp;                    \
        }                                   \
    }                                       \
    a[len] = elem;                          \
})

#define binary_search(a, len, elem)         \
({                                          \
    int32_t result = -1;                    \
    int32_t l = 0, r = 0;                   \
    while (l <= r) {                        \
        int32_t m = (l + r) / 2;            \
        if (a[m] < elem) {                  \
            r = m + 1;                      \
        } else if (a[m] > elem) {           \
            l = m - 1;                      \
        } else {                            \
            result = m;                     \
            break;                          \
        }                                   \
    }                                       \
    result;                                 \
})



// ----------- MASK MACRO FUNCTIONS

#define mask_overlap(mask1, mask2)              \
({                                              \
    uint64_t result = 0;                        \
    for (int32_t i = 0; i < MASK_PARTS; i++) {  \
        result |= (mask1).v[i] & (mask2).v[i];  \
    }                                           \
    ((bool) result);                            \
})
#define mask_count(mask)                            \
({                                                  \
    uint64_t result = 0;                            \
    for (int32_t i = 0; i < MASK_PARTS; i++) {      \
        result += __builtin_popcountll((mask).v[i]);\
    }                                               \
    result;                                         \
})
#define mask_reset(mask)                        \
({                                              \
    for (int32_t i = 0; i < MASK_PARTS; i++) {  \
        (mask).v[i] = 0ULL;                     \
    }                                           \
})
#define mask_get(mask, i) (((mask).v[((uint64_t)(i)) / 64ULL] >> (((uint64_t)(i)) % 64ULL)) & 1ULL)
#define mask_set(mask, i, n) ((mask).v[((uint64_t)(i)) / 64ULL] |= ((uint64_t)(n)) << (((uint64_t)(i)) % 64ULL))

#endif
