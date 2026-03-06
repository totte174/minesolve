/**
 * minesolve.h — Public C API for the minesolve library.
 *
 * Usage:
 *   1. Initialise a board with ms_board_init().
 *   2. Parse an ASCII board state with ms_board_parse().
 *   3. Call ms_solve() or ms_probabilities() to analyse the position.
 */

#ifndef MINESOLVE_H
#define MINESOLVE_H

#include <stdint.h>
#include <stdbool.h>

#define MINESOLVE_VERSION_MAJOR 0
#define MINESOLVE_VERSION_MINOR 2
#define MINESOLVE_VERSION_PATCH 0

/* Helper macros for stringifying integer macro values. */
#define MINESOLVE_STRINGIFY_(x) #x
#define MINESOLVE_STRINGIFY(x)  MINESOLVE_STRINGIFY_(x)

/** Version string literal, e.g. "0.2.0". */
#define MINESOLVE_VERSION_STRING \
    MINESOLVE_STRINGIFY(MINESOLVE_VERSION_MAJOR) "." \
    MINESOLVE_STRINGIFY(MINESOLVE_VERSION_MINOR) "." \
    MINESOLVE_STRINGIFY(MINESOLVE_VERSION_PATCH)

/** Maximum number of squares on a board (w * h must not exceed this). */
#define MS_MAX_SQUARES 1024

/** Maximum supported search depth for ms_solve(). */
#define MS_MAX_DEPTH 8

/* ----------- Status codes ----------- */

typedef enum MsStatus {
    MS_OK                   = 0, /**< Success. */
    MS_OK_FALLBACK,              /**< Solver fell back to a simple equation solver. Probabilities are only approximate for all values except 0 and 1. */
    MS_ERR_COMPUTATIONAL_LIMIT,  /**< Search exceeded computational budget; result may be approximate. */
    MS_ERR_INTERNAL_LIMIT,       /**< An internal capacity was exceeded. */
    MS_ERR_INVALID_BOARD,        /**< Board state is logically inconsistent. */
    MS_ERR_INVALID_PARAM,        /**< Invalid function parameters. */
    MS_ERR_UNKNOWN,              /**< Unclassified error. */
} MsStatus;

/* ----------- Board ----------- */

/**
 * MsBoard represents the current visible state of a Minesweeper board.
 *
 * Fields:
 *   w, h      — board dimensions.
 *   mine_c    — total number of mines.
 *   unknown_c — number of unrevealed (unknown) squares.
 *   v[]       — revealed value (0-8) for each square; 0 if unknown.
 *   known[]   — true if square is revealed.
 *   wrapping_borders — whether the board wraps at edges.
 *
 * Squares are stored in row-major order: index = y * w + x.
 */
typedef struct MsBoard {
    int32_t w;
    int32_t h;
    int32_t mine_c;
    int32_t unknown_c;
    int32_t v[MS_MAX_SQUARES];
    bool    known[MS_MAX_SQUARES];
    bool    mines[MS_MAX_SQUARES]; /**< Only valid during simulation; leave false for analysis. */
    bool    wrapping_borders;
} MsBoard;

/* ----------- Solve result ----------- */

/**
 * Result of ms_solve().
 *
 * Fields:
 *   status     — MS_OK on success; MS_OK_FALLBACK if the result came from
 *                the approximate fallback solver.
 *   best_move  — flat index (y * w + x) of the recommended square to reveal.
 *   p_mine     — at depth 1 (or MS_OK_FALLBACK): probability [0, 1] that
 *                best_move contains a mine.
 *                at depth > 1: probability of losing (hitting a mine) within
 *                the depth-step lookahead window when playing optimally from
 *                best_move onward.  Values of exactly 0 or 1 are always exact.
 */
typedef struct MsResult {
    MsStatus status;
    int32_t  best_move;
    double   p_mine;
} MsResult;

/* ----------- API ----------- */

/**
 * Initialise all fields of a board.
 *
 * @param board    Board to initialise.
 * @param w        Width.
 * @param h        Height.  w * h must be <= MS_MAX_SQUARES.
 * @param mines    Total mine count.
 * @param wrapping Enable wrapping borders.
 */
void ms_board_init(MsBoard* board, int32_t w, int32_t h, int32_t mines, bool wrapping);

/**
 * Parse an ASCII board string into a pre-initialised board.
 *
 * Character mapping:
 *   '0'-'8'        — revealed square with that many adjacent mines.
 *   ' '            — revealed square with 0 adjacent mines.
 *   '.', 'x', '?'  — unknown (unrevealed) square.
 *   '\n'           — row separator (trailing columns filled as unknown).
 *   '\r'           — ignored.
 *
 * @param board    Board previously set up with ms_board_init().
 * @param input    ASCII board string (need not be NUL-terminated).
 * @param len      Length of input in bytes.
 */
void ms_board_parse(MsBoard* board, const char* input, int32_t len);

/**
 * Find the best square to reveal.
 *
 * Uses a lookahead tree search up to the given depth.  Falls back to a
 * single-level probability solve if the search budget is exceeded.
 *
 * @param board    Current board state.
 * @param depth    Search depth in [1, MS_MAX_DEPTH].
 * @param result   Output: best move and its mine probability.
 * @param p_out    Optional output array (length w*h) of per-square mine
 *                 probabilities.  Pass NULL to skip.
 * @return MS_OK on success, or an error code.
 */
MsStatus ms_solve(MsBoard* board, int32_t depth, MsResult* result, double* p_out);

/**
 * Compute per-square mine probabilities without searching for the best move.
 *
 * @param board    Current board state.
 * @param p_out    Output array (length w*h) filled with probabilities in [0, 1].
 * @return MS_OK on success, or an error code.
 */
MsStatus ms_probabilities(MsBoard* board, double* p_out);

#endif /* MINESOLVE_H */
