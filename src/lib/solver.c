#include "solver.h"

#ifdef TRANSPOSITION_TABLE
static TranspositionTable transposition_table;

/* Initializes the transposition table. */
void transposition_table_init() {
    for (int32_t i = 0; i < HASHTABLE_SIZE; i++) {
        transposition_table.buckets[i] = -1;
    }
    transposition_table.allocated_size = 4096;
    transposition_table.current_size = 0;
    transposition_table.allocated_entries = malloc(4096 * sizeof(TranspositionTableEntry));
}

/* Frees the transposition table. */
void transposition_table_deinit() {
    free(transposition_table.allocated_entries);
}

/* Computes a hash of the solver search state. */
int32_t solver_search_state_hash(SolverSearchState* state) {
    int32_t hash = 0;
    for (int32_t i = 0; i < state->depth; i++) {
        hash = (HASH_R * hash + state->positions[i]) % HASHTABLE_SIZE;
        hash = (HASH_R * hash + state->values[i]) % HASHTABLE_SIZE;
    }
    return hash;
}

/* Recursively searches for a matching entry in the transposition table. */
TranspositionTableEntry* transposition_table_get_recursive(SolverSearchState* state,
                                                           TranspositionTableEntry* entry) {
    bool is_entry = true;
    if (entry->state.depth != state->depth) is_entry = false;
    for (int32_t j = 0; j < entry->state.depth; j++) {
        if (entry->state.positions[j] != state->positions[j]) is_entry = false;
        if (entry->state.values[j] != state->values[j]) is_entry = false;
    }
    if (is_entry) return entry;
    if (entry->next == -1) return NULL;
    return transposition_table_get_recursive(state, transposition_table.allocated_entries + entry->next);
}

/* Looks up the transposition table for a given state. */
TranspositionTableEntry* transposition_table_get(SolverSearchState* state) {
    int32_t hash = solver_search_state_hash(state);
    if (transposition_table.buckets[hash] == -1) return NULL;
    return transposition_table_get_recursive(state, transposition_table.allocated_entries + transposition_table.buckets[hash]);
}

/* Inserts a new entry into the transposition table. */
void transposition_table_set(TranspositionTableEntry* new_entry) {
    if (transposition_table.allocated_size == transposition_table.current_size) {
        transposition_table.allocated_size = transposition_table.allocated_size << 1;
        transposition_table.allocated_entries = realloc(transposition_table.allocated_entries, transposition_table.allocated_size * sizeof(TranspositionTableEntry));
    }

    int32_t hash = solver_search_state_hash(&new_entry->state);
    if (transposition_table.buckets[hash] == -1) {
        TranspositionTableEntry* entry = transposition_table.allocated_entries + transposition_table.current_size;
        transposition_table.buckets[hash] = transposition_table.current_size;
        transposition_table.current_size++;
        *entry = *new_entry;
        entry->next = -1;
    }
    else {
        TranspositionTableEntry* prev_entry = transposition_table.allocated_entries + transposition_table.buckets[hash];
        while (prev_entry->next != -1) {
            prev_entry = transposition_table.allocated_entries + prev_entry->next;
        }
        TranspositionTableEntry* entry = transposition_table.allocated_entries + transposition_table.current_size;
        prev_entry->next = transposition_table.current_size;
        transposition_table.current_size++;
        *entry = *new_entry;
        entry->next = -1;
    }
}
#endif

/* Adds a position/value pair to the search state while keeping the list sorted. */
void push_move(SolverSearchState* state, int32_t pos, int32_t val) {
    // Add position to state while keeping list sorted
    int32_t temp_pos, temp_val;
    for (int32_t i = 0; i < state->depth; i++) {
        if (state->positions[i] > pos) {
            temp_pos = pos;
            temp_val = val;
            pos = state->positions[i];
            val = state->values[i];
            state->positions[i] = temp_pos;
            state->values[i] = temp_val;
        }
    }
    state->positions[state->depth] = pos;
    state->values[state->depth] = val;
    state->depth++;
}

/* Recursively searches the game tree to find the move minimizing loss probability. */
void tree_search(MsBoard* board, SolverSearchState* state,
                 int32_t max_depth, MsResult* result, double* total_combinations,
                 Frontier* frontier, ProbabilityMap* pmap) {
    // This shouldn't happen
    if (state->depth >= max_depth) {
        *total_combinations = 1;
        result->p_loss = 0.0;
        return;
    }

    #ifdef TRANSPOSITION_TABLE
    // Return if this position is in transposition table
    TranspositionTableEntry* table_entry = transposition_table_get(state);
    if (table_entry != NULL) {
        *result = table_entry->result;
        *total_combinations = table_entry->total_combinations;
        return;
    }
    #endif


    // Get probability-map for current board
    result->status = build_pmap(board, frontier, pmap);
    *total_combinations = pmap->total_weight;

    if (result->status) {
        #ifdef TRANSPOSITION_TABLE
        TranspositionTableEntry entry = {
            .state = *state,
            .result = *result,
            .total_combinations = *total_combinations,
        };
        transposition_table_set(&entry);
        #endif
        return;
    }


    // If last step of search, only check for best probability
    if (max_depth - state->depth == 1) {
        find_safest(board, frontier, pmap, &result->p_loss, &result->move);
        #ifdef TRANSPOSITION_TABLE
        TranspositionTableEntry entry = {
            .state = *state,
            .result = *result,
            .total_combinations = *total_combinations,
        };
        transposition_table_set(&entry);
        #endif
        return;
    }



    result->p_loss = 1.0;
    result->move = -1;

    int32_t sorted_pos[MAX_FRONTIER_SIZE];
    double sorted_p[MAX_FRONTIER_SIZE];
    int32_t sorted_c = rank_candidates(board, frontier, pmap,
                                       sorted_pos, sorted_p);

    if (sorted_p[0] == 0.0){
        if (state->depth == 0) {
            result->p_loss = 0.0;
            result->move = sorted_pos[0];
            return;
        }
        else {
            int32_t zero_count = 0;
            while (zero_count < sorted_c) {
                if (sorted_p[zero_count] != 0.0) break;
                zero_count++;
            }
            if (zero_count == max_depth - state->depth) {
                result->p_loss = 0.0;
                result->move = sorted_pos[0];
                return;
            }
        }
        sorted_c = 1;
    }


    MsBoard new_board = *board;
    new_board.unrevealed_c--;
    Frontier new_frontier;
    ProbabilityMap new_pmap;
    for (int32_t i = 0; i < sorted_c; i++) {
        int32_t pos = sorted_pos[i];
        double p = sorted_p[i];
        if (p >= result->p_loss) break;

        new_board.revealed[pos] = true;

        double total_combs = 0;
        double avg_p = 0.0;
        bool any_computational_limit = false;
        int32_t adjacent_unknown_c = get_adjacent_unknown_c(&new_board, pos);

        for (int32_t v = 0; v <= adjacent_unknown_c; v++) {
            new_board.hint[pos] = v;
            MsResult new_result;
            double new_total_combinations;
            SolverSearchState new_state = *state;
            push_move(&new_state, pos, v);

            tree_search(&new_board, &new_state, max_depth, &new_result, &new_total_combinations, &new_frontier, &new_pmap);
            if (new_result.status == MS_ERR_COMPUTATIONAL_LIMIT) any_computational_limit = true;
            if (!new_result.status) {
                avg_p += new_result.p_loss * new_total_combinations;
                total_combs += new_total_combinations;
            }
        }

        if (total_combs == 0.0) {
            if (any_computational_limit) result->status = MS_ERR_COMPUTATIONAL_LIMIT;
            else result->status = MS_ERR_INVALID_BOARD;
            break;
        }

        avg_p /= total_combs;
        new_board.revealed[pos] = false;

        double new_p = 1.0 - (1.0 - p) * (1.0 - avg_p);
        if (new_p < result->p_loss) {
            result->p_loss = new_p;
            result->move = pos;
        }
    }

    #ifdef TRANSPOSITION_TABLE
    TranspositionTableEntry entry = {
        .state = *state,
        .result = *result,
        .total_combinations = *total_combinations,
    };
    transposition_table_set(&entry);
    #endif
}

void get_solver_result(MsBoard* board, int32_t max_depth, MsResult* result, double* p_a) {
    #ifdef TRANSPOSITION_TABLE
    transposition_table_init();
    #endif

    int32_t safe_sqs_left = board->unrevealed_c - board->mine_count;

    if (board->unrevealed_c == board->w * board->h) max_depth = 1;
    else max_depth = min(max_depth, safe_sqs_left);

    static SolverSearchState state;
    static Frontier frontier;
    static ProbabilityMap pmap;
    double total_combinations;
    state.depth = 0;
    tree_search(board, &state, max_depth, result, &total_combinations, &frontier, &pmap);

    if (p_a != NULL) pmap_to_board(board, &frontier, &pmap, p_a);

    #ifdef TRANSPOSITION_TABLE
    transposition_table_deinit();
    #endif
}

void get_solver_result_basic(MsBoard* board, MsResult* result, double* p_a) {
    static Frontier frontier;
    static ProbabilityMap pmap;
    result->status = build_pmap_basic(board, &frontier, &pmap);
    find_safest(board, &frontier, &pmap, &result->p_loss, &result->move);

    if (p_a != NULL) pmap_to_board(board, &frontier, &pmap, p_a);
}

MsStatus get_board_probabilities(MsBoard* board, double* p_a) {
    static Frontier frontier;
    static ProbabilityMap pmap;
    MsStatus status = build_pmap(board, &frontier, &pmap);
    if (status) return status;

    pmap_to_board(board, &frontier, &pmap, p_a);
    return status;
}
