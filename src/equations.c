#include "equations.h"

void get_base_edge_and_equations(Board *board, Edge *edge, EquationSet *equation_set) {
    /**
     * Initializes Edge and EquationSet.
     * 
     * This function initializes Edge and EquationSet struct using given board.
     * Creates one equation in equation set for every known square that has adjacent
     * unknown squares.
     * 
     * @param Board* current board to initialize edge and equation set from.
     * @param Edge* edge to initialize.
     * @param EquationSet* equation_set equation set to create equations for and initialize.
     */

    edge->edge_c = 0;
    edge->exterior_c = 0;
    edge->edge_solved_c = 0;
    edge->split_c = 0;

    equation_set->equation_c = 0;
    equation_set->split_c = 0;
    mask_reset(equation_set->solved_mask);
    mask_reset(equation_set->solved_mines);

    for (int32_t p = 0; p < board->w * board->h; p++) {
        if (board->known[p]){
            int32_t adj[8];
            int32_t adj_c = get_adjacent_unknown(board, p, adj);
            if (adj_c > 0) {
                Equation* equation = equation_set->equations + equation_set->equation_c;
                mask_reset(equation->mask);
                equation->amount = board->v[p];

                int32_t adj[8];
                int32_t adj_c = get_adjacent_unknown(board, p, adj);

                for (int32_t j = 0; j < adj_c; j++) {
                    int32_t ind = -1;
                    for (int32_t k = 0; k < edge->edge_c; k++) {
                        if (edge->edge[k] == adj[j]) ind = k;
                    }
                    if (ind != -1) mask_set(equation->mask, ind, 1);
                    else {
                        edge->edge[edge->edge_c] = adj[j];
                        mask_set(equation->mask, edge->edge_c, 1);
                        edge->edge_c++;
                    }
                }
                equation_set->equation_c++;
            }
        }
    }
    edge->exterior_c = board->unknown_c - edge->edge_c;
}

bool is_subequation(Equation *eq, Equation *super_eq) {
        /**
     * Returns true if eq is a subequation of super_eq.
     * 
     * This function returns true if eq is a subequation of super_eq.
     * An equation is a sub-equation of another super-equation if every square
     * in the equation is present in the super equation.
     * 
     * @param Equation* sub-equation 
     * @param Equation* super-equation
     * @return true if eq is a subequation of super_eq otherwise false
     */
    if (!mask_overlap(eq->mask, super_eq->mask)) return false;
    for (int32_t i = 0; i < MASK_PARTS; i++) {
        if (eq->mask.v[i] & ~super_eq->mask.v[i]) return false;
    }
    return true;
}

bool equations_intersect(Equation *eq1, Equation *eq2) {
    /**
     * Returns true if eq1 and eq2 share any common squares.
     * 
     * @param Equation* eq1
     * @param Equation* eq2
     * @return true if eq1 and eq2 share any squares
     */
    return mask_overlap(eq1->mask, eq2->mask);
}

void remove_subequation(Equation *eq, Equation *super_eq) {
    /**
     * Removes intersecting squares between eq and super_eq from super_eq.
     * 
     * @param Equation *eq1
     * @param Equation *eq2
     */
    for (int32_t i = 0; i < MASK_PARTS; i++) {
        super_eq->mask.v[i] &= ~eq->mask.v[i];
    }
    super_eq->amount -= eq->amount;
}

bool remove_solved(EquationSet *equation_set, FaultStatus *fault_status){
    bool any_changed = false;

    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        Equation* eq = equation_set->equations + i;
        if (eq->amount > mask_count(eq->mask) || eq->amount < 0){
            // Contradiction, no valid permutations can satisfy this equation
            *fault_status = fault_invalid_board;
            return false;
        }

        if (mask_count(eq->mask) == eq->amount){ 
            //All unknown in equation are mines

            for (int32_t j = 0; j < MASK_PARTS; j++) {
                if (~equation_set->solved_mines.v[j] & equation_set->solved_mask.v[j] & eq->mask.v[j]) { 
                    // Contradiction, same square is both solved 1 and solved 0
                    *fault_status = fault_invalid_board;
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
        else if (eq->amount == 0) { 
            //All unknown in equation are non-mines
            for (int32_t j = 0; j < MASK_PARTS; j++) {
                if (equation_set->solved_mines.v[j] & equation_set->solved_mask.v[j] & eq->mask.v[j]) { 
                    // Contradiction, same square is both solved 1 and solved 0
                    *fault_status = fault_invalid_board;
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
                eq->amount -= __builtin_popcountll(equation_set->solved_mines.v[i] & eq->mask.v[i]);
                eq->mask.v[i] &= ~equation_set->solved_mask.v[i];
            }
            any_changed = true;
        }
    }
    return any_changed;
}

bool remove_subequations(EquationSet *equation_set) {
    bool any_changed = false;
    for (int32_t i = 0; i < equation_set->equation_c; i++) {
        for (int32_t j = i + 1; j < equation_set->equation_c; j++) {
            if (mask_overlap(equation_set->equations[i].mask, equation_set->equations[j].mask)) {
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
    }
    return any_changed;
}

FaultStatus reduce(EquationSet *equation_set) {
    FaultStatus fault_status = valid_status;
    bool any_changed = true;
    while (any_changed && !fault_status) {
        any_changed = false;
        any_changed |= remove_subequations(equation_set);
        any_changed |= remove_solved(equation_set, &fault_status);
    }
    return fault_status;
}

void split_equations(Edge *edge, EquationSet *equation_set, ProbabilityMap *pmap) {
    int32_t split_labels[MAX_SQUARES] = {0};

    // Create variables for search algorithm
    int32_t queue[MAX_EDGE_SIZE];
    int32_t q;
    bool explored[MAX_SQUARES] = {false};

    //Depth first search on equation splits
    int32_t label = 1;
    for (int32_t start_eq = 0; start_eq < equation_set->equation_c; start_eq++) {
        if (split_labels[start_eq] != 0) continue;
        queue[0] = start_eq;
        q = 0;
        explored[start_eq] = true;

        while (q>=0) {
            if (q >= MAX_EDGE_SIZE) printf("Queue full - split_equations");
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

    //Split edge and correct equations
    edge->split_c = equation_set->split_c;
    int32_t start_edge = 0;
    int32_t new_edge[MAX_EDGE_SIZE];
    int32_t new2old_edge[MAX_EDGE_SIZE];
    for (int32_t split_i = 0; split_i < equation_set->split_c; split_i++) {
        int32_t end_edge = start_edge;
        Mask split_mask = {.v = {0}};

        // Create mask of all equations in split
        for(int32_t i = 0; i < equation_set->splits_length[split_i]; i++) {
            for (int32_t j = 0; j < MASK_PARTS; j++) {
                split_mask.v[j] |= equation_set->equations[equation_set->splits_start[split_i] + i].mask.v[j];
            }
        }

        // Set new_edge of split
        for(int32_t i = 0; i < edge->edge_c; i++) {
            if (mask_get(split_mask, i)) {
                new_edge[end_edge] = edge->edge[i];
                new2old_edge[end_edge] = i;
                end_edge++;
            }
        }

        for(int32_t i = 0; i < equation_set->splits_length[split_i]; i++) {
            Mask new_mask = {.v = {0}};
            for(int32_t j = start_edge; j < end_edge; j++) {
                if(mask_get(equation_set->equations[equation_set->splits_start[split_i] + i].mask, new2old_edge[j])) {
                    mask_set(new_mask, j-start_edge, 1);
                }
            }
            equation_set->equations[equation_set->splits_start[split_i] + i].mask = new_mask;
        }
        edge->splits_start[split_i] = start_edge;
        edge->splits_length[split_i] = end_edge - start_edge;
        start_edge = end_edge;
    }
    
    //Add solved edge
    edge->edge_solved_c = 0;
    for(int32_t i = 0; i < edge->edge_c; i++) {
        if (mask_get(equation_set->solved_mask, i)) {
            pmap->p_solved[edge->edge_solved_c] = (double) mask_get(equation_set->solved_mines, i);
            edge->edge_solved[edge->edge_solved_c] = edge->edge[i];
            edge->edge_solved_c++;
        }
    }

    //Update edge
    if (edge->split_c == 0) edge->edge_c = 0;
    else edge->edge_c = edge->splits_start[edge->split_c-1] + edge->splits_length[edge->split_c-1];
    for(int32_t i = 0; i < edge->edge_c; i++) {
        edge->edge[i] = new_edge[i];
    }
}

FaultStatus get_equation_set(Board *board, Edge *edge, EquationSet *equation_set, ProbabilityMap *pmap){
    get_base_edge_and_equations(board, edge, equation_set);

    FaultStatus fault_status = reduce(equation_set);
    if (fault_status) return fault_status;

    split_equations(edge, equation_set, pmap);
    return fault_status;
}