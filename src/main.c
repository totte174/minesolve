#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <strings.h>

#include "permutations.h"
#include "board.h"
#include "statistics.h"
#include "game.h"
#include "common.h"

const char *argp_program_version =
    "mssolve 0.1";
const char *argp_program_bug_address =
    "<totte174@gmail.com>";

/* Program documentation. */
static char doc[] =
    "mssolve -- a program to analyze minesweeper boards";

/* A description of the arguments we accept. */
static char args_doc[] = "[<BOARD>]";

/* The options we understand. */
static struct argp_option options[] = {
    {"file", 'f', "FILE", 0, "Read board from FILE instead of argument or stdin."},
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output"},
    {"width", 'w', "WIDTH", 0, "Width of minesweeper board"},
    {"height", 'h', "HEIGHT", 0, "Height of minesweeper board"},
    {"mines", 'm', "MINES", 0, "Number of mines present in minesweeper board"},
    {"diff", 'd', "DIFFICULTY", 0, "Use preset for width, height and number of mines: beginner, intermediate, or expert."},
    {"alpha", 'a', "ALPHA", 0, "Alpha parameter to use with solver."},
    {"beta", 'b', "BETA", 0, "Beta parameter to use with solver."},
    {"test", 't', "NUM", 0, "Let solver play NUM games and output the number of wins."},
    {"p_only", 'p', 0, 0, "Only use probability for determining value "},
    {0}};

/* Used by main to communicate with parse_opt. */
struct arguments
{
    char* board;
    char* output_file;
    int32_t width, height, mines, test_games;
    double alpha, beta;
    bool p_only;
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    struct arguments *arguments = state->input;

    switch (key)
    {
    case 'o':
        arguments->output_file = arg;
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
    case 'd':
        if (strcasecmp(arg, "expert")) {
            arguments->width = 30;
            arguments->height = 16;
            arguments->mines = 99;
        }
        else if (strcasecmp(arg, "intermediate")) {
            arguments->width = 16;
            arguments->height = 16;
            arguments->mines = 40;
        }
        else if (strcasecmp(arg, "beginner")) {
            arguments->width = 9;
            arguments->height = 9;
            arguments->mines = 10;
        }
        else argp_usage(state);
        break;
    case 'a':
        arguments->alpha = strtod(arg, NULL);
        break;
    case 'b':
        arguments->beta = strtod(arg, NULL);
        break;
    case 't':
        arguments->test_games = atoi(arg);
        break;
    case 'p':
        arguments->p_only = true;
        break;


    case ARGP_KEY_ARG:
        if (state->arg_num >= 1 ) argp_usage(state); // Too many arguments.
        arguments->board = arg;

        break;

    case ARGP_KEY_END:
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

int32_t main(int32_t argc, char** argv)
{
    srand(time(NULL)); // NEVER FORGET
    struct arguments arguments;

    /* Default values. */
    arguments.board = "";
    arguments.width = 30;
    arguments.height = 16;
    arguments.mines = 99;
    arguments.test_games = 100000;
    arguments.alpha = 0;
    arguments.beta = 0;
    arguments.output_file = "";
    arguments.p_only = false;

    /* Parse our arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.height * arguments.width <= 0 || arguments.height * arguments.width > MAX_SQUARES) exit(1);
    if (arguments.mines <= 0 || arguments.mines > MAX_MINES) exit(1);
    if (arguments.test_games < 0) exit(1);

    int32_t wins = 0;
    for (int32_t i = 0; i < arguments.test_games; i++)
    {
        if (play_game(arguments.width, arguments.height, arguments.mines, arguments.alpha, arguments.beta, arguments.p_only)) wins++;
    }
    printf("%'d\n", wins);
    exit(0);
}