<h1 align="center">
minesolve
</h1>
<p align="center">
A highly optimized Minesweeper solver implemented in C, designed for efficiency and high win rates.
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

This produces three build artifacts:

| Artifact | Description |
|---|---|
| `build/minesolve` | CLI binary |
| `build/libminesolve.a` | Static library |
| `build/libminesolve.so` | Shared library |

### Makefile shortcuts

```sh
make          # configure + build (Release)
make debug    # configure + build (Debug)
make lib      # build library targets only
make install  # install to system (CMAKE_INSTALL_PREFIX)
make clean    # remove build directory
```

### Optional: profiling build

```sh
cmake -B build -DMINESOLVE_ENABLE_PROFILING=ON
make -C build
```

## CLI usage

```
Usage: minesolve [OPTION...] [BOARD]
```

### Board configuration

| Option | Description |
|---|---|
| `-c beginner\|intermediate\|expert` | Use a preset (9×9/10, 16×16/40, 30×16/99) |
| `-w N`, `-h N`, `-m N` | Width, height, mine count |
| `-W` | Enable wrapping borders |

### Input

A board can be supplied via:

```sh
minesolve -f board.txt          # file
minesolve < board.txt           # stdin
minesolve "2...21112222..."     # inline argument
```

**Board format:** digits `0`–`8` for revealed squares, space for revealed 0, `.`/`x`/`?` for unknown, newlines as row separators.

### Actions

| Option | Description |
|---|---|
| *(default)* | Print `(x, y)` of the best square to reveal |
| `-p` | Print per-square mine probability grid |
| `-S` | Display the board |
| `-s N` | Simulate N games and print the win count |
| `-d N` | Search depth 1–8 (default: 1) |
| `-a` | Plain ASCII output (no ANSI / Unicode) |

### Examples

```sh
# Solve a board from a file at depth 2
minesolve --config=expert --depth=2 -f board.txt

# Show mine probabilities
minesolve --config=expert --probability < board.txt

# Run 1000 simulated expert games
minesolve --config=expert --simulate=1000

# Watch the solver play in the terminal
minesolve --config=expert --simulate=10 --show-board
```

## Docker

```sh
docker build -t minesolve .
docker run minesolve --simulate=1000
```
