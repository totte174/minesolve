#ifndef BOARD_H
#define BOARD_H

#include "common.h"

/* MsBoard lifecycle */
void board_init(MsBoard* board, int32_t w, int32_t h, int32_t mines, bool wrapping);
void board_parse(MsBoard* board, const char* buf, int32_t buf_size);

/* Adjacency queries */
int32_t get_adjacent(MsBoard* board, int32_t p, int32_t* adj);
int32_t get_adjacent_c(MsBoard* board, int32_t p);
int32_t get_adjacent_unknown(MsBoard* board, int32_t p, int32_t* adj);
int32_t get_adjacent_unknown_c(MsBoard* board, int32_t p);
bool is_edge(MsBoard* board, int32_t p);

/* Display */
void print_board(MsBoard* board);
void print_board_pretty(MsBoard* board, bool move_cursor);

#endif