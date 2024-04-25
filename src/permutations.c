#include <stdint.h>
#include <stdbool.h>
#include <alloca.h>
#include <malloc.h>

#include "permutations.h"

void set_equations(Board* board, Border* border, EquationSet* equation_set) {
    equation_set->unknown_c = 0;
    equation_set->equation_c = border->border_known_c;
    equation_set->unknown_c = border->border_unknown_c;

    for (int32_t i = 0; i < border->border_unknown_c; i++) {
        equation_set->solved[i] = -1;
    }

    for (int32_t i = 0; i < border->border_known_c; i++) {
        Equation* equation = equation_set->equations[i];
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

bool remove_solved(EquationSet* equation_set){
    bool any_changed = false;
    int32_t new_i = 0;
    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        if (equation_set->equations[i]->unknown_c == equation_set->equations[i]->amount){
            for (int32_t j = 0; j < equation_set->equations[i]->unknown_c; j++){
                equation_set->solved[equation_set->equations[i]->unknown[j]] = 1;
            }
            any_changed = true;
        }
        else if (equation_set->equations[i]->amount == 0) {
            for (int32_t j = 0; j < equation_set->equations[i]->unknown_c; j++){
                equation_set->solved[equation_set->equations[i]->unknown[j]] = 0;
            }
            any_changed = true;
        }
        else {
            equation_set->equations[new_i++] = equation_set->equations[i];
        }
    }
    equation_set->equation_c = new_i;

    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        int32_t new_j = 0;
        for (int32_t j = 0; j < equation_set->equations[i]->unknown_c; j++){
            if (equation_set->solved[equation_set->equations[i]->unknown[j]] == -1) {
                equation_set->equations[i]->unknown[new_j++] = equation_set->equations[i]->unknown[j];
            }
            else {
                equation_set->equations[i]->amount -= equation_set->solved[equation_set->equations[i]->unknown[j]];
                any_changed = true;
            }
        }
        equation_set->equations[i]->unknown_c = new_j;
    }
    return any_changed;
}

bool remove_subequations(EquationSet* equation_set) {
    bool any_changed = false;
    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        for (int32_t j = i + 1; j < equation_set->equation_c; j++) {
            if (is_subequation(equation_set->equations[i], equation_set->equations[j])){
                remove_subequation(equation_set->equations[i], equation_set->equations[j]);
                any_changed = true;
            }
            if (is_subequation(equation_set->equations[j], equation_set->equations[i])){
                remove_subequation(equation_set->equations[j], equation_set->equations[i]);
                any_changed = true;
            }
        }
    }
    return any_changed;
}

void reduce(EquationSet* equation_set) {
    bool any_changed = true;
    while (any_changed) {

        if (DEBUG) {
            printf("-------------------------------------------------------\n");
            for (int32_t i = 0; i < equation_set->equation_c; i++) {
                printf("Equation: %d\n", i);
                print_equation(equation_set->equations[i]);
            }
        }
        any_changed = false;
        any_changed |= remove_subequations(equation_set);
        any_changed |= remove_solved(equation_set);
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
                if (i != cur && equations_intersect(equation_set->equations[cur], equation_set->equations[i]) && !explored[i]){
                    queue[++q] = i;
                    explored[i] = true;
                }
            }
        }
        label++;
    }

    if (DEBUG) {
        printf("--------SPLIT_LABELS---------\n");
        for (int32_t i = 0; i < equation_set->equation_c; i++) printf("%d ", i);
        printf("\n");
        for (int32_t i = 0; i < equation_set->equation_c; i++) printf("%d ", split_labels[i]);
        printf("\n");
    }

    // Sort by labels and add ends to splits[]
    equation_set->split_c = label - 1;
    int32_t prev_set = 0;
    for (int32_t label = 1; label <= equation_set->split_c; label++) {
        int32_t prev_eq = prev_set;
        for (int32_t i = prev_set; i < equation_set->equation_c; i++) {
            if (split_labels[i] == label) {
                Equation* temp = equation_set->equations[prev_eq];
                equation_set->equations[prev_eq] = equation_set->equations[i];
                equation_set->equations[i] = temp;
                split_labels[i] = split_labels[prev_eq];
                prev_eq++;
            }
        }
        equation_set->splits[label-1] = prev_eq;
        prev_set = prev_eq;
    }
}

void equation_permutations(Equation* equation, PermutationSet* permutation_set){
    uint64_t mask[PERMUTATION_PARTS] = {0};
    for (int32_t i = 0; i < equation->unknown_c; i++) {
        mask[equation->unknown[i]/64] |= 1ULL << ((uint64_t)(equation->unknown[i] % 64));
    }

    permutation_set->permutation_c = 0;

    for (uint64_t x = 0; x < (1 << (uint64_t)equation->unknown_c); x++) {
        if (__builtin_popcountll(x) == equation->amount) {
            uint64_t mines[PERMUTATION_PARTS] = {0};
            for (uint64_t i = 0; i < equation->unknown_c; i++) {
                if (x & (1 << i)) {
                    mines[equation->unknown[i]/64] |= 1LL << ((uint64_t)(equation->unknown[i] % 64));
                }
            }
            permutation_set->permutations[permutation_set->permutation_c].mine_c = equation->amount;
            memcpy(permutation_set->permutations[permutation_set->permutation_c].mask, &mask, sizeof(mask));
            memcpy(permutation_set->permutations[permutation_set->permutation_c].mines, &mines, sizeof(mines));
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
    new_permutation->mine_c = 0;
    for (int32_t i = 0; i < PERMUTATION_PARTS; i++) {
        if (permutation1->mask[i] & permutation2->mask[i] & ((permutation1->mines[i] & ~permutation2->mines[i]) | (~permutation1->mines[i] & permutation2->mines[i]))) return false;
        new_permutation->mask[i] = permutation1->mask[i] | permutation2->mask[i];
        new_permutation->mines[i] = permutation1->mines[i] | permutation2->mines[i];
        new_permutation->mine_c += __builtin_popcountll(new_permutation->mines[i]);
    }
    return true;
}

void join_permutationsets(PermutationSet* permutation_set1, PermutationSet* permutation_set2) {
    // Join permutationsets into permutation_set1

    // Copy permutation_set1 to temp_permutation_set
    int32_t permutation_set_size = sizeof(PermutationSet) + permutation_set1->permutation_c * sizeof(Permutation);
    PermutationSet* temp_permutation_set = malloc(permutation_set_size);
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
    free(temp_permutation_set);
}

void permutations_of_split(EquationSet* equation_set, int32_t split_i, PermutationSet* permutation_set) {
    PermutationSet* temp_perm_set = alloca(sizeof(PermutationSet) + 64 * sizeof(Permutation));
    // First set permutationset to first equations permutations
    int32_t equation_start, equation_end;
    if (split_i == 0) equation_start = 0;
    else equation_start = equation_set->splits[split_i-1];
    equation_end = equation_set->splits[split_i];

    equation_permutations(equation_set->equations[equation_start], permutation_set);

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
        equation_permutations(equation_set->equations[i], temp_perm_set);
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

void trivial_permutation_set(EquationSet* equation_set, PermutationSet* permutation_set) {
    permutation_set->permutation_c = 1;
    permutation_set->permutations[0].mine_c = 0;
    for (int32_t i = 0; i < PERMUTATION_PARTS; i++) {
        permutation_set->permutations[0].mines[i] = 0;
        permutation_set->permutations[0].mask[i] = 0;
    }
    for (uint64_t i = 0; i < equation_set->unknown_c; i++) {
        if (equation_set->solved[i] == 0){
            permutation_set->permutations[0].mask[i / 64] |= 1ULL << (i % 64);
        }
        else if (equation_set->solved[i] == 1){
            permutation_set->permutations[0].mask[i / 64] |= 1ULL << (i % 64);
            permutation_set->permutations[0].mines[i / 64] |= 1ULL << (i % 64);
            permutation_set->permutations[0].mine_c += 1;
        }
    }
}

void print_permutations(int32_t border_unknown_c, PermutationSet* permutation_set) {
    printf("Permutation size: %d\n", permutation_set->permutation_c);
    for (int32_t i = 0; i < permutation_set->permutation_c; i++) {
        for (uint64_t j = 0; j < border_unknown_c; j++) {
            if ((permutation_set->permutations[i].mask[j / 64] >> (j % 64)) & 1ULL) {
                printf("%llu", (permutation_set->permutations[i].mines[j / 64] >> (j % 64)) & 1ULL);
            }
            else {
                printf(" ");
            }
        }
        printf(":%d\n", permutation_set->permutations[i].mine_c);
    }
}

PermutationSet* get_permutations(Board* board, Border* border){
    // Statically allocated memory for equations and permutations
    static Equation equations[MAX_SQUARES];
    static EquationSet equation_set;

    static uint8_t permutation_set_mem[MAX_PERMUTATIONS*sizeof(Permutation) + sizeof(PermutationSet)];
    static PermutationSet* permutation_set = (PermutationSet*) permutation_set_mem;

    static uint8_t temp_permutation_set_mem[(MAX_PERMUTATIONS / 4)*sizeof(Permutation) + sizeof(PermutationSet)];
    static PermutationSet* temp_permutation_set = (PermutationSet*) temp_permutation_set_mem;

    for (int32_t i = 0; i < border->border_known_c; i++) equation_set.equations[i] = equations + i;

    // Initialize equations
    set_equations(board, border, &equation_set);

    if (DEBUG) {
        for (int32_t i = 0; i < equation_set.equation_c; i++) {
            printf("Equation: %d\n", i);
            print_equation(equation_set.equations[i]);
        }
    }
    

    // Reduce equations
    reduce(&equation_set);

    if (DEBUG) {
        printf("Solved: '");
        for (int32_t i = 0; i < border->border_unknown_c; i++) {
            if (equation_set.solved[i] == -1) printf(" ");
            else printf("%d", equation_set.solved[i]);
        }
        printf("'\n");
    

        printf("REDUCED!\n");
        for (int32_t i = 0; i < equation_set.equation_c; i++) {
            printf("Equation: %d\n", i);
            print_equation(equation_set.equations[i]);
        }    
    }

    //Split equations
    split(&equation_set);
    
    if (DEBUG) {
        printf("--- SPLITS -----\n");
        for (int32_t i = 0; i < equation_set.split_c; i++) printf("%d ", equation_set.splits[i]);
        for (int32_t i = 0; i < equation_set.equation_c; i++) {
            printf("\nEquation: %d\n", i);
            print_equation(equation_set.equations[i]);
        }    
    }
    

    // Create trivial permutation of solved equations
    trivial_permutation_set(&equation_set, permutation_set);
    //printf("Permutation counts per split: ");
    for (int32_t i = 0; i < equation_set.split_c; i++) {
        if (DEBUG) print_permutations(border->border_unknown_c, permutation_set);
 
        permutations_of_split(&equation_set, i, temp_permutation_set);
        //printf("%d ", temp_permutation_set->permutation_c);
        join_permutationsets(permutation_set, temp_permutation_set);
    }
    //printf("Total: %d\n", permutation_set->permutation_c);

    if (DEBUG) 
        print_permutations(border->border_unknown_c, permutation_set);

    return permutation_set;
}