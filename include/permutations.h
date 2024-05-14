#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "common.h"

void get_equations(Board* board, Border* border, EquationSet* equation_set);
void print_equation(Equation* eq, int32_t border_unknown_c);
void print_equation_set(EquationSet* equation_set);
bool is_subequation(Equation* eq, Equation* super_eq);
bool equations_intersect(Equation* eq1, Equation* eq2);
void remove_subequation(Equation* eq, Equation* super_eq);
bool remove_solved(EquationSet* equation_set);
bool remove_subequations(EquationSet* equation_set);
void reduce(EquationSet* equation_set);
void split(EquationSet* equation_set);
int32_t equation_permutations(Equation* equation, Permutation* permutations);
bool permutation_intersect(Permutation* permutation1, Permutation* permutation2);
bool join_permutations(Permutation* new_permutation, Permutation* permutation1, Permutation* permutation2);
int32_t join_permutation_arrays(Permutation* permutations1, int32_t permutations1_c, Permutation* permutations2, int32_t permutations2_c);
void permutations_of_splits(EquationSet* equation_set, PermutationSet* permutation_set);
int32_t mine_c(Permutation* permutation);
void print_permutation(Permutation* permutation, int32_t border_unknown_c);
void print_permutation_set(PermutationSet* permutation_set, int32_t border_unknown_c);
PermutationSet* get_permutations(Board* board, Border* border);

#endif