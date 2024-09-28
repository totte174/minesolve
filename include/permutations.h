#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

#include <alloca.h>
#include <malloc.h>

#include "common.h"
#include "equations.h"

#define MIN_PERMUTATIONS (1ULL << 22ULL)
#define MAX_PERMUTATIONS (1ULL << 27ULL)

typedef struct PermutationSet {
    Mask* permutations;
    int32_t permutation_size;
    int32_t permutation_c;
    bool initialized;

    int32_t splits_length[MAX_EDGE_SIZE];
    int32_t splits_start[MAX_EDGE_SIZE];
    int32_t split_c;
} PermutationSet;

int32_t mine_c(Mask* permutation);
FaultStatus get_permutation_set(Board* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap);
void permutation_set_deinit(PermutationSet* permutation_set);
#endif