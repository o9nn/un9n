// FlatSearch.h
// Alpha-beta over the META board only. The search never descends into a sub-board as a
// sub-GAME - sub-boards reach the search purely through the leaf evaluation.
//
// That distinction is the whole architecture:
//
//   SEARCHING the nesting  -> branching factor multiplies by the sub-tree; intractable.
//   EVALUATING the nesting -> branching factor unchanged; depth lives in the eval function.
//
// Node counts are recorded so the intractability claim can be measured rather than asserted -
// see Tests/StandaloneFractalBoard.cpp, which compares this against a deliberately naive
// "resolve each sub-board by search" variant and reports the blow-up.

#pragma once

#include <algorithm>
#include <cstdint>
#include <random>
#include "NestedBoard.h"
#include "NestedEvaluator.h"

namespace Fractal
{

struct FSearchStats
{
    uint64_t Nodes = 0;
    uint64_t LeafEvals = 0;
    int MaxDepthReached = 0;
};

struct FSearchConfig
{
    int Depth = 6;
    EEvalMode Mode = EEvalMode::Nested;
    /** Small random tiebreak so equal-scoring games do not become identical every time. */
    uint32_t Seed = 0;
};

namespace Detail
{
    inline int AlphaBeta(FNestedBoard& B, int Depth, int Alpha, int Beta,
                         EEvalMode Mode, FSearchStats& Stats)
    {
        ++Stats.Nodes;

        if (B.IsOver() || Depth <= 0)
        {
            ++Stats.LeafEvals;
            return Evaluate(B, Mode);
        }

        std::vector<FMove> Moves;
        B.LegalMoves(Moves);
        if (Moves.empty())
        {
            ++Stats.LeafEvals;
            return Evaluate(B, Mode);
        }

        const bool bMaximizing = (B.ToMove == EMark::X);
        int Best = bMaximizing ? (SCORE_O_WIN - 1) : (SCORE_X_WIN + 1);

        for (const FMove& M : Moves)
        {
            FNestedBoard Child = B;
            Child.Play(M);
            const int Val = AlphaBeta(Child, Depth - 1, Alpha, Beta, Mode, Stats);

            if (bMaximizing)
            {
                Best = std::max(Best, Val);
                Alpha = std::max(Alpha, Best);
            }
            else
            {
                Best = std::min(Best, Val);
                Beta = std::min(Beta, Best);
            }
            if (Beta <= Alpha) break;   // prune
        }
        return Best;
    }
}

/** Choose a move for the side to move. */
inline FMove SearchBestMove(const FNestedBoard& Board, const FSearchConfig& Cfg, FSearchStats& Stats)
{
    std::vector<FMove> Moves;
    Board.LegalMoves(Moves);
    if (Moves.empty()) return FMove{};

    std::mt19937 Rng(Cfg.Seed);
    std::shuffle(Moves.begin(), Moves.end(), Rng);   // tiebreak variety, identical for both modes

    const bool bMaximizing = (Board.ToMove == EMark::X);
    FMove Best = Moves[0];
    int BestScore = bMaximizing ? (SCORE_O_WIN - 1) : (SCORE_X_WIN + 1);

    int Alpha = SCORE_O_WIN - 1;
    int Beta  = SCORE_X_WIN + 1;

    for (const FMove& M : Moves)
    {
        FNestedBoard Child = Board;
        Child.Play(M);
        const int Val = Detail::AlphaBeta(Child, Cfg.Depth - 1, Alpha, Beta, Cfg.Mode, Stats);

        if (bMaximizing)
        {
            if (Val > BestScore) { BestScore = Val; Best = M; }
            Alpha = std::max(Alpha, BestScore);
        }
        else
        {
            if (Val < BestScore) { BestScore = Val; Best = M; }
            Beta = std::min(Beta, BestScore);
        }
    }
    return Best;
}

/**
 * The NAIVE alternative, implemented only so the intractability claim can be measured.
 *
 * This is what "run the engine on the nested board" means if taken literally: to evaluate a
 * position, actually SEARCH each unfinished sub-board to resolve what it is worth, then combine.
 * It is not used to play - it exists to have its node count compared against the flat search.
 *
 * SubDepth is the depth of the inner search per sub-board. Note the cost is multiplicative:
 * every leaf of the outer search spawns up to 9 inner searches, each of which explores its own
 * tree. This is the shape that makes literal fractal search die.
 */
inline int EvaluateBySubSearch(const FNestedBoard& B, int SubDepth, FSearchStats& Stats)
{
    ++Stats.Nodes;

    if (B.IsOver()) { return Evaluate(B, EEvalMode::Flat); }

    int Score = ScoreGrid(B.MetaGrid(), 220, 40);

    for (int b = 0; b < 9; ++b)
    {
        const FSubBoard& Sub = B.Boards[b];
        if (Sub.IsFinished()) continue;

        // Resolve this sub-board by SEARCHING it as its own game.
        FNestedBoard Isolated;
        Isolated.Boards[b] = Sub;
        Isolated.ToMove = B.ToMove;
        Isolated.ForcedBoard = int8_t(b);

        Score += Detail::AlphaBeta(Isolated, SubDepth, SCORE_O_WIN - 1, SCORE_X_WIN + 1,
                                   EEvalMode::Flat, Stats) / 64;
    }
    return Score;
}

} // namespace Fractal
