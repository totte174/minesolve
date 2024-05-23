#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "statistics.h"

double gini(double p) {
    return p * (1 - p) * 4;
} 

double entropy(double p) {
    if (p <= 0) return 0;
    if (p >= 1) return 0;
    return - p * log2(p) - (1-p)*log2(1-p);
}

double choose(double n, double k) {
    if (n < k) return 0;
    if (k == 0) return 1;
    return (n * choose(n - 1, k - 1)) / k;
}

void initialize_pmap(ProbabilityMap* pmap, int32_t n, int32_t border_unknown_c) {
    pmap->border_unknown_c = border_unknown_c;
    pmap->n = n;
    pmap->valid = true;
    pmap->comb_total = 0;
    pmap->p_outside = 0;
    for (int32_t i = 0; i < border_unknown_c; i++){
        pmap->p_border_unknown[i] = 0;
    }
}

void get_pmap(Board* board, Border* border, PermutationSet* permutation_set,
                      ProbabilityMap* pmap, int32_t forced_i, int32_t n) {
    static int32_t split_mine_c_min[MAX_BORDER_UNKNOWN];
    static int32_t split_mine_c_max[MAX_BORDER_UNKNOWN];
    if (n < 0) { 
        pmap->valid = false;
        return;
    }

    initialize_pmap(pmap, n, border->border_unknown_c);
    int32_t total_mine_c_min = mine_c(&permutation_set->solved_permutation);
    int32_t total_mine_c_max = mine_c(&permutation_set->solved_permutation);
    for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) 
    {
        split_mine_c_min[split_i] = MAX_MINES;
        split_mine_c_max[split_i] = 0;
        for (int32_t i = 0; i < permutation_set->splits_length[split_i]; i++){
            Permutation* perm = permutation_set->permutations + permutation_set->splits_start[split_i] + i;
            if (forced_i >= 0 && mask_get(perm->mines, forced_i)) continue;
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
    double rel_mine_c_p_border_unknown[MAX_MINE_C_DIFF][MAX_BORDER_UNKNOWN] = {0};
    double rel_mine_c_total_combs[MAX_MINE_C_DIFF] = {0};
    for (int32_t i = 0; i < border->border_unknown_c; i++) {
        if (mask_get(permutation_set->solved_permutation.mines, i)) {
            rel_mine_c_p_border_unknown[0][i] = 1;
        }
    }   
    rel_mine_c_total_combs[0] = 1;
    int32_t cur_max_rel_mine_c = 0;

    for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) {
        double split_rel_mine_c_p_border_unknown[MAX_MINE_C_DIFF][MAX_BORDER_UNKNOWN] = {0};
        double split_rel_mine_c_total_combs[MAX_MINE_C_DIFF] = {0};        

        for (int32_t i = 0; i < permutation_set->splits_length[split_i]; i++) {
            Permutation* perm = permutation_set->permutations + permutation_set->splits_start[split_i] + i;
            if (forced_i >= 0 && mask_get(perm->mines, forced_i)) continue;

            int32_t rel_mine_c = mine_c(perm) - split_mine_c_min[split_i];
            split_rel_mine_c_total_combs[rel_mine_c] += 1;

            for (int32_t i = 0; i < border->border_unknown_c; i++) {
                if (mask_get(perm->mines, i)) {
                    split_rel_mine_c_p_border_unknown[rel_mine_c][i] += 1;
                }
            }    
        }

        double temp_rel_mine_c_p_border_unknown[MAX_MINE_C_DIFF][MAX_BORDER_UNKNOWN];
        double temp_rel_mine_c_total_combs[MAX_MINE_C_DIFF];
        for (int32_t rel_mine_c = 0; rel_mine_c <= cur_max_rel_mine_c; rel_mine_c++) {
            for (int32_t i = 0; i < border->border_unknown_c; i++) {
                temp_rel_mine_c_p_border_unknown[rel_mine_c][i] = rel_mine_c_p_border_unknown[rel_mine_c][i];
                rel_mine_c_p_border_unknown[rel_mine_c][i] = 0;
            }
            temp_rel_mine_c_total_combs[rel_mine_c] = rel_mine_c_total_combs[rel_mine_c];
            rel_mine_c_total_combs[rel_mine_c] = 0;
        }

        for (int32_t rel_mine_c = 0; rel_mine_c <= cur_max_rel_mine_c; rel_mine_c++) {
            for (int32_t split_rel_mine_c = 0; split_rel_mine_c <= split_mine_c_max[split_i] - split_mine_c_min[split_i]; split_rel_mine_c++) {
                for (int32_t i = 0; i < border->border_unknown_c; i++) {
                    rel_mine_c_p_border_unknown[rel_mine_c + split_rel_mine_c][i] += 
                        temp_rel_mine_c_p_border_unknown[rel_mine_c][i] * split_rel_mine_c_total_combs[split_rel_mine_c] +
                        split_rel_mine_c_p_border_unknown[split_rel_mine_c][i] * temp_rel_mine_c_total_combs[rel_mine_c];
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
        pmap->p_outside += rel_mine_c_total_combs[rel_mine_c] * rel_mine_c_weighting[rel_mine_c] * ((double) k);
        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            pmap->p_border_unknown[i] += rel_mine_c_p_border_unknown[rel_mine_c][i] * rel_mine_c_weighting[rel_mine_c];
        }
    }

    if (pmap->comb_total == 0) { //No valid permutations
        pmap->valid = false;
        return;
    }

    if (n>0) pmap->p_outside /= pmap->comb_total * ((double) n);
    for (int32_t i = 0; i < border->border_unknown_c; i++) {
        pmap->p_border_unknown[i] /= pmap->comb_total;
    }
    pmap->comb_total *= choose(n, board->mine_c - total_mine_c_min_valid);
}

void get_auxiliary_information(ProbabilityMap* pmap, int32_t forced_i, double* p2, double* gini_impurity, double* information_gain) {
    *p2 = 1;
    *gini_impurity = 0;
    *information_gain = 0;
    if (!pmap->valid) {
        *gini_impurity = MAX_SQUARES;
        *information_gain = MAX_SQUARES;
        return;
    }
    
    for (int32_t i = 0; i < pmap->border_unknown_c; i++) {
        if (i==forced_i) continue;
        *p2 = min(*p2, pmap->p_border_unknown[i]);
        *gini_impurity += gini(pmap->p_border_unknown[i]);
        *information_gain += entropy(pmap->p_border_unknown[i]);
    }
    if (pmap->n > 0) {
        *p2 = min(*p2, pmap->p_outside);
        *gini_impurity += ((double)pmap->n) * gini(pmap->p_outside);
        *information_gain += ((double)pmap->n) * entropy(pmap->p_outside);
    }
    if (USE_DIVIDE_AUXILLARY){
        *gini_impurity /= pmap->border_unknown_c + pmap->n;
        *information_gain /= pmap->border_unknown_c + pmap->n;
    }
}

void get_value(Board* board, BoardStatistics* statistics, double alpha, double beta, double eta) {
    for (int32_t i = 0; i < board->w * board->h; i++) {
        if (board->known[i]) {
            statistics->value[i] = -INFINITY;
        }
        else {
            int32_t x = i % board->w;
            int32_t y = i / board->w;
            int32_t manhattan_d = x + y;
            int32_t chebyshev_d = max(x,y);
            int32_t adj[8];
            int32_t adj_unknown = get_adjacent_unknown(board, i, adj);

            double p12 = 1 - (1 - statistics->p[i]) * (1 - statistics->p2[i]);
            double weighted_p = (1 - eta) * statistics->p[i] + eta * p12;

            statistics->value[i] =  - weighted_p
                                    + PRIO_PERIMETER    * is_on_perimeter(board, i)     * 0.0001
                                    + PRIO_CORNER       * is_on_corner(board, i)        * 0.0001
                                    - PRIO_MANHATTAN    * manhattan_d                   * 0.000001
                                    - PRIO_CHEBYSHEV    * chebyshev_d                   * 0.000001
                                    - PRIO_ADJACENT     * adj_unknown                   * 0.00001
                                    - alpha             * statistics->gini_impurity[i]
                                    - beta              * statistics->information_gain[i];
        }
    }
}

void print_statistics(Board* board, BoardStatistics* statistics, bool p, bool p2, bool gini, bool inf_gain, bool value) {
    if (p) {
        printf("Probability (Total combinations: %f)\n", statistics->total_combinations);
        for (int32_t y = 0; y < board->h; y++) {
            for (int32_t x = 0; x < board->w; x++) {
                if (board->known[y*board->w + x]) printf("      ");
                else printf("%.3f ", statistics->p[y*board->w + x]);
            }
            printf("\n");
        }
    }
    if (p2) {
        printf("Second move probability\n");
        for (int32_t y = 0; y < board->h; y++) {
            for (int32_t x = 0; x < board->w; x++) {
                if (board->known[y*board->w + x]) printf("      ");
                else printf("%.3f ", statistics->p2[y*board->w + x]);
            }
            printf("\n");
        }
    }
    if (gini) {
        printf("Gini impurity\n");
        for (int32_t y = 0; y < board->h; y++) {
            for (int32_t x = 0; x < board->w; x++) {
                if (board->known[y*board->w + x]) printf("      ");
                else printf("%05.1f ", statistics->gini_impurity[y*board->w + x]);
            }
            printf("\n");
        }
    }
    if (inf_gain) {
        printf("Information Gain\n");
        for (int32_t y = 0; y < board->h; y++) {
            for (int32_t x = 0; x < board->w; x++) {
                if (board->known[y*board->w + x]) printf("      ");
                else printf("%05.1f ", statistics->information_gain[y*board->w + x]);
            }
            printf("\n");
        }
    }
    if (value) {
        printf("Value\n");
        for (int32_t y = 0; y < board->h; y++) {
            for (int32_t x = 0; x < board->w; x++) {
                if (board->known[y*board->w + x]) printf("      ");
                else printf("%05.1f ", statistics->value[y*board->w + x]);
            }
            printf("\n");
        }
    }

}

BoardStatistics* get_statistics(Board* board, Border* border, PermutationSet* permutation_set, Arguments* args) {
    static BoardStatistics statistics;
    static ProbabilityMap main_pmap;
    static ProbabilityMap outside_pmap;
    static ProbabilityMap border_pmaps[MAX_BORDER_UNKNOWN];

    get_pmap(board, border, permutation_set, &main_pmap, -1, border->outside_unknown_c);
    statistics.total_combinations = main_pmap.comb_total;
    if (!main_pmap.valid) {
        printf("NOT VALID");
        exit(1);
    }

    if (args->p_only){
        for (int32_t i = 0; i < board->w * board->h; i++) {
            if (!board->known[i]) {
                statistics.p[i] = main_pmap.p_outside;
                statistics.p2[i] = 0;
                statistics.gini_impurity[i] = 0;
                statistics.information_gain[i] = 0;
            }
            else {
                statistics.p[i] = 0;
                statistics.p2[i] = 0;
                statistics.gini_impurity[i] = 0;
                statistics.information_gain[i] = 0;
            }
        }
        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            statistics.p[border->border_unknown[i]] = main_pmap.p_border_unknown[i];
        }
    }
    else {
        double outside_p2, outside_gini_impurity, outside_information_gain;
        get_pmap(board, border, permutation_set, &outside_pmap, -1, border->outside_unknown_c-1);
        get_auxiliary_information(&outside_pmap, -1, &outside_p2, &outside_gini_impurity, &outside_information_gain);

        for (int32_t i = 0; i < board->w * board->h; i++) {
            if (!board->known[i]) {
                statistics.p[i] = main_pmap.p_outside;
                statistics.p2[i] = outside_p2;
                statistics.gini_impurity[i] = outside_gini_impurity;
                statistics.information_gain[i] = outside_information_gain;
            }
            else {
                statistics.p[i] = 0;
                statistics.p2[i] = 0;
                statistics.gini_impurity[i] = 0;
                statistics.information_gain[i] = 0; 
            }
        }
        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            statistics.p[border->border_unknown[i]] = main_pmap.p_border_unknown[i];

            if (main_pmap.p_border_unknown[i] == 0.0) {
                border_pmaps[i] = main_pmap;
            } else if (main_pmap.p_border_unknown[i] == 1.0){
                border_pmaps[i].valid = false;
            }
            else{
                get_pmap(board, border, permutation_set, border_pmaps + i, i, border->outside_unknown_c);
            }
            get_auxiliary_information(border_pmaps + i, i, statistics.p2 + border->border_unknown[i], 
                                                           statistics.gini_impurity + border->border_unknown[i], 
                                                           statistics.information_gain + border->border_unknown[i]);
        }
    }

    get_value(board, &statistics, args->alpha, args->beta, args->eta);

    double best_p = 2;
    double best_value = -INFINITY;
    int32_t best_p_positions[MAX_SQUARES];
    int32_t best_value_positions[MAX_SQUARES];
    int32_t best_p_c = 0;
    int32_t best_value_c = 0;

    for (int32_t i = 0; i < board->w*board->h; i++) {
        if (statistics.p[i] < best_p && !board->known[i]) {
            best_p = statistics.p[i];
            best_p_positions[0] = i;
            best_p_c = 1;
        }
        else if (statistics.p[i] == best_p && !board->known[i]) {
            best_p_positions[best_p_c++] = i;
        }
        if (statistics.value[i] > best_value && !board->known[i]) {
            best_value = statistics.value[i];
            best_value_positions[0] = i;
            best_value_c = 1;
        }
        else if (statistics.value[i] == best_value && !board->known[i]) {
            best_value_positions[best_value_c++] = i;
        }
    }
    if (USE_RANDOM_STRATEGY) {
        statistics.best_p = best_p_positions[rand() % best_p_c];
        statistics.best_value = best_value_positions[rand() % best_value_c];
    }
    else {
        statistics.best_p = best_p_positions[0];
        statistics.best_value = best_value_positions[0];
    }
    
    return &statistics;
}