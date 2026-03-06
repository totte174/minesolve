#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "common.h"
#include "equations.h"
#include "permutations.h"
#include "board.h"

MsStatus get_pmap(MsBoard* board, Edge* edge, ProbabilityMap* pmap);
MsStatus get_pmap_basic(MsBoard* board, Edge* edge, ProbabilityMap* pmap);
void pmap_to_board(MsBoard* board, Edge* edge, ProbabilityMap* pmap, double* prob_a);
void get_lowest_probability(MsBoard* board, Edge* edge, ProbabilityMap* pmap, 
                            double* p, int32_t* pos);
int32_t get_best_evaluations(MsBoard* board, Edge* edge, ProbabilityMap* pmap, 
                             int32_t* best_pos, double* best_p);

#endif