#include "solver.h"

double tree_search(Board* board, int32_t next_move, int32_t max_depth, double cur_p, double cur_best_p) {
    if (max_depth <= 0) return cur_p;
    if (next_move < 0) return 1.0;

    Edge edge;
    PermutationSet permutation_set;
    ProbabilityMap pmap;
    Board new_board = *board;
    new_board.known[next_move] = true;
    
    double p_v[9];
    double combs_v[9];
    for (int32_t v = 0; v < 9; v++) {
        new_board.v[next_move] = v;
        double best_p = cur_best_p;

        get_permutation_set(&new_board, &edge, &permutation_set, &pmap);
        get_pmap(&new_board, &edge, &permutation_set, &pmap);
        destroy_permutations(&permutation_set);
        if (!pmap.valid) {
            combs_v[v] = 0;
            continue;
        }

        // Sort by probability
        int32_t sorted_edge[MAX_EDGE_SIZE];
        for (int32_t i = 0; i < edge.edge_c; i++) sorted_edge[i] = i;
        for (int32_t i = 0; i < edge.edge_c-1; i++){
            int j_min = i;
            for (int32_t j = i+1; j < edge.edge_c; j++){
                if (pmap.p_edge[sorted_edge[j]] < pmap.p_edge[sorted_edge[j_min]]) j_min = j;
            }
            if (j_min != i){
                int32_t temp = sorted_edge[i];
                sorted_edge[i] = sorted_edge[j_min];
                sorted_edge[j_min] = temp;
            }
        }

        for(int32_t i = 0; i < edge.edge_solved_c; i++) {
            if (pmap.p_solved[i] == 0.0) {
                best_p = min(best_p, tree_search(&new_board, edge.edge_solved[i], max_depth-1, cur_p, best_p));
            }
        }
        for(int32_t i_ = 0; i_ < edge.edge_c; i_++) {
            int32_t i = sorted_edge[i_];

            double next_p = 1 - (1 - cur_p) * (1 - pmap.p_edge[i]);
            if (next_p >= best_p) break;

            best_p = min(best_p, tree_search(&new_board, edge.edge[i], max_depth-1, next_p, best_p));
        }
        if (edge.exterior_c > 0) {
            double next_p = 1 - (1 - cur_p) * (1 - pmap.p_exterior);
            if (next_p < best_p) {

                //Find exterior point with lowest surrounding adjacent unknown (usually corners)
                int32_t best_exterior = -1;
                int32_t lowest_adjacent_unknown = 9;
                int32_t adj[8];
                for (int32_t i = 0; i < edge.exterior_c; i++) {
                    int32_t adj_c = get_adjacent_unknown(&new_board, edge.exterior[i], adj);
                    if (adj_c < lowest_adjacent_unknown) {
                        lowest_adjacent_unknown = adj_c;
                        best_exterior = edge.exterior[i];
                    }
                }
                
                best_p = min(best_p, tree_search(&new_board, best_exterior, max_depth-1, next_p, best_p));
            }
        }
        
        p_v[v] = best_p;
        combs_v[v] = pmap.comb_total;
    }
    
    double total_combs = 0;
    double avg_p = 0.0;
    for (int32_t v = 0; v < 9; v++) {
        avg_p += p_v[v] * combs_v[v];
        total_combs += combs_v[v];
    }
    if (total_combs == 0) return 1.0;
    avg_p /= total_combs;
    return avg_p;
}

void print_probability(Board* board, SolverResult* solver_result) {
    for (int32_t y = 0; y < board->h; y++) {
        for (int32_t x = 0; x < board->w; x++) {
            if (board->known[y*board->w + x]) printf("      ");
            else printf("%.3f ", solver_result->p[y*board->w + x]);
        }
        printf("\n");
    }
}

void get_solver_result(Board* board, Arguments* args, SolverResult* solver_result) {
    static Edge edge;
    static PermutationSet permutation_set;
    static ProbabilityMap pmap;
    solver_result->valid = true;

    get_permutation_set(board, &edge, &permutation_set, &pmap);
    get_pmap(board, &edge, &permutation_set, &pmap);
    destroy_permutations(&permutation_set);
    solver_result->best_1step = pmap_to_board(board, &edge, &pmap, solver_result->p);
    if (!pmap.valid) {
        solver_result->valid = false;
        return;
    }

    int32_t sorted_edge[MAX_EDGE_SIZE];
    for (int32_t i = 0; i < edge.edge_c; i++) sorted_edge[i] = i;
    for (int32_t i = 0; i < edge.edge_c-1; i++){
        int j_min = i;
        for (int32_t j = i+1; j < edge.edge_c; j++){
            if (pmap.p_edge[sorted_edge[j]] < pmap.p_edge[sorted_edge[j_min]]) j_min = j;
        }
        if (j_min != i){
            int32_t temp = sorted_edge[i];
            sorted_edge[i] = sorted_edge[j_min];
            sorted_edge[j_min] = temp;
        }
    }
    //printf("pmap exterior:%f\n", pmap.p_exterior);
    //printf("edge_c: %d\n", edge.edge_c);
    //printf("solved_c: %d\n", edge.edge_solved_c);
    //printf("exterior_c: %d\n", edge.exterior_c);

    int32_t max_depth = min(MAX_SEARCH_DEPTH, board->unknown_c - board->mine_c);

    double best_p = 1.00001;
    solver_result->best_search = -1;
    for(int32_t i = 0; i < edge.edge_solved_c; i++) {
        if (pmap.p_solved[i] == 0.0) {
            best_p = 0.0;
            solver_result->best_search = edge.edge_solved[i];
            break;
        }
    }
    for(int32_t i_ = 0; i_ < edge.edge_c; i_++) {
        int32_t i = sorted_edge[i_];
        if (pmap.p_edge[i] >= best_p) break;
        double new_p = tree_search(board, edge.edge[i], max_depth - 1, pmap.p_edge[i], best_p);
        if(new_p < best_p) {
            best_p = new_p;
            solver_result->best_search = edge.edge[i];
        }
    }
    if (pmap.p_exterior < best_p && edge.exterior_c > 0) {

        //Find exterior point with lowest surrounding adjacent unknown (usually corners)
        int32_t best_exterior = -1;
        int32_t lowest_adjacent_unknown = 9;
        int32_t adj[8];
        for (int32_t i = 0; i < edge.exterior_c; i++) {
            int32_t adj_c = get_adjacent_unknown(board, edge.exterior[i], adj);
            if (adj_c < lowest_adjacent_unknown) {
                lowest_adjacent_unknown = adj_c;
                best_exterior = edge.exterior[i];
            }
        }
        
        double new_p = tree_search(board, best_exterior, max_depth - 1, pmap.p_exterior, best_p);
        if(new_p < best_p) {
            best_p = new_p;
            solver_result->best_search = best_exterior;
        }
    }

    if (solver_result->best_search == -1) {
        solver_result->valid = false;
    }
}