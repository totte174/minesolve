# minesolve

A highly optimized Minesweeper solver implemented in C designed for efficiency and high win rates.
It utilizes combinatorial and probability analysis as well as game tree search to find optimal move for current board.

## Performance of solver
The solver's performance at various search depths was assessed by playing through millions of randomly generated __expert__ ($`30\times16`$, 99 mines) Minesweeper games. The results are summarized in the table below.

| Search depth | Games played     | Winrate      | Winrate LCB*   | Winrate UCB*   |
|--------------|------------------|--------------|----------------|----------------|
| 1            | $`10\;000\;000`$ | $`39.002\%`$ | $`38.972\%`$   | $`39.032\%`$   |
| 2            | $`10\;000\;000`$ | $`40.272\%`$ | $`40.241\%`$   | $`40.302\%`$   |
| 3            | $`1\;000\;000`$  | $`40.680\%`$ | $`40.584\%`$   | $`40.777\%`$   |
| 4            | $`100\;000`$     | $`40.551\%`$ | $`40.247\%`$   | $`40.856\%`$   |
| 5            | -                | -            | -              | -              |

\*Lower- and upper confidence bounds were calculated using the [***Jeffreys interval***](https://en.wikipedia.org/wiki/Binomial_proportion_confidence_interval#Jeffreys_interval) with $`\alpha=0.05`$.

Thanks to significant optimizations, the solver can solve boards very quickly at lower search depths. For example, completing a full __expert__ Minesweeper game with a search depth of __1__ takes an average of $`2.23`$ __ms__. However, as search depth increases, the computation time grows exponentially, with an average of $`836`$ __ms__ required to complete a game at search depth __4__.

## Building minesolve

Run ```make minesolve``` to compile ```./bin/minesolve```.

Requires ```gcc``` (or [w64devkit](https://github.com/skeeto/w64devkit/releases/)) to compile (no external libraries needed).

## Currently working on
- work on CLI to allow analysis of boards (right now it can only simulate full games)
- generalize to N-dimensional minesweeper
- allow boards with wrapping edges
