#include "commands.h"

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

void simulate(Arguments* args) {
    static Board board;
    static SearchResult solver_result;
    static double p_a[MAX_SQUARES];
    board.h = args->height;
    board.w = args->width;
    board.wrapping_borders = args->wrapping_borders;
    if (args->show_board && !args->ascii) printf("\e[2J"); // Clear screen

    int32_t wins = 0;
    for (int32_t game_c = 0; game_c < args->test_games; game_c++) {
        generate_board(args->mines, &board);
        bool first_move = true;
        bool alive = true;

        while(board.unknown_c > args->mines && alive) {
            if(args->show_board) {
                if (args->ascii) {
                    print_board(&board);
                    printf("Wins: %d/%d [%.6f]\n", wins, game_c, ((double) wins) / ((double) max(1, game_c)));
                } else {
                    print_board_pretty(&board, true);
                    printf("Wins: %d/%d [%.6f]\n", wins, game_c, ((double) wins) / ((double) max(1, game_c)));
                    struct timespec ts = {
                        .tv_nsec = 50000000,
                        .tv_sec = 0,
                    };
                    nanosleep(&ts, NULL); // Add a delay so play is visible
                }
            }

            get_solver_result(&board, args, &solver_result, p_a);

            if (solver_result.fault_status == fault_computational_limit) { // If computational limit reached - only do basic solve
                get_solver_result_basic(&board, args, &solver_result, p_a);
            }
            
            if (solver_result.fault_status == fault_invalid_board){
                print_board(&board);
                fprintf(stderr, "INVALID BOARD\n");
                exit(1);
            }

            if (solver_result.fault_status == fault_internal_limit){
                print_board(&board);
                fprintf(stderr, "INTERNAL LIMIT REACHED\n");
                exit(1);
            }

            if (board.known[solver_result.best_move]){
                print_board(&board);
                fprintf(stderr, "BEST VALUE IS KNOWN\n");
                exit(1);
            }

            if (p_a[solver_result.best_move] == 0) {
                // If multiple solved zeros, click them all
                for (int32_t i = 0; i < board.w * board.h; i++) {
                    if (p_a[i] == 0 && !board.known[i]) {
                        if (!move(&board, i, first_move)) {
                            fprintf(stderr, "MINE WHERE P=0\n");
                            exit(1);
                        }
                    }
                }
            }
            else if (!move(&board, solver_result.best_move, first_move)) {
                alive = false;
            }
            first_move = false;
        }
        if(args->show_board) {
            if (args->ascii) {
                print_board(&board);
            } else {
                print_board_pretty(&board, true);
            }
            printf("Wins: %d/%d [%.6f]\n", wins, game_c, ((double) wins) / ((double) max(1, game_c)));
        }
        if (alive) wins++;
    }
    if (!args->show_board) printf("%d\n", wins);
}

void parse_board(Board *board, Arguments *args) {
    board->mine_c = args->mines;
    board->h = args->height;
    board->w = args->width;
    board->wrapping_borders = args->wrapping_borders;
    board->unknown_c = board->w * board->h;
    for (int32_t j = 0; j < board->w * board->h; j++) {
        board->known[j] = false;
        board->v[j] = 0;
    }

    int32_t i = 0;
    for (int32_t j = 0; j < args->buf_size; j++) {
        char c = args->buf[j];
        if (c == '\r') continue;
        else if (c == '\n') {
            if (i % board->w != 0) i = (i / board->w + 1) * board->w;
        }
        else if (c >= '0' && c <= '8') {
            board->known[i] = true;
            board->v[i] = (int32_t) (c - '0');
            board->unknown_c--;
            i++;
        }
        else if (c == ' ') {
            board->known[i] = true;
            board->v[i] = 0;
            board->unknown_c--;
            i++;
        }
        else if (c == '.' || c == 'x' || c == 'X' || c == '?') {
            board->known[i] = false;
            i++;
        }
    }
}

void show_board(Arguments *args) {
    static Board board;
    parse_board(&board, args);

    if (args->ascii) {
        print_board(&board);
    } else {
        print_board_pretty(&board, false);
    }
}

void solve_board(Arguments *args) {
    static SearchResult solver_result;
    static Board board;
    parse_board(&board, args);

    if (board.unknown_c == board.mine_c) {
        fprintf(stderr, "Board is solved.\n");
        exit(1);
    }

    get_solver_result(&board, args, &solver_result, NULL);
    if (solver_result.fault_status == fault_computational_limit) { // If computational limit reached - only do basic solve
        get_solver_result_basic(&board, args, &solver_result, NULL);
    }

    if (solver_result.fault_status == fault_invalid_board){
        fprintf(stderr, "Invalid board.\n");
        exit(1);
    }

    if (solver_result.fault_status == fault_internal_limit){
        fprintf(stderr, "Internal limit reached.\n");
        exit(1);
    }

    printf("(%d, %d)\n", solver_result.best_move % board.w, solver_result.best_move / board.w);
}

void show_probability(Arguments *args) {
    static SearchResult solver_result;
    static double p_a[MAX_SQUARES];
    static Board board;
    parse_board(&board, args);

    if (board.unknown_c == board.mine_c) {
        fprintf(stderr, "Board is solved.\n");
        exit(1);
    }

    get_solver_result(&board, args, &solver_result, p_a);
    if (solver_result.fault_status == fault_computational_limit) { // If computational limit reached - only do basic solve
        get_solver_result_basic(&board, args, &solver_result, p_a);
    }

    if (solver_result.fault_status == fault_invalid_board){
        fprintf(stderr, "Invalid board.\n");
        exit(1);
    }

    if (solver_result.fault_status == fault_internal_limit){
        fprintf(stderr, "Internal limit reached.\n");
        exit(1);
    }

    for (int32_t y = 0; y < board.h; y++) {
        for (int32_t x = 0; x < board.w; x++) {
            if (board.known[y*board.w+ x]){
                printf("     ");
            }
            else {
                printf("%.2f ", p_a[y*board.w+ x]);
            }
        }
        printf("\n");
    }
}