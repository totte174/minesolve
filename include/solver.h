#ifndef SOLVER_H
#define SOLVER_H

#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "permutations.h"
#include "probability.h"

typedef struct SolverResult {
    int32_t best_search;
    int32_t best_1step;
    double total_combinations;
    bool valid;

    double p[MAX_SQUARES];
} SolverResult;

void print_probability(Board* board, SolverResult* solver_result);
void get_solver_result(Board* board, Arguments* args, SolverResult* solver_result);

#endif