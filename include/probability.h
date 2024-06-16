#ifndef PROBABILITY_H
#define PROBABILITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common.h"
#include "permutations.h"

void get_pmap(Board* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap);
int32_t pmap_to_board(Board* board, Edge* edge, ProbabilityMap* pmap, double* prob_a);

#endif