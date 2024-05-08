#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <alloca.h>
#include <math.h>

#include "statistics.h"
#include "common.h"

double gini(double p);
double entropy(double p);
void initialize_intermediate(IntermediateStatistics* intermediate, int32_t n, int32_t border_unknown_c);
void get_intermediates(Board* board, Border* border, PermutationSet* permutation_set,
                       IntermediateStatistics* main_intermediate, IntermediateStatistics* outside_intermediate,
                       IntermediateStatistics* border_intermediates);
bool get_border_probabilities(IntermediateStatistics* intermediate, double* border_probabilities, double* outside_probability);

void get_border_gini_and_information_gain(IntermediateStatistics* border_intermediates, int32_t intermediate_c, 
                                          IntermediateStatistics* outside_intermediate,
                                          double* border_gini_impurity, double* border_information_gain, 
                                          double* outside_gini_impurity, double* outside_information_gain,
                                          double* outside_p2, double* border_p2);

void get_gini_and_information_gain(Board* board, Border* border, IntermediateStatistics* border_intermediates,
                                   IntermediateStatistics* outside_intermediate,
                                   BoardStatistics* statistics);

void get_probability(Board* board, Border* border, IntermediateStatistics* intermediate, BoardStatistics* statistics);
void get_value(Board* board, BoardStatistics* statistics, double alpha, double beta);
void print_statistics(Board* board, BoardStatistics* statistics, bool p, bool p2, bool gini, bool inf_gain, bool value);
BoardStatistics* get_statistics(Board* board, Border* border, PermutationSet* permutation_set, double alpha, double beta, bool p_only);

#endif