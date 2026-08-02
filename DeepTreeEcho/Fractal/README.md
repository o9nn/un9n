# Fractal Board — nest the evaluation, not the search

The proposal: nest chess boards fractally (each square contains a board), map it onto a game
world, and run an engine on the result.

That does not merely run slowly. It leaves the domain where search is a coherent verb — a parent
"move" whose value requires resolving a sub-game multiplies the branching factor by the entire
sub-tree. Chess is already ~35 branching over ~80 plies; nesting one level makes a meta-move
depend on a whole subordinate game.

**The escape is to nest the *evaluation* and keep the *search* flat.** Sub-board states become
features feeding the parent's evaluation function, never nodes to expand. Depth lives in the eval,
not the tree. This is the same move NNUE makes for chess: compress "what does this position tell
me" into a function fast enough to call millions of times, rather than searching further.

That claim is testable, so it is tested rather than asserted.

## The game

**Ultimate Tic-Tac-Toe** — nine 3×3 sub-boards in a 3×3 meta-grid. Winning a sub-board claims that
meta-cell; claim three in a row to win. It is the minimal *honest* instance of a fractal board:
the nesting is the game's native mechanic, not a wrapper bolted onto a flat game, and it is small
enough that the intractability claim can be measured instead of hand-waved.

Its defining mechanic — and the one a naive evaluator misses completely:

> **The cell you play in dictates which sub-board your opponent must play in next.**

A move is therefore never just "claim this square", it is also "send them *there*". Sending an
opponent into a board where they already have a live threat is a blunder no board-counting
evaluator can see. Sending them to a *finished* board is worse: it hands them a free move anywhere.

| File | Role |
|---|---|
| `NestedBoard.h` | UTTT rules. The send, the finished-board free move, drawn boards claiming nothing. |
| `NestedEvaluator.h` | The two competing evaluators — flat vs nested. |
| `FlatSearch.h` | Alpha-beta over the meta board, plus a deliberately naive sub-search variant used only to measure the blow-up. |
| `Tests/StandaloneFractalBoard.cpp` | 17 assertions. |

## The experiment

Two evaluators, **identical search** — same algorithm, same depth, same move ordering, same
tiebreak seed stream. The *only* difference is the leaf evaluation:

- **Flat** — sees only the meta-grid: which sub-boards are claimed by whom. Deliberately
  *competent* within that information (meta lines, threats, cell weights), not a strawman. It is
  simply blind to the internals of unfinished sub-boards.
- **Nested** — the same meta evaluation, plus every unfinished sub-board's positional state folded
  in, plus the send.

Each evaluator plays both colours, so a first-player edge (which is real in UTTT) cannot decide it.

## Results

| Test | Result |
|---|---|
| Rules correctness | 8/8 — including the finished-board free move and drawn-board ownership |
| Sub-search blow-up | **14.8×** more nodes for a *single* ply of nesting, at equal outer depth |
| **Nested vs flat, head to head** | **83.1%** for nested (66–13–1 over 80 games) |
| Send mechanic in isolation | flat scores the two sends **identically** (diff 0); nested separates them by 52 |
| Colour fairness | mirror match 9–9–2 |
| Determinism | same seed, same game |

The 14.8× is for *one* ply of nesting at shallow depth. It compounds per level and per outer ply —
which is why the literal fractal-chess version does not run at all, and why the flat-search
restructuring is not a compromise but the thing that makes it exist.

## A bug the test caught

Test [4] failed on the first run: nested evaluation scored **+29** for sending O into O's own
strong board, when it should have been negative.

The send term was written as `Score -= Sign * (Sign * TargetScore)`. Since `Sign² = 1` that
collapses to `Score -= TargetScore` — the exact inverse of correct. The free-move term was
inverted the same way. **The evaluator was actively preferring to send opponents where they were
strongest.**

It still won its match at 70%, because the sub-board nesting carried it *despite* the send term
fighting. Fixing the signs took it to **83.1%** — so the bug had been costing ~13 points while
hiding behind a passing headline number. An inverted heuristic is worse than an absent one, and
only the isolated test in [4] could distinguish them; the head-to-head alone would have shipped it.

## What this does and does not establish

**Established:** literal nested search is measurably intractable; nested evaluation under flat
search is both tractable and *stronger*; the send mechanic is real and flat evaluation is provably
blind to it.

**Not established:** that this scales to nested *chess* specifically (UTTT sub-boards are trivially
evaluable; chess sub-positions are not), or that a hand-tuned nested evaluator beats a *learned*
one. The natural next step is replacing `EvaluateNested` with a small trained network over the same
features — at which point this is NNUE with an explicitly hierarchical feature set, and the
sub-board search from `EvaluateBySubSearch` becomes the *training data generator* rather than the
runtime cost.

## On the rest of the original idea

- **Rendering the evaluation as a field over a world map** is the genuinely novel part and remains
  unbuilt here. An engine's move distribution over terrain is a scalar landscape — visible
  pressure, contested territory, lines of force. That is the engine's mind made visible, and it is
  a rendering problem, not a search one.
- **"Probability amplitudes in superposition"** describes a weighted distribution over a search
  tree. Real amplitudes are complex and can interfere *destructively*; classical search weights
  only ever add. The metaphor maps onto something real (alpha-beta pruning *is* attenuation,
  extensions *are* amplification) — and being classical is exactly what makes it implementable.
- **Stable Diffusion** is the wrong tool for move mechanics (1–5 s/frame against millisecond
  search, and no notion of motion) and the right tool for piece *appearance* — pre-generated per
  archetype, cached, zero runtime cost.
- **Imperfect information.** Chess engines assume perfect information. The moment pieces map to
  agents with hidden intent, Stockfish is the wrong engine and
  `../Mastery/Competitive/CounterAdaptivePolicy.h` is the right one — opponent modelling, safe
  exploitation, retreat to equilibrium when read.
