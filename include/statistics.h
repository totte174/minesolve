#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <alloca.h>
#include <math.h>

#include "permutations.h"
#include "common.h"
#include "board.h"

double gini(double p);
double entropy(double p);
void initialize_pmap(ProbabilityMap* pmap, int32_t n, int32_t border_unknown_c);
void get_border_gini_and_information_gain(ProbabilityMap* border_pmaps, int32_t pmap_c, 
                                          ProbabilityMap* outside_pmap,
                                          double* border_gini_impurity, double* border_information_gain, 
                                          double* outside_gini_impurity, double* outside_information_gain,
                                          double* outside_p2, double* border_p2);

void get_gini_and_information_gain(Board* board, Border* border, ProbabilityMap* border_pmaps,
                                   ProbabilityMap* outside_pmap,
                                   BoardStatistics* statistics);

void get_probability(Board* board, Border* border, ProbabilityMap* pmap, BoardStatistics* statistics);
void get_value(Board* board, BoardStatistics* statistics, double alpha, double beta, double eta);
void print_statistics(Board* board, BoardStatistics* statistics, bool p, bool p2, bool gini, bool inf_gain, bool value);
BoardStatistics* get_statistics(Board* board, Border* border, PermutationSet* permutation_set, Arguments* args);

#endif