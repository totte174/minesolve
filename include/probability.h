#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "common.h"
#include "equations.h"
#include "solutions.h"
#include "board.h"

/** Computes per-square mine probabilities by enumerating all valid solutions. */
MsStatus build_pmap(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap);
/** Computes approximate probabilities using equations only (no enumeration). */
MsStatus build_pmap_basic(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap);
/** Writes pmap probabilities into a flat array indexed by square. */
void pmap_to_board(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap, double* prob_a);
/** Returns the square and probability with lowest mine risk. */
void find_safest(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap,
                 double* p, int32_t* pos);
/** Returns candidate moves sorted by mine probability ascending. */
int32_t rank_candidates(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap,
                        int32_t* best_pos, double* best_p);

#endif
