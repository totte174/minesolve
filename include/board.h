#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

int32_t get_adjacent(Board* board, int32_t p, int32_t* adj);
bool is_border_unknown(Board* board, int32_t p);
bool is_border_known(Board* board, int32_t p);
void get_border(Board* board, Border* border);
void print_board(Board* board);

#endif