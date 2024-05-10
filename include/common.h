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
#define MAX_PERMUTATIONS (16*1024*1024)
#define MAX_MINES 256
#define MAX_SQUARES 1024
#define MAX_MINE_C_DIFF 18
#define PERMUTATION_PARTS (MAX_BORDER_UNKNOWN / 64) // Longest possible border_unknown_c to be <= PERMUTATION_PARTS * 64
#define MAX_BORDER_UNKNOWN (3*64)

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

typedef struct Equation {
    int32_t unknown[8]; // An entry here will be an index in border.border_unknown
    int32_t unknown_c;
    int32_t amount;
} Equation;

typedef struct EquationSet {
    int32_t equation_c;
    Equation* equations[MAX_SQUARES];
    int32_t unknown_c;
    int32_t solved[MAX_BORDER_UNKNOWN];
    int32_t splits[MAX_BORDER_UNKNOWN];
    int32_t split_c;
} EquationSet;

typedef struct Permutation {
    int32_t mine_c;
    uint64_t mask[PERMUTATION_PARTS];
    uint64_t mines[PERMUTATION_PARTS];
} Permutation;

typedef struct PermutationSet {
    Permutation solved_permutation;
    int32_t permutation_c;
    int32_t total_permutation_c;
    int32_t splits_length[MAX_BORDER_UNKNOWN];
    int32_t splits_start[MAX_BORDER_UNKNOWN];
    int32_t split_c;
    Permutation permutations[MAX_PERMUTATIONS];
} PermutationSet;

typedef struct BoardStatistics {
    int32_t best_p;
    int32_t best_value;

    double p[MAX_SQUARES];
    double p2[MAX_SQUARES];
    double gini_impurity[MAX_SQUARES];
    double information_gain[MAX_SQUARES];
    double value[MAX_SQUARES];
} BoardStatistics;

typedef struct IntermediateStatistics {
    int32_t border_unknown_c;
    int32_t n;
    bool valid;
    double comb_total;
    double p_border_unknown[MAX_BORDER_UNKNOWN];
    double p_outside;
} IntermediateStatistics;

#endif