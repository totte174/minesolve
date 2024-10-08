#ifndef COMMANDS_H
#define COMMANDS_H

#include <time.h>

#include "common.h"
#include "board.h"
#include "solver.h"

void simulate(Arguments* args);
void show_board(Arguments *args);
void solve_board(Arguments *args);
void show_probability(Arguments *args);

#endif