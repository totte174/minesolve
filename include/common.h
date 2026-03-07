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

#define MAX_SQUARES      MS_MAX_SQUARES   /* 1024 */
#define MAX_SEARCH       MS_MAX_DEPTH     /* 8    */
#define MAX_MINE_RANGE   18
#define MAX_FRONTIER_SIZE (3 * 64)
#define MASK_PARTS       (MAX_FRONTIER_SIZE / 64)
#define MAX_BUF          (MAX_SQUARES * 2)
//#define TRANSPOSITION_TABLE

// ----------- INTERNAL STRUCTS

typedef struct Frontier {
    int32_t frontier[MAX_FRONTIER_SIZE];
    int32_t frontier_c;
    int32_t group_length[MAX_FRONTIER_SIZE];
    int32_t group_start[MAX_FRONTIER_SIZE];
    int32_t group_c;

    int32_t unconstrained_c;

    int32_t solved[MAX_FRONTIER_SIZE];
    int32_t solved_c;
} Frontier;

typedef struct ProbabilityMap {
    double total_weight;
    double p_frontier[MAX_FRONTIER_SIZE];
    double p_solved[MAX_FRONTIER_SIZE];
    double p_unconstrained;
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
