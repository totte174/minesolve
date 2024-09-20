# mssolve

A highly optimized Minesweeper solver implemented in C designed for effiency and high win rates.
It utilizes calculation of mine probabilities and searching future possible moves to find optimal move for current board.

## Building

Run ```make``` to compile ```bin/mssolve```.

Requires ```gcc``` (or [w64devkit](https://github.com/skeeto/w64devkit/releases/)) to compile.

## Future features
- generalize to N-dimensional minesweeper
- generalize to allow enabling of wrapping edges

## Possible improvements to stability
- Improvements to equation reduction to possibly split the really hard border which cause the solver to run out of memory
