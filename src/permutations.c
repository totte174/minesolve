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

FaultStatus join_new_permutations(PermutationSet* permutation_set, Mask* mask1, Mask* mask2, int32_t split_i, Mask* new_permutations, int32_t new_permutations_c) {
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
                    free(temp_permutations);
                    return fault_computational_limit;
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
    return valid_status;
}

inline int32_t mine_c(Mask* permutation) {
    return mask_count(*permutation);
}

FaultStatus get_permutations_from_split(PermutationSet* permutation_set, EquationSet* equation_set, int32_t split_i) {
    FaultStatus fault_status = valid_status;
    static Mask temp_equation_perms[256];
    int32_t split_start = permutation_set->permutation_c;
    permutation_set->splits_start[split_i] = split_start;

    int32_t equation_start = equation_set->splits_start[split_i];
    int32_t equation_end = equation_set->splits_start[split_i] + equation_set->splits_length[split_i];

    Mask split_mask = equation_set->equations[equation_start].mask;
    permutation_set->splits_length[split_i] = equation_permutations(equation_set->equations + equation_start, permutation_set->permutations + split_start);
    
    for (int32_t i = equation_start + 1; i < equation_end; i++) {
        int32_t equation_perms = equation_permutations(equation_set->equations + i, temp_equation_perms);

        fault_status = join_new_permutations(permutation_set, &split_mask, &equation_set->equations[i].mask, 
                                             split_i, temp_equation_perms, equation_perms);
        if (fault_status) return fault_status;

        for (int32_t j = 0; j < MASK_PARTS; j++) {
            split_mask.v[j] |= equation_set->equations[i].mask.v[j];
        }
    }
    permutation_set->permutation_c += permutation_set->splits_length[split_i];
    return fault_status;
}

FaultStatus get_permutation_set(Board* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap){
    static EquationSet equation_set;

    permutation_set->initialized = false;

    FaultStatus fault_status = get_equation_set(board, edge, &equation_set, pmap);
    if (fault_status) return fault_status;

    permutation_set->permutation_size = MIN_PERMUTATIONS;
    permutation_set->permutations = malloc(sizeof(Mask) * permutation_set->permutation_size);
    permutation_set->permutation_c = 0;
    permutation_set->initialized = true;
    permutation_set->split_c = equation_set.split_c;

    for (int32_t split_i = 0; split_i < equation_set.split_c; split_i++) {
        fault_status = get_permutations_from_split(permutation_set, &equation_set, split_i);
        if (fault_status) return fault_status;
    }
    return fault_status;
}

void permutation_set_deinit(PermutationSet* permutation_set) {
    if (permutation_set->initialized) free(permutation_set->permutations);
}