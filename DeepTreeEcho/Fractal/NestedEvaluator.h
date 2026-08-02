// NestedEvaluator.h
// The architectural claim, made falsifiable.
//
// The proposal was to SEARCH the nested tree. That is intractable: a parent move whose value
// requires resolving a sub-game multiplies the branching factor by the sub-game's whole tree.
// The escape is to nest the EVALUATION instead and keep the search flat - sub-board states
// become features feeding the parent's evaluation, never nodes to expand. (This is the same
// move NNUE makes for chess: compress "what does this position tell me" into a fast function
// rather than searching further.)
//
// To make that claim falsifiable rather than plausible, two evaluators are defined here and
// played head to head under IDENTICAL search:
//
//   FLAT   - sees only the meta-grid: which sub-boards are claimed by whom. It is a *competent*
//            evaluator of that information (meta lines, threats, cell weights), not a strawman.
//            It is simply blind to the internals of unfinished sub-boards.
//
//   NESTED - the same meta evaluation, PLUS the positional state of every unfinished sub-board,
//            PLUS the send mechanic.
//
// FAIRNESS. Both use the same search function, same depth, same move ordering, same time. The
// ONLY difference is the leaf evaluation. If nested wins, the difference is attributable to
// nesting rather than to search effort - which is the entire point.
//
// THE SEND. The feature a flat evaluator cannot represent at all: your move dictates which
// sub-board your opponent must play in next. Sending an opponent to a board where they already
// have a strong position is a blunder invisible to any evaluator that only counts claimed
// boards. Sending them to a FINISHED board is worse still - it hands them a free move anywhere.

#pragma once

#include "NestedBoard.h"

namespace Fractal
{

/** Positive scores favour X, negative favour O. Mate scores dominate everything. */
constexpr int SCORE_X_WIN = 100000;
constexpr int SCORE_O_WIN = -100000;

/** Positional weight of each cell in a 3x3 grid - centre is most connected, corners next. */
inline const std::array<int, 9>& CellWeights()
{
    static const std::array<int, 9> W = {{3, 2, 3,
                                          2, 4, 2,
                                          3, 2, 3}};
    return W;
}

/**
 * Line-based score for a generic 3x3 grid of marks.
 * Rewards two-in-a-row with the third cell empty (a live threat) and open lines.
 */
inline int ScoreGrid(const std::array<EMark, 9>& G, int TwoWeight, int OneWeight)
{
    int Score = 0;

    for (int i = 0; i < 9; ++i)
    {
        if (G[i] == EMark::X) Score += CellWeights()[i];
        else if (G[i] == EMark::O) Score -= CellWeights()[i];
    }

    for (const auto& L : WinLines())
    {
        int X = 0, O = 0;
        for (int idx : L)
        {
            if (G[idx] == EMark::X) ++X;
            else if (G[idx] == EMark::O) ++O;
        }
        // Only uncontested lines carry threat value.
        if (X > 0 && O > 0) continue;
        if (X == 2) Score += TwoWeight;
        else if (X == 1) Score += OneWeight;
        else if (O == 2) Score -= TwoWeight;
        else if (O == 1) Score -= OneWeight;
    }
    return Score;
}

/**
 * FLAT evaluator: meta-grid only.
 *
 * Deliberately competent within its information: it understands meta lines, meta threats and
 * cell importance. It just cannot see inside an unfinished sub-board. A strawman here would
 * invalidate the experiment, so this is as strong as it can be given what it looks at.
 */
inline int EvaluateFlat(const FNestedBoard& B)
{
    if (B.Result == EBoardResult::XWins) return SCORE_X_WIN;
    if (B.Result == EBoardResult::OWins) return SCORE_O_WIN;
    if (B.Result == EBoardResult::Drawn) return 0;

    // Meta threats are worth far more than sub-board detail, hence the large weights.
    int Score = ScoreGrid(B.MetaGrid(), 220, 40);

    // TEMPO / FREE-MOVE. Legitimately a flat feature: it needs only the meta-grid (which boards
    // are claimed) and the forced-board index. Being sent to a CLAIMED board grants a free move
    // anywhere, which is a transferable asset.
    //
    // This term exists because a meta-grid-only evaluator is otherwise DEGENERATE, not merely
    // weak: with no sub-board claimed yet it returns exactly 0 for every position, so its whole
    // search tree is zeros and move choice falls to tie-breaks. Measured on this implementation:
    // 1 distinct score across all 81 opening moves, first non-zero at ply ~29.5 of a ~59-ply
    // game. A baseline that plays randomly for half the game is not a control - beating it
    // measures nothing about architecture. This is the strongest evaluator the flat information
    // set actually supports.
    const int SentSign = (B.ToMove == EMark::X) ? 1 : -1;
    const bool bSentToDeadBoard = (B.ForcedBoard >= 0) && (B.Boards[B.ForcedBoard].Owner() != EMark::None);
    if (B.ForcedBoard < 0 || bSentToDeadBoard)
    {
        Score += SentSign * 55;
    }

    // Side to move is worth a little; also flat.
    Score += SentSign * 8;

    return Score;
}

/**
 * NESTED evaluator: meta-grid + every unfinished sub-board + the send.
 */
inline int EvaluateNested(const FNestedBoard& B)
{
    if (B.Result == EBoardResult::XWins) return SCORE_X_WIN;
    if (B.Result == EBoardResult::OWins) return SCORE_O_WIN;
    if (B.Result == EBoardResult::Drawn) return 0;

    // 1. The same meta evaluation the flat version uses.
    int Score = ScoreGrid(B.MetaGrid(), 220, 40);

    // 2. NESTING: fold each unfinished sub-board's internal position into the parent score,
    //    weighted by how much that meta-cell matters. This is the "evaluate, do not search"
    //    step - the sub-board's whole game tree is compressed into one number.
    for (int b = 0; b < 9; ++b)
    {
        const FSubBoard& Sub = B.Boards[b];
        if (Sub.IsFinished()) continue;

        const int SubScore = ScoreGrid(Sub.Cells, 12, 2);
        // A sub-board's value scales with the strategic value of the meta-cell it would claim.
        Score += (SubScore * CellWeights()[b]) / 4;
    }

    // 3. THE SEND: value the position the side to move has just been placed IN. Invisible to flat.
    //
    // Sign convention is easy to invert here, and inverting it is worse than omitting the term
    // entirely - it makes the evaluator actively prefer to send opponents where they are
    // strongest. (An earlier version did exactly that: it computed `-= Sign * (Sign * X)`, which
    // collapses to `-= X` since Sign^2 == 1, flipping the whole term. It still won its match,
    // because the sub-board nesting below carried it *despite* the send term fighting.)
    //
    // Stated plainly: scores are X-positive, and being sent somewhere favourable to YOU is good
    // for YOU. ScoreGrid already encodes X-favourability, so a live target contributes with its
    // natural sign regardless of who is being sent.
    const int Target = B.ForcedBoard;
    const int SentSign = (B.ToMove == EMark::X) ? 1 : -1;   // +1 when X is the one being sent

    if (Target < 0 || B.Boards[Target].IsFinished())
    {
        // Sent to a dead board => free choice anywhere. A real gift to whoever receives it.
        Score += SentSign * 55;
    }
    else
    {
        // Sent to a live board: its X-favourability IS the advantage transferred to the sent side.
        const int TargetScore = ScoreGrid(B.Boards[Target].Cells, 12, 2);
        Score += (TargetScore * 3) / 2;
    }

    return Score;
}

/** Which evaluation the search should use. */
enum class EEvalMode { Flat, Nested };

inline int Evaluate(const FNestedBoard& B, EEvalMode Mode)
{
    return (Mode == EEvalMode::Nested) ? EvaluateNested(B) : EvaluateFlat(B);
}

} // namespace Fractal
