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
#include "game.h"
#include "common.h"

const char *argp_program_version = "mssolve 0.1";
const char *argp_program_bug_address = "<totte174@gmail.com>";
static char doc[] = "mssolve -- a program to analyze minesweeper boards";
static char args_doc[] = "[<BOARD>]";

static struct argp_option options[] = {
    {"file", 'f', "FILE", 0, "Read board from FILE instead of argument or stdin."},
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output"},
    {"width", 'w', "WIDTH", 0, "Width of minesweeper board"},
    {"height", 'h', "HEIGHT", 0, "Height of minesweeper board"},
    {"mines", 'm', "MINES", 0, "Number of mines present in minesweeper board"},
    {"config", 'c', "CONFIGURATION", 0, "Use preset configuration for width, height and number of mines: beginner, intermediate, or expert."},
    {"alpha", 'a', "ALPHA", 0, "Alpha parameter to use with solver. (Unused right now)"},
    {"max_depth", 'd', "MAX_DEPTH", 0, "Maximum depth for solver to search."},
    {"min_brute", 'b', "MIN_BRUTE", 0, "Minimum amount of safe squares left for solver to enter brute force mode."},
    {"test", 't', "NUM", 0, "Let solver play NUM games and output the number of wins."},
    {"p_only", 'p', 0, 0, "Only use probability for determining value "},
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
    case 'c':
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
    case 'd':
        arguments->max_depth = atoi(arg);
        break;
    case 'b':
        arguments->min_brute = atoi(arg);
        break;
    case 't':
        arguments->test_games = atoi(arg);
        break;
    case 'p':
        arguments->p_only = true;
        break;


    case ARGP_KEY_ARG:
        if (state->arg_num >= 1 ) argp_usage(state); // Too many arguments.
        strcpy(arguments->board, arg); // Change so it is not unsafe
        break;

    case ARGP_KEY_END:
        if (arguments->height * arguments->width <= 0 || arguments->height * arguments->width > MAX_SQUARES) argp_usage(state);
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
    srand(time(NULL)); // NEVER FORGET

    Arguments arguments = {
        .width = 30,
        .height = 16,
        .mines = 99,
        .test_games = 10000000, //0
        .alpha = 0,
        .max_depth = 1,
        .min_brute = 0,
        .output_file = "",
        .p_only = false,
    };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    if (arguments.alpha == 0) arguments.p_only = true;

    int32_t wins = 0;
    for (int32_t i = 0; i < arguments.test_games; i++)
    {
        if (play_game(&arguments)) wins++;
    }
    printf("%d\n", wins);
    exit(0);
}