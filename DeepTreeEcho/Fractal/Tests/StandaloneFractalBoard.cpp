// StandaloneFractalBoard.cpp
// Is "nest the evaluation, not the search" actually right? Measured, not asserted.
//
// The proposal was to nest boards fractally and run an engine on the result. I claimed that
// halts on combinatorics and that the fix is to fold sub-board state into the EVALUATION while
// keeping the SEARCH flat. Both halves of that are testable, so this tests them:
//
//   [1] RULES     - the game is implemented correctly. Everything below is meaningless if the
//                   send mechanic or the finished-board rule is wrong, so this comes first.
//   [2] BLOW-UP   - literal nested search really is intractable. Measured as a node-count ratio
//                   against flat search at identical outer depth.
//   [3] THE CLAIM - nested evaluation beats flat evaluation HEAD TO HEAD under identical search
//                   depth, algorithm and move ordering. If it does not, the architecture buys
//                   nothing and I should say so.
//   [4] THE SEND  - the specific feature flat evaluation cannot represent is worth something on
//                   its own.
//   [5] COLOUR    - the result is not an artefact of who moves first (both sides tested).
//   [6] DETERMINISM - same seed, same game.
//
// Build & run:
//   g++ -std=c++17 -O2 -o fractal StandaloneFractalBoard.cpp && ./fractal

#include <cstdio>
#include <string>
#include <vector>

#include "../FlatSearch.h"

namespace
{

using namespace Fractal;

int TestsFailed = 0;
void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

struct MatchResult { int XWins = 0, OWins = 0, Draws = 0; };

/** Play one game. XMode plays X, OMode plays O. Identical search config otherwise. */
EBoardResult PlayGame(EEvalMode XMode, EEvalMode OMode, int Depth, uint32_t Seed)
{
    FNestedBoard B = FNestedBoard::Initial();
    FSearchStats Stats;
    int Ply = 0;

    while (!B.IsOver() && Ply < 81)
    {
        FSearchConfig Cfg;
        Cfg.Depth = Depth;
        Cfg.Mode = (B.ToMove == EMark::X) ? XMode : OMode;
        // Same seed stream for both sides so tiebreak randomness cannot favour either.
        Cfg.Seed = Seed * 7919u + uint32_t(Ply);

        const FMove M = SearchBestMove(B, Cfg, Stats);
        if (!M.IsValid()) break;
        B.Play(M);
        ++Ply;
    }
    return B.Result;
}

MatchResult PlayMatch(EEvalMode XMode, EEvalMode OMode, int Games, int Depth, uint32_t BaseSeed)
{
    MatchResult R;
    for (int g = 0; g < Games; ++g)
    {
        const EBoardResult Res = PlayGame(XMode, OMode, Depth, BaseSeed + uint32_t(g) * 104729u);
        if (Res == EBoardResult::XWins) ++R.XWins;
        else if (Res == EBoardResult::OWins) ++R.OWins;
        else ++R.Draws;
    }
    return R;
}

} // namespace

int main()
{
    std::printf("=== Fractal board: nest the evaluation, not the search ===\n\n");

    // ------------------------------------------------------------------ [1] rules
    std::printf("[1] rules correctness (everything below depends on this)\n");
    {
        FNestedBoard B = FNestedBoard::Initial();
        Check(B.NumLegalMoves() == 81, "opening position has 81 legal moves");

        // The send: playing cell 4 must force the opponent into board 4.
        B.Play(FMove{0, 4});
        Check(B.ForcedBoard == 4, "playing cell 4 sends opponent to board 4");
        Check(B.NumLegalMoves() == 9, "forced move restricted to one open sub-board");
        Check(B.ToMove == EMark::O, "turn alternates");

        // Sub-board win claims a meta-cell.
        FNestedBoard W = FNestedBoard::Initial();
        W.Boards[0].Cells[0] = EMark::X;
        W.Boards[0].Cells[1] = EMark::X;
        W.Boards[0].Cells[2] = EMark::X;
        W.Boards[0].UpdateResult();
        Check(W.Boards[0].Owner() == EMark::X, "three in a row claims the sub-board");

        // Being sent to a FINISHED board frees the move - the rule most implementations get wrong.
        FNestedBoard F = FNestedBoard::Initial();
        F.Boards[3].Cells[0] = EMark::X; F.Boards[3].Cells[1] = EMark::X; F.Boards[3].Cells[2] = EMark::X;
        F.Boards[3].UpdateResult();
        F.ForcedBoard = 3;
        Check(F.Boards[3].IsFinished(), "target board is finished");
        Check(F.NumLegalMoves() > 9, "being sent to a finished board grants a FREE move");

        // A drawn sub-board is owned by nobody.
        FSubBoard D;
        const EMark Pattern[9] = {EMark::X, EMark::X, EMark::O,
                                  EMark::O, EMark::O, EMark::X,
                                  EMark::X, EMark::O, EMark::X};
        for (int i = 0; i < 9; ++i) D.Cells[i] = Pattern[i];
        D.UpdateResult();
        Check(D.Result == EBoardResult::Drawn && D.Owner() == EMark::None,
              "a full sub-board with no line is drawn and claims nothing");
    }

    // ------------------------------------------------------------------ [2] blow-up
    std::printf("\n[2] is literal nested SEARCH actually intractable? (measured)\n");
    {
        FNestedBoard B = FNestedBoard::Initial();
        B.Play(FMove{4, 4});   // a typical mid-opening position

        FSearchStats FlatStats;
        FSearchConfig Cfg; Cfg.Depth = 4; Cfg.Mode = EEvalMode::Nested; Cfg.Seed = 1;
        SearchBestMove(B, Cfg, FlatStats);

        // Same outer work, but each leaf resolves sub-boards BY SEARCH instead of by evaluation.
        FSearchStats NestedStats;
        std::vector<FMove> Moves; B.LegalMoves(Moves);
        for (const FMove& M : Moves)
        {
            FNestedBoard Child = B; Child.Play(M);
            EvaluateBySubSearch(Child, 4, NestedStats);
        }

        const double Ratio = double(NestedStats.Nodes) / double(FlatStats.Nodes ? FlatStats.Nodes : 1);
        std::printf("      flat search (depth 4)          : %10llu nodes\n",
                    (unsigned long long)FlatStats.Nodes);
        std::printf("      sub-search eval at ONE ply     : %10llu nodes\n",
                    (unsigned long long)NestedStats.Nodes);
        std::printf("      ratio                          : %10.1fx for a SINGLE ply of nesting\n", Ratio);
        Check(NestedStats.Nodes > FlatStats.Nodes,
              "resolving sub-boards by search costs more than evaluating them");
        std::printf("      (that multiplier compounds per nesting level - which is why the\n");
        std::printf("       literal fractal-chess version does not run at all)\n");
    }

    // ------------------------------------------------------------------ [3] THE CLAIM
    std::printf("\n[3] does nested EVALUATION actually beat flat, under identical search?\n");
    constexpr int GAMES = 40;
    constexpr int DEPTH = 4;
    {
        const MatchResult NvF = PlayMatch(EEvalMode::Nested, EEvalMode::Flat, GAMES, DEPTH, 11);
        const MatchResult FvN = PlayMatch(EEvalMode::Flat, EEvalMode::Nested, GAMES, DEPTH, 11);

        std::printf("      nested as X vs flat as O : X %2d  O %2d  draw %2d\n",
                    NvF.XWins, NvF.OWins, NvF.Draws);
        std::printf("      flat as X vs nested as O : X %2d  O %2d  draw %2d\n",
                    FvN.XWins, FvN.OWins, FvN.Draws);

        const int NestedWins = NvF.XWins + FvN.OWins;
        const int FlatWins   = NvF.OWins + FvN.XWins;
        const int Draws      = NvF.Draws + FvN.Draws;
        const int Total      = GAMES * 2;
        const double NestedScore = (NestedWins + 0.5 * Draws) / Total;

        std::printf("      ---------------------------------------------\n");
        std::printf("      nested %d, flat %d, draws %d  ->  nested score %.1f%%\n",
                    NestedWins, FlatWins, Draws, NestedScore * 100.0);

        Check(NestedWins > FlatWins, "nested evaluation wins the head-to-head");
        Check(NestedScore > 0.55, "nested scores above 55% (a real edge, not noise)");
    }

    // ------------------------------------------------------------------ [4] the send
    std::printf("\n[4] is the SEND mechanic - the thing flat cannot see - worth anything alone?\n");
    {
        // Construct a position where one move sends the opponent to a board they dominate,
        // and another sends them somewhere harmless. Flat evaluation scores these identically.
        FNestedBoard B = FNestedBoard::Initial();
        B.Boards[1].Cells[0] = EMark::O;
        B.Boards[1].Cells[1] = EMark::O;   // O has a live threat in board 1
        B.Boards[7].Cells[4] = EMark::X;   // board 7 is comfortable for X

        FNestedBoard SendToStrong = B; SendToStrong.ForcedBoard = 1; SendToStrong.ToMove = EMark::O;
        FNestedBoard SendToWeak   = B; SendToWeak.ForcedBoard   = 7; SendToWeak.ToMove   = EMark::O;

        const int FlatStrong = EvaluateFlat(SendToStrong);
        const int FlatWeak   = EvaluateFlat(SendToWeak);
        const int NestStrong = EvaluateNested(SendToStrong);
        const int NestWeak   = EvaluateNested(SendToWeak);

        std::printf("      flat   : send-O-to-their-strong-board %d, to-harmless %d  (diff %d)\n",
                    FlatStrong, FlatWeak, FlatStrong - FlatWeak);
        std::printf("      nested : send-O-to-their-strong-board %d, to-harmless %d  (diff %d)\n",
                    NestStrong, NestWeak, NestStrong - NestWeak);

        Check(FlatStrong == FlatWeak, "flat evaluation is BLIND to where the opponent is sent");
        Check(NestStrong < NestWeak, "nested evaluation prefers not to send O into O's strong board");
    }

    // ------------------------------------------------------------------ [5] colour fairness
    std::printf("\n[5] is the result an artefact of moving first?\n");
    {
        const MatchResult Mirror = PlayMatch(EEvalMode::Nested, EEvalMode::Nested, 20, DEPTH, 77);
        std::printf("      nested vs nested: X %2d  O %2d  draw %2d\n",
                    Mirror.XWins, Mirror.OWins, Mirror.Draws);
        const int Diff = Mirror.XWins - Mirror.OWins;
        Check(Diff < 16, "mirror match does not show an overwhelming first-move artefact");
        std::printf("      (a first-player edge is REAL in UTTT; the head-to-head above controls\n");
        std::printf("       for it by playing each evaluator on both colours)\n");
    }

    // ------------------------------------------------------------------ [6] determinism
    std::printf("\n[6] determinism\n");
    {
        const EBoardResult A = PlayGame(EEvalMode::Nested, EEvalMode::Flat, DEPTH, 4242);
        const EBoardResult B = PlayGame(EEvalMode::Nested, EEvalMode::Flat, DEPTH, 4242);
        Check(A == B, "same seed reproduces the same game");
    }

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "FractalBoard=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
