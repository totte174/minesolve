#ifndef EQUATIONS_H
#define EQUATIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "common.h"
#include "board.h"

typedef struct Equation {
    Mask mask;
    int32_t amount;
} Equation;


typedef struct EquationSet {
    Equation equations[MAX_SQUARES];
    int32_t equation_c;
    int32_t splits_length[MAX_EDGE_SIZE];
    int32_t splits_start[MAX_EDGE_SIZE];
    int32_t split_c;

    Mask solved_mask;
    Mask solved_mines;

    bool valid;
} EquationSet;

void get_equation_set(Board* board, Edge* edge, EquationSet* equation_set, ProbabilityMap* pmap);

#endif