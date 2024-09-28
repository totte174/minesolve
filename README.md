# mssolve

A highly optimized Minesweeper solver implemented in C designed for effiency and high win rates.
It utilizes combinatorial and probability analysis as well as game tree search to find optimal move for current board.

## Building

Run ```make mssolve``` to compile ```./bin/mssolve```.

Requires ```gcc``` (or [w64devkit](https://github.com/skeeto/w64devkit/releases/)) to compile (no external libraries needed).

## Currently working on
- work on CLI to allow analysis of boards (right now it can only simulate full games)
- generalize to N-dimensional minesweeper
- allow boards with wrapping edges

## Possible improvements to stability
- Improvements to equation reduction to possibly split the really hard border which cause the solver to run out of memory
