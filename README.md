<h1 align="center">
minesolve
</h1>
<p align="center">
A highly optimized Minesweeper solver implemented in C designed for efficiency and high win rates.
</p>
<p align="center">
<img src="./assets/gif/simulate.gif">
</p>

## Performance of solver
The solver's performance at various search depths was assessed by playing through millions of randomly generated __expert__ ($`30\times16`$, 99 mines) Minesweeper games. The results are summarized in the table below.

| Search depth | Games played     | Winrate      | Winrate LCB*   | Winrate UCB*   |
|     :-:      |     :-:          |     :-:      |     :-:        |     :-:            |
| 1            | $`10\;000\;000`$ | $`39.002\%`$ | $`38.972\%`$   | $`39.032\%`$   |
| 2            | $`10\;000\;000`$ | $`40.272\%`$ | $`40.241\%`$   | $`40.302\%`$   |
| 3            | $`1\;000\;000`$  | $`40.680\%`$ | $`40.584\%`$   | $`40.777\%`$   |
| 4            | $`1\;000\;000`$  | $`40.801\%`$ | $`40.705\%`$   | $`40.896\%`$   |
| 5            | -                | -            | -              | -              |

\*Lower- and upper confidence bounds were calculated using the [***Jeffreys interval***](https://en.wikipedia.org/wiki/Binomial_proportion_confidence_interval#Jeffreys_interval) using $`\alpha=0.05`$.

Thanks to significant optimizations, the solver can solve boards very quickly at lower search depths. For example, completing a full __expert__ Minesweeper game with a search depth of __1__ takes approximately $`\sim 2.20`$ __ms__. However, as search depth increases, the computation time grows exponentially, taking approximately $`\sim 830`$ __ms__ to complete a game at search depth __4__.

## Building and running **minesolve**
```sh
$ make minesolve
$ ./bin/minesolve --simulate=1000
387
```
Requires ```gcc``` (or [w64devkit](https://github.com/skeeto/w64devkit/releases/) on Windows) to compile (no external libraries needed).

## Running **minesolve** using Docker
```sh
$ docker build -t totte174/minesolve .
$ docker run totte174/minesolve --simulate=1000
391
```

## Currently working on
- work on CLI to allow analysis of boards (right now it can only simulate full games)
- generalize to N-dimensional minesweeper
- allow boards with wrapping edges
