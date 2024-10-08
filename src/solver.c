#include "solver.h"

#ifdef TRANSPOSITION_TABLE
static TranspositionTable transposition_table;

void transposition_table_init() {
    for (int32_t i = 0; i < HASHTABLE_SIZE; i++) {
        transposition_table.buckets[i] = -1;
    }    
    transposition_table.allocated_size = 4096;
    transposition_table.current_size = 0;
    transposition_table.allocated_entries = malloc(4096 * sizeof(TranspositionTableEntry));
}

void transposition_table_deinit() {
    free(transposition_table.allocated_entries);
}

int32_t solver_search_state_hash(SolverSearchState* state) {
    int32_t hash = 0;
    for (int32_t i = 0; i < state->search_depth; i++) {
        hash = (HASH_R * hash + state->prev_positions[i]) % HASHTABLE_SIZE;
        hash = (HASH_R * hash + state->prev_val[i]) % HASHTABLE_SIZE;
    }
    return hash;
}

TranspositionTableEntry* transposition_table_get_recursive(SolverSearchState* state, 
                                                           TranspositionTableEntry* entry) {
    bool is_entry = true;
    if (entry->state.search_depth != state->search_depth) is_entry = false;
    for (int32_t j = 0; j < entry->state.search_depth; j++) {
        if (entry->state.prev_positions[j] != state->prev_positions[j]) is_entry = false;
        if (entry->state.prev_val[j] != state->prev_val[j]) is_entry = false;
    }
    if (is_entry) return entry;
    if (entry->next == -1) return NULL;
    return transposition_table_get_recursive(state, transposition_table.allocated_entries + entry->next);
}

TranspositionTableEntry* transposition_table_get(SolverSearchState* state) {
    int32_t hash = solver_search_state_hash(state);
    if (transposition_table.buckets[hash] == -1) return NULL;
    return transposition_table_get_recursive(state, transposition_table.allocated_entries + transposition_table.buckets[hash]);
}

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

void state_add_position(SolverSearchState* state, int32_t pos, int32_t val) {
    // Add position to state while keeping list sorted
    int32_t temp_pos, temp_val;
    for (int32_t i = 0; i < state->search_depth; i++) {
        if (state->prev_positions[i] > pos) {
            temp_pos = pos;
            temp_val = val;
            pos = state->prev_positions[i];
            val = state->prev_val[i];
            state->prev_positions[i] = temp_pos;
            state->prev_val[i] = temp_val;
        }
    }
    state->prev_positions[state->search_depth] = pos;
    state->prev_val[state->search_depth] = val;
    state->search_depth++;
}

void tree_search(Board* board, SolverSearchState* state,
                 int32_t max_depth, SearchResult* result,
                 Edge* edge, ProbabilityMap* pmap) {
    // This shouldn't happen
    if (state->search_depth >= max_depth) {
        result->total_combinations = 1;
        result->p = 0.0;
        return;
    }

    #ifdef TRANSPOSITION_TABLE
    // Return if this position is in transposition table
    TranspositionTableEntry* table_entry = transposition_table_get(state);
    if (table_entry != NULL) {
        *result = table_entry->result;
        return;
    }
    #endif


    // Get probability-map for current board
    result->fault_status = get_pmap(board, edge, pmap);
    result->total_combinations = pmap->comb_total;

    if (result->fault_status) {
        #ifdef TRANSPOSITION_TABLE
        TranspositionTableEntry entry = {
            .state = *state,
            .result = *result,
        };
        transposition_table_set(&entry);
        #endif
        return;
    }


    // If last step of search, only check for best probability
    if (max_depth - state->search_depth == 1) {
        get_lowest_probability(board, edge, pmap, &result->p, &result->best_move);
        #ifdef TRANSPOSITION_TABLE
        TranspositionTableEntry entry = {
            .state = *state,
            .result = *result,
        };
        transposition_table_set(&entry);
        #endif
        return;
    }



    result->p = 1.0;
    result->best_move = -1;

    int32_t sorted_pos[MAX_EDGE_SIZE];
    double sorted_p[MAX_EDGE_SIZE];
    int32_t sorted_c = get_best_evaluations(board, edge, pmap, 
                                            sorted_pos, sorted_p);
    
    if (sorted_p[0] == 0.0){
        if (state->search_depth == 0) {
            result->p = 0.0;
            result->best_move = sorted_pos[0];
            return;
        }
        else {
            int32_t zero_count = 0;
            while (zero_count < sorted_c) {
                if (sorted_p[zero_count] != 0.0) break;
                zero_count++;
            }
            if (zero_count == max_depth - state->search_depth) {
                result->p = 0.0;
                result->best_move = sorted_pos[0];
                return;
            }
        }
        sorted_c = 1;
    }
    

    Board new_board = *board;
    new_board.unknown_c--;
    Edge new_edge;
    ProbabilityMap new_pmap;
    for (int32_t i = 0; i < sorted_c; i++) {
        int32_t pos = sorted_pos[i];
        double p = sorted_p[i];
        if (p >= result->p) break;

        new_board.known[pos] = true;

        double total_combs = 0;
        double avg_p = 0.0;
        bool any_computational_limit = false;
        int32_t adjacent_unknown_c = get_adjacent_unknown_c(&new_board, pos);

        for (int32_t v = 0; v <= adjacent_unknown_c; v++) {
            new_board.v[pos] = v;
            SearchResult new_result;
            SolverSearchState new_state = *state;
            state_add_position(&new_state, pos, v);
            
            tree_search(&new_board, &new_state, max_depth, &new_result, &new_edge, &new_pmap);
            if (new_result.fault_status == fault_computational_limit) any_computational_limit = true;
            if (!new_result.fault_status) {
                avg_p += new_result.p * new_result.total_combinations;
                total_combs += new_result.total_combinations;
            }
        }

        if (total_combs == 0.0) {
            if (any_computational_limit) result->fault_status = fault_computational_limit;
            else result->fault_status = fault_invalid_board;
            break;
        }

        avg_p /= total_combs;
        new_board.known[pos] = false;

        double new_p = 1.0 - (1.0 - p) * (1.0 - avg_p);
        if (new_p < result->p) {
            result->p = new_p;
            result->best_move = pos;
        }
    }

    #ifdef TRANSPOSITION_TABLE
    TranspositionTableEntry entry = {
        .state = *state,
        .result = *result,
    };
    transposition_table_set(&entry);
    #endif
}

void get_solver_result(Board* board, Arguments* args, SearchResult* result, double* p_a) {
    #ifdef TRANSPOSITION_TABLE
    transposition_table_init();
    #endif

    int32_t safe_sqs_left = board->unknown_c - board->mine_c;
    int32_t max_depth;

    if (board->unknown_c == board->w * board->h) max_depth = 1;
    else max_depth = min(args->max_depth, safe_sqs_left);

    static SolverSearchState state;
    static Edge edge;
    static ProbabilityMap pmap;
    state.search_depth = 0;
    tree_search(board, &state, max_depth, result, &edge, &pmap);

    if (p_a != NULL) pmap_to_board(board, &edge, &pmap, p_a);

    #ifdef TRANSPOSITION_TABLE
    transposition_table_deinit();
    #endif
}

void get_solver_result_basic(Board* board, Arguments* args, SearchResult* result, double* p_a) {
    static Edge edge;
    static ProbabilityMap pmap;
    result->fault_status = get_pmap_basic(board, &edge, &pmap);
    result->total_combinations = 0.0;
    get_lowest_probability(board, &edge, &pmap, &result->p, &result->best_move);

    if (p_a != NULL) pmap_to_board(board, &edge, &pmap, p_a);
}

FaultStatus get_board_probabilities(Board* board, double* p_a) {
    static Edge edge;
    static ProbabilityMap pmap;
    FaultStatus fault_status = get_pmap(board, &edge, &pmap);
    if (fault_status) return fault_status;

    pmap_to_board(board, &edge, &pmap, p_a);
    return fault_status;
}
