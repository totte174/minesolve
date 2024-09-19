#ifndef PROBABILITY_H
#define PROBABILITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common.h"
#include "permutations.h"
#include "board.h"

void get_pmap(Board* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap);
int32_t pmap_to_board(Board* board, Edge* edge, ProbabilityMap* pmap, double* prob_a);
void get_lowest_probability(Board* board, Edge* edge, ProbabilityMap* pmap, 
                            double* p, int32_t* pos);
int32_t get_best_evaluations(Board* board, Edge* edge, ProbabilityMap* pmap, 
                             int32_t* best_pos, double* best_p);

#endif