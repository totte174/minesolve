#include <argp.h>
#include <unistd.h>

#include "commands.h"

const char *argp_program_version = "minesolve " MINESOLVE_VERSION_STRING;
const char *argp_program_bug_address = "<https://github.com/totte174/minesolve/issues>";
static char doc[] =
    "minesolve -- a Minesweeper board analyzer and solver\n"
    "\n"
    "Reads a board from FILE, stdin, or a positional argument and either\n"
    "solves it (default), displays it, or runs a simulation.\n"
    "\n"
    "MsBoard format: digits 0-8 for revealed squares, '.' for unknown,\n"
    "space for revealed zero, newlines as row separators.";
static char args_doc[] = "[BOARD]";

static struct argp_option options[] = {
    {0, 0, 0, 0, "Input:", 1},
    {"file",             'f', "FILE",          0, "Read board from FILE instead of argument or stdin.", 1},

    {0, 0, 0, 0, "MsBoard configuration:", 2},
    {"config",           'c', "PRESET",        0, "Use a preset: beginner (9x9, 10 mines), intermediate (16x16, 40), expert (30x16, 99).", 2},
    {"width",            'w', "N",             0, "MsBoard width (default: 30).", 2},
    {"height",           'h', "N",             0, "MsBoard height (default: 16).", 2},
    {"mines",            'm', "N",             0, "Number of mines (default: 99).", 2},
    {"wrapping-borders", 'W', 0,               0, "Enable wrapping borders.", 2},

    {0, 0, 0, 0, "Solver options:", 3},
    {"depth",            'd', "N",             0, "Search depth for lookahead (default: 1, max: 8).", 3},

    {0, 0, 0, 0, "Actions (default: solve):", 4},
    {"simulate",         's', "N",             0, "Simulate N games and print the win count.", 4},
    {"show-board",       'S', 0,               0, "Print the board instead of solving. With --simulate, shows each move.", 4},
    {"probability",      'p', 0,               0, "Print mine probabilities for each unknown square.", 4},
    {"ascii",            'a', 0,               0, "Use plain ASCII output (no ANSI colors or Unicode).", 4},
    {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct Arguments *arguments = state->input;

    switch (key) {
    case 'w':
        arguments->width = atoi(arg);
        if (arguments->width == 0) argp_usage(state);
        break;
    case 'h':
        arguments->height = atoi(arg);
        if (arguments->height == 0) argp_usage(state);
        break;
    case 'm':
        arguments->mines = atoi(arg);
        if (arguments->mines == 0) argp_usage(state);
        break;
    case 'W':
        arguments->wrapping_borders = true;
        break;
    case 'c':
        if (strcasecmp(arg, "expert") == 0) {
            arguments->width = 30;
            arguments->height = 16;
            arguments->mines = 99;
        } else if (strcasecmp(arg, "intermediate") == 0) {
            arguments->width = 16;
            arguments->height = 16;
            arguments->mines = 40;
        } else if (strcasecmp(arg, "beginner") == 0) {
            arguments->width = 9;
            arguments->height = 9;
            arguments->mines = 10;
        } else {
            argp_error(state, "unknown preset '%s': expected beginner, intermediate, or expert", arg);
        }
        break;
    case 'd':
        arguments->max_depth = atoi(arg);
        if (arguments->max_depth < 1 || arguments->max_depth > MS_MAX_DEPTH) {
            argp_error(state, "depth must be between 1 and %d", MS_MAX_DEPTH);
        }
        break;
    case 's':
        arguments->test_games = atoi(arg);
        if (arguments->test_games <= 0) argp_usage(state);
        break;
    case 'a':
        arguments->ascii = true;
        break;
    case 'S':
        arguments->show_board = true;
        break;
    case 'f': {
        FILE *file = fopen(arg, "r");
        if (!file) {
            argp_error(state, "cannot open file: %s", arg);
        }
        int32_t n = fread(arguments->buf, 1, sizeof(arguments->buf) - 1, file);
        fclose(file);
        if (n <= 0) {
            argp_error(state, "file is empty or unreadable: %s", arg);
        }
        arguments->buf_size = n;
        break;
    }
    case 'p':
        arguments->show_probability = true;
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num >= 1) argp_usage(state);
        arguments->buf_size = strlen(arg);
        if ((size_t)arguments->buf_size >= sizeof(arguments->buf)) {
            argp_error(state, "board string too long");
        }
        strncpy(arguments->buf, arg, sizeof(arguments->buf) - 1);
        break;

    case ARGP_KEY_END:
        if (arguments->height <= 0 || arguments->width <= 0 ||
            arguments->height * arguments->width > MS_MAX_SQUARES) {
            argp_error(state, "board dimensions out of range (max %d squares)", MS_MAX_SQUARES);
        }
        if (arguments->mines <= 0 || arguments->mines > arguments->height * arguments->width) {
            argp_error(state, "mine count must be between 1 and %d", arguments->height * arguments->width);
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

int main(int argc, char** argv) {
    // Initialize random seed
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    srand((uint32_t) ts.tv_nsec);

    Arguments arguments = {
        .buf_size = 0,

        .width = 30,
        .height = 16,
        .mines = 99,
        .wrapping_borders = false,

        .test_games = 0,
        .max_depth = 1,

        .show_board = false,
        .show_probability = false,
        .ascii = false,
    };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // Read stdin if piped in and no board provided yet
    if (arguments.buf_size == 0 && !isatty(fileno(stdin))) {
        int32_t n = read(STDIN_FILENO, arguments.buf, sizeof(arguments.buf) - 1);
        if (n > 0) arguments.buf_size = n;
    }

    if (arguments.test_games != 0) {
        simulate(&arguments);
    } else if (arguments.buf_size > 0) {
        if (arguments.show_board) show_board(&arguments);
        else if (arguments.show_probability) show_probability(&arguments);
        else solve_board(&arguments);
    } else {
        fprintf(stderr, "minesolve: no board provided (use -f FILE, stdin, or pass board as argument)\n");
        argp_help(&argp, stderr, ARGP_HELP_SEE, argv[0]);
        return 1;
    }
    return 0;
}
