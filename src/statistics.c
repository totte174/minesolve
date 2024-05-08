#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <alloca.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

#include "statistics.h"
#include "common.h"
#include "permutations.h"

double gini(double p) {
    return p * (1 - p);
} 

double entropy(double p) {
    if (p <= 0) return 0;
    return -p * log2(p);
}

void initialize_intermediate(IntermediateStatistics* intermediate, int32_t n, int32_t border_unknown_c) {
    intermediate->border_unknown_c = border_unknown_c;
    intermediate->n = n;
    intermediate->valid = true;
    intermediate->comb_total = 0;
    intermediate->p_outside = 0;
    for (int32_t i = 0; i < border_unknown_c; i++){
        intermediate->p_border_unknown[i] = 0;
    }
}

void get_intermediates(Board* board, Border* border, PermutationSet* permutation_set,
                       IntermediateStatistics* main_intermediate, IntermediateStatistics* outside_intermediate,
                       IntermediateStatistics* border_intermediates) {
    static double normalized_combs[MAX_MINES];
    static double normalized_combs_n_less[MAX_MINES];
    const int32_t max_k_diff = 10;

    int32_t mine_c_ref = permutation_set->solved_permutation.mine_c;
    for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) 
    {
        mine_c_ref += permutation_set->permutations[permutation_set->splits_start[split_i]].mine_c;
    }

    int32_t n = border->outside_unknown_c;
    int32_t k_ref = min(n, max(0, board->mine_c - mine_c_ref));
    int32_t k_min = max(0, k_ref-max_k_diff);
    int32_t k_max = min(n, k_ref + max_k_diff);
    
    normalized_combs[k_ref] = 1.0;
    normalized_combs_n_less[k_ref] = 1.0;
    for (int32_t k = k_ref + 1; k <= k_max; k++){
        normalized_combs[k] = normalized_combs[k-1] * ((double)(n-k+1))/((double)k);
        normalized_combs_n_less[k] = normalized_combs_n_less[k-1] * ((double)(n-k))/((double)k);
    }
    for (int32_t k = k_ref - 1; k >= k_min && k >= 0; k--){
        normalized_combs[k] = normalized_combs[k+1] * ((double)k+1)/((double)(n-k));
        normalized_combs_n_less[k] = normalized_combs_n_less[k+1] * ((double)k+1)/((double)(n-k-1));
    }
   

    initialize_intermediate(main_intermediate, n, border->border_unknown_c);
    initialize_intermediate(outside_intermediate, n-1, border->border_unknown_c);
    for (int32_t i = 0; i < border->border_unknown_c; i++)
    {
        initialize_intermediate(border_intermediates + i, n, border->border_unknown_c);
    }


    for (int32_t perm_i = 0; perm_i < permutation_set->total_permutation_c; perm_i++) {
        static Permutation perm;
        memcpy(&perm, &permutation_set->solved_permutation, sizeof(Permutation));
        int32_t real_perm_start = 1;
        for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) {
            int32_t real_perm_i = permutation_set->splits_start[split_i] + ((perm_i / real_perm_start) % permutation_set->splits_length[split_i]);
            join_permutations_noconflict(&perm, &perm, permutation_set->permutations + real_perm_i);
            real_perm_start *= permutation_set->splits_length[split_i];
        }
        //print_permutation(border->border_unknown_c, &perm);

        int32_t k = board->mine_c - perm.mine_c;
        if (k < 0 || k > n) continue;
        // DEBUG
        if (abs(k-k_ref) > max_k_diff) {
            printf("K OUT OF BOUNDS");
            exit(1);
        }

        main_intermediate->p_outside += ((double)k) * normalized_combs[k];
        outside_intermediate->p_outside += ((double)k) * normalized_combs_n_less[k];
        main_intermediate->comb_total += normalized_combs[k];
        outside_intermediate->comb_total += normalized_combs_n_less[k];
        

        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            if ((perm.mines[i / 64] >> (i % 64)) & 1) {
                main_intermediate->p_border_unknown[i] += normalized_combs[k];
                outside_intermediate->p_border_unknown[i] += normalized_combs_n_less[k];
            }
            else {
                for (int32_t j = 0; j < border->border_unknown_c; j++) {
                    if ((perm.mines[j / 64] >> (j % 64)) & 1) {
                        border_intermediates[i].p_border_unknown[j] += normalized_combs[k];
                    }
                }
                border_intermediates[i].comb_total += normalized_combs[k];
                border_intermediates[i].p_outside += ((double)k) * normalized_combs[k];
            }
        }
    }

    if (main_intermediate->comb_total == 0) {
        main_intermediate->valid = false;
    }
    else {
        if (n > 0) main_intermediate->p_outside /= main_intermediate->comb_total * ((double)n);
        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            main_intermediate->p_border_unknown[i] /= main_intermediate->comb_total;
        }
    }

    if (outside_intermediate->comb_total == 0) {
        outside_intermediate->valid = false;
    }
    else {
        if (n-1 > 0) outside_intermediate->p_outside /= outside_intermediate->comb_total * ((double)(n-1));
        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            outside_intermediate->p_border_unknown[i] /= outside_intermediate->comb_total;
        }
    }

    for (int32_t inter_i = 0; inter_i < border->border_unknown_c; inter_i++) {
        if (border_intermediates[inter_i].comb_total == 0) {
            border_intermediates[inter_i].valid = false;
            continue;
        }
        if (n>0) border_intermediates[inter_i].p_outside /= border_intermediates[inter_i].comb_total * ((double)n);

        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            border_intermediates[inter_i].p_border_unknown[i] /= border_intermediates[inter_i].comb_total;
        }
    }
}

void get_probability_p_only(Board* board, Border* border, PermutationSet* permutation_set, IntermediateStatistics* main_intermediate, BoardStatistics* statistics) {
    static double normalized_combs[MAX_MINES];
    const int32_t max_k_diff = 10;

    int32_t mine_c_ref = permutation_set->solved_permutation.mine_c;
    for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) 
    {
        mine_c_ref += permutation_set->permutations[permutation_set->splits_start[split_i]].mine_c;
    }

    int32_t n = border->outside_unknown_c;
    int32_t k_ref = min(n, max(0, board->mine_c - mine_c_ref));
    int32_t k_min = max(0, k_ref-max_k_diff);
    int32_t k_max = min(n, k_ref + max_k_diff);
    
    normalized_combs[k_ref] = 1.0;
    for (int32_t k = k_ref + 1; k <= k_max; k++){
        normalized_combs[k] = normalized_combs[k-1] * ((double)(n-k+1))/((double)k);
    }
    for (int32_t k = k_ref - 1; k >= k_min && k >= 0; k--){
        normalized_combs[k] = normalized_combs[k+1] * ((double)k+1)/((double)(n-k));
    }
   

    initialize_intermediate(main_intermediate, n, border->border_unknown_c);

    for (int32_t perm_i = 0; perm_i < permutation_set->total_permutation_c; perm_i++) {
        static Permutation perm;
        memcpy(&perm, &permutation_set->solved_permutation, sizeof(Permutation));
        int32_t real_perm_start = 1;
        for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) {
            int32_t real_perm_i = permutation_set->splits_start[split_i] + ((perm_i / real_perm_start) % permutation_set->splits_length[split_i]);
            join_permutations_noconflict(&perm, &perm, permutation_set->permutations + real_perm_i);
            real_perm_start *= permutation_set->splits_length[split_i];
        }

        int32_t k = board->mine_c - perm.mine_c;
        if (k < 0 || k > n) continue;

        main_intermediate->p_outside += ((double)k) * normalized_combs[k];
        main_intermediate->comb_total += normalized_combs[k];
        

        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            if ((perm.mines[i / 64] >> (i % 64)) & 1) {
                main_intermediate->p_border_unknown[i] += normalized_combs[k];
            }
        }
    }

    if (main_intermediate->comb_total == 0) {
        main_intermediate->valid = false;
    }
    else {
        if (n > 0) main_intermediate->p_outside /= main_intermediate->comb_total * ((double)n);
        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            main_intermediate->p_border_unknown[i] /= main_intermediate->comb_total;
        }
    }

    if (!main_intermediate->valid) {
        printf("NOT VALID");
        exit(1);
    }
    for (int32_t i = 0; i < board->w * board->h; i++) {
        if (!board->known[i]) {
            statistics->p[i] = main_intermediate->p_outside;
        }
        else {
            statistics->p[i] = 0;
        }
        statistics->gini_impurity[i] = 0;
        statistics->information_gain[i] = 0;
        statistics->p2[i] = 0;
    }
    for (int32_t i = 0; i < border->border_unknown_c; i++) {
        statistics->p[border->border_unknown[i]] = main_intermediate->p_border_unknown[i];
    }
}

void get_border_gini_and_information_gain(IntermediateStatistics* border_intermediates, int32_t intermediate_c, 
                                          IntermediateStatistics* outside_intermediate,
                                          double* border_gini_impurity, double* border_information_gain, 
                                          double* outside_gini_impurity, double* outside_information_gain,
                                          double* border_p2, double* outside_p2) {
    *outside_gini_impurity = 0;
    *outside_information_gain = 0;
    *outside_p2 = 1;
    for (int32_t inter_i = 0; inter_i < intermediate_c; inter_i++) {
        border_p2[inter_i] = 1;
        border_gini_impurity[inter_i] = 0;
        border_information_gain[inter_i] = 0;
        if (!border_intermediates[inter_i].valid) continue;
        for (int32_t i = 0; i < border_intermediates[inter_i].border_unknown_c; i++) {
            if (!isfinite(border_intermediates[inter_i].p_border_unknown[i])) {
                border_p2[inter_i] = INFINITY;
                border_gini_impurity[inter_i] = INFINITY;
                border_information_gain[inter_i] = INFINITY;
                //printf("INFINITE PROBABILITY!");

            }
            else {
                if (i!=inter_i) border_p2[inter_i] = min(border_p2[inter_i], border_intermediates[inter_i].p_border_unknown[i]);
                border_gini_impurity[inter_i] += gini(border_intermediates[inter_i].p_border_unknown[i]);
                border_information_gain[inter_i] += entropy(border_intermediates[inter_i].p_border_unknown[i]);
            }
        }
        border_p2[inter_i] = min(border_p2[inter_i], border_intermediates[inter_i].p_outside);
        border_gini_impurity[inter_i] += ((double)border_intermediates[inter_i].n) * gini(border_intermediates[inter_i].p_outside);
        border_information_gain[inter_i] += ((double)border_intermediates[inter_i].n) * entropy(border_intermediates[inter_i].p_outside);
    }
    if (!outside_intermediate->valid) return;
    for (int32_t i = 0; i < outside_intermediate->border_unknown_c; i++) {
        *outside_p2 = min(*outside_p2, outside_intermediate->p_border_unknown[i]);
        *outside_gini_impurity += gini(outside_intermediate->p_border_unknown[i]);
        *outside_information_gain += entropy(outside_intermediate->p_border_unknown[i]);
    }
    *outside_p2 = min(*outside_p2, outside_intermediate->p_outside);
    *outside_gini_impurity += ((double)outside_intermediate->n) * gini(outside_intermediate->p_outside);
    *outside_information_gain += ((double)outside_intermediate->n) * entropy(outside_intermediate->p_outside);

}

void get_gini_and_information_gain(Board* board, Border* border, IntermediateStatistics* border_intermediates,
                                   IntermediateStatistics* outside_intermediate,
                                   BoardStatistics* statistics) {
    double border_p2[MAX_BORDER_UNKNOWN];
    double border_gini_impurity[MAX_BORDER_UNKNOWN];
    double border_information_gain[MAX_BORDER_UNKNOWN];
    double outside_p2;
    double outside_gini_impurity;
    double outside_information_gain;

    get_border_gini_and_information_gain(border_intermediates, border->border_unknown_c, outside_intermediate, 
                                         border_gini_impurity, border_information_gain, &outside_gini_impurity, 
                                         &outside_information_gain, border_p2, &outside_p2);

    for (int32_t i = 0; i < board->w * board->h; i++) {
        if (!board->known[i]) {
            statistics->p2[i] = outside_p2;
            statistics->gini_impurity[i] = outside_gini_impurity;
            statistics->information_gain[i] = outside_information_gain;
        }
        else {
            statistics->p2[i] = 0;
            statistics->gini_impurity[i] = 0;
            statistics->information_gain[i] = 0;
        }
    }
    //printf("Valid: ");
    for (int32_t i = 0; i < border->border_unknown_c; i++) {
        //printf("%d", border_intermediates[i].valid);
        statistics->p2[border->border_unknown[i]] = border_p2[i];
        statistics->gini_impurity[border->border_unknown[i]] = border_gini_impurity[i];
        statistics->information_gain[border->border_unknown[i]] = border_information_gain[i];
    }
    //printf("\nBorder valid: %d\n", outside_intermediate->valid);
    
}

void get_probability(Board* board, Border* border, IntermediateStatistics* main_intermediate, BoardStatistics* statistics) {
    if (!main_intermediate->valid) {
        printf("NOT VALID");
        exit(1);
    }
    for (int32_t i = 0; i < board->w * board->h; i++) {
        if (!board->known[i]) {
            statistics->p[i] = main_intermediate->p_outside;
        }
        else {
            statistics->p[i] = 0;
        }
    }
    for (int32_t i = 0; i < border->border_unknown_c; i++) {
        statistics->p[border->border_unknown[i]] = main_intermediate->p_border_unknown[i];
    }
}

void get_value(Board* board, BoardStatistics* statistics, double alpha, double beta) {
    const double eta = 0.0;
    const double prio_perimeter = 0.001;
    const double prio_corner = 0.001;
    for (int32_t i = 0; i < board->w * board->h; i++) {
        if (board->known[i]) {
            statistics->value[i] = -INFINITY;
        }
        else {
            statistics->value[i] =  - statistics->p[i]
                                    + prio_perimeter    * is_on_perimeter(board, i)
                                    + prio_corner       * is_on_corner(board, i)
                                    - eta               * statistics->p2[i]
                                    - alpha             * statistics->gini_impurity[i]
                                    - beta              * statistics->information_gain[i];
        }
    }
}

void print_statistics(Board* board, BoardStatistics* statistics, bool p, bool p2, bool gini, bool inf_gain, bool value) {
    if (p) {
        printf("Probability\n");
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

BoardStatistics* get_statistics(Board* board, Border* border, PermutationSet* permutation_set, double alpha, double beta, bool p_only) {
    static BoardStatistics statistics;
    static IntermediateStatistics main_intermediate;
    static IntermediateStatistics outside_intermediate;
    static IntermediateStatistics border_intermediates[MAX_BORDER_UNKNOWN];

    if (!p_only){
        get_intermediates(board, border, permutation_set, &main_intermediate, &outside_intermediate, border_intermediates);
        get_probability(board, border, &main_intermediate, &statistics);
        get_gini_and_information_gain(board, border, border_intermediates, &outside_intermediate, &statistics);
    }
    else {
        get_probability_p_only(board, border, permutation_set, &main_intermediate, &statistics);
    }
    get_value(board, &statistics, alpha, beta);


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
    statistics.best_p = best_p_positions[rand() % best_p_c];
    statistics.best_value = best_value_positions[rand() % best_value_c];

    return &statistics;
}