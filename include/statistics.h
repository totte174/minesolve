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

double choose(double n, double k);
double gini(double p);
double entropy(double p);
double combinations_from_kcount(KCount* kcount, int32_t n);
void initialize_intermediate(IntermediateStatistics* intermediate, int32_t n, int32_t border_unknown_c);
void get_intermediates(Board* board, Border* border, PermutationSet* permutation_set,
                       IntermediateStatistics* main_intermediate, IntermediateStatistics* outside_intermediate,
                       IntermediateStatistics* border_intermediates);
bool get_border_probabilities(IntermediateStatistics* intermediate, double* border_probabilities, double* outside_probability);

void get_border_gini_and_information_gain(IntermediateStatistics* border_intermediates, int32_t intermediate_c, 
                                          IntermediateStatistics* outside_intermediate,
                                          double* border_gini_impurity, double* border_information_gain, 
                                          double* outside_gini_impurity, double* outside_information_gain);

void get_gini_and_information_gain(Board* board, Border* border, IntermediateStatistics* border_intermediates,
                                   IntermediateStatistics* outside_intermediate,
                                   double* gini_impurity, double* information_gain);

void get_probability(Board* board, Border* border, IntermediateStatistics* intermediate, double* p);
void print_statistics(Board* board, BoardStatistics* statistics, bool p, bool gini, bool inf_gain, bool value);
BoardStatistics* get_statistics(Board* board, Border* border, PermutationSet* permutation_set);

#endif