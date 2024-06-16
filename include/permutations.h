#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "equations.h"

typedef struct PermutationSet {
    Mask* permutations;
    int32_t permutation_size;
    int32_t permutation_c;

    int32_t splits_length[MAX_EDGE_SIZE];
    int32_t splits_start[MAX_EDGE_SIZE];
    int32_t split_c;

    bool valid;
} PermutationSet;

int32_t mine_c(Mask* permutation);
void get_permutation_set(Board* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap);
void destroy_permutations(PermutationSet* permutation_set);
#endif