#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

void calculate_adjacent_bombs(Board* board);
void generate_board(int32_t w, int32_t h, int32_t bomb_c, Board* board);
bool move(Board* board, int32_t i, bool first_click);
bool play_game(int32_t w, int32_t h, int32_t bomb_c);

#endif