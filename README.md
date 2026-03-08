<h1 align="center">
minesolve
</h1>
<p align="center">
A highly optimized Minesweeper solver implemented in C, designed for efficiency and high win rates.
<br>
Try out the solver <a href="https://minesweeper.totte.net/">here!</a>
</p>
<p align="center">
<img src="./assets/gif/simulate.gif">
</p>

## Performance

Win rates were measured over millions of randomly generated **expert** ($`30\times16`$, 99 mines) games.

| Search depth | Games played     | Win rate     | Win rate LCB\* | Win rate UCB\* |
|     :-:      |     :-:          |     :-:      |     :-:        |     :-:        |
| 1            | $`10\;000\;000`$ | $`39.002\%`$ | $`38.972\%`$   | $`39.032\%`$   |
| 2            | $`10\;000\;000`$ | $`40.272\%`$ | $`40.241\%`$   | $`40.302\%`$   |
| 3            | $`1\;000\;000`$  | $`40.680\%`$ | $`40.584\%`$   | $`40.777\%`$   |
| 4            | $`1\;000\;000`$  | $`40.801\%`$ | $`40.705\%`$   | $`40.896\%`$   |

\*Confidence bounds use the [Jeffreys interval](https://en.wikipedia.org/wiki/Binomial_proportion_confidence_interval#Jeffreys_interval) with $`\alpha=0.05`$.

Computation time scales exponentially with depth — a full expert game takes ~2.2 ms at depth 1 and ~830 ms at depth 4.

## Building

Requires **GCC** or **Clang** and **CMake ≥ 3.15**.

```sh
cmake -B build
make -C build
```

Produces `build/minesolve` (CLI), `build/libminesolve.a`, and `build/libminesolve.so`.

## CLI usage

```
minesolve [OPTION...] [BOARD]
```

| Option | Description |
|---|---|
| `-c`, `--config=beginner\|intermediate\|expert` | Use a preset (9×9/10, 16×16/40, 30×16/99) |
| `-w`, `--width=N` / `-h`, `--height=N` / `-m`, `--mines=N` | Custom board dimensions |
| `-W`, `--wrap` | Enable wrapping borders |
| `-f`, `--file=FILE` | Read board from file |
| `-d`, `--depth=N` | Search depth 1–8 (default: 1) |
| `-p`, `--probability` | Print per-square mine probability grid |
| `-s`, `--simulate=N` | Simulate N games and print win count |
| `-S`, `--show-board` | Display the board |
| `-a`, `--ascii` | Plain ASCII output (no ANSI / Unicode) |

**Board format:** digits `0`–`8` for revealed squares, `.`/`x`/`?` for unknown, newlines as row separators. Supply via `-f`, stdin, or inline argument.

```sh
minesolve --config=expert --depth=2 --file=board.txt   # solve for best move at depth 2
minesolve --config=expert --probability < board.txt    # show bomb probabilities
minesolve --config=expert --simulate=1000              # simulate 1000 games and print number of wins
minesolve --config=expert --simulate=100 --show-board  # watch the solver play random games
```

## Docker

```sh
docker build -t minesolve .
docker run minesolve --simulate=1000
```
