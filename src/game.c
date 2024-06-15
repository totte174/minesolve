#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "game.h"
#include "board.h"
#include "common.h"
#include "permutations.h"
#include "statistics.h"

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
    static Border border;
    generate_board(args->width, args->height, args->mines, &board);
    bool first_move = true;
    //move(&board, 0, true);
    //first_move = false;

    while(board.unknown_c > args->mines) {
        get_border(&board, &border);
        if (DEBUG) {
            print_board(&board);
            printf("Border known: ");
            for (int32_t i = 0; i < border.border_known_c; i++) {
                printf("(%d,%d),", border.border_known[i]%board.w, border.border_known[i]/board.w);
            }
            printf("\n");
            printf("Border known count: %d\n", border.border_known_c);
            printf("Outside known count: %d\n", border.outside_known_c);
            printf("Border unknown: ");
            for (int32_t i = 0; i < border.border_unknown_c; i++) {
                printf("(%d,%d),", border.border_unknown[i]%board.w, border.border_unknown[i]/board.w);
            }
            printf("\n");
            printf("Border unknown count: %d\n", border.border_unknown_c);
            printf("Outside unknown count: %d\n", border.outside_unknown_c);

        }

        if (border.border_unknown_c > MAX_BORDER_UNKNOWN) {
            printf("BORDER_UNKNOWN_C > MAX_BORDER_UNKNOWN\n");
            exit(1);
        }
        
        PermutationSet* perm_set = get_permutations(&board, &border);
        //printf("Total Permutations: %d\n", perm_set->total_permutation_c);

        BoardStatistics* statistics = get_statistics(&board, &border, perm_set, args);

        if (board.mines[statistics->best_value] && !first_move) {
        //print_statistics(&board, statistics, true, false, false, false, false);
        //print_board(&board);
        //printf("Unknown left: %d\n", board.unknown_c);
        //print_permutation_set(perm_set, border.border_unknown_c);
        //print_statistics(&board, statistics, true, true, true, true, true);
        //printf("Best move: %d or (%d,%d), p=%f\n", statistics->best_value, statistics->best_value % args->width, statistics->best_value / args->width, statistics->p[statistics->best_value]);
        //char s[2];
        //fgets(s, 2, stdin);
        }

        if (board.known[statistics->best_value]){
            printf("BEST VALUE IS KNOWN\n");
            exit(1);
        }

        if (statistics->p[statistics->best_p] == 0) {
            for (int32_t i = 0; i < board.w * board.h; i++) {
                if (statistics->p[i] == 0 && !board.known[i]) {
                    if (!move(&board, i, first_move)) {
                        printf("MINE WHERE P=0\n");
                        exit(1);
                    }
                    first_move = false;
                }
            }
        }
        else if (!move(&board, statistics->best_value, first_move)) {
            //printf("loss\n");
            //print_board(&board);
            return false;
        }
        first_move = false;

    }
    //print_board(&board);
    return true;
}