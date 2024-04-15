#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <alloca.h>
#include <math.h>

#include "statistics.h"
#include "common.h"

double choose(double n, double k) {
    if (n < k) return 0;
    if (k == 0) return 1;
    return (n * choose(n-1, k-1)) / k;
}

double gini(double p) {
    return p * (1 - p);
} 

double entropy(double p) {
    if (p <= 0) return 0;
    return -p * log2(p);
}

double combinations_from_kcount(KCount* kcount, int32_t n){
    double combinations = 0;
    for (int32_t k = 0; k < MAX_BOMBS; k++){
        if (kcount->count[k] > 0) {
            combinations += kcount->count[k] * choose(n, k);
        }
    }
    return combinations;
}

void initialize_intermediate(IntermediateStatistics* intermediate, int32_t n, int32_t border_unknown_c) {
    // Maybe use memset??
    intermediate->n = n;
    intermediate->border_unknown_c = border_unknown_c;
    for (int32_t k = 0; k < MAX_BOMBS; k++){
        intermediate->total.count[k] = 0;
    }
    for (int32_t i = 0; i < border_unknown_c; i++) {
        for (int32_t k = 0; k < MAX_BOMBS; k++){
            intermediate->per_square[i].count[k] = 0;
        }
    }
}

void get_intermediates(Board* board, Border* border, PermutationSet* permutation_set,
                       IntermediateStatistics* main_intermediate, IntermediateStatistics* outside_intermediate,
                       IntermediateStatistics* border_intermediates) {
    initialize_intermediate(main_intermediate, border->outside_unknown_c, border->border_unknown_c);
    initialize_intermediate(outside_intermediate, border->outside_unknown_c - 1, border->border_unknown_c);
    for (int32_t i = 0; i < border->border_unknown_c; i++) initialize_intermediate(&border_intermediates[i], border->outside_unknown_c, border->border_unknown_c);


    for (int32_t perm_i = 0; perm_i < permutation_set->permutation_c; perm_i++) {
        if (permutation_set->permutations[perm_i].bomb_amount > board->bomb_c) continue;
        
        int32_t k = board->bomb_c - permutation_set->permutations[perm_i].bomb_amount;
        main_intermediate->total.count[k] += 1;
        outside_intermediate->total.count[k] += 1;
        

        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            if ((permutation_set->permutations[perm_i].bombs[i / 64] >> (i % 64)) & 1) {
                main_intermediate->per_square[i].count[k] += 1;
                outside_intermediate->per_square[i].count[k] += 1;
            }
            else {
                for (int32_t j = 0; j < border->border_unknown_c; j++) {
                    if ((permutation_set->permutations[perm_i].bombs[j / 64] >> (j % 64)) & 1) {
                        border_intermediates[i].per_square[j].count[k] += 1;
                    }
                }
                border_intermediates[i].total.count[k] += 1;
            }
        }
    }
}

bool get_border_probabilities(IntermediateStatistics* intermediate, double* border_probabilities, double* outside_probability) {
    for (int32_t i = 0; i < intermediate->border_unknown_c; i++) {
        border_probabilities[i] = combinations_from_kcount(&intermediate->per_square[i], intermediate->n);
    }
    *outside_probability = 0;
    for (int32_t k = 0; k < MAX_BOMBS; k++){
        if (intermediate->total.count[k] > 0) {
            *outside_probability += intermediate->total.count[k] * choose(intermediate->n, k) * k / intermediate->n;
        }
    }

    double total_combinations = combinations_from_kcount(&intermediate->total, intermediate->n);
    if (total_combinations == 0) return false;
    for (int32_t i = 0; i < intermediate->border_unknown_c; i++) border_probabilities[i] /= total_combinations;
    *outside_probability /= total_combinations;
    return true;
}

void get_border_gini_and_information_gain(IntermediateStatistics* border_intermediates, int32_t intermediate_c, 
                                          IntermediateStatistics* outside_intermediate,
                                          double* border_gini_impurity, double* border_information_gain, 
                                          double* outside_gini_impurity, double* outside_information_gain) {
    double border_probabilities[MAX_SQUARES];
    double outside_probability;
    *outside_gini_impurity = 0;
    *outside_information_gain = 0;
    for (int32_t inter_i = 0; inter_i < intermediate_c; inter_i++) {
        bool valid_p = get_border_probabilities(&border_intermediates[inter_i], border_probabilities, &outside_probability);

        border_gini_impurity[inter_i] = 0;
        border_information_gain[inter_i] = 0;
        if (!valid_p) continue;
        for (int32_t i = 0; i < border_intermediates[inter_i].border_unknown_c; i++) {
            if (!isfinite(border_probabilities[i])) {
                border_gini_impurity[inter_i] = 0;
                border_information_gain[inter_i] = 0;

            }
            else {
                border_gini_impurity[inter_i] += gini(border_probabilities[i]);
                border_information_gain[inter_i] += entropy(border_probabilities[i]);
            }
        }
        border_gini_impurity[inter_i] += border_intermediates[inter_i].n * gini(outside_probability);
        border_information_gain[inter_i] += border_intermediates[inter_i].n * entropy(outside_probability);
    }
    bool valid_p = get_border_probabilities(outside_intermediate, border_probabilities, &outside_probability);
    if (!valid_p) return;
    for (int32_t i = 0; i < outside_intermediate->border_unknown_c; i++) {
        *outside_gini_impurity += gini(border_probabilities[i]);
        *outside_information_gain += entropy(border_probabilities[i]);
    }
    *outside_gini_impurity += outside_intermediate->n * gini(outside_probability);
    *outside_information_gain += outside_intermediate->n * entropy(outside_probability);

}

void get_gini_and_information_gain(Board* board, Border* border, IntermediateStatistics* border_intermediates,
                                   IntermediateStatistics* outside_intermediate,
                                   double* gini_impurity, double* information_gain) {
    double border_gini_impurity[MAX_SQUARES];
    double border_information_gain[MAX_SQUARES];
    double outside_gini_impurity;
    double outside_information_gain;

    get_border_gini_and_information_gain(border_intermediates, border->border_unknown_c, outside_intermediate, 
                                         border_gini_impurity, border_information_gain, &outside_gini_impurity, &outside_information_gain);

    for (int32_t i = 0; i < board->w * board->h; i++) {
        if (!board->known[i]) {
            gini_impurity[i] = outside_gini_impurity;
            information_gain[i] = outside_information_gain;
        }
        else {
            gini_impurity[i] = 0;
            information_gain[i] = 0;
        }
    }
    for (int32_t i = 0; i < border->border_unknown_c; i++) {
        gini_impurity[border->border_unknown[i]] = border_gini_impurity[i];
        information_gain[border->border_unknown[i]] = border_information_gain[i];
    }
    
}

void get_probability(Board* board, Border* border, IntermediateStatistics* intermediate, double* p) {
    double* border_probabilities = alloca(border->border_unknown_c * sizeof(double));
    double outside_probability;
    get_border_probabilities(intermediate, border_probabilities, &outside_probability);

    for (int32_t i = 0; i < board->w * board->h; i++) {
        if (!board->known[i]) {
            p[i] = outside_probability;
        }
        else {
            p[i] = 0;
        }
    }
    for (int32_t i = 0; i < border->border_unknown_c; i++) {
        p[border->border_unknown[i]] = border_probabilities[i];
    }
}

void print_statistics(Board* board, BoardStatistics* statistics, bool p, bool gini, bool inf_gain, bool value) {
    if (p) {
        printf("Probability\n");
        for (int32_t y = 0; y < board->h; y++) {
            for (int32_t x = 0; x < board->w; x++) {
                printf("%.3f ", statistics->p[y*board->w + x]);
            }
            printf("\n");
        }
    }
    if (gini) {
        printf("Gini impurity\n");
        for (int32_t y = 0; y < board->h; y++) {
            for (int32_t x = 0; x < board->w; x++) {
                printf("%05.1f ", statistics->gini_impurity[y*board->w + x]);
            }
            printf("\n");
        }
    }
    if (inf_gain) {
        printf("Information Gain\n");
        for (int32_t y = 0; y < board->h; y++) {
            for (int32_t x = 0; x < board->w; x++) {
                printf("%05.1f ", statistics->information_gain[y*board->w + x]);
            }
            printf("\n");
        }
    }

}

BoardStatistics* get_statistics(Board* board, Border* border, PermutationSet* permutation_set) {
    static BoardStatistics statistics;
    static IntermediateStatistics main_intermediate;
    static IntermediateStatistics outside_intermediate;
    static IntermediateStatistics border_intermediates[MAX_SQUARES];

    get_intermediates(board, border, permutation_set, &main_intermediate, &outside_intermediate, border_intermediates);
    get_probability(board, border, &main_intermediate, statistics.p);
    get_gini_and_information_gain(board, border, border_intermediates, &outside_intermediate, statistics.gini_impurity, statistics.information_gain);
    
    double best_p = 2;
    for (int32_t i = 0; i < board->w*board->h; i++) {
        if (statistics.p[i] < best_p && !board->known[i]) {
            statistics.best_p = i;
            best_p = statistics.p[i];
        }
    }

    return &statistics;
}