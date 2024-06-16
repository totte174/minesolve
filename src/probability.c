#include "probability.h"

double choose(double n, double k) {
    if (n < k) return 0;
    if (k == 0) return 1;
    return (n * choose(n - 1, k - 1)) / k;
}

void get_pmap(Board* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap) {
    static int32_t split_mine_c_min[MAX_EDGE_SIZE];
    static int32_t split_mine_c_max[MAX_EDGE_SIZE];
    int32_t n = edge->exterior_c;

    pmap->valid = permutation_set->valid && (n >=    0);
    if(!pmap->valid) return;

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
    for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) 
    {
        split_mine_c_min[split_i] = MAX_MINES;
        split_mine_c_max[split_i] = 0;
        for (int32_t i = 0; i < permutation_set->splits_length[split_i]; i++){
            Mask* perm = permutation_set->permutations + permutation_set->splits_start[split_i] + i;
            
            split_mine_c_min[split_i] = min(split_mine_c_min[split_i], mine_c(perm));
            split_mine_c_max[split_i] = max(split_mine_c_max[split_i], mine_c(perm));
        }
        if (split_mine_c_max[split_i] < split_mine_c_min[split_i]) { //Not valid permutation in split -> not valid pmap
            pmap->valid = false;
            return;
        }
        total_mine_c_min += split_mine_c_min[split_i];
        total_mine_c_max += split_mine_c_max[split_i];
    }

    if (total_mine_c_max - total_mine_c_min >= MAX_MINE_C_DIFF) {
        printf("MAX MINE C DIFF REACHED");
        exit(1);
    }
    double rel_mine_c_p_edge[MAX_MINE_C_DIFF][MAX_EDGE_SIZE] = {0};
    double rel_mine_c_total_combs[MAX_MINE_C_DIFF] = {0};
    rel_mine_c_total_combs[0] = 1;
    int32_t cur_max_rel_mine_c = 0;

    for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) {
        double split_rel_mine_c_p_edge[MAX_MINE_C_DIFF][MAX_EDGE_SIZE] = {0};
        double split_rel_mine_c_total_combs[MAX_MINE_C_DIFF] = {0};        

        for (int32_t i = 0; i < permutation_set->splits_length[split_i]; i++) {
            Mask* perm = permutation_set->permutations + permutation_set->splits_start[split_i] + i;

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
        pmap->valid = false;
        return;
    }

    if (n>0) pmap->p_exterior /= pmap->comb_total * ((double) n);
    for (int32_t i = 0; i < edge->edge_c; i++) {
        pmap->p_edge[i] /= pmap->comb_total;
    }
    pmap->comb_total *= choose(n, board->mine_c - total_mine_c_min_valid);
}

int32_t pmap_to_board(Board* board, Edge* edge, ProbabilityMap* pmap, double* prob_a) {
    if (!pmap->valid) {
        return -1;
    }
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