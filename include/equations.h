#ifndef EQUATIONS_H
#define EQUATIONS_H

#include "common.h"
#include "board.h"

typedef struct Equation {
    Mask mask;
    int32_t mine_count;
} Equation;

typedef struct EquationSet {
    Equation equations[MAX_SQUARES];
    int32_t equation_c;
    int32_t group_length[MAX_FRONTIER_SIZE];
    int32_t group_start[MAX_FRONTIER_SIZE];
    int32_t group_c;

    Mask solved_mask;
    Mask solved_mines;
} EquationSet;

/** Builds and fully reduces the equation set for the current board state. */
MsStatus build_equation_set(MsBoard* board, Frontier* frontier, EquationSet* equation_set, ProbabilityMap* pmap);

#endif
