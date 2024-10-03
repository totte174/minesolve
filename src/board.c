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

int32_t get_adjacent_c(Board* board, int32_t p) {
    int32_t i = 0;
    int32_t a_adj[] =  {p - board->w - 1,   p - board->w,   p - board->w + 1,
                        p            - 1,   p,              p            + 1,
                        p + board->w - 1,   p + board->w,   p + board->w + 1};
    for (int j = 0; j < 9; j++) {
        if (a_adj[j] < 0 || a_adj[j] >= board->w * board->h) continue;
        if (p % board->w == 0 && j % 3 == 0) continue;
        if (p % board->w == board->w-1 && j % 3 == 2) continue; 
        if (a_adj[j] == p) continue;
        i++;
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

int32_t get_adjacent_unknown_c(Board* board, int32_t p) {
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
        i++;
    }
    return i;
}

bool is_edge(Board* board, int32_t p) {
    int32_t adj[8];
    int32_t adj_c = get_adjacent(board, p, adj);
    for (int j = 0; j < adj_c; j++) {
        if (board->known[adj[j]] != board->known[p]) return true;
    }
    return false;
}

void print_board(Board* board) {
    printf("+");
    for (int32_t x = 0; x < board->w; x++) printf("-");
    printf("+\n");
    for (int32_t y = 0; y < board->h; y++) {
        printf("|");
        for (int32_t x = 0; x < board->w; x++) {
            if (board->known[y*board->w + x]){
                if (board->v[y*board->w + x] == 0) printf(" ");
                else printf("%d", board->v[y*board->w + x]);
            }
            else {
                printf(".");
            }
        }
        printf("|\n");
    }
    printf("+");
    for (int32_t x = 0; x < board->w; x++) printf("-");
    printf("+\n");
}

void print_board_pretty(Board* board) {
    static char color_nums[][10] = {
        [0] = " ",
        [1] = "\e[1;94m" "1",
        [2] = "\e[1;32m" "2",
        [3] = "\e[1;91m" "3",
        [4] = "\e[1;34m" "4",
        [5] = "\e[1;31m" "5",
        [6] = "\e[1;36m" "6",
        [7] = "\e[1;30m" "7",
        [8] = "\e[1;30m" "8",
        [9] = "\e[0m"    "⬝",
    };

    printf("\e[?25l"); // Make cursor invisible
    printf("\e[H"); // Move cursor to start
    
    printf("┏");
    for (int32_t x = 0; x < board->w; x++) printf("━");
    printf("┓\n");

    for (int32_t y = 0; y < board->h; y++) {
        printf("┃");
        for (int32_t x = 0; x < board->w; x++) {
            if (board->known[y*board->w + x]){
                printf("%s", color_nums[board->v[y*board->w + x]]);
            }
            else {
                printf("%s", color_nums[9]);
            }
        }
        printf("\e[0m"); // Reset
        printf("┃\n");
    }
    printf("┗");
    for (int32_t x = 0; x < board->w; x++) printf("━");
    printf("┛\n");

    printf("\e[?25h"); // Make cursor visible
}