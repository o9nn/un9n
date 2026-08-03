# Fractal Board — nest the evaluation, not the search

The proposal: nest chess boards fractally (each square contains a board), map it onto a game
world, and run an engine on the result.

That does not merely run slowly — but **not for the reason I first gave.** I originally claimed the
branching factor becomes ~35^64. That is wrong twice over: it describes a *different* game (one
where a meta-move plays on all 64 sub-boards at once), and 35^64 ≈ 10^99 is *smaller* than plain
chess's ~10^120, so as a standalone figure it argues nested chess is **easier** than chess.

The correct shape: if resolving a node at level *k* requires solving a game at level *k−1*, the
branching factor stays ~35 and the blow-up is a **cost-per-node tower**, not a wider tree. Work ≈
`nodes(1)^d` — roughly 10^240 at depth 2 unpruned, 10^120 with ideal alpha-beta. And at depth ≥ 3
you lose to *state representation* before search even begins: 64^d squares means depth 3 is 262,144
squares and depth 6 is ~7×10^10. You cannot instantiate the position, never mind search it.

(Related correction: "effective branching factor" has a specific meaning in engine literature —
`nodes[d]/nodes[d−1]`, which is **under 2** for a modern engine, not 35. The ~35 figure is average
legal moves. Applying √b on top of an already-effective figure double-counts the pruning.)

**The proposed escape is to nest the *evaluation* and keep the *search* flat.** Sub-board states
become features feeding the parent's evaluation function, never nodes to expand — depth lives in
the eval, not the tree.

Two caveats, both learned the hard way and both detailed below. First, this is sound **only when
subgames are independent**; where a mechanic couples them, per-subgame evaluation is lossy.
Second, I originally credited this to NNUE — that was wrong. NNUE is about *incremental update
cost*, and it made Stockfish evaluate **better**, not search less. It is not an instance of
hierarchical nesting.

## The game

**Ultimate Tic-Tac-Toe** — nine 3×3 sub-boards in a 3×3 meta-grid. Winning a sub-board claims that
meta-cell; claim three in a row to win.

**Important correction: UTTT is *not* an instance of the nested-search construction above,** and I
originally described it as one. Nobody ever resolves a sub-board to make a meta-move — UTTT is a
single *flat* game on 81 cells with a coupling constraint. Its branching factor is ~6–7, which is
**lower** than plain tic-tac-toe's opening 9, because the send-constraint usually restricts you to
one 9-cell board. Nesting the board there *reduces* branching. It is evidence against the intuition
that nesting explodes *b*, not for it.

What UTTT is good for is a *different*, narrower question: does folding sub-board state into
evaluation beat ignoring it, under identical search? That question it answers cleanly. It does not
speak to the tractability of nested search, which is argued above on arithmetic rather than
demonstrated here.

(UTTT is also **solved** under the ENS-2020 rule variant — first player forces a win in ≤43 moves —
though not under the standard competitive variant.)

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
| **Nested vs flat, head to head** | **78.1%** for nested (61–16–3 over 80 games) |
| Send mechanic in isolation | flat scores the two sends **identically** (diff 0); nested separates them by 52 |
| Colour fairness | mirror match 9–9–2 |
| Determinism | same seed, same game |

The 14.8× is for *one* ply of nesting at shallow depth in UTTT, where sub-boards are 9 cells. It
is an illustration of the shape, not a measurement of the chess case — the argument that literal
fractal chess cannot run rests on the arithmetic at the top of this file, not on this number.

## The baseline was rigged, and I only found out by being checked

An independent review flagged that the flat evaluator was not weak but **degenerate**, and probing
my own code confirmed it exactly:

- flat evaluation produced **1 distinct score across all 81 opening moves** (range `[0,0]`)
- first non-zero score at **ply 29.5** of a ~59-ply game

A meta-grid-only evaluator returns 0 until some sub-board is actually *claimed*, so its entire
search tree was zeros and move choice fell through to the random tie-break. The flat engine played
**randomly for half the game.** The original 83.1% therefore measured "an evaluator that plays
randomly for 30 plies loses to one that doesn't" — near-tautological, and nothing about
architecture.

Fixed by giving flat every feature its information set legitimately supports — the tempo/free-move
term (computable from the meta-grid plus the forced-board index alone) and side-to-move. Flat now
has signal from ply 1, and the honest number is **78.1%**.

It is still 1 distinct score across the 81 *opening* moves — but that is now
information-theoretic rather than lazy: from an empty symmetric board there genuinely is no
meta-level information distinguishing them.

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

**Established, narrowly:** in UTTT, under identical search, folding sub-board state into the
evaluation beats ignoring it (78.1% over 80 games, each engine playing both colours). The send is a
real feature that a meta-only evaluator is provably blind to — it scores two very different sends
identically. Resolving sub-boards by search costs measurably more than evaluating them (14.8× at
one ply, in this game).

**Not established — and this list grew after review:**

- **That UTTT tests the nested-search claim at all.** It doesn't (see above). The intractability
  argument stands on arithmetic; this code does not demonstrate it.
- **That "nest the evaluation" generalises.** The escape route is sound when subgames are
  *independent*. UTTT's subgames are **coupled** by the send mechanic, so per-sub-board evaluation
  is provably lossy there. The reviewer's specific counter-claim — that evaluation-free MCTS over
  the joint state outperforms hand-crafted-eval minimax at UTTT — is **untested here** and would be
  the decisive experiment. I am recording it rather than quietly omitting it.
- **That this scales to nested chess.** UTTT sub-boards are trivially evaluable; chess
  sub-positions are not.
- **That the experiment is tournament-grade.** A rigorous version needs three arms (meta-only /
  unweighted local sum / meta-relevance-weighted local), fixed *node* budgets rather than fixed
  depth, paired colour-swapped randomised openings, ~2000 games or SPRT, and a fixed external
  reference opponent. 80 games at fixed depth is a smoke test, not an evaluation.

**Also corrected:** my NNUE analogy was wrong. NNUE is about *incremental update cost* — it made
Stockfish evaluate **better**, not search less — and it is not an instance of hierarchical nesting.

A further structural point I had missed, worth recording: **nesting breaks transposition tables.**
TT reuse requires a subgame's value to be context-independent, but in the nested construction the
parent position determines what winning the subgame is *worth*, so parent context must enter the
hash key and reuse collapses. That is the same independence condition that governs whether the
evaluation-nesting escape is sound at all.

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
