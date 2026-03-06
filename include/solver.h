#ifndef SOLVER_H
#define SOLVER_H

#include "math.h"
#include "common.h"
#include "probability.h"

typedef struct SolverSearchState {
    int32_t search_depth;
    int32_t prev_positions[MAX_SEARCH];
    int32_t prev_val[MAX_SEARCH];
} SolverSearchState;

#ifdef TRANSPOSITION_TABLE
#define HASH_R 31
#define HASHTABLE_SIZE 8192*4

typedef struct TranspositionTableEntry {
    SolverSearchState state;
    MsResult result;
    double total_combinations;
    int32_t next;
} TranspositionTableEntry;

typedef struct TranspositionTable {
    int32_t buckets[HASHTABLE_SIZE];
    int32_t allocated_size, current_size;
    TranspositionTableEntry* allocated_entries;
} TranspositionTable;
#endif

void get_solver_result(MsBoard* board, int32_t max_depth, MsResult* result, double* p_a);
void get_solver_result_basic(MsBoard* board, MsResult* result, double* p_a);
MsStatus get_board_probabilities(MsBoard* board, double* p_a);

#endif