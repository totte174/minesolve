#ifndef COMMANDS_H
#define COMMANDS_H

#include <time.h>
#include "common.h"
#include "board.h"

#define MAX_BUF_CLI (MAX_SQUARES * 2)

typedef struct Arguments {
    char buf[MAX_BUF_CLI];
    int32_t buf_size;

    int32_t width, height, mines, test_games, max_depth;
    bool wrapping_borders, ascii, show_board, show_probability;
} Arguments;

void simulate(Arguments* args);
void show_board(Arguments* args);
void solve_board(Arguments* args);
void show_probability(Arguments* args);

#endif
