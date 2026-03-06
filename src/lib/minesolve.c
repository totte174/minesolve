#include "minesolve.h"
#include "board.h"
#include "solver.h"

void ms_board_init(MsBoard* board, int32_t w, int32_t h, int32_t mines, bool wrapping) {
    board_init(board, w, h, mines, wrapping);
}

void ms_board_parse(MsBoard* board, const char* input, int32_t len) {
    board_parse(board, input, len);
}

MsStatus ms_solve(MsBoard* board, int32_t depth, MsResult* result, double* p_out) {
    if (depth < 1) return MS_ERR_INVALID_PARAM;
    if (depth > MS_MAX_DEPTH) return MS_ERR_INVALID_PARAM;

    *result = (MsResult){ .status = MS_OK, .best_move = -1, .p_mine = 1.0 };

    get_solver_result(board, depth, result, p_out);

    if (result->status == MS_ERR_COMPUTATIONAL_LIMIT) {
        get_solver_result_basic(board, result, p_out);
        if (result->status == MS_OK) result->status = MS_OK_FALLBACK;
    }

    return result->status;
}

MsStatus ms_probabilities(MsBoard* board, double* p_out) {
    return get_board_probabilities(board, p_out);
}
