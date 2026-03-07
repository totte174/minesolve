#ifndef PERMUTATIONS_H
#define PERMUTATIONS_H

#include <alloca.h>
#include <malloc.h>

#include "common.h"
#include "equations.h"

#define MIN_PERMUTATIONS (1ULL << 22ULL)
#define MAX_PERMUTATIONS (1ULL << 26ULL)

typedef struct SolutionSet {
    Mask* solutions;
    int32_t solution_size;
    int32_t solution_c;
    bool initialized;

    int32_t group_length[MAX_FRONTIER_SIZE];
    int32_t group_start[MAX_FRONTIER_SIZE];
    int32_t group_c;
} SolutionSet;

/** Returns the number of mines set in solution mask. */
int32_t count_mines(Mask* solution);
/** Builds the complete set of valid mine assignments for the frontier. */
MsStatus build_solution_set(MsBoard* board, Frontier* frontier, SolutionSet* solution_set, ProbabilityMap* pmap);
/** Frees the solution set heap allocation. */
void solution_set_free(SolutionSet* solution_set);

#endif
