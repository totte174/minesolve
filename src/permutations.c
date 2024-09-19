#include <stdint.h>
#include <stdbool.h>
#include <alloca.h>
#include <malloc.h>
#include <stdlib.h>

#include "permutations.h"

int32_t equation_permutations(Equation* equation, Mask* permutations){
    int32_t map[8];
    int32_t j = 0;
    for (int32_t i = 0; i < MAX_EDGE_SIZE; i++) {
        if (mask_get(equation->mask, i)) map[j++] = i;
    }

    int32_t permutation_c = 0;
    int32_t unknown_c = mask_count(equation->mask);
    for (uint64_t x = 0; x < (1 << unknown_c); x++) {
        if (__builtin_popcountll(x) == equation->amount) {
            mask_reset(permutations[permutation_c]);

            for (uint64_t i = 0; i < unknown_c; i++) {
                if (x & (1 << i)) {
                    mask_set(permutations[permutation_c], map[i], 1);
                }
            }
            permutation_c++;
        }
    }
    return permutation_c;
}

bool permutation_intersect(Mask* permutation1, Mask* permutation2) {
    return mask_overlap(*permutation1, *permutation2);
}

bool join_permutations(Mask* mask1, Mask* mask2, Mask* new_permutation, Mask* permutation1, Mask* permutation2) {
    // Join permutation1 and permutation2 into new_permutation, if they dont work together return false
    for (int32_t i = 0; i < MASK_PARTS; i++) {
        if (mask1->v[i] & mask2->v[i] & ((permutation1->v[i] & ~permutation2->v[i]) | (~permutation1->v[i] & permutation2->v[i]))) return false;
        new_permutation->v[i] = permutation1->v[i] | permutation2->v[i];
    }
    return true;
}

void join_new_permutations(PermutationSet* permutation_set, Mask* mask1, Mask* mask2, int32_t split_i, Mask* new_permutations, int32_t new_permutations_c) {
    // Join permutations into permutations1

    // Copy permutations1 to temp_permutation_set
    Mask* temp_permutations = malloc(sizeof(Mask) * permutation_set->splits_length[split_i]);
    memcpy(temp_permutations,
           permutation_set->permutations + permutation_set->splits_start[split_i], 
           sizeof(Mask) * permutation_set->splits_length[split_i]);
    int32_t permutation_c = 0;

    for (int32_t i = 0; i < permutation_set->splits_length[split_i]; i++) {
        for (int32_t j = 0; j < new_permutations_c; j++) {
            if (permutation_set->permutation_c + permutation_c == permutation_set->permutation_size) {
                if (permutation_set->permutation_size >= MAX_PERMUTATIONS) { //Quick fix so it doesn't consume ginormouz amounts of RAM
                    //printf("OUTOFMEM\n");
                    permutation_set->splits_length[split_i] = permutation_c;
                    free(temp_permutations);
                    return;
                }

                permutation_set->permutation_size = permutation_set->permutation_size << 1;
                permutation_set->permutations = realloc(permutation_set->permutations, sizeof(Mask) * permutation_set->permutation_size);
            }
            if (join_permutations(mask1, mask2, permutation_set->permutations + permutation_set->splits_start[split_i] + permutation_c, 
                                  temp_permutations + i, new_permutations + j)) {
                permutation_c++;
            }
        }
    }
    permutation_set->splits_length[split_i] = permutation_c;
    free(temp_permutations);
}

inline int32_t mine_c(Mask* permutation) {
    return mask_count(*permutation);
}

//void print_permutation(Mask* permutation, int32_t border_unknown_c) {
//    for (uint64_t j = 0; j < border_unknown_c; j++) {
//        if (mask_get(*permutation, j)) {
//            printf("%llu", mask_get(*permutation, j));
//        }
//        else {
//            printf(" ");
//        }
//    }
//    printf(":%lu\n", mask_count(*permutation));
//}

//void print_permutation_set(MaskSet* permutation_set, int32_t border_unknown_c) {
//    printf("Solved permutation:\n");
//    print_permutation(&permutation_set->solved_permutation, border_unknown_c);
//    printf("Masks: %d\n", permutation_set->permutation_c);
//    for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) {
//        printf("--- Split %d\n", split_i);
//        for (int32_t i = 0; i < permutation_set->splits_length[split_i]; i++) {
//            print_permutation(permutation_set->permutations + i + permutation_set->splits_start[split_i], border_unknown_c);
//        }
//    }
//}

void get_permutations_from_split(PermutationSet* permutation_set, EquationSet* equation_set, int32_t split_i) {
    static Mask temp_equation_perms[256];
    int32_t split_start = permutation_set->permutation_c;
    permutation_set->splits_start[split_i] = split_start;

    int32_t equation_start = equation_set->splits_start[split_i];
    int32_t equation_end = equation_set->splits_start[split_i] + equation_set->splits_length[split_i];

    Mask split_mask = equation_set->equations[equation_start].mask;
    permutation_set->splits_length[split_i] = equation_permutations(equation_set->equations + equation_start, permutation_set->permutations + split_start);
    
    for (int32_t i = equation_start + 1; i < equation_end; i++) {
        int32_t equation_perms = equation_permutations(equation_set->equations + i, temp_equation_perms);

        join_new_permutations(permutation_set, &split_mask, &equation_set->equations[i].mask, 
                              split_i, temp_equation_perms, equation_perms);

        for (int32_t j = 0; j < MASK_PARTS; j++) {
            split_mask.v[j] |= equation_set->equations[i].mask.v[j];
        }
    }
    permutation_set->permutation_c += permutation_set->splits_length[split_i];
}

void get_permutation_set(Board* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap){
    static EquationSet equation_set;

    permutation_set->initialized = false;

    get_equation_set(board, edge, &equation_set, pmap);
    permutation_set->valid = equation_set.valid;
    if (!permutation_set->valid) return;

    permutation_set->permutation_size = MIN_PERMUTATIONS;
    permutation_set->permutations = malloc(sizeof(Mask) * permutation_set->permutation_size);
    permutation_set->permutation_c = 0;
    permutation_set->initialized = true;
    permutation_set->split_c = equation_set.split_c;

    for (int32_t split_i = 0; split_i < equation_set.split_c; split_i++) {
        get_permutations_from_split(permutation_set, &equation_set, split_i);
    }
}

void permutation_set_deinit(PermutationSet* permutation_set) {
    free(permutation_set->permutations);
}

//int32_t get_splits(Edge* edge, int32_t* positions, int32_t pos_c, int32_t* splits) {
//    int32_t splits_found = 0;
//
//    for (int32_t split_i = 0; split_i < edge->split_c; split_i++) {
//        for (int32_t i = edge->splits_start[split_i];
//             i < edge->splits_start[split_i] + edge->splits_length[split_i]; i++) {
//            
//            for (int32_t j = 0; j < pos_c; j++) {
//                if (edge->edge[i] == positions[j]) {
//                    bool split_exists = false;
//                    for (int32_t k = 0; k < splits_found; k++) {
//                        if (splits[k] == split_i) split_exists = true;
//                    }
//                    if (!split_exists) {
//                        splits[splits_found++] = split_i;
//                    }
//                }
//            }
//        }
//    }
//    return splits_found;
//}

//void get_permutation_set_from_old(Board* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap, 
//                                  Edge* prev_edge, PermutationSet* prev_perm_set, ProbabilityMap* prev_pmap,
//                                  int32_t pos, int32_t v) {
//    permutation_set->permutation_size = prev_perm_set->permutation_size;
//    permutation_set->permutations = malloc(sizeof(Mask) * permutation_set->permutation_size);
//    permutation_set->permutation_c = prev_perm_set->permutation_c;
//    permutation_set->valid = true;
//    memcpy(edge, prev_edge, sizeof(Edge));
//    //memcpy(permutation_set->permutations, prev_perm_set->permutations,
//    //       prev_perm_set->permutation_c * sizeof(Mask));
//    memcpy(pmap, prev_pmap, sizeof(pmap));
//
//    int32_t adj[8];
//    int32_t adj_c = get_adjacent_unknown(board, pos, adj);
//    if (v > adj_c || v < 0) {
//        permutation_set->valid = false;
//        pmap->valid = false;
//        return;
//    }
//    int32_t adj_split;
//    bool adj_in_split = (bool) get_splits(edge, adj, adj_c, &adj_split);
//
//    int32_t pos_split;
//    bool pos_in_split = (bool) get_splits(edge, &pos, 1, &pos_split);
//
//    bool in_split = adj_in_split && pos_in_split;
//    int32_t current_split;
//    if (adj_in_split) current_split = adj_split;
//    if (pos_in_split) current_split = pos_split;
//
//    if (!in_split) {
//        memcpy(permutation_set->permutations, prev_perm_set->permutations,
//               prev_perm_set->permutation_c * sizeof(Mask));
//        if (v == 0 || v == adj_c) {
//            double mine = (double) v == adj_c;
//            for (int32_t i = 0; i < adj_c; i++) {
//                bool already_solved = false;
//                for (int32_t j = 0; j < edge->edge_solved_c; j++) {
//                    if (edge->edge_solved[j] == adj[i]) {
//                        if (pmap->p_solved[j] != mine) {
//                            permutation_set->valid = false;
//                            pmap->valid = false;
//                            return;
//                        }
//                        already_solved = true;
//                        break;
//                    }
//                }
//                if (already_solved) continue;
//                edge->edge_solved[edge->edge_solved_c] = adj[i];
//                pmap->p_solved[edge->edge_solved_c] = mine;
//                edge->edge_solved_c++;
//            }
//        }
//    }
//    else {
//        memcpy(permutation_set->permutations, prev_perm_set->permutations,
//               prev_perm_set->permutation_c * sizeof(Mask));
//        if (v == 0 || v == adj_c) {
//            double mine = (double) v == adj_c;
//            for (int32_t i = 0; i < adj_c; i++) {
//                bool already_solved = false;
//                for (int32_t j = 0; j < edge->edge_solved_c; j++) {
//                    if (edge->edge_solved[j] == adj[i]) {
//                        if (pmap->p_solved[j] != mine) {
//                            permutation_set->valid = false;
//                            pmap->valid = false;
//                            return;
//                        }
//                        already_solved = true;
//                        break;
//                    }
//                }
//                if (already_solved) continue;
//                edge->edge_solved[edge->edge_solved_c] = adj[i];
//                pmap->p_solved[edge->edge_solved_c] = mine;
//                edge->edge_solved_c++;
//            }
//
//            int32_t after_i = permutation_set->splits_start[current_split];
//            for (int32_t i = permutation_set->splits_start[current_split];
//                 i < permutation_set->splits_start[current_split] + permutation_set->splits_length[current_split]; i++) {
//
//                bool valid_perm = true;
//                for (int32_t j = 0; j < edge->splits_length[current_split]; j++) {
//                    
//                    if (edge->edge[edge->splits_start[current_split] + j] == pos) {
//                        
//                    }
//                }
//            }
//        }
//    }
//}
//
//
//bool unknown_in_one_split(Board* board, Edge* edge, int32_t pos) {
//    int32_t adj[9];
//    int32_t adj_c = get_adjacent_unknown(board, pos, adj);
//    adj[adj_c++] = pos;
//    
//    int32_t split;
//    bool split_found = false;
//
//    int32_t splits[9];
//    return get_splits(edge, adj, adj_c, splits) < 2;
//}