#include <stdint.h>
#include <stdbool.h>
#include <alloca.h>
#include <malloc.h>

#include "permutations.h"

void set_equations(Board* board, Border* border, Equation** equations) {
    for (int32_t i = 0; i < border->border_known_c; i++) {
        Equation* equation = equations[i];
        equation->unknown_c = 0;
        equation->amount = board->v[border->border_known[i]];

        int32_t adj[8];
        int32_t adj_c = get_adjacent(board, border->border_known[i], adj);

        for (int32_t j = 0; j < adj_c; j++) {
            if (!board->known[adj[j]]) {
                for (int32_t k = 0; k < border->border_unknown_c; k++) {
                    if (border->border_unknown[k] == adj[j]) {
                        equation->unknown[equation->unknown_c++] = k;
                        break;
                    }
                }
            }
        }
    }
}

void print_equation(Equation* eq) {
    printf("Equation - unknown_c: %d amount: %d\n", eq->unknown_c, eq->amount);
    for (int32_t i = 0; i < eq->unknown_c; i++) {
        printf("%d ", eq->unknown[i]);   
    }
    printf("\n");
}

bool is_subequation(Equation* eq, Equation* super_eq) {
    if (eq->unknown_c == 0 || super_eq->unknown_c == 0) return false;
    for (int32_t i = 0; i < eq->unknown_c; i++) {
        bool any_match = false;
        for (int32_t j = 0; j < super_eq->unknown_c; j++) {
            if (eq->unknown[i] == super_eq->unknown[j]) any_match = true;
        }
        if (!any_match) return false;
    }
    return true;
}

bool equations_intersect(Equation* eq1, Equation* eq2) {
    for (int32_t i = 0; i < eq1->unknown_c; i++) {
        for (int32_t j = 0; j < eq2->unknown_c; j++) {
            if (eq1->unknown[i] == eq2->unknown[j]) return true;
        }
    }
    return false;
}

void remove_subequation(Equation* eq, Equation* super_eq) {
    int32_t new_i = 0;
    for (int32_t i = 0; i < super_eq->unknown_c; i++) {
        bool any_match = false;
        for (int32_t j = 0; j < eq->unknown_c; j++) {
            if (super_eq->unknown[i] == eq->unknown[j]) any_match = true;
        }
        if (!any_match) {
            super_eq->unknown[new_i++] = super_eq->unknown[i];
        }
    }
    super_eq->unknown_c = new_i;
    super_eq->amount -= eq->amount;
}

int32_t remove_solved(Equation** equations, int32_t equation_c, int32_t* solved){
    int32_t new_i = 0;
    for (int32_t i = 0; i < equation_c; i++) {
        if (equations[i]->unknown_c == equations[i]->amount){
            for (int32_t j = 0; j < equations[i]->unknown_c; j++){
                solved[equations[i]->unknown[j]] = 1;
            }
        }
        else if (equations[i]->amount == 0) {
            for (int32_t j = 0; j < equations[i]->unknown_c; j++){
                solved[equations[i]->unknown[j]] = 0;
            }
        }
        else {
            equations[new_i++] = equations[i];
        }
    }
    equation_c = new_i;
    for (int32_t i = 0; i < equation_c; i++) {
        int32_t new_j = 0;
        for (int32_t j = 0; j < equations[i]->unknown_c; j++){
            if (solved[equations[i]->unknown[j]] == -1) {
                equations[i]->unknown[new_j++] = equations[i]->unknown[j];
            }
            else {
                equations[i]->amount -= solved[equations[i]->unknown[j]];
            }
        }
        equations[i]->unknown_c = new_j;
    }
    return equation_c;
}

bool remove_subequations(Equation** equations, int32_t equation_c) {
    bool any_changed = false;
    for (int32_t i = 0; i < equation_c; i++) {
        for (int32_t j = i + 1; j < equation_c; j++) {
            if (is_subequation(equations[i], equations[j])){
                remove_subequation(equations[i], equations[j]);
                any_changed = true;
            }
            if (is_subequation(equations[j], equations[i])){
                remove_subequation(equations[j], equations[i]);
                any_changed = true;
            }
        }
    }
    return any_changed;
}

int32_t reduce(Equation** equations, int32_t equation_c, int32_t* solved) {
    bool any_changed = true;
    while (any_changed) {

        if (DEBUG) {
            printf("-------------------------------------------------------\n");
            for (int32_t i = 0; i < equation_c; i++) {
                printf("Equation: %d\n", i);
                print_equation(equations[i]);
            }
        }
        any_changed = false;
        any_changed |= remove_subequations(equations, equation_c);
        int32_t new_c = remove_solved(equations, equation_c, solved);
        any_changed |= new_c != equation_c;
        equation_c = new_c;
    }
    return equation_c;
}

int32_t split(Equation** equations, int32_t equation_c, int32_t* splits) {
    // Allocate memory for labels
    int32_t* split_labels = alloca(equation_c * sizeof(int32_t));
    memset(split_labels, 0, equation_c * sizeof(int32_t));

    // Create variables for search algorithm
    int32_t queue[200];
    int32_t q;
    bool* explored = alloca(equation_c * sizeof(int32_t));

    int32_t label = 1;
    for (int32_t start_eq = 0; start_eq < equation_c; start_eq++) {
        if (split_labels[start_eq] != 0) continue;
        // DFS
        queue[0] = start_eq;
        q = 0;
        memset(explored, 0, equation_c * sizeof(bool));
        explored[start_eq] = true;

        while (q>=0) {
            int32_t cur = queue[q];
            q--;
            split_labels[cur] = label;

            for (int32_t i = start_eq + 1; i < equation_c; i++) { //This can start at start_eq + 1 since all before will be labeled
                if (i != cur && equations_intersect(equations[cur], equations[i]) && !explored[i]){
                    queue[++q] = i;
                    explored[i] = true;
                }
            }
        }
        label++;
    }

    if (DEBUG) {
        printf("--------SPLIT_LABELS---------\n");
        for (int32_t i = 0; i < equation_c; i++) printf("%d ", i);
        printf("\n");
        for (int32_t i = 0; i < equation_c; i++) printf("%d ", split_labels[i]);
        printf("\n");
    }

    // Sort by labels and add ends to splits[]
    int32_t split_c = label - 1;
    int32_t prev_set = 0;
    for (int32_t label = 1; label <= split_c; label++) {
        int32_t prev_eq = prev_set;
        for (int32_t i = prev_set; i < equation_c; i++) {
            if (split_labels[i] == label) {
                Equation* temp = equations[prev_eq];
                equations[prev_eq] = equations[i];
                equations[i] = temp;
                split_labels[i] = split_labels[prev_eq];
                prev_eq++;
            }
        }
        splits[label-1] = prev_eq;
        prev_set = prev_eq;
    }

    return split_c;    
}

void equation_permutations(Equation* equation, PermutationSet* permutation_set){
    uint64_t mask[PERMUTATION_PARTS] = {0};
    for (int32_t i = 0; i < equation->unknown_c; i++) {
        mask[equation->unknown[i]/64] |= 1ULL << ((uint64_t)(equation->unknown[i] % 64));
    }

    permutation_set->permutation_c = 0;

    for (uint64_t x = 0; x < (1 << (uint64_t)equation->unknown_c); x++) {
        if (__builtin_popcountll(x) == equation->amount) {
            uint64_t bombs[PERMUTATION_PARTS] = {0};
            for (uint64_t i = 0; i < equation->unknown_c; i++) {
                if (x & (1 << i)) {
                    bombs[equation->unknown[i]/64] |= 1LL << ((uint64_t)(equation->unknown[i] % 64));
                }
            }
            permutation_set->permutations[permutation_set->permutation_c].bomb_amount = equation->amount;
            memcpy(permutation_set->permutations[permutation_set->permutation_c].mask, &mask, sizeof(mask));
            memcpy(permutation_set->permutations[permutation_set->permutation_c].bombs, &bombs, sizeof(bombs));
            permutation_set->permutation_c++;
        }
    }
}

bool permutation_intersect(Permutation* permutation1, Permutation* permutation2) {
    for (int32_t i = 0; i < PERMUTATION_PARTS; i++) {
        if (permutation1->mask[i] & permutation2->mask[i]) return true;
    }
    return false;
}

bool join_permutations(Permutation* new_permutation, Permutation* permutation1, Permutation* permutation2) {
    // Join permutation1 and permutation2 into new_permutation, if they dont work together return false
    new_permutation->bomb_amount = 0;
    for (int32_t i = 0; i < PERMUTATION_PARTS; i++) {
        if (permutation1->mask[i] & permutation2->mask[i] & ((permutation1->bombs[i] & ~permutation2->bombs[i]) | (~permutation1->bombs[i] & permutation2->bombs[i]))) return false;
        new_permutation->mask[i] = permutation1->mask[i] | permutation2->mask[i];
        new_permutation->bombs[i] = permutation1->bombs[i] | permutation2->bombs[i];
        new_permutation->bomb_amount += __builtin_popcountll(new_permutation->bombs[i]);
    }
    return true;
}

void join_permutationsets(PermutationSet* permutation_set1, PermutationSet* permutation_set2) {
    // Join permutationsets into permutation_set1

    // Copy permutation_set1 to temp_permutation_set
    int32_t permutation_set_size = sizeof(PermutationSet) + permutation_set1->permutation_c * sizeof(Permutation);
    PermutationSet* temp_permutation_set = alloca(permutation_set_size);
    memcpy(temp_permutation_set, permutation_set1, permutation_set_size);
    permutation_set1->permutation_c = 0;

    for (int32_t i = 0; i < temp_permutation_set->permutation_c; i++) {
        for (int32_t j = 0; j < permutation_set2->permutation_c; j++) {
            if (join_permutations(&permutation_set1->permutations[permutation_set1->permutation_c], 
                                  &temp_permutation_set->permutations[i], &permutation_set2->permutations[j])) {
                permutation_set1->permutation_c++;
            }
        }
    }
}

void permutations_of_split(Equation** equations, int32_t equation_start, int32_t equation_end, PermutationSet* permutation_set) {
    PermutationSet* temp_perm_set = alloca(sizeof(PermutationSet) + 64* sizeof(Permutation));
    // First set permutationset to first equations permutations
    equation_permutations(equations[equation_start], permutation_set);

    if (DEBUG) {
        printf("---------split permutations: %d - %d ---------------\n", equation_start, equation_end);
        printf("Equation: %d\n", equation_start);
        printf("Permutations:\n");
        print_permutations(40, permutation_set);
        printf("Total Permutations:\n");
        print_permutations(40, permutation_set);
        printf("\n");
    }
    

    //Then join with rest of equations permutations
    for (int32_t i = equation_start + 1; i < equation_end; i++) {
        equation_permutations(equations[i], temp_perm_set);
        join_permutationsets(permutation_set, temp_perm_set);
        if (DEBUG) {
            printf("Equation: %d\n", i);
            printf("Permutations:\n");
            print_permutations(40, temp_perm_set);
            printf("Total Permutations:\n");
            print_permutations(40, permutation_set);
            printf("\n");
        }
        
    }
}

void trivial_permutation_set(Border* border, int32_t* solved, PermutationSet* permutation_set) {
    permutation_set->permutation_c = 1;
    permutation_set->permutations[0].bomb_amount = 0;
    for (int32_t i = 0; i < PERMUTATION_PARTS; i++) {
        permutation_set->permutations[0].bombs[i] = 0;
        permutation_set->permutations[0].mask[i] = 0;
    }
    for (uint64_t i = 0; i < border->border_unknown_c; i++) {
        if (solved[i] == 0){
            permutation_set->permutations[0].mask[i / 64] |= 1ULL << (i % 64);
        }
        else if (solved[i] == 1){
            permutation_set->permutations[0].mask[i / 64] |= 1ULL << (i % 64);
            permutation_set->permutations[0].bombs[i / 64] |= 1ULL << (i % 64);
            permutation_set->permutations[0].bomb_amount += 1;
        }
    }
}

void print_permutations(int32_t border_unknown_c, PermutationSet* permutation_set) {
    printf("Permutation size: %d\n", permutation_set->permutation_c);
    for (int32_t i = 0; i < permutation_set->permutation_c; i++) {
        for (uint64_t j = 0; j < border_unknown_c; j++) {
            if ((permutation_set->permutations[i].mask[j / 64] >> (j % 64)) & 1ULL) {
                printf("%llu", (permutation_set->permutations[i].bombs[j / 64] >> (j % 64)) & 1ULL);
            }
            else {
                printf(" ");
            }
        }
        printf(":%d\n", permutation_set->permutations[i].bomb_amount);
    }
}

void get_permutations(Board* board, Border* border, PermutationSet* permutation_set){
    // Statically allocated memory for equations and permutations
    static EquationSet equation_set;
    static uint8_t permutation_set_mem[MAX_PERMUTATIONS*sizeof(Permutation) + sizeof(PermutationSet)];
    static PermutationSet* permutation_set = (PermutationSet*) permutation_set_mem;

    //Allocate memory for equations and create pointers
    int32_t equation_c = border->border_known_c;
    Equation* equations_s = alloca(equation_c * sizeof(Equation));
    Equation** equations = alloca(equation_c * sizeof(Equation*));
    for (int32_t i = 0; i < equation_c; i++) equations[i] = &equations_s[i];

    // Initialize equations
    set_equations(board, border, equations);

    if (DEBUG) {
        for (int32_t i = 0; i < equation_c; i++) {
            printf("Equation: %d\n", i);
            print_equation(equations[i]);
        }
    }
    

    // Reduce equations
    int32_t* solved = alloca(sizeof(int32_t) * border->border_unknown_c);
    for (int32_t i = 0; i < border->border_unknown_c; i++) solved[i] = -1;
    equation_c = reduce(equations, equation_c, solved);

    if (DEBUG) {
        printf("Solved: '");
        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            if (solved[i] == -1) printf(" ");
            else printf("%d", solved[i]);
        }
        printf("'\n");
    

        printf("REDUCED!\n");
        for (int32_t i = 0; i < equation_c; i++) {
            printf("Equation: %d\n", i);
            print_equation(equations[i]);
        }    
    }

    //Split equations
    int32_t splits[200]; // Lets hope there's not more than 200 splits ;)
    int32_t split_c = split(equations, equation_c, splits);
    
    if (DEBUG) {
        printf("--- SPLITS -----\n");
        for (int32_t i = 0; i < split_c; i++) printf("%d ", splits[i]);
        for (int32_t i = 0; i < equation_c; i++) {
            printf("\nEquation: %d\n", i);
            print_equation(equations[i]);
        }    
    }
    

    // Create trivial permutation of solved equations
    trivial_permutation_set(border, solved, permutation_set);

    PermutationSet* temp_perm_set = malloc(sizeof(PermutationSet) + 128UL * 1024UL * sizeof(Permutation));
    int32_t split_prev = 0;
    for (int32_t i = 0; i < split_c; i++) {
        if (DEBUG) print_permutations(border->border_unknown_c, permutation_set);
 
        permutations_of_split(equations, split_prev, splits[i], temp_perm_set);
        join_permutationsets(permutation_set, temp_perm_set);
        split_prev = splits[i];
    }

    if (DEBUG) print_permutations(border->border_unknown_c, permutation_set);

    // Free memory
    free(temp_perm_set);
}