#include <argp.h>

#include "commands.h"
#include "common.h"

const char *argp_program_version = "minesolve 0.1";
const char *argp_program_bug_address = "<totte174@gmail.com>";
static char doc[] = "minesolve -- a program to analyze minesweeper boards";
static char args_doc[] = "[<BOARD>]";

static struct argp_option options[] = {
    {"file", 'f', "FILE", 0, "Read board from FILE instead of argument or stdin."},
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output."},
    {"width", 'w', "WIDTH", 0, "Width of minesweeper board."},
    {"height", 'h', "HEIGHT", 0, "Height of minesweeper board."},
    {"mines", 'm', "MINES", 0, "Number of mines present in minesweeper board."},
    {"wrapping-borders", 'W', 0, 0, "Set wrapping borders in minesweeper board."},
    {"config", 'c', "CONFIGURATION", 0, "Use preset configuration for width, height and number of mines: beginner, intermediate, or expert."},
    {"depth", 'd', "DEPTH", 0, "Maximum depth for solver to search."},
    {"simulate", 's', "NUM", 0, "Let solver play NUM games and output the number of wins."},
    {"show-board", 'S', 0, 0, "Print out board instead of running solver. If used with --simulate it will print the board after every move."},
    {"ascii", 'a', 0, 0, "Used with --show-board, specifies that it should print the board without ANSI escape codes or Unicode."},
    {0}};


static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct Arguments *arguments = state->input;

    switch (key)
    {
    case 'o':
        strcpy(arguments->output_file, arg); // Change so it is not unsafe
        break;
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

    case ARGP_KEY_ARG:
        if (state->arg_num >= 1 ) argp_usage(state); // Too many arguments.
        strcpy(arguments->board, arg); // Change so it is not unsafe
        break;

    case ARGP_KEY_END:
        if ((arguments->height * arguments->width <= 0) || (arguments->height * arguments->width > MAX_SQUARES)) argp_usage(state);
        if (arguments->mines <= 0 || arguments->mines > MAX_MINES) argp_usage(state);
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
        .width = 30,
        .height = 16,
        .mines = 99,
        .wrapping_borders = false,

        .test_games = 1000, //EDITED
        .max_depth = 1,

        .show_board = false,
        .ascii = false,
    };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    simulate(&arguments);
    exit(0);
}