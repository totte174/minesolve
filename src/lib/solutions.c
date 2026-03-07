#include "solutions.h"

/* Enumerates all valid mine assignments for a single equation into solutions array. */
int32_t equation_solutions(Equation* equation, Mask* solutions){
    int32_t map[8];
    int32_t j = 0;
    for (int32_t i = 0; i < MAX_FRONTIER_SIZE; i++) {
        if (mask_get(equation->mask, i)) map[j++] = i;
    }

    int32_t unknown_c = mask_count(equation->mask);
    int32_t amount = equation->mine_count;
    if (amount == 0) { mask_reset(solutions[0]); return 1; }
    if (amount > unknown_c) return 0;

    // Gosper's hack for generating combinations
    int32_t solution_c = 0;
    uint64_t x = (1ULL << amount) - 1;
    uint64_t limit = 1ULL << unknown_c;
    while (x < limit) {
        mask_reset(solutions[solution_c]);
        for (int32_t i = 0; i < unknown_c; i++)
            if (x & (1ULL << i)) mask_set(solutions[solution_c], map[i], 1);
        solution_c++;
        uint64_t c = x & (~x + 1);
        uint64_t r = x + c;
        x = (((r ^ x) >> 2) / c) | r;
    }
    return solution_c;
}

/* Joins new solutions for an equation into the current group's solution list. */
MsStatus join_solutions(SolutionSet* solution_set, Mask* mask1, Mask* mask2, int32_t group_i, Mask* new_solutions, int32_t new_solutions_c) {
    // Copy current group's solutions to temp buffer
    static Mask* temp_solutions = NULL;
    static int32_t temp_cap = 0;
    int32_t needed = solution_set->group_length[group_i];
    if (needed > temp_cap) {
        temp_solutions = realloc(temp_solutions, sizeof(Mask) * needed);
        temp_cap = needed;
    }
    memcpy(temp_solutions,
           solution_set->solutions + solution_set->group_start[group_i],
           sizeof(Mask) * needed);

    // Precompute overlap mask — constant across all solution pairs
    uint64_t ov0 = mask1->v[0] & mask2->v[0];
    uint64_t ov1 = mask1->v[1] & mask2->v[1];
    uint64_t ov2 = mask1->v[2] & mask2->v[2];

    int32_t solution_c = 0;
    for (int32_t i = 0; i < needed; i++) {
        // Ensure capacity for up to new_solutions_c more results before inner loop
        if (solution_set->solution_c + solution_c + new_solutions_c > solution_set->solution_size) {
            if ((uint64_t)solution_set->solution_size >= MAX_PERMUTATIONS) {
                return MS_ERR_COMPUTATIONAL_LIMIT;
            }
            solution_set->solution_size <<= 1;
            solution_set->solutions = realloc(solution_set->solutions, sizeof(Mask) * solution_set->solution_size);
        }

        Mask* p1 = temp_solutions + i;
        for (int32_t j = 0; j < new_solutions_c; j++) {
            Mask* p2 = new_solutions + j;
            if ((ov0 & (p1->v[0] ^ p2->v[0])) |
                (ov1 & (p1->v[1] ^ p2->v[1])) |
                (ov2 & (p1->v[2] ^ p2->v[2]))) continue;
            Mask* out = solution_set->solutions + solution_set->group_start[group_i] + solution_c;
            out->v[0] = p1->v[0] | p2->v[0];
            out->v[1] = p1->v[1] | p2->v[1];
            out->v[2] = p1->v[2] | p2->v[2];
            solution_c++;
        }
    }
    solution_set->group_length[group_i] = solution_c;
    return MS_OK;
}

inline int32_t count_mines(Mask* solution) {
    return mask_count(*solution);
}

/* Builds all valid mine assignments for a single equation group. */
MsStatus build_group_solutions(SolutionSet* solution_set, EquationSet* equation_set, int32_t group_i) {
    MsStatus status = MS_OK;
    static Mask temp_equation_sols[256];
    int32_t group_start = solution_set->solution_c;
    solution_set->group_start[group_i] = group_start;

    int32_t eq_start = equation_set->group_start[group_i];
    int32_t eq_end = equation_set->group_start[group_i] + equation_set->group_length[group_i];

    Mask group_mask = equation_set->equations[eq_start].mask;
    solution_set->group_length[group_i] = equation_solutions(equation_set->equations + eq_start, solution_set->solutions + group_start);

    for (int32_t i = eq_start + 1; i < eq_end; i++) {
        int32_t eq_sols = equation_solutions(equation_set->equations + i, temp_equation_sols);

        status = join_solutions(solution_set, &group_mask, &equation_set->equations[i].mask,
                                group_i, temp_equation_sols, eq_sols);
        if (status) return status;

        for (int32_t j = 0; j < MASK_PARTS; j++) {
            group_mask.v[j] |= equation_set->equations[i].mask.v[j];
        }
    }
    solution_set->solution_c += solution_set->group_length[group_i];
    return status;
}

MsStatus build_solution_set(MsBoard* board, Frontier* frontier, SolutionSet* solution_set, ProbabilityMap* pmap){
    static EquationSet equation_set;

    solution_set->initialized = false;

    MsStatus status = build_equation_set(board, frontier, &equation_set, pmap);
    if (status) return status;

    solution_set->solution_size = MIN_PERMUTATIONS;
    solution_set->solutions = malloc(sizeof(Mask) * solution_set->solution_size);
    solution_set->solution_c = 0;
    solution_set->initialized = true;
    solution_set->group_c = equation_set.group_c;

    for (int32_t group_i = 0; group_i < equation_set.group_c; group_i++) {
        status = build_group_solutions(solution_set, &equation_set, group_i);
        if (status) return status;
    }
    return status;
}

void solution_set_free(SolutionSet* solution_set) {
    if (solution_set->initialized) {
        free(solution_set->solutions);
        solution_set->initialized = false;
    }
}
