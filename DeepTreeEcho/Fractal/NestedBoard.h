// NestedBoard.h
// Ultimate Tic-Tac-Toe - the minimal honest instance of a "fractal board".
//
// WHY THIS GAME. The proposal was to nest chess boards inside chess squares and search the
// result. That does not merely run slowly, it leaves the domain where search is a coherent
// verb: a parent "move" whose value requires resolving a sub-game multiplies the branching
// factor by the sub-game's entire tree.
//
// UTTT is the same structure at a size where the claim can actually be TESTED rather than
// asserted: 9 sub-boards in a 3x3 meta-grid, winning a sub-board claims that meta-cell, win
// the meta-board to win. Critically the nesting is NATIVE - it is the game's actual mechanic,
// not a wrapper bolted onto a flat game.
//
// The mechanic that gives UTTT its depth, and that a naive engine misses entirely: THE CELL YOU
// PLAY IN DICTATES WHICH SUB-BOARD YOUR OPPONENT MUST PLAY IN NEXT. A move is therefore never
// just "claim this square" - it is also "send my opponent there". Sending them to a board where
// they are already winning is a blunder no material-counting evaluator can see.
//
// Plain C++17, no engine types, deterministic.

#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace Fractal
{

enum class EMark : uint8_t { None = 0, X = 1, O = 2 };
enum class EBoardResult : uint8_t { Open = 0, XWins = 1, OWins = 2, Drawn = 3 };

inline EMark Other(EMark M) { return M == EMark::X ? EMark::O : EMark::X; }

/** A move: which sub-board (0-8), and which cell within it (0-8). */
struct FMove
{
    int8_t Board = -1;
    int8_t Cell  = -1;

    bool IsValid() const { return Board >= 0 && Board < 9 && Cell >= 0 && Cell < 9; }
    bool operator==(const FMove& O) const { return Board == O.Board && Cell == O.Cell; }
};

/** The eight winning lines of a 3x3 grid. Shared by sub-boards and the meta-board. */
inline const std::array<std::array<int, 3>, 8>& WinLines()
{
    static const std::array<std::array<int, 3>, 8> Lines = {{
        {{0,1,2}}, {{3,4,5}}, {{6,7,8}},   // rows
        {{0,3,6}}, {{1,4,7}}, {{2,5,8}},   // columns
        {{0,4,8}}, {{2,4,6}}               // diagonals
    }};
    return Lines;
}

/** One 3x3 sub-board. */
struct FSubBoard
{
    std::array<EMark, 9> Cells{};
    EBoardResult Result = EBoardResult::Open;

    FSubBoard() { Cells.fill(EMark::None); }

    bool IsFinished() const { return Result != EBoardResult::Open; }

    bool IsFull() const
    {
        for (EMark C : Cells) { if (C == EMark::None) return false; }
        return true;
    }

    /** Recompute Result after a placement. */
    void UpdateResult()
    {
        for (const auto& L : WinLines())
        {
            const EMark A = Cells[L[0]];
            if (A != EMark::None && Cells[L[1]] == A && Cells[L[2]] == A)
            {
                Result = (A == EMark::X) ? EBoardResult::XWins : EBoardResult::OWins;
                return;
            }
        }
        // A full board with no line is drawn; it claims no meta-cell for either side.
        if (IsFull()) { Result = EBoardResult::Drawn; }
    }

    /** Owner of this board for meta purposes. Drawn boards are owned by nobody. */
    EMark Owner() const
    {
        if (Result == EBoardResult::XWins) return EMark::X;
        if (Result == EBoardResult::OWins) return EMark::O;
        return EMark::None;
    }
};

/** Full nested game state. */
struct FNestedBoard
{
    std::array<FSubBoard, 9> Boards{};
    EMark ToMove = EMark::X;

    /** Sub-board the mover is constrained to, or -1 for a free move. */
    int8_t ForcedBoard = -1;

    EBoardResult Result = EBoardResult::Open;

    bool IsOver() const { return Result != EBoardResult::Open; }

    /** Meta-grid of sub-board owners - what a FLAT evaluator sees. */
    std::array<EMark, 9> MetaGrid() const
    {
        std::array<EMark, 9> G{};
        for (int i = 0; i < 9; ++i) { G[i] = Boards[i].Owner(); }
        return G;
    }

    /**
     * Legal moves.
     *
     * The rule that trips up most implementations: the mover is constrained to ForcedBoard ONLY
     * if that board is still open. If the target board is already finished (won or drawn), the
     * constraint evaporates and the move is free. Getting this wrong yields an engine that
     * silently plays a different game.
     */
    void LegalMoves(std::vector<FMove>& Out) const
    {
        Out.clear();
        if (IsOver()) return;

        const bool bFree = (ForcedBoard < 0) || Boards[ForcedBoard].IsFinished();

        for (int b = 0; b < 9; ++b)
        {
            if (!bFree && b != ForcedBoard) continue;
            if (Boards[b].IsFinished()) continue;
            for (int c = 0; c < 9; ++c)
            {
                if (Boards[b].Cells[c] == EMark::None) { Out.push_back(FMove{int8_t(b), int8_t(c)}); }
            }
        }
    }

    int NumLegalMoves() const
    {
        std::vector<FMove> M; LegalMoves(M); return static_cast<int>(M.size());
    }

    /** Recompute the meta result from sub-board ownership. */
    void UpdateMetaResult()
    {
        const auto G = MetaGrid();
        for (const auto& L : WinLines())
        {
            const EMark A = G[L[0]];
            if (A != EMark::None && G[L[1]] == A && G[L[2]] == A)
            {
                Result = (A == EMark::X) ? EBoardResult::XWins : EBoardResult::OWins;
                return;
            }
        }
        // No meta-line available and nowhere left to play -> drawn.
        bool bAnyOpen = false;
        for (const FSubBoard& B : Boards) { if (!B.IsFinished()) { bAnyOpen = true; break; } }
        if (!bAnyOpen)
        {
            // Decided on count of claimed boards, a common tournament tiebreak; a true draw if equal.
            int X = 0, O = 0;
            for (const FSubBoard& B : Boards)
            {
                if (B.Owner() == EMark::X) ++X; else if (B.Owner() == EMark::O) ++O;
            }
            Result = (X > O) ? EBoardResult::XWins : (O > X ? EBoardResult::OWins : EBoardResult::Drawn);
        }
    }

    /** Apply a move. Assumes legality (callers use LegalMoves). */
    void Play(const FMove& M)
    {
        FSubBoard& B = Boards[M.Board];
        B.Cells[M.Cell] = ToMove;
        B.UpdateResult();

        UpdateMetaResult();

        // THE SEND: the cell just played dictates the opponent's sub-board.
        ForcedBoard = M.Cell;
        ToMove = Other(ToMove);
    }

    static FNestedBoard Initial() { return FNestedBoard(); }
};

} // namespace Fractal
