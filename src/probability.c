#include "probability.h"

double choose(double n, double k) {
    if (n < k) return 0;
    if (k == 0) return 1;
    return (n * choose(n - 1, k - 1)) / k;
}

FaultStatus get_pmap(Board* board, Edge* edge, ProbabilityMap* pmap) {
    static PermutationSet permutation_set;
    FaultStatus fault_status = valid_status;
    fault_status = get_permutation_set(board, edge, &permutation_set, pmap);
    if (fault_status) {
        permutation_set_deinit(&permutation_set);
        return fault_status;
    }

    static int32_t split_mine_c_min[MAX_EDGE_SIZE];
    static int32_t split_mine_c_max[MAX_EDGE_SIZE];
    int32_t n = edge->exterior_c;

    if(n < 0) {
        permutation_set_deinit(&permutation_set);
        fault_status = fault_invalid_board;
        return fault_status;
    }

    pmap->comb_total = 0;
    pmap->p_exterior = 0;
    for (int32_t i = 0; i < edge->edge_c; i++){
        pmap->p_edge[i] = 0;
    }

    int32_t solved_mines = 0;
    for (int32_t i = 0; i < edge->edge_solved_c; i++) {
        solved_mines += (int32_t) pmap->p_solved[i];
    }
    int32_t total_mine_c_min = solved_mines;
    int32_t total_mine_c_max = solved_mines;
    for (int32_t split_i = 0; split_i < permutation_set.split_c; split_i++) 
    {
        split_mine_c_min[split_i] = MAX_MINES;
        split_mine_c_max[split_i] = 0;
        for (int32_t i = 0; i < permutation_set.splits_length[split_i]; i++){
            Mask* perm = permutation_set.permutations + permutation_set.splits_start[split_i] + i;
            
            split_mine_c_min[split_i] = min(split_mine_c_min[split_i], mine_c(perm));
            split_mine_c_max[split_i] = max(split_mine_c_max[split_i], mine_c(perm));
        }
        if (split_mine_c_max[split_i] < split_mine_c_min[split_i]) { //Not valid permutation in split -> not valid pmap
            permutation_set_deinit(&permutation_set);
            fault_status = fault_invalid_board;
            return fault_status;
        }
        total_mine_c_min += split_mine_c_min[split_i];
        total_mine_c_max += split_mine_c_max[split_i];
    }

    if (total_mine_c_max - total_mine_c_min >= MAX_MINE_C_DIFF) {
        permutation_set_deinit(&permutation_set);
        fault_status = fault_internal_limit;
        return fault_status;
    }
    double rel_mine_c_p_edge[MAX_MINE_C_DIFF][MAX_EDGE_SIZE] = {0};
    double rel_mine_c_total_combs[MAX_MINE_C_DIFF] = {0};
    rel_mine_c_total_combs[0] = 1;
    int32_t cur_max_rel_mine_c = 0;

    for (int32_t split_i = 0; split_i < permutation_set.split_c; split_i++) {
        double split_rel_mine_c_p_edge[MAX_MINE_C_DIFF][MAX_EDGE_SIZE] = {0};
        double split_rel_mine_c_total_combs[MAX_MINE_C_DIFF] = {0};        

        for (int32_t i = 0; i < permutation_set.splits_length[split_i]; i++) {
            Mask* perm = permutation_set.permutations + permutation_set.splits_start[split_i] + i;

            int32_t rel_mine_c = mine_c(perm) - split_mine_c_min[split_i];
            split_rel_mine_c_total_combs[rel_mine_c] += 1;

            for (int32_t j = 0; j < edge->splits_length[split_i]; j++) {
                if (mask_get(*perm, j)) {
                    split_rel_mine_c_p_edge[rel_mine_c][edge->splits_start[split_i] + j] += 1.0;
                }
            }    
        }

        double temp_rel_mine_c_p_edge[MAX_MINE_C_DIFF][MAX_EDGE_SIZE];
        double temp_rel_mine_c_total_combs[MAX_MINE_C_DIFF];
        for (int32_t rel_mine_c = 0; rel_mine_c <= cur_max_rel_mine_c; rel_mine_c++) {
            for (int32_t i = 0; i < edge->edge_c; i++) {
                temp_rel_mine_c_p_edge[rel_mine_c][i] = rel_mine_c_p_edge[rel_mine_c][i];
                rel_mine_c_p_edge[rel_mine_c][i] = 0;
            }
            temp_rel_mine_c_total_combs[rel_mine_c] = rel_mine_c_total_combs[rel_mine_c];
            rel_mine_c_total_combs[rel_mine_c] = 0;
        }

        for (int32_t rel_mine_c = 0; rel_mine_c <= cur_max_rel_mine_c; rel_mine_c++) {
            for (int32_t split_rel_mine_c = 0; split_rel_mine_c <= split_mine_c_max[split_i] - split_mine_c_min[split_i]; split_rel_mine_c++) {
                for (int32_t i = 0; i < edge->edge_c; i++) {
                    rel_mine_c_p_edge[rel_mine_c + split_rel_mine_c][i] += 
                        temp_rel_mine_c_p_edge[rel_mine_c][i] * split_rel_mine_c_total_combs[split_rel_mine_c] +
                        split_rel_mine_c_p_edge[split_rel_mine_c][i] * temp_rel_mine_c_total_combs[rel_mine_c];
                }
                rel_mine_c_total_combs[rel_mine_c + split_rel_mine_c] += temp_rel_mine_c_total_combs[rel_mine_c] * split_rel_mine_c_total_combs[split_rel_mine_c];
            }
        }

        cur_max_rel_mine_c += split_mine_c_max[split_i] - split_mine_c_min[split_i];
    }

    int32_t total_mine_c_min_valid = max(total_mine_c_min, board->mine_c - n);
    int32_t total_mine_c_max_valid = min(total_mine_c_max, board->mine_c);
    double rel_mine_c_weighting[MAX_MINE_C_DIFF];
    rel_mine_c_weighting[total_mine_c_min_valid - total_mine_c_min] = 1.0;
    for (int32_t rel_mine_c = total_mine_c_min_valid - total_mine_c_min + 1; rel_mine_c <= total_mine_c_max_valid - total_mine_c_min; rel_mine_c++) {
        int32_t k = board->mine_c - total_mine_c_min - rel_mine_c;
        rel_mine_c_weighting[rel_mine_c] = rel_mine_c_weighting[rel_mine_c - 1] * ((double)k+1)/((double)(n-k));
    }

    for (int32_t rel_mine_c = total_mine_c_min_valid - total_mine_c_min; rel_mine_c <= total_mine_c_max_valid - total_mine_c_min; rel_mine_c++) {
        int32_t k = board->mine_c - total_mine_c_min - rel_mine_c;
        pmap->comb_total += rel_mine_c_total_combs[rel_mine_c] * rel_mine_c_weighting[rel_mine_c];
        pmap->p_exterior += rel_mine_c_total_combs[rel_mine_c] * rel_mine_c_weighting[rel_mine_c] * ((double) k);
        for (int32_t i = 0; i < edge->edge_c; i++) {
            pmap->p_edge[i] += rel_mine_c_p_edge[rel_mine_c][i] * rel_mine_c_weighting[rel_mine_c];
        }
    }

    if (pmap->comb_total == 0) { //No valid permutations
        permutation_set_deinit(&permutation_set);
        fault_status = fault_invalid_board;
        return fault_status;
    }

    if (n>0) pmap->p_exterior /= pmap->comb_total * ((double) n);
    for (int32_t i = 0; i < edge->edge_c; i++) {
        pmap->p_edge[i] /= pmap->comb_total;
    }
    pmap->comb_total *= choose(n, board->mine_c - total_mine_c_min_valid);

    permutation_set_deinit(&permutation_set);
    return fault_status;
}

int32_t pmap_to_board(Board* board, Edge* edge, ProbabilityMap* pmap, double* prob_a) {
    for (int32_t i = 0; i < board->w * board->h; i++) {
        prob_a[i] = 0.0;
    }
    for (int32_t i = 0; i < edge->edge_solved_c; i++) {
        prob_a[edge->edge_solved[i]] = pmap->p_solved[i];
    }
    for (int32_t i = 0; i < edge->exterior_c; i++) {
        prob_a[edge->exterior[i]] = pmap->p_exterior;
    }
    for (int32_t i = 0; i < edge->edge_c; i++) {
        prob_a[edge->edge[i]] = pmap->p_edge[i];
    }
    double best_p = 2;
    int32_t best_p_pos = -1;
    for (int32_t i = 0; i < board->w*board->h; i++) {
        if (prob_a[i] < best_p && !board->known[i]) {
            best_p = prob_a[i];
            best_p_pos = i;
        }
    }
    return best_p_pos;
}

void get_lowest_probability(Board* board, Edge* edge, ProbabilityMap* pmap, 
                            double* p, int32_t* pos) {
    *p = 1.0;
    for (int32_t i = 0; i < edge->edge_solved_c; i++) {
        if (pmap->p_solved[i] == 0.0) {
            *p = 0.0;
            *pos = edge->edge_solved[i];
            return;
        }
    }
    
    for (int32_t i = 0; i < edge->edge_c; i++) {
        if (pmap->p_edge[i] < *p) {
            *p = pmap->p_edge[i];
            *pos = edge->edge[i];
        }
    }

    if (edge->exterior_c > 0 && pmap->p_exterior < *p) {
        *p = pmap->p_exterior;
        int32_t lowest_adjacent_unknown = 9;
        int32_t adj_c;
        for (int32_t i = 0; i < edge->exterior_c; i++) {
            adj_c = get_adjacent_unknown_c(board, edge->exterior[i]);
            if (adj_c < lowest_adjacent_unknown) {
                lowest_adjacent_unknown = adj_c;
                *pos = edge->exterior[i];
            }
        }
    }
}

int32_t get_best_evaluations(Board* board, Edge* edge, ProbabilityMap* pmap, 
                             int32_t* best_pos, double* best_p) {
    // If solved safe position, return only that position
    for (int32_t i = 0; i < edge->edge_solved_c; i++) {
        if (pmap->p_solved[i] == 0.0) {
            *best_pos = edge->edge_solved[i];
            *best_p = 0.0;
            return 1;
        }
    }

    int32_t best_c = 0;

    // Edge positions
    for (int32_t i = 0; i < edge->edge_c; i++) {
        if (pmap->p_edge[i] < 1.0) {
            best_p[best_c] = pmap->p_edge[i];
            best_pos[best_c] = edge->edge[i];
            best_c++;
        }
    }

    //Find the exterior point with least adjacent unknown (usually corners)
    if (edge->exterior_c > 0) {
        int32_t lowest_adjacent_unknown = 9;
        int32_t adj_c;
        for (int32_t i = 0; i < edge->exterior_c; i++) {
            adj_c = get_adjacent_unknown_c(board, edge->exterior[i]);
            if (adj_c < lowest_adjacent_unknown) {
                lowest_adjacent_unknown = adj_c;
                best_pos[best_c] = edge->exterior[i];
            }
        }
        best_p[best_c] = pmap->p_exterior;
        best_c++;
    }

    // Sort by probability ( in O(n^2) :P )
    int32_t temp_pos;
    double temp_p;
    for (int32_t i = 0; i < best_c; i++) {
        for (int32_t j = i + 1; j < best_c; j++) {
            if (best_p[i] > best_p[j]) {
                temp_pos = best_pos[i];
                temp_p = best_p[i];

                best_pos[i] = best_pos[j];
                best_p[i] = best_p[j];
                
                best_pos[j] = temp_pos;
                best_p[j] = temp_p;
            }
        }
    }

    if (best_p[0] == 0.0 && best_c > 0) best_c = 1;

    return best_c;
}

