#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define DEBUG false
#define MAX_PERMUTATIONS (1024ULL*1024ULL)
#define MAX_BOMBS 256
#define MAX_SQUARES 1024
#define PERMUTATION_PARTS 2 // Longest possible border_unknown_c to be <= PERMUTATION_PARTS * 64

typedef struct Board {
    int32_t w;
    int32_t h;
    int32_t bomb_c;
    int32_t unknown_c;
    int32_t v[MAX_SQUARES];
    bool known[MAX_SQUARES];
    bool bomb[MAX_SQUARES];
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
    Equation equations[MAX_SQUARES];
    int32_t unknown_c;
    int32_t solved[MAX_SQUARES];
    int32_t splits[MAX_SQUARES];
    int32_t split_c;
} EquationSet;

typedef struct Permutation {
    int32_t bomb_amount;
    uint64_t mask[PERMUTATION_PARTS];
    uint64_t bombs[PERMUTATION_PARTS];
} Permutation;

typedef struct PermutationSet {
    int32_t permutation_c;
    Permutation permutations[];
} PermutationSet;

typedef struct BoardStatistics {
    int32_t best_p;
    int32_t best_value;

    double p[MAX_SQUARES];
    double gini_impurity[MAX_SQUARES];
    double information_gain[MAX_SQUARES];
    double value[MAX_SQUARES];
} BoardStatistics;

typedef struct KCount {
    int32_t count[MAX_BOMBS];
} KCount;

typedef struct IntermediateStatistics {
    int32_t border_unknown_c;
    int32_t n;
    KCount total;
    KCount per_square[MAX_SQUARES];
} IntermediateStatistics;

#endif