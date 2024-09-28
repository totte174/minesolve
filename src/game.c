#include "game.h"

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

void generate_board(int32_t mine_c, Board* board) {
    board->mine_c = 0;
    board->unknown_c = board->w*board->h;
    for (int32_t i = 0; i < board->w*board->h; i++) {
        board->v[i] = 0;
        board->known[i] = false;
        board->mines[i] = false;
    }

    while (board->mine_c < mine_c) {
        int32_t i = rand() % (board->w*board->h);
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
                int32_t new_i = rand() % (board->w*board->h);
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
    static SearchResult solver_result;
    static double p_a[MAX_SQUARES];
    board.h = args->height;
    board.w = args->width;
    board.wrapping_borders = args->wrapping_borders;
    generate_board(args->mines, &board);
    bool first_move = true;

    while(board.unknown_c > args->mines) {
        //print_board(&board);
        get_solver_result_and_board_probabilities(&board, args, &solver_result, p_a);

        if (solver_result.fault_status == fault_computational_limit) { // If computational limit reached - fick first square
            for (int32_t i = 0; i < board.w * board.h; i++) {
                if (!board.known[i]) {
                    if (!move(&board, i, first_move)) {
                        return false;
                    }
                    first_move = false;
                    break;
                }
            }
            continue;
        }
        
        if (solver_result.fault_status == fault_invalid_board){
            print_board(&board);
            printf("INVALID BOARD\n");
            exit(1);
        }

        if (solver_result.fault_status == fault_internal_limit){
            print_board(&board);
            printf("INTERNAL LIMIT REACHED\n");
            exit(1);
        }

        if (board.known[solver_result.best_move]){
            printf("BEST VALUE IS KNOWN\n");
            exit(1);
        }

        if (p_a[solver_result.best_move] == 0) {
            for (int32_t i = 0; i < board.w * board.h; i++) {
                if (p_a[i] == 0 && !board.known[i]) {
                    if (!move(&board, i, first_move)) {
                        printf("MINE WHERE P=0\n");
                        exit(1);
                        return false;
                    }
                    first_move = false;
                }
            }
        }
        else if (!move(&board, solver_result.best_move, first_move)) {
            return false;
        }
        first_move = false;
    }
    return true;
}