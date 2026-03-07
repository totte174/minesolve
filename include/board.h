#ifndef BOARD_H
#define BOARD_H

#include "common.h"

/* MsBoard lifecycle */
/** Initializes board and precomputes adjacency table. */
void board_init(MsBoard* board, int32_t w, int32_t h, int32_t mines, bool wrapping);
/** Parses an ASCII board string into board. */
void board_parse(MsBoard* board, const char* buf, int32_t buf_size);

/* Adjacency queries */
/** Returns all neighbors of p into adj. */
int32_t get_adjacent(MsBoard* board, int32_t p, int32_t* adj);
/** Returns neighbor count of p. */
int32_t get_adjacent_c(MsBoard* board, int32_t p);
/** Returns unrevealed neighbors of p into adj. */
int32_t get_adjacent_unknown(MsBoard* board, int32_t p, int32_t* adj);
/** Returns unrevealed neighbor count of p. */
int32_t get_adjacent_unknown_c(MsBoard* board, int32_t p);
/** True if p has at least one revealed neighbor. */
bool is_frontier(MsBoard* board, int32_t p);

/* Display */
/** Prints board as ASCII. */
void print_board(MsBoard* board);
/** Prints board with colors and Unicode. */
void print_board_pretty(MsBoard* board, bool move_cursor);

#endif
