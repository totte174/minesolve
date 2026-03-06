#include "permutations.h"

int32_t equation_permutations(Equation* equation, Mask* permutations){
    int32_t map[8];
    int32_t j = 0;
    for (int32_t i = 0; i < MAX_EDGE_SIZE; i++) {
        if (mask_get(equation->mask, i)) map[j++] = i;
    }

    int32_t unknown_c = mask_count(equation->mask);
    int32_t amount = equation->amount;
    if (amount == 0) { mask_reset(permutations[0]); return 1; }
    if (amount > unknown_c) return 0;

    // Gosper's hack for generating combinations
    int32_t permutation_c = 0;
    uint64_t x = (1ULL << amount) - 1;
    uint64_t limit = 1ULL << unknown_c;
    while (x < limit) {
        mask_reset(permutations[permutation_c]);
        for (int32_t i = 0; i < unknown_c; i++)
            if (x & (1ULL << i)) mask_set(permutations[permutation_c], map[i], 1);
        permutation_c++;
        uint64_t c = x & (~x + 1);
        uint64_t r = x + c;
        x = (((r ^ x) >> 2) / c) | r;
    }
    return permutation_c;
}

MsStatus join_new_permutations(PermutationSet* permutation_set, Mask* mask1, Mask* mask2, int32_t split_i, Mask* new_permutations, int32_t new_permutations_c) {
    // Copy current split's permutations to temp buffer
    static Mask* temp_permutations = NULL;
    static int32_t temp_cap = 0;
    int32_t needed = permutation_set->splits_length[split_i];
    if (needed > temp_cap) {
        temp_permutations = realloc(temp_permutations, sizeof(Mask) * needed);
        temp_cap = needed;
    }
    memcpy(temp_permutations,
           permutation_set->permutations + permutation_set->splits_start[split_i],
           sizeof(Mask) * needed);

    // Precompute overlap mask — constant across all permutation pairs
    uint64_t ov0 = mask1->v[0] & mask2->v[0];
    uint64_t ov1 = mask1->v[1] & mask2->v[1];
    uint64_t ov2 = mask1->v[2] & mask2->v[2];

    int32_t permutation_c = 0;
    for (int32_t i = 0; i < needed; i++) {
        // Ensure capacity for up to new_permutations_c more results before inner loop
        if (permutation_set->permutation_c + permutation_c + new_permutations_c > permutation_set->permutation_size) {
            if ((uint64_t)permutation_set->permutation_size >= MAX_PERMUTATIONS) {
                return MS_ERR_COMPUTATIONAL_LIMIT;
            }
            permutation_set->permutation_size <<= 1;
            permutation_set->permutations = realloc(permutation_set->permutations, sizeof(Mask) * permutation_set->permutation_size);
        }

        Mask* p1 = temp_permutations + i;
        for (int32_t j = 0; j < new_permutations_c; j++) {
            Mask* p2 = new_permutations + j;
            if ((ov0 & (p1->v[0] ^ p2->v[0])) |
                (ov1 & (p1->v[1] ^ p2->v[1])) |
                (ov2 & (p1->v[2] ^ p2->v[2]))) continue;
            Mask* out = permutation_set->permutations + permutation_set->splits_start[split_i] + permutation_c;
            out->v[0] = p1->v[0] | p2->v[0];
            out->v[1] = p1->v[1] | p2->v[1];
            out->v[2] = p1->v[2] | p2->v[2];
            permutation_c++;
        }
    }
    permutation_set->splits_length[split_i] = permutation_c;
    return MS_OK;
}

inline int32_t mine_c(Mask* permutation) {
    return mask_count(*permutation);
}

MsStatus get_permutations_from_split(PermutationSet* permutation_set, EquationSet* equation_set, int32_t split_i) {
    MsStatus fault_status = MS_OK;
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

MsStatus get_permutation_set(MsBoard* board, Edge* edge, PermutationSet* permutation_set, ProbabilityMap* pmap){
    static EquationSet equation_set;

    permutation_set->initialized = false;

    MsStatus fault_status = get_equation_set(board, edge, &equation_set, pmap);
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
    if (permutation_set->initialized) {
        free(permutation_set->permutations);
        permutation_set->initialized = false;
    }
}
