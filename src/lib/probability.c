#include "probability.h"

double choose(double n, double k) {
    if (n < k) return 0;
    if (k == 0) return 1;
    return (n * choose(n - 1, k - 1)) / k;
}

MsStatus build_pmap(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap) {
    static SolutionSet solution_set;
    MsStatus status = MS_OK;
    status = build_solution_set(board, frontier, &solution_set, pmap);
    if (status) {
        solution_set_free(&solution_set);
        return status;
    }

    static int32_t group_mine_c_min[MAX_FRONTIER_SIZE];
    static int32_t group_mine_c_max[MAX_FRONTIER_SIZE];
    int32_t n = frontier->unconstrained_c;

    if(n < 0) {
        solution_set_free(&solution_set);
        status = MS_ERR_INVALID_BOARD;
        return status;
    }

    pmap->total_weight = 0;
    pmap->p_unconstrained = 0;
    for (int32_t i = 0; i < frontier->frontier_c; i++){
        pmap->p_frontier[i] = 0;
    }

    int32_t solved_mines = 0;
    for (int32_t i = 0; i < frontier->solved_c; i++) {
        solved_mines += (int32_t) pmap->p_solved[i];
    }
    int32_t total_mine_c_min = solved_mines;
    int32_t total_mine_c_max = solved_mines;
    for (int32_t group_i = 0; group_i < solution_set.group_c; group_i++)
    {
        group_mine_c_min[group_i] = MAX_SQUARES;
        group_mine_c_max[group_i] = 0;
        for (int32_t i = 0; i < solution_set.group_length[group_i]; i++){
            Mask* sol = solution_set.solutions + solution_set.group_start[group_i] + i;

            int32_t mc = count_mines(sol);
            group_mine_c_min[group_i] = min(group_mine_c_min[group_i], mc);
            group_mine_c_max[group_i] = max(group_mine_c_max[group_i], mc);
        }
        if (group_mine_c_max[group_i] < group_mine_c_min[group_i]) { //Not valid solution in group -> not valid pmap
            solution_set_free(&solution_set);
            status = MS_ERR_INVALID_BOARD;
            return status;
        }
        total_mine_c_min += group_mine_c_min[group_i];
        total_mine_c_max += group_mine_c_max[group_i];
    }

    if (total_mine_c_max - total_mine_c_min >= MAX_MINE_RANGE) {
        solution_set_free(&solution_set);
        status = MS_ERR_INTERNAL_LIMIT;
        return status;
    }
    static double rel_mine_c_p_frontier[MAX_MINE_RANGE][MAX_FRONTIER_SIZE];
    int32_t range = total_mine_c_max - total_mine_c_min;
    for (int32_t r = 0; r <= range; r++)
        memset(rel_mine_c_p_frontier[r], 0, sizeof(double) * frontier->frontier_c);
    double rel_mine_c_total_combs[MAX_MINE_RANGE] = {0};
    rel_mine_c_total_combs[0] = 1;
    int32_t cur_max_rel_mine_c = 0;

    for (int32_t group_i = 0; group_i < solution_set.group_c; group_i++) {
        static double group_rel_mine_c_p_frontier[MAX_MINE_RANGE][MAX_FRONTIER_SIZE];
        int32_t group_range = group_mine_c_max[group_i] - group_mine_c_min[group_i];
        for (int32_t r = 0; r <= group_range; r++)
            memset(group_rel_mine_c_p_frontier[r], 0, sizeof(double) * frontier->frontier_c);
        double group_rel_mine_c_total_combs[MAX_MINE_RANGE] = {0};

        for (int32_t i = 0; i < solution_set.group_length[group_i]; i++) {
            Mask* sol = solution_set.solutions + solution_set.group_start[group_i] + i;

            int32_t rel_mine_c = count_mines(sol) - group_mine_c_min[group_i];
            group_rel_mine_c_total_combs[rel_mine_c] += 1;

            for (int32_t j = 0; j < frontier->group_length[group_i]; j++) {
                if (mask_get(*sol, j)) {
                    group_rel_mine_c_p_frontier[rel_mine_c][frontier->group_start[group_i] + j] += 1.0;
                }
            }
        }

        static double temp_rel_mine_c_p_frontier[MAX_MINE_RANGE][MAX_FRONTIER_SIZE];
        double temp_rel_mine_c_total_combs[MAX_MINE_RANGE];
        for (int32_t rel_mine_c = 0; rel_mine_c <= cur_max_rel_mine_c; rel_mine_c++) {
            for (int32_t i = 0; i < frontier->frontier_c; i++) {
                temp_rel_mine_c_p_frontier[rel_mine_c][i] = rel_mine_c_p_frontier[rel_mine_c][i];
                rel_mine_c_p_frontier[rel_mine_c][i] = 0;
            }
            temp_rel_mine_c_total_combs[rel_mine_c] = rel_mine_c_total_combs[rel_mine_c];
            rel_mine_c_total_combs[rel_mine_c] = 0;
        }

        for (int32_t rel_mine_c = 0; rel_mine_c <= cur_max_rel_mine_c; rel_mine_c++) {
            for (int32_t group_rel_mine_c = 0; group_rel_mine_c <= group_mine_c_max[group_i] - group_mine_c_min[group_i]; group_rel_mine_c++) {
                for (int32_t i = 0; i < frontier->frontier_c; i++) {
                    rel_mine_c_p_frontier[rel_mine_c + group_rel_mine_c][i] +=
                        temp_rel_mine_c_p_frontier[rel_mine_c][i] * group_rel_mine_c_total_combs[group_rel_mine_c] +
                        group_rel_mine_c_p_frontier[group_rel_mine_c][i] * temp_rel_mine_c_total_combs[rel_mine_c];
                }
                rel_mine_c_total_combs[rel_mine_c + group_rel_mine_c] += temp_rel_mine_c_total_combs[rel_mine_c] * group_rel_mine_c_total_combs[group_rel_mine_c];
            }
        }

        cur_max_rel_mine_c += group_mine_c_max[group_i] - group_mine_c_min[group_i];
    }

    int32_t total_mine_c_min_valid = max(total_mine_c_min, board->mine_count - n);
    int32_t total_mine_c_max_valid = min(total_mine_c_max, board->mine_count);
    double rel_mine_c_weighting[MAX_MINE_RANGE];
    rel_mine_c_weighting[total_mine_c_min_valid - total_mine_c_min] = 1.0;
    for (int32_t rel_mine_c = total_mine_c_min_valid - total_mine_c_min + 1; rel_mine_c <= total_mine_c_max_valid - total_mine_c_min; rel_mine_c++) {
        int32_t k = board->mine_count - total_mine_c_min - rel_mine_c;
        rel_mine_c_weighting[rel_mine_c] = rel_mine_c_weighting[rel_mine_c - 1] * ((double)k+1)/((double)(n-k));
    }

    for (int32_t rel_mine_c = total_mine_c_min_valid - total_mine_c_min; rel_mine_c <= total_mine_c_max_valid - total_mine_c_min; rel_mine_c++) {
        int32_t k = board->mine_count - total_mine_c_min - rel_mine_c;
        pmap->total_weight += rel_mine_c_total_combs[rel_mine_c] * rel_mine_c_weighting[rel_mine_c];
        pmap->p_unconstrained += rel_mine_c_total_combs[rel_mine_c] * rel_mine_c_weighting[rel_mine_c] * ((double) k);
        for (int32_t i = 0; i < frontier->frontier_c; i++) {
            pmap->p_frontier[i] += rel_mine_c_p_frontier[rel_mine_c][i] * rel_mine_c_weighting[rel_mine_c];
        }
    }

    if (pmap->total_weight == 0) { //No valid solutions
        solution_set_free(&solution_set);
        status = MS_ERR_INVALID_BOARD;
        return status;
    }

    if (n>0) pmap->p_unconstrained /= pmap->total_weight * ((double) n);
    for (int32_t i = 0; i < frontier->frontier_c; i++) {
        pmap->p_frontier[i] /= pmap->total_weight;
    }
    pmap->total_weight *= choose(n, board->mine_count - total_mine_c_min_valid);

    solution_set_free(&solution_set);
    return status;
}

MsStatus build_pmap_basic(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap) {
    /**
     * This will return a pmap where only equation set logic is used,
     * where solved square probabilities will be correct but all frontier
     * squares will have the same probability.
     */
    EquationSet equation_set;

    MsStatus status = build_equation_set(board, frontier, &equation_set, pmap);
    if (status) return status;

    int32_t solved_mines = 0;
    for (int32_t i = 0; i < frontier->solved_c; i++) {
        solved_mines += (int32_t) pmap->p_solved[i];
    }
    double p = 0.0;
    if (board->unrevealed_c - frontier->solved_c > 0) {
        p = ((double) (board->mine_count - solved_mines)) / ((double) (board->unrevealed_c - frontier->solved_c));
    }
    pmap->p_unconstrained = p;
    for (int32_t i = 0; i < frontier->frontier_c; i++) pmap->p_frontier[i] = p;

    return status;
}

void pmap_to_board(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap, double* prob_a) {
    for (int32_t i = 0; i < board->w * board->h; i++) {
        if (board->revealed[i]) prob_a[i] = 0.0;
        else prob_a[i] = pmap->p_unconstrained;
    }
    for (int32_t i = 0; i < frontier->solved_c; i++) {
        prob_a[frontier->solved[i]] = pmap->p_solved[i];
    }
    for (int32_t i = 0; i < frontier->frontier_c; i++) {
        prob_a[frontier->frontier[i]] = pmap->p_frontier[i];
    }
}

void find_safest(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap,
                 double* p, int32_t* pos) {
    *p = 1.0;
    for (int32_t i = 0; i < frontier->solved_c; i++) {
        if (pmap->p_solved[i] == 0.0) {
            *p = 0.0;
            *pos = frontier->solved[i];
            return;
        }
    }

    for (int32_t i = 0; i < frontier->frontier_c; i++) {
        if (pmap->p_frontier[i] < *p) {
            *p = pmap->p_frontier[i];
            *pos = frontier->frontier[i];
        }
    }

    // If unconstrained probability best, find one with lowest adjacent unknown count
    if (frontier->unconstrained_c > 0 && pmap->p_unconstrained < *p) {
        *p = pmap->p_unconstrained;
        int32_t lowest_adjacent_unknown = 9;
        int32_t adj_c, adj[8];
        for (int32_t p = 0; p < board->w * board->h; p++) {
            if (board->revealed[p]) continue;
            adj_c = get_adjacent(board, p, adj);

            bool on_frontier = false;
            for (int32_t j = 0; j < adj_c; j++) if (board->revealed[adj[j]]) on_frontier = true;
            if (!on_frontier && adj_c < lowest_adjacent_unknown) {
                lowest_adjacent_unknown = adj_c;
                *pos = p;
            }
        }
    }
}

int32_t rank_candidates(MsBoard* board, Frontier* frontier, ProbabilityMap* pmap,
                        int32_t* best_pos, double* best_p) {
    // If solved safe position, return only that position
    for (int32_t i = 0; i < frontier->solved_c; i++) {
        if (pmap->p_solved[i] == 0.0) {
            *best_pos = frontier->solved[i];
            *best_p = 0.0;
            return 1;
        }
    }

    int32_t best_c = 0;

    // Frontier positions
    for (int32_t i = 0; i < frontier->frontier_c; i++) {
        if (pmap->p_frontier[i] < 1.0) {
            best_p[best_c] = pmap->p_frontier[i];
            best_pos[best_c] = frontier->frontier[i];
            best_c++;
        }
    }

    //Find the unconstrained point with least adjacent unknown (usually corners)
    if (frontier->unconstrained_c > 0) {
        int32_t lowest_adjacent_unknown = 9;
        int32_t adj_c, adj[8];
        for (int32_t p = 0; p < board->w * board->h; p++) {
            if (board->revealed[p]) continue;
            adj_c = get_adjacent(board, p, adj);

            bool on_frontier = false;
            for (int32_t j = 0; j < adj_c; j++) if (board->revealed[adj[j]]) on_frontier = true;
            if (!on_frontier && adj_c < lowest_adjacent_unknown) {
                lowest_adjacent_unknown = adj_c;
                best_pos[best_c] = p;
            }
        }
        best_p[best_c] = pmap->p_unconstrained;
        best_c++;
    }

    // Sort by probability ( in O(n^2) :P )
    int32_t temp_pos;
    double temp_p;
    for (int32_t i = 0; i < best_c; i++) {
        for (int32_t j = i + 1; j < best_c; j++) {
            if (best_p[i] > best_p[j]) {
                temp_pos = best_pos[i];
                temp_p = best_p[i];

                best_pos[i] = best_pos[j];
                best_p[i] = best_p[j];

                best_pos[j] = temp_pos;
                best_p[j] = temp_p;
            }
        }
    }

    return best_c;
}
