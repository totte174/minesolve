#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <alloca.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>

#include "permutations.h"
#include "board.h"
#include "statistics.h"
#include "game.h"
#include "common.h"

int32_t main(int32_t argc, char* argv[]) {
    srand(time(NULL)); //NEVER FORGET


    int32_t w = 30;
    int32_t h = 16;
    int32_t bomb_c = 99;

    int32_t wins = 0;
    int32_t games = 1000;
    for (int32_t i = 0; i < games; i++) {
        if (play_game(w, h, bomb_c)) wins++;
    }
    printf("Wins: %d/%d = %f\n", wins, games, ((double) wins) / ((double) games));
    
}