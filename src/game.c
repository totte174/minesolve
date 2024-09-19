#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "game.h"
#include "board.h"
#include "common.h"
#include "solver.h"

void calculate_adjacent_mines(Board* board) {
    for (int32_t i = 0; i < board->w*board->h; i++) {
        board->v[i] = 0;
        int32_t adj[8];
        int32_t adj_c = get_adjacent(board, i, adj);
        for (int32_t j = 0; j < adj_c; j++) {
            if (board->mines[adj[j]]) board->v[i] += 1;
        }
    }
}

void generate_board(int32_t w, int32_t h, int32_t mine_c, Board* board) {
    board->w = w;
    board->h = h;
    board->mine_c = 0;
    board->unknown_c = w*h;
    for (int32_t i = 0; i < w*h; i++) {
        board->v[i] = 0;
        board->known[i] = false;
        board->mines[i] = false;
    }

    while (board->mine_c < mine_c) {
        int32_t i = rand() % (w*h);
        if (board->mines[i]) continue;
        board->mines[i] = true;
        board->mine_c += 1; 
    }

    calculate_adjacent_mines(board);
}

bool move(Board* board, int32_t i, bool first_click) {
    if (board->known[i]) return true;

    if (board->mines[i]) {
        if (first_click) {
            while (true) {
                int32_t new_i = rand() % board->w*board->h;
                if (board->mines[new_i]) continue;
                board->mines[new_i] = true;
                break;
            }
            board->mines[i] = false;
            calculate_adjacent_mines(board);
            return move(board, i, false);
        }
        else return false;
    }
    else {
        board->known[i] = true;
        board->unknown_c -= 1;
        if (board->v[i] == 0) {
            int32_t adj[8];
            int32_t adj_c = get_adjacent(board, i, adj);
            for (int32_t j = 0; j < adj_c; j++) move(board, adj[j], false);
        }
        return true;
    }
}

bool play_game(Arguments* args) {
    static Board board;
    SolverResult solver_result;
    generate_board(args->width, args->height, args->mines, &board);
    bool first_move = true;
    //move(&board, 0, true);
    //first_move = false;

    while(board.unknown_c > args->mines) {
        //print_board(&board);
        get_solver_result(&board, args, &solver_result);
        
        //print_probability(&board, &solver_result);
        //printf("Best search move: %d or (%d,%d), p=%f\n", solver_result.best_search, solver_result.best_search % args->width, solver_result.best_search / args->width, solver_result.p[solver_result.best_search]);
        //if (board.mines[solver_result.best_search] && !first_move) {
        //print_statistics(&board, statistics, true, false, false, false, false);
        //printf("Unknown left: %d\n", board.unknown_c);
        //print_permutation_set(perm_set, border.border_unknown_c);
        //print_statistics(&board, statistics, true, true, true, true, true);
        //printf("Best move: %d or (%d,%d), p=%f\n", statistics->best_value, statistics->best_value % args->width, statistics->best_value / args->width, statistics->p[statistics->best_value]);
        //char s[2];
        //fgets(s, 2, stdin);
        //}
        if (!solver_result.valid){
            print_board(&board);
            printf("SOLVER RESULT NOT VALID\n");
            exit(1);
        }

        if (board.known[solver_result.best_search]){
            printf("BEST VALUE IS KNOWN\n");
            exit(1);
        }

        if (solver_result.p[solver_result.best_1step] == 0) {
            for (int32_t i = 0; i < board.w * board.h; i++) {
                if (solver_result.p[i] == 0 && !board.known[i]) {
                    if (!move(&board, i, first_move)) {
                        #ifdef DEBUG
                        printf("MINE WHERE P=0\n");
                        exit(1);
                        #endif
                        return false;
                    }
                    first_move = false;
                }
            }
        }
        else if (!move(&board, solver_result.best_search, first_move)) {
            //printf("loss\n");
            //print_board(&board);
            return false;
        }
        first_move = false;

    }
    //print_board(&board);
    return true;
}