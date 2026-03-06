#include "board.h"

// Global adjacency table — valid for a fixed (w, h, wrapping_borders) geometry.
// Rebuilt automatically when board_init is called with new dimensions.
static int32_t s_adj[MAX_SQUARES][8];
static int8_t  s_adj_c[MAX_SQUARES];
static int32_t s_adj_w = -1, s_adj_h = -1;
static bool    s_adj_wrapping = false;

static void build_adj_table(MsBoard* board) {
    s_adj_w = board->w; s_adj_h = board->h; s_adj_wrapping = board->wrapping_borders;

    for (int32_t p = 0; p < board->w * board->h; p++) {
        int32_t i = 0;
        int32_t x = p % board->w, y = p / board->w;
        if (board->wrapping_borders) {
            int32_t w = board->w, h = board->h;
            int32_t xl = (x == 0)   ? w-1 : x-1,  xr = (x == w-1) ? 0   : x+1;
            int32_t yu = (y == 0)   ? h-1 : y-1,  yd = (y == h-1) ? 0   : y+1;
            s_adj[p][i++] = yu*w + xl;  s_adj[p][i++] = yu*w + x;  s_adj[p][i++] = yu*w + xr;
            s_adj[p][i++] = y *w + xl;                               s_adj[p][i++] = y *w + xr;
            s_adj[p][i++] = yd*w + xl;  s_adj[p][i++] = yd*w + x;  s_adj[p][i++] = yd*w + xr;
        } else {
            bool left  = x > 0,        right = x < board->w - 1;
            bool above = y > 0,        below = y < board->h - 1;
            if (above) {
                if (left)  s_adj[p][i++] = p - board->w - 1;
                s_adj[p][i++] = p - board->w;
                if (right) s_adj[p][i++] = p - board->w + 1;
            }
            if (left)  s_adj[p][i++] = p - 1;
            if (right) s_adj[p][i++] = p + 1;
            if (below) {
                if (left)  s_adj[p][i++] = p + board->w - 1;
                s_adj[p][i++] = p + board->w;
                if (right) s_adj[p][i++] = p + board->w + 1;
            }
        }
        s_adj_c[p] = (int8_t) i;
    }
}

void board_init(MsBoard* board, int32_t w, int32_t h, int32_t mines, bool wrapping) {
    board->w = w;
    board->h = h;
    board->mine_c = mines;
    board->wrapping_borders = wrapping;
    board->unknown_c = w * h;
    for (int32_t i = 0; i < w * h; i++) {
        board->known[i] = false;
        board->v[i] = 0;
    }
    build_adj_table(board);
}

void board_parse(MsBoard* board, const char* buf, int32_t buf_size) {
    int32_t i = 0;
    for (int32_t j = 0; j < buf_size; j++) {
        char c = buf[j];
        if (c == '\r') continue;
        else if (c == '\n') {
            if (i % board->w != 0) i = (i / board->w + 1) * board->w;
        }
        else if (c >= '0' && c <= '8') {
            board->known[i] = true;
            board->v[i] = (int32_t)(c - '0');
            board->unknown_c--;
            i++;
        }
        else if (c == ' ') {
            board->known[i] = true;
            board->v[i] = 0;
            board->unknown_c--;
            i++;
        }
        else if (c == '.' || c == 'x' || c == 'X' || c == '?') {
            board->known[i] = false;
            i++;
        }
    }
}

int32_t get_adjacent(MsBoard* board, int32_t p, int32_t* adj) {
    (void)board;
    int8_t c = s_adj_c[p];
    for (int8_t k = 0; k < c; k++) adj[k] = s_adj[p][k];
    return c;
}

int32_t get_adjacent_c(MsBoard* board, int32_t p) {
    (void)board;
    return s_adj_c[p];
}

int32_t get_adjacent_unknown(MsBoard* board, int32_t p, int32_t* adj) {
    int32_t c = 0;
    int8_t n = s_adj_c[p];
    for (int8_t k = 0; k < n; k++) {
        int32_t nb = s_adj[p][k];
        if (!board->known[nb]) adj[c++] = nb;
    }
    return c;
}

int32_t get_adjacent_unknown_c(MsBoard* board, int32_t p) {
    int32_t c = 0;
    int8_t n = s_adj_c[p];
    for (int8_t k = 0; k < n; k++) c += !board->known[s_adj[p][k]];
    return c;
}

bool is_edge(MsBoard* board, int32_t p) {
    bool kp = board->known[p];
    int8_t n = s_adj_c[p];
    for (int8_t k = 0; k < n; k++)
        if (board->known[s_adj[p][k]] != kp) return true;
    return false;
}

void print_board(MsBoard* board) {
    for (int32_t y = 0; y < board->h; y++) {
        for (int32_t x = 0; x < board->w; x++) {
            if (board->known[y*board->w + x]){
                if (board->v[y*board->w + x] == 0) printf(" ");
                else printf("%d", board->v[y*board->w + x]);
            }
            else {
                printf(".");
            }
        }
        printf("\n");
    }
}

void print_board_pretty(MsBoard* board, bool move_cursor) {
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

    if (move_cursor) {
        printf("\e[?25l"); // Make cursor invisible
        printf("\e[H"); // Move cursor to start
    }

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

    if (move_cursor) printf("\e[?25h"); // Make cursor visible
}
