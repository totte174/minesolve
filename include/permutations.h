#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "common.h"

void set_equations(Board* board, Border* border, EquationSet* equation_set);
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
void join_permutations_noconflict(Permutation* new_permutation, Permutation* permutation1, Permutation* permutation2);
int32_t join_permutation_arrays(Permutation* permutations1, int32_t permutations1_c, Permutation* permutations2, int32_t permutations2_c);
void permutations_of_splits(EquationSet* equation_set, PermutationSet* permutation_set);
void solved_permutation(EquationSet* equation_set, Permutation* permutation);
void print_permutation(int32_t border_unknown_c, Permutation* permutation);
void print_permutation_set(int32_t border_unknown_c, PermutationSet* permutation_set);
PermutationSet* get_permutations(Board* board, Border* border);

#endif