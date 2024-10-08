#include <argp.h>
#include <unistd.h>

#include "commands.h"
#include "common.h"

const char *argp_program_version = "minesolve 0.1";
const char *argp_program_bug_address = "<totte174@gmail.com>";
static char doc[] = "minesolve -- a program to analyze minesweeper boards";
static char args_doc[] = "[<BOARD>]";

static struct argp_option options[] = {
    {"file",                'f', "FILE",            0, "Read board from FILE instead of argument or stdin.", 0},

    {"width",               'w', "WIDTH",           0, "Width of minesweeper board.", 1},
    {"height",              'h', "HEIGHT",          0, "Height of minesweeper board.", 2},
    {"mines",               'm', "MINES",           0, "Number of mines present in minesweeper board.", 3},
    {"wrapping-borders",    'W', 0,                 0, "Set wrapping borders in minesweeper board.", 4},
    {"config",              'c', "CONFIGURATION",   0, "Use preset configuration for width, height and number of mines: beginner, intermediate, or expert.", 5},

    {"depth",               'd', "DEPTH",           0, "Maximum depth for solver to search.", 6},

    {"simulate",            's', "NUM",             0, "Let solver play NUM games and output the number of wins.", 7},
    {"show-board",          'S', 0,                 0, "Print out board instead of running solver. If used with --simulate it will print the board after every move.", 8},
    {"ascii",               'a', 0,                 0, "Used with --show-board, specifies that it should print the board without ANSI escape codes or Unicode.", 9},
    {"probability",         'p', 0,                 0, "Show mine probability of each square in board.", 10},
    {0}};


static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct Arguments *arguments = state->input;

    switch (key)
    {
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
        }
        else if (strcasecmp(arg, "intermediate") == 0) {
            arguments->width = 16;
            arguments->height = 16;
            arguments->mines = 40;
        }
        else if (strcasecmp(arg, "beginner") == 0) {
            arguments->width = 9;
            arguments->height = 9;
            arguments->mines = 10;
        }
        else argp_usage(state);
        break;
    case 'd':
        arguments->max_depth = atoi(arg);
        break;
    case 's':
        arguments->test_games = atoi(arg);
        break;
    case 'a':
        arguments->ascii = true;
        break;
    case 'S':
        arguments->show_board = true;
        break;
    case 'f':
        FILE *file = fopen(arg, "r");
        if (file) {
            int32_t n = fread(arguments->buf, 1, sizeof(arguments->buf), file);
            if (n > 0) arguments->buf_size = n; 
            else {
                fprintf(stderr, "File is empty or cannot be read: %s\n", arg);
                exit(1);
            }
        }
        else {
            fprintf(stderr, "Cannot read file: %s\n", arg);
            exit(1);
        }
        fclose(file);
        break;
    case 'p':
        arguments->show_probability = true;
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num >= 1 ) argp_usage(state); // Too many arguments.
        arguments->buf_size = strlen(arg);
        if (arguments->buf_size > sizeof(arguments->buf)) argp_usage(state); // String too long
        strncpy(arguments->buf, arg, sizeof(arguments->buf) - 1);
        break;

    case ARGP_KEY_END:
        if ((arguments->height * arguments->width <= 0) || (arguments->height * arguments->width > MAX_SQUARES)) {
            fprintf(stderr, "Invalid size of board.");
            exit(1);
        }
        if (arguments->mines <= 0 || arguments->mines > MAX_MINES || arguments->mines > arguments->height * arguments->width) {
            fprintf(stderr, "Invalid number of mines.");
            exit(1);
        }
        if (arguments->test_games < 0) argp_usage(state);
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int32_t main(int32_t argc, char** argv)
{
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

    // Read stdin if piped into
    if (arguments.buf_size == 0 && !isatty(fileno(stdin))) {
        int32_t n = read(STDIN_FILENO, arguments.buf, sizeof(arguments.buf));
        if (n > 0) arguments.buf_size = n;
    }

    if (arguments.test_games != 0) {
        simulate(&arguments);
    }
    else if (arguments.buf_size > 0) {
        if (arguments.show_board) show_board(&arguments);
        else if (arguments.show_probability) show_probability(&arguments);
        else solve_board(&arguments);
    }
    else {
        fprintf(stderr, "No board provided\n");
    }
    exit(0);
}