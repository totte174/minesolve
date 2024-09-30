# minesolve

A highly optimized Minesweeper solver implemented in C designed for effiency and high win rates.
It utilizes combinatorial and probability analysis as well as game tree search to find optimal move for current board.

## Performance of solver
| Search depth | Games played     | Winrate      | Winrate LCB*   | Winrate UCB*   |
|--------------|------------------|--------------|----------------|----------------|
| 1            | $`10\;000\;000`$ | $`39.002\%`$ | $`38.972\%`$   | $`39.032\%`$   |
| 2            | $`10\;000\;000`$ | $`40.272\%`$ | $`40.241\%`$   | $`40.302\%`$   |
| 3            | $`1\;000\;000`$  | $`40.680\%`$ | $`40.584\%`$   | $`40.777\%`$   |
| 4            | $`100\;000`$     | $`40.551\%`$ | $`40.247\%`$   | $`40.856\%`$   |
| 5            | -                | -            | -              | -              |

\*Lower- and upper confidence bounds were calculated using the [__Jeffreys interval__](https://en.wikipedia.org/wiki/Binomial_proportion_confidence_interval#Jeffreys_interval).

## Building minesolve

Run ```make minesolve``` to compile ```./bin/minesolve```.

Requires ```gcc``` (or [w64devkit](https://github.com/skeeto/w64devkit/releases/)) to compile (no external libraries needed).

## Currently working on
- work on CLI to allow analysis of boards (right now it can only simulate full games)
- generalize to N-dimensional minesweeper
- allow boards with wrapping edges

## Possible improvements to stability
- Improvements to equation reduction to possibly split the really hard border which cause the solver to run out of memory
