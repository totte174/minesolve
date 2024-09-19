#ifndef SOLVER_H
#define SOLVER_H

#include <stdint.h>
#include <stdbool.h>

#include "math.h"
#include "common.h"
#include "permutations.h"
#include "probability.h"

typedef struct SolverSearchState {
    int32_t search_depth;
    int32_t prev_positions[MAX_SEARCH];
    int32_t prev_val[MAX_SEARCH];
} SolverSearchState;

typedef struct SearchResult {
    double p;
    double total_combinations;
    int32_t best_move;
} SearchResult;

#ifdef TRANSPOSITION_TABLE
#define HASH_R 31
#define HASHTABLE_SIZE 8192*4

typedef struct TranspositionTableEntry TranspositionTableEntry;
struct TranspositionTableEntry {
    SolverSearchState state;
    SearchResult result;
    int32_t next;
};

typedef struct TranspositionTable {
    int32_t buckets[HASHTABLE_SIZE];
    int32_t allocated_size, current_size;
    TranspositionTableEntry* allocated_entries;
} TranspositionTable;
#endif

typedef struct SolverResult {
    int32_t best_search;
    int32_t best_1step;
    double total_combinations;
    bool valid;

    double p[MAX_SQUARES];
} SolverResult;

void get_solver_result(Board* board, Arguments* args, SolverResult* solver_result);

#endif