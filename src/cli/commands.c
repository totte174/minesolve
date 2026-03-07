#include "commands.h"

/* ----------- Simulation helpers (CLI-only) ----------- */

static void calculate_adjacent_mines(MsBoard* board) {
    for (int32_t i = 0; i < board->w * board->h; i++) {
        board->hint[i] = 0;
        int32_t adj[8];
        int32_t adj_c = get_adjacent(board, i, adj);
        for (int32_t j = 0; j < adj_c; j++) {
            if (board->mines[adj[j]]) board->hint[i] += 1;
        }
    }
}

static void generate_board(int32_t mine_c, MsBoard* board) {
    board->mine_count = 0;
    board->unrevealed_c = board->w * board->h;
    for (int32_t i = 0; i < board->w * board->h; i++) {
        board->hint[i] = 0;
        board->revealed[i] = false;
        board->mines[i] = false;
    }

    while (board->mine_count < mine_c) {
        int32_t i = rand() % (board->w * board->h);
        if (board->mines[i]) continue;
        board->mines[i] = true;
        board->mine_count += 1;
    }

    calculate_adjacent_mines(board);
}

static bool board_move(MsBoard* board, int32_t i, bool first_click) {
    if (board->revealed[i]) return true;

    if (board->mines[i]) {
        if (first_click) {
            while (true) {
                int32_t new_i = rand() % (board->w * board->h);
                if (board->mines[new_i]) continue;
                board->mines[new_i] = true;
                break;
            }
            board->mines[i] = false;
            calculate_adjacent_mines(board);
            return board_move(board, i, false);
        }
        else return false;
    }
    else {
        board->revealed[i] = true;
        board->unrevealed_c -= 1;
        if (board->hint[i] == 0) {
            int32_t adj[8];
            int32_t adj_c = get_adjacent(board, i, adj);
            for (int32_t j = 0; j < adj_c; j++) board_move(board, adj[j], false);
        }
        return true;
    }
}

/* ----------- CLI commands ----------- */

void simulate(Arguments* args) {
    static MsBoard board;
    static MsResult solver_result;
    static double p_a[MAX_SQUARES];
    ms_board_init(&board, args->width, args->height, args->mines, args->wrapping_borders);
    if (args->show_board && !args->ascii) printf("\e[2J"); // Clear screen

    int32_t wins = 0;
    for (int32_t game_c = 0; game_c < args->test_games; game_c++) {
        generate_board(args->mines, &board);
        bool first_move = true;
        bool alive = true;

        while (board.unrevealed_c > args->mines && alive) {
            if (args->show_board) {
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

            ms_solve(&board, args->max_depth, &solver_result, p_a);

            if (solver_result.status == MS_ERR_INVALID_BOARD) {
                print_board(&board);
                fprintf(stderr, "INVALID BOARD\n");
                exit(1);
            }

            if (solver_result.status == MS_ERR_INTERNAL_LIMIT) {
                print_board(&board);
                fprintf(stderr, "INTERNAL LIMIT REACHED\n");
                exit(1);
            }

            if (board.revealed[solver_result.move]) {
                print_board(&board);
                fprintf(stderr, "BEST VALUE IS KNOWN\n");
                exit(1);
            }

            if (p_a[solver_result.move] == 0) {
                // If multiple solved zeros, click them all
                for (int32_t i = 0; i < board.w * board.h; i++) {
                    if (p_a[i] == 0 && !board.revealed[i]) {
                        if (!board_move(&board, i, first_move)) {
                            fprintf(stderr, "MINE WHERE P=0\n");
                            exit(1);
                        }
                    }
                }
            }
            else if (!board_move(&board, solver_result.move, first_move)) {
                alive = false;
            }
            first_move = false;
        }
        if (args->show_board) {
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

void show_board(Arguments* args) {
    static MsBoard board;
    ms_board_init(&board, args->width, args->height, args->mines, args->wrapping_borders);
    ms_board_parse(&board, args->buf, args->buf_size);

    if (args->ascii) {
        print_board(&board);
    } else {
        print_board_pretty(&board, false);
    }
}

void solve_board(Arguments* args) {
    static MsResult solver_result;
    static MsBoard board;
    ms_board_init(&board, args->width, args->height, args->mines, args->wrapping_borders);
    ms_board_parse(&board, args->buf, args->buf_size);

    if (board.unrevealed_c == board.mine_count) {
        fprintf(stderr, "Board is already solved.\n");
        exit(1);
    }

    ms_solve(&board, args->max_depth, &solver_result, NULL);

    if (solver_result.status == MS_ERR_INVALID_BOARD) {
        fprintf(stderr, "Invalid board.\n");
        exit(1);
    }

    if (solver_result.status == MS_ERR_INTERNAL_LIMIT) {
        fprintf(stderr, "Internal limit reached.\n");
        exit(1);
    }

    if (solver_result.status == MS_OK_FALLBACK) {
        fprintf(stderr, "Warning: search budget exceeded; result from approximate solver.\n");
    }

    printf("(%d, %d)\n", solver_result.move % board.w, solver_result.move / board.w);
}

void show_probability(Arguments* args) {
    static MsResult solver_result;
    static double p_a[MS_MAX_SQUARES];
    static MsBoard board;
    ms_board_init(&board, args->width, args->height, args->mines, args->wrapping_borders);
    ms_board_parse(&board, args->buf, args->buf_size);

    if (board.unrevealed_c == board.mine_count) {
        fprintf(stderr, "Board is already solved.\n");
        exit(1);
    }

    ms_solve(&board, args->max_depth, &solver_result, p_a);

    if (solver_result.status == MS_ERR_INVALID_BOARD) {
        fprintf(stderr, "Invalid board.\n");
        exit(1);
    }

    if (solver_result.status == MS_ERR_INTERNAL_LIMIT) {
        fprintf(stderr, "Internal limit reached.\n");
        exit(1);
    }

    if (solver_result.status == MS_OK_FALLBACK) {
        fprintf(stderr, "Warning: search budget exceeded; probabilities from approximate solver.\n");
    }

    for (int32_t y = 0; y < board.h; y++) {
        for (int32_t x = 0; x < board.w; x++) {
            if (board.revealed[y * board.w + x]) {
                printf("     ");
            }
            else {
                printf("%.2f ", p_a[y * board.w + x]);
            }
        }
        printf("\n");
    }
}
