#ifndef BOARD_H
#define BOARD_H

#include "common.h"

int32_t get_adjacent(Board* board, int32_t p, int32_t* adj);
int32_t get_adjacent_c(Board* board, int32_t p);
int32_t get_adjacent_unknown(Board* board, int32_t p, int32_t* adj);
int32_t get_adjacent_unknown_c(Board* board, int32_t p);
bool is_edge(Board* board, int32_t p);
void print_board(Board* board);

#endif