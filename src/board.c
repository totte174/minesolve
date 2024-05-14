#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "board.h"

int32_t get_adjacent(Board* board, int32_t p, int32_t* adj) {
    int32_t i = 0;
    int32_t a_adj[] =  {p - board->w - 1,   p - board->w,   p - board->w + 1,
                        p - 1,              p,              p + 1,
                        p + board->w - 1,   p + board->w,   p + board->w + 1};
    for (int j = 0; j < 9; j++) {
        if (a_adj[j] < 0 || a_adj[j] >= board->w * board->h) continue;
        if (p % board->w == 0 && j % 3 == 0) continue;
        if (p % board->w == board->w-1 && j % 3 == 2) continue; 
        if (a_adj[j] == p) continue;
        adj[i++] = a_adj[j];
    }
    return i;
}

int32_t get_adjacent_unknown(Board* board, int32_t p, int32_t* adj) {
    int32_t i = 0;
    int32_t a_adj[] =  {p - board->w - 1,   p - board->w,   p - board->w + 1,
                        p - 1,              p,              p + 1,
                        p + board->w - 1,   p + board->w,   p + board->w + 1};
    for (int j = 0; j < 9; j++) {
        if (a_adj[j] < 0 || a_adj[j] >= board->w * board->h) continue;
        if (p % board->w == 0 && j % 3 == 0) continue;
        if (p % board->w == board->w-1 && j % 3 == 2) continue; 
        if (a_adj[j] == p) continue;
        if (board->known[a_adj[j]]) continue;
        adj[i++] = a_adj[j];
    }
    return i;
}

bool is_border_unknown(Board* board, int32_t p) {
    if (board->known[p]) return false;
    int32_t adj[8];
    int32_t adj_c = get_adjacent(board, p, adj);
    for (int j = 0; j < adj_c; j++) {
        if (board->known[adj[j]]) return true;
    }
    return false;
}

bool is_border_known(Board* board, int32_t p) {
    if (!board->known[p]) return false;
    int32_t adj[8];
    int32_t adj_c = get_adjacent(board, p, adj);
    for (int j = 0; j < adj_c; j++) {
        if (!board->known[adj[j]]) return true;
    }
    return false;
}

void get_border(Board* board, Border* border) {
    border->border_known_c = 0;
    border->border_unknown_c = 0;
    border->outside_known_c = 0;
    border->outside_unknown_c = 0;
    for (int32_t p = 0; p < board->w * board->h; p++) {
        if (!board->known[p]) {
            if (is_border_unknown(board, p)) {
                border->border_unknown[border->border_unknown_c++] = p;
            }
            else {
                border->outside_unknown_c++;
            }
        }
        else {
            if (is_border_known(board, p)) {
                border->border_known[border->border_known_c++] = p;
            }
            else {
                border->outside_known_c++;
            }
        }
    }
}

bool is_on_perimeter(Board* board, int32_t p) {
    return (p / board->w == 0 || p / board->w == board->h - 1 || // is top or bottom
            p % board->w == 0 || p % board->w == board->w - 1);  // Left or right
}

bool is_on_corner(Board* board, int32_t p) {
    return (p / board->w == 0 || p / board->w == board->h - 1) && // is top or bottom
           (p % board->w == 0 || p % board->w == board->w - 1);  // Left or right
}

void print_board(Board* board) {
    printf("+");
    for (int32_t x = 0; x < board->w; x++) printf("-");
    printf("+\n");
    for (int32_t y = 0; y < board->h; y++) {
        printf("|");
        for (int32_t x = 0; x < board->w; x++) {
            if (board->known[y*board->w + x]){
                printf("%d", board->v[y*board->w + x]);
            }
            else {
                printf(" ");
            }
        }
        printf("|\n");
    }
    printf("+");
    for (int32_t x = 0; x < board->w; x++) printf("-");
    printf("+\n");
}