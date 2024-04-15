#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "common.h"

void set_equations(Board* board, Border* border, Equation** equations);
bool is_subequation(Equation* eq, Equation* super_eq);
bool equations_intersect(Equation* eq1, Equation* eq2);
void remove_subequation(Equation* eq, Equation* super_eq);
int32_t remove_solved(Equation** equations, int32_t equation_c, int32_t* solved);
bool remove_subequations(Equation** equations, int32_t equation_c);
int32_t reduce(Equation** equations, int32_t equation_c, int32_t* solved);
int32_t split(Equation** equations, int32_t equation_c, int32_t* splits);
void equation_permutations(Equation* equation, PermutationSet* permutation_set);
bool permutation_intersect(Permutation* permutation1, Permutation* permutation2);
bool join_permutations(Permutation* new_permutation, Permutation* permutation1, Permutation* permutation2);
void join_permutationsets(PermutationSet* permutation_set1, PermutationSet* permutation_set2);
void permutations_of_split(Equation** equations, int32_t equation_start, int32_t equation_end, PermutationSet* permutation_set);
void print_permutations(int32_t border_unknown_c, PermutationSet* permutation_set);
void get_permutations(Board* board, Border* border, PermutationSet* permutation_set);

#endif