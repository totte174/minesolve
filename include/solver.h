#ifndef SOLVER_H
#define SOLVER_H

#include "math.h"
#include "common.h"
#include "probability.h"

typedef struct SolverSearchState {
    int32_t depth;
    int32_t positions[MAX_SEARCH];
    int32_t values[MAX_SEARCH];
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

/** Finds the best move using depth-limited lookahead search. */
void get_solver_result(MsBoard* board, int32_t max_depth, MsResult* result, double* p_a);
/** Finds the best move using single-depth probability only. */
void get_solver_result_basic(MsBoard* board, MsResult* result, double* p_a);
/** Computes per-square mine probabilities for the current board. */
MsStatus get_board_probabilities(MsBoard* board, double* p_a);

#endif
