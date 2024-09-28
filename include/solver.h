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

typedef struct SearchResult {
    double p;
    double total_combinations;
    int32_t best_move;
    FaultStatus fault_status;
} SearchResult;

#ifdef TRANSPOSITION_TABLE
#define HASH_R 31
#define HASHTABLE_SIZE 8192*4

typedef struct TranspositionTableEntry {
    SolverSearchState state;
    SearchResult result;
    int32_t next;
} TranspositionTableEntry;

typedef struct TranspositionTable {
    int32_t buckets[HASHTABLE_SIZE];
    int32_t allocated_size, current_size;
    TranspositionTableEntry* allocated_entries;
} TranspositionTable;
#endif

void get_solver_result(Board* board, Arguments* args, SearchResult* result);
void get_solver_result_and_board_probabilities(Board* board, Arguments* args, SearchResult* result, double* p_a);
FaultStatus get_board_probabilities(Board* board, double* p_a);

#endif