#include <stdint.h>
#include <stdbool.h>
#include <alloca.h>
#include <malloc.h>
#include <stdlib.h>

#include "permutations.h"

void get_equations(Board* board, Border* border, EquationSet* equation_set) {
    equation_set->border_unknown_c = border->border_unknown_c;
    equation_set->equation_c = border->border_known_c;
    equation_set->split_c = 0;

    mask_reset(equation_set->solved.mask);
    mask_reset(equation_set->solved.mines);

    for (int32_t i = 0; i < border->border_known_c; i++) {
        Equation* equation = equation_set->equations + i;
        mask_reset(equation->mask);
        equation->amount = board->v[border->border_known[i]];

        int32_t adj[8];
        int32_t adj_c = get_adjacent_unknown(board, border->border_known[i], adj);

        for (int32_t k = 0; k < border->border_unknown_c; k++) {
            for (int32_t j = 0; j < adj_c; j++) {
                if (border->border_unknown[k] == adj[j]) {
                    mask_set(equation->mask, k, 1);
                    break;
                }
            }
        }
    }
}

void print_equation(Equation* eq, int32_t border_unknown_c) {
    for (uint64_t j = 0; j < border_unknown_c; j++) {
        if (mask_get(eq->mask, j)) {
            printf("%d", 1);
        }
        else {
            printf(" ");
        }
    }
    printf(":%d\n", eq->amount);
}

void print_equation_set(EquationSet* equation_set) {
    if (mask_count(equation_set->solved.mask) > 0) {
        printf("Solved:\n");
        print_permutation(&equation_set->solved, equation_set->border_unknown_c);
    }
    printf("Equations: %d\n", equation_set->equation_c);
    if (equation_set->split_c == 0) {
        for(int32_t i = 0; i < equation_set->equation_c; i++) {
            print_equation(equation_set->equations + i, equation_set->border_unknown_c);
        }
    }
    else {
        for(int32_t j = 0; j < equation_set->split_c; j++) {
            printf("--- Split %d\n", j);
            for(int32_t i = 0; i < equation_set->splits_length[j]; i++) {
                print_equation(equation_set->equations + equation_set->splits_start[j] + i, equation_set->border_unknown_c);
            }
        }   
    }
}

bool is_subequation(Equation* eq, Equation* super_eq) {
    if (!mask_overlap(eq->mask, super_eq->mask)) return false;
    for (int32_t i = 0; i < MASK_PARTS; i++) {
        if (eq->mask.v[i] & ~super_eq->mask.v[i]) return false;
    }
    return true;
}

bool equations_intersect(Equation* eq1, Equation* eq2) {
    return mask_overlap(eq1->mask, eq2->mask);
}

void remove_subequation(Equation* eq, Equation* super_eq) {
    for (int32_t i = 0; i < MASK_PARTS; i++) {
        super_eq->mask.v[i] &= ~eq->mask.v[i];
    }
    super_eq->amount -= eq->amount;
}

bool remove_solved(EquationSet* equation_set){
    bool any_changed = false;
    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        Equation* eq = equation_set->equations + i;
        if (mask_count(eq->mask) == eq->amount){ 
            //All unknown in equation are mines
            for (int32_t i = 0; i < MASK_PARTS; i++) {
                equation_set->solved.mask.v[i] |= eq->mask.v[i];
                equation_set->solved.mines.v[i] |= eq->mask.v[i];
            }
            equation_set->equations[i] = equation_set->equations[equation_set->equation_c - 1];
            i--;
            equation_set->equation_c--;
            any_changed = true;
        }
        else if (eq->amount == 0) { 
            //All unknown in equation are non-mines
            for (int32_t i = 0; i < MASK_PARTS; i++) {
                equation_set->solved.mask.v[i] |= eq->mask.v[i];
            }
            equation_set->equations[i] = equation_set->equations[equation_set->equation_c - 1];
            i--;
            equation_set->equation_c--;
            any_changed = true;
        }
    }

    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        //Remove solved from equations
        Equation* eq = equation_set->equations + i;
        if (mask_overlap(equation_set->solved.mask, eq->mask)){
            for (int32_t i = 0; i < MASK_PARTS; i++) {
                eq->amount -= __builtin_popcountll(equation_set->solved.mines.v[i] & eq->mask.v[i]);
                eq->mask.v[i] &= ~equation_set->solved.mask.v[i];
            }
            any_changed = true;
        }
    }
    return any_changed;
}

bool remove_subequations(EquationSet* equation_set) {
    bool any_changed = false;
    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        for (int32_t j = i + 1; j < equation_set->equation_c; j++) {
            if (is_subequation(equation_set->equations + i, equation_set->equations + j)){
                remove_subequation(equation_set->equations + i, equation_set->equations + j);
                any_changed = true;
            }
            if (is_subequation(equation_set->equations + j, equation_set->equations + i)){
                remove_subequation(equation_set->equations + j, equation_set->equations + i);
                any_changed = true;
            }
        }
    }
    return any_changed;
}

void reduce(EquationSet* equation_set) {
    bool any_changed = true;
    int32_t iterations = 0;
    while (any_changed) {
        any_changed = false;
        any_changed |= remove_subequations(equation_set);
        any_changed |= remove_solved(equation_set);
        iterations++;
    }
}

void split(EquationSet* equation_set) {
    // Allocate memory for labels
    int32_t split_labels[MAX_SQUARES] = {0};

    // Create variables for search algorithm
    int32_t queue[200];
    int32_t q;
    bool explored[MAX_SQUARES] = {false};

    int32_t label = 1;
    for (int32_t start_eq = 0; start_eq < equation_set->equation_c; start_eq++) {
        if (split_labels[start_eq] != 0) continue;
        // DFS
        queue[0] = start_eq;
        q = 0;
        explored[start_eq] = true;

        while (q>=0) {
            int32_t cur = queue[q];
            q--;
            split_labels[cur] = label;

            for (int32_t i = start_eq + 1; i < equation_set->equation_c; i++) { //This can start at start_eq + 1 since all before will be labeled
                if (i != cur && equations_intersect(equation_set->equations + cur, equation_set->equations + i) && !explored[i]){
                    queue[++q] = i;
                    explored[i] = true;
                }
            }
        }
        label++;
    }

    // Sort by labels and add ends to splits[]
    equation_set->split_c = label - 1;
    int32_t start_eq = 0;
    for (int32_t label = 1; label <= equation_set->split_c; label++) {
        int32_t end_eq = start_eq;
        for (int32_t i = start_eq; i < equation_set->equation_c; i++) {
            if (split_labels[i] == label) {
                Equation temp = equation_set->equations[end_eq];
                equation_set->equations[end_eq] = equation_set->equations[i];
                equation_set->equations[i] = temp;
                split_labels[i] = split_labels[end_eq];
                end_eq++;
            }
        }
        equation_set->splits_start[label-1] = start_eq;
        equation_set->splits_length[label-1] = end_eq - start_eq;
        start_eq = end_eq;
    }
}

int32_t equation_permutations(Equation* equation, Permutation* permutations){
    int32_t map[8];
    int32_t j = 0;
    for (int32_t i = 0; i < MAX_BORDER_UNKNOWN; i++) {
        if (mask_get(equation->mask, i)) map[j++] = i;
    }

    int32_t permutation_c = 0;
    int32_t unknown_c = mask_count(equation->mask);
    for (uint64_t x = 0; x < (1 << unknown_c); x++) {
        if (__builtin_popcountll(x) == equation->amount) {
            permutations[permutation_c].mask = equation->mask;
            mask_reset(permutations[permutation_c].mines);

            for (uint64_t i = 0; i < unknown_c; i++) {
                if (x & (1 << i)) {
                    mask_set(permutations[permutation_c].mines, map[i], 1);
                }
            }
            permutation_c++;
        }
    }
    return permutation_c;
}

bool permutation_intersect(Permutation* permutation1, Permutation* permutation2) {
    return mask_overlap(permutation1->mask, permutation2->mask);
}

bool join_permutations(Permutation* new_permutation, Permutation* permutation1, Permutation* permutation2) {
    // Join permutation1 and permutation2 into new_permutation, if they dont work together return false
    for (int32_t i = 0; i < MASK_PARTS; i++) {
        if (permutation1->mask.v[i] & permutation2->mask.v[i] & ((permutation1->mines.v[i] & ~permutation2->mines.v[i]) | (~permutation1->mines.v[i] & permutation2->mines.v[i]))) return false;
        new_permutation->mask.v[i] = permutation1->mask.v[i] | permutation2->mask.v[i];
        new_permutation->mines.v[i] = permutation1->mines.v[i] | permutation2->mines.v[i];
    }
    return true;
}

int32_t join_permutation_arrays(Permutation* permutations1, int32_t permutations1_c, Permutation* permutations2, int32_t permutations2_c) {
    // Join permutations into permutations1

    // Copy permutations1 to temp_permutation_set
    Permutation* temp_permutations = malloc(sizeof(Permutation) * permutations1_c);
    memcpy(temp_permutations, permutations1, sizeof(Permutation) * permutations1_c);
    int32_t permutation_c = 0;

    for (int32_t i = 0; i < permutations1_c; i++) {
        for (int32_t j = 0; j < permutations2_c; j++) {
            if (permutation_c >= MAX_PERMUTATIONS / 2) { // Quick fix for the really hard boards that lead to crashes
                free(temp_permutations);
                return permutation_c;
            }
            if (join_permutations(permutations1 + permutation_c, 
                                  temp_permutations + i, permutations2 + j)) {
                permutation_c++;
            }
        }
    }
    free(temp_permutations);
    return permutation_c;
}

void permutations_of_splits(EquationSet* equation_set, PermutationSet* permutation_set) {
    Permutation temp_equation_perms[256];

    permutation_set->permutation_c = 0;
    permutation_set->split_c = equation_set->split_c;
    for (int32_t split_i = 0; split_i < equation_set->split_c; split_i++) {
        int32_t split_start = permutation_set->permutation_c;
        permutation_set->splits_start[split_i] = split_start;

        int32_t equation_start = equation_set->splits_start[split_i];
        int32_t equation_end = equation_set->splits_start[split_i] + equation_set->splits_length[split_i];

        int32_t split_permutations = equation_permutations(equation_set->equations + equation_start, permutation_set->permutations + split_start);
        
        for (int32_t i = equation_start + 1; i < equation_end; i++) {
            int32_t equation_perms = equation_permutations(equation_set->equations + i, temp_equation_perms);
            split_permutations = join_permutation_arrays(permutation_set->permutations + split_start, split_permutations, temp_equation_perms, equation_perms);
        }
        permutation_set->splits_length[split_i] = split_permutations;
        permutation_set->permutation_c += split_permutations;
    }
}

inline int32_t mine_c(Permutation* permutation) {
    return mask_count(permutation->mines);
}

void print_permutation(Permutation* permutation, int32_t border_unknown_c) {
    for (uint64_t j = 0; j < border_unknown_c; j++) {
        if (mask_get(permutation->mask, j)) {
            printf("%llu", mask_get(permutation->mines, j));
        }
        else {
            printf(" ");
        }
    }
    printf(":%lu\n", mask_count(permutation->mines));
}

void print_permutation_set(PermutationSet* permutation_set, int32_t border_unknown_c) {
    printf("Solved permutation:\n");
    print_permutation(&permutation_set->solved_permutation, border_unknown_c);
    printf("Permutations: %d\n", permutation_set->permutation_c);
    for (int32_t split_i = 0; split_i < permutation_set->split_c; split_i++) {
        printf("--- Split %d\n", split_i);
        for (int32_t i = 0; i < permutation_set->splits_length[split_i]; i++) {
            print_permutation(permutation_set->permutations + i + permutation_set->splits_start[split_i], border_unknown_c);
        }
    }
}

PermutationSet* get_permutations(Board* board, Border* border){
    // Statically allocated memory for equations and permutations
    EquationSet equation_set;
    static PermutationSet permutation_set;

    get_equations(board, border, &equation_set);

    reduce(&equation_set);

    split(&equation_set);


    permutation_set.solved_permutation = equation_set.solved;
    permutations_of_splits(&equation_set, &permutation_set);

    if (DEBUG) 
        print_permutation_set(&permutation_set, border->border_unknown_c);

    return &permutation_set;
}