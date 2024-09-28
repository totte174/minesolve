#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

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


// ----------- HARDCODED LIMITATIONS

#define MAX_MINES 256
#define MAX_SQUARES 1024
#define MAX_MINE_C_DIFF 18
#define MAX_EDGE_SIZE (3*64)
#define MASK_PARTS (MAX_EDGE_SIZE / 64)
#define MAX_SEARCH 6
//#define TRANSPOSITION_TABLE

// ----------- UNIVERSAL STRUCTS & ENUMS

typedef enum FaultStatus {
    valid_status = 0u,
    fault_computational_limit,
    fault_internal_limit,
    fault_invalid_board,
    fault_unknown,
} FaultStatus;

typedef struct Arguments
{
    char board[2*MAX_SQUARES];
    char output_file[512];
    int32_t width, height, mines, test_games, max_depth;
    bool wrapping_borders;
} Arguments;

typedef struct Board {
    int32_t w;
    int32_t h;
    int32_t mine_c;
    int32_t unknown_c;
    int32_t v[MAX_SQUARES];
    bool known[MAX_SQUARES];
    bool mines[MAX_SQUARES];
    bool wrapping_borders;
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
    double comb_total;
    double p_edge[MAX_EDGE_SIZE];
    double p_solved[MAX_EDGE_SIZE];
    double p_exterior;
} ProbabilityMap;

typedef struct Mask {
    uint64_t v[MASK_PARTS];
} Mask;

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