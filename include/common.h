#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

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

#define DEBUG false
#define MAX_MINES 256
#define MAX_SQUARES 1024
#define MAX_MINE_C_DIFF 18
#define MASK_PARTS (MAX_EDGE_SIZE / 64)
#define MAX_EDGE_SIZE (3*64)
#define MAX_SEARCH_DEPTH 3
#define MIN_PERMUTATIONS (2ULL << 18ULL)
#define MAX_PERMUTATIONS (2ULL << 26ULL)

typedef struct Arguments
{
    char board[2*MAX_SQUARES];
    char output_file[512];
    int32_t width, height, mines, test_games;
    double alpha, beta, eta;
    bool p_only;
} Arguments;

typedef struct Board {
    int32_t w;
    int32_t h;
    int32_t mine_c;
    int32_t unknown_c;
    int32_t v[MAX_SQUARES];
    bool known[MAX_SQUARES];
    bool mines[MAX_SQUARES];
} Board;

typedef struct Edge {
    int32_t edge[MAX_EDGE_SIZE];
    int32_t edge_c;
    int32_t splits_length[MAX_EDGE_SIZE];
    int32_t splits_start[MAX_EDGE_SIZE];
    int32_t split_c;

    int32_t exterior[MAX_SQUARES];
    int32_t exterior_c;

    int32_t edge_solved[MAX_EDGE_SIZE];
    int32_t edge_solved_c;
} Edge;

typedef struct ProbabilityMap {
    bool valid;
    double comb_total;
    double p_edge[MAX_EDGE_SIZE];
    double p_solved[MAX_EDGE_SIZE];
    double p_exterior;
} ProbabilityMap;

typedef struct Mask {
    uint64_t v[MASK_PARTS];
} Mask;

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