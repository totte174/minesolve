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
void equation_permutations(Equation* equation, PermutationSet* permutation_set);
bool permutation_intersect(Permutation* permutation1, Permutation* permutation2);
bool join_permutations(Permutation* new_permutation, Permutation* permutation1, Permutation* permutation2);
void join_permutationsets(PermutationSet* permutation_set1, PermutationSet* permutation_set2);
void permutations_of_split(EquationSet* equation_set, int32_t split_i, PermutationSet* permutation_set);
void print_permutations(int32_t border_unknown_c, PermutationSet* permutation_set);
PermutationSet* get_permutations(Board* board, Border* border);

#endif