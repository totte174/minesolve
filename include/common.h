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

// Priority and same value strategy
#define PRIO_PERIMETER false
#define PRIO_CORNER false
#define PRIO_MANHATTAN false
#define PRIO_CHEBYSHEV false
#define PRIO_ADJACENT true

#define USE_RANDOM_STRATEGY false
#define USE_DIVIDE_AUXILLARY true

#define DEBUG false
#define MAX_PERMUTATIONS (16*1024*1024)
#define MAX_MINES 256
#define MAX_SQUARES 1024
#define MAX_MINE_C_DIFF 18
#define MASK_PARTS (MAX_BORDER_UNKNOWN / 64)
#define MAX_BORDER_UNKNOWN (3*64)

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

typedef struct Border {
    int32_t border_unknown[MAX_SQUARES];
    int32_t border_unknown_c;
    int32_t outside_unknown_c;

    int32_t border_known[MAX_SQUARES];
    int32_t border_known_c;
    int32_t outside_known_c;
} Border;

typedef struct Mask {
    uint64_t v[MASK_PARTS];
} Mask;

typedef struct Permutation {
    Mask mask;
    Mask mines;
} Permutation;

typedef struct PermutationSet {
    Permutation solved_permutation;

    Permutation permutations[MAX_PERMUTATIONS];
    int32_t permutation_c;

    int32_t splits_length[MAX_BORDER_UNKNOWN];
    int32_t splits_start[MAX_BORDER_UNKNOWN];
    int32_t split_c;
} PermutationSet;

typedef struct Equation {
    Mask mask;
    int32_t amount;
} Equation;

typedef struct EquationSet {
    Equation equations[MAX_SQUARES];
    int32_t equation_c;

    int32_t border_unknown_c;

    Permutation solved;

    int32_t splits_length[MAX_BORDER_UNKNOWN];
    int32_t splits_start[MAX_BORDER_UNKNOWN];
    int32_t split_c;
} EquationSet;

typedef struct BoardStatistics {
    int32_t best_p;
    int32_t best_value;
    double total_combinations;

    double p[MAX_SQUARES];
    double p2[MAX_SQUARES];
    double gini_impurity[MAX_SQUARES];
    double information_gain[MAX_SQUARES];
    double value[MAX_SQUARES];
} BoardStatistics;

typedef struct ProbabilityMap {
    int32_t border_unknown_c;
    int32_t n;
    bool valid;
    double comb_total;
    double p_border_unknown[MAX_BORDER_UNKNOWN];
    double p_outside;
} ProbabilityMap;

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