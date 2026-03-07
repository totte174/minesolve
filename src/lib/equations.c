#include "equations.h"

/* Initializes Frontier and EquationSet from the board.
 * Creates one equation per revealed square that has adjacent unrevealed squares. */
void extract_frontier(MsBoard *board, Frontier *frontier, EquationSet *equation_set) {
    frontier->frontier_c = 0;
    frontier->unconstrained_c = 0;
    frontier->solved_c = 0;
    frontier->group_c = 0;

    equation_set->equation_c = 0;
    equation_set->group_c = 0;
    mask_reset(equation_set->solved_mask);
    mask_reset(equation_set->solved_mines);

    static int32_t edge_index[MAX_SQUARES];  // 0 = absent; N+1 = index N

    for (int32_t p = 0; p < board->w * board->h; p++) {
        if (board->revealed[p]){
            int32_t adj[8];
            int32_t adj_c = get_adjacent_unknown(board, p, adj);
            if (adj_c > 0) {
                Equation* equation = equation_set->equations + equation_set->equation_c;
                mask_reset(equation->mask);
                equation->mine_count = board->hint[p];

                for (int32_t j = 0; j < adj_c; j++) {
                    int32_t ind = edge_index[adj[j]] - 1;
                    if (ind >= 0) mask_set(equation->mask, ind, 1);
                    else {
                        edge_index[adj[j]] = frontier->frontier_c + 1;
                        frontier->frontier[frontier->frontier_c] = adj[j];
                        mask_set(equation->mask, frontier->frontier_c, 1);
                        frontier->frontier_c++;
                    }
                }
                equation_set->equation_c++;
            }
        }
    }
    for (int32_t i = 0; i < frontier->frontier_c; i++) edge_index[frontier->frontier[i]] = 0;
    frontier->unconstrained_c = board->unrevealed_c - frontier->frontier_c;
}


/* Returns true if eq1 and eq2 share any common squares. */
bool equations_intersect(Equation *eq1, Equation *eq2) {
    return mask_overlap(eq1->mask, eq2->mask);
}

/* Removes the squares of eq from super_eq and subtracts eq's mine count. */
void subtract_equation(Equation *eq, Equation *super_eq) {
    for (int32_t i = 0; i < MASK_PARTS; i++) {
        super_eq->mask.v[i] &= ~eq->mask.v[i];
    }
    super_eq->mine_count -= eq->mine_count;
}

/* Removes solved (all-mine or all-safe) equations from the set and propagates. */
bool remove_solved(EquationSet *equation_set, MsStatus *status){
    bool any_changed = false;

    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        Equation* eq = equation_set->equations + i;
        if (eq->mine_count < 0 || (uint64_t)eq->mine_count > mask_count(eq->mask)){
            // Contradiction, no valid permutations can satisfy this equation
            *status = MS_ERR_INVALID_BOARD;
            return false;
        }

        if (mask_count(eq->mask) == (uint64_t)eq->mine_count){
            //All unknown in equation are mines

            for (int32_t j = 0; j < MASK_PARTS; j++) {
                if (~equation_set->solved_mines.v[j] & equation_set->solved_mask.v[j] & eq->mask.v[j]) {
                    // Contradiction, same square is both solved 1 and solved 0
                    *status = MS_ERR_INVALID_BOARD;
                    return false;
                }

                equation_set->solved_mask.v[j] |= eq->mask.v[j];
                equation_set->solved_mines.v[j] |= eq->mask.v[j];
            }

            equation_set->equations[i] = equation_set->equations[equation_set->equation_c - 1];
            i--;
            equation_set->equation_c--;
            any_changed = true;
        }
        else if (eq->mine_count == 0) {
            //All unknown in equation are non-mines
            for (int32_t j = 0; j < MASK_PARTS; j++) {
                if (equation_set->solved_mines.v[j] & equation_set->solved_mask.v[j] & eq->mask.v[j]) {
                    // Contradiction, same square is both solved 1 and solved 0
                    *status = MS_ERR_INVALID_BOARD;
                    return false;
                }
                equation_set->solved_mask.v[j] |= eq->mask.v[j];
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
        if (mask_overlap(equation_set->solved_mask, eq->mask)){
            for (int32_t i = 0; i < MASK_PARTS; i++) {
                eq->mine_count -= __builtin_popcountll(equation_set->solved_mines.v[i] & eq->mask.v[i]);
                eq->mask.v[i] &= ~equation_set->solved_mask.v[i];
            }
            any_changed = true;
        }
    }
    return any_changed;
}

/* Eliminates subset equations from supersets. */
bool reduce_subsets(EquationSet *equation_set) {
    bool any_changed = false;
    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        for (int32_t j = i + 1; j < equation_set->equation_c; j++) {
            if (mask_overlap(equation_set->equations[i].mask, equation_set->equations[j].mask)) {
                bool i_sub_j = true, j_sub_i = true;
                for (int32_t k = 0; k < MASK_PARTS; k++) {
                    if (equation_set->equations[i].mask.v[k] & ~equation_set->equations[j].mask.v[k]) i_sub_j = false;
                    if (equation_set->equations[j].mask.v[k] & ~equation_set->equations[i].mask.v[k]) j_sub_i = false;
                }
                if (i_sub_j) { subtract_equation(equation_set->equations + i, equation_set->equations + j); any_changed = true; }
                if (j_sub_i) { subtract_equation(equation_set->equations + j, equation_set->equations + i); any_changed = true; }
            }
        }
    }
    return any_changed;
}

/* Iteratively reduces the equation set until no further simplification is possible. */
MsStatus reduce_equations(EquationSet *equation_set) {
    MsStatus status = MS_OK;
    bool any_changed = true;
    while (any_changed && !status) {
        any_changed = false;
        any_changed |= reduce_subsets(equation_set);
        any_changed |= remove_solved(equation_set, &status);
    }
    return status;
}

/* Partitions equations into independent groups and splits the frontier accordingly. */
void find_groups(Frontier *frontier, EquationSet *equation_set, ProbabilityMap *pmap) {
    int32_t split_labels[MAX_SQUARES] = {0};

    int32_t queue[MAX_FRONTIER_SIZE];
    int32_t q;
    bool explored[MAX_SQUARES] = {false};

    //Depth first search on equation groups
    int32_t label = 1;
    for (int32_t start_eq = 0; start_eq < equation_set->equation_c; start_eq++) {
        if (split_labels[start_eq] != 0) continue;
        queue[0] = start_eq;
        q = 0;
        explored[start_eq] = true;

        while (q>=0) {
            if (q >= MAX_FRONTIER_SIZE) {
                fprintf(stderr, "Queue full - find_groups");
                exit(1);
            }
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

    // Sort equations by label
    equation_set->group_c = label - 1;
    int32_t eq_start = 0;
    for (int32_t label = 1; label <= equation_set->group_c; label++) {
        int32_t eq_end = eq_start;
        for (int32_t i = eq_start; i < equation_set->equation_c; i++) {
            if (split_labels[i] == label) {
                Equation temp = equation_set->equations[eq_end];
                equation_set->equations[eq_end] = equation_set->equations[i];
                equation_set->equations[i] = temp;
                split_labels[i] = split_labels[eq_end];
                eq_end++;
            }
        }
        equation_set->group_start[label-1] = eq_start;
        equation_set->group_length[label-1] = eq_end - eq_start;
        eq_start = eq_end;
    }

    //Split frontier and correct equations
    frontier->group_c = equation_set->group_c;
    int32_t start_edge = 0;
    int32_t new_frontier[MAX_FRONTIER_SIZE];
    int32_t new2old_edge[MAX_FRONTIER_SIZE];
    for (int32_t group_i = 0; group_i < equation_set->group_c; group_i++) {
        int32_t end_edge = start_edge;
        Mask group_mask = {.v = {0}};

        // Create mask of all equations in group
        for(int32_t i = 0; i < equation_set->group_length[group_i]; i++) {
            for (int32_t j = 0; j < MASK_PARTS; j++) {
                group_mask.v[j] |= equation_set->equations[equation_set->group_start[group_i] + i].mask.v[j];
            }
        }

        // Set new_frontier of group
        for(int32_t i = 0; i < frontier->frontier_c; i++) {
            if (mask_get(group_mask, i)) {
                new_frontier[end_edge] = frontier->frontier[i];
                new2old_edge[end_edge] = i;
                end_edge++;
            }
        }

        for(int32_t i = 0; i < equation_set->group_length[group_i]; i++) {
            Mask new_mask = {.v = {0}};
            for(int32_t j = start_edge; j < end_edge; j++) {
                if(mask_get(equation_set->equations[equation_set->group_start[group_i] + i].mask, new2old_edge[j])) {
                    mask_set(new_mask, j-start_edge, 1);
                }
            }
            equation_set->equations[equation_set->group_start[group_i] + i].mask = new_mask;
        }
        frontier->group_start[group_i] = start_edge;
        frontier->group_length[group_i] = end_edge - start_edge;
        start_edge = end_edge;
    }

    //Add solved frontier entries
    frontier->solved_c = 0;
    for(int32_t i = 0; i < frontier->frontier_c; i++) {
        if (mask_get(equation_set->solved_mask, i)) {
            pmap->p_solved[frontier->solved_c] = (double) mask_get(equation_set->solved_mines, i);
            frontier->solved[frontier->solved_c] = frontier->frontier[i];
            frontier->solved_c++;
        }
    }

    //Update frontier
    if (frontier->group_c == 0) frontier->frontier_c = 0;
    else frontier->frontier_c = frontier->group_start[frontier->group_c-1] + frontier->group_length[frontier->group_c-1];
    for(int32_t i = 0; i < frontier->frontier_c; i++) {
        frontier->frontier[i] = new_frontier[i];
    }
}

MsStatus build_equation_set(MsBoard *board, Frontier *frontier, EquationSet *equation_set, ProbabilityMap *pmap){
    extract_frontier(board, frontier, equation_set);

    MsStatus status = reduce_equations(equation_set);
    if (status) return status;

    find_groups(frontier, equation_set, pmap);
    return status;
}
