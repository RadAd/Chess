#include <tchar.h>

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include "../ChessEngine/Board.h"
#include "../ChessEngine/Moves.h"

#include "PrintBoard.h"
#include "Utils.h"

void test_known_move_stats(const bool print = false);
void test_peterellisjones();

void print(const Pos p)
{
    _tprintf(_T("%c%c"), _T('a') + p.x, _T('0') + (Board::Height - p.y));
}

void PrintBoardMoves(const Board& b, const Colour c)
{
    size_t moves = 0;
    for (const Piece p : AllPieces)
    {
        const PieceDef pd = { c, p };
        const std::set<Pos> ps = b.GetPieces(pd);
        for (const auto p : ps)
        {
            const std::set<Pos> m = GetMoves(b, p);
            moves += m.size();
            if (!m.empty())
            {
                _tprintf(_T("  piece %c  "), GetIcon(pd.p));
                print(p);
                _tprintf(_T(" ->"));
                for (const auto mp : m)
                {
                    const Board nb = DoMove(b, p, mp);
                    const PieceDef pd = b.GetPiece(mp);
                    _tprintf(_T(" "));
                    if (pd != PieceDef::empty)  _tprintf(_T("x"));
                    print(mp);
                    if (IsInCheck(nb, OtherColour(c)))  _tprintf(_T("+"));
                    _tprintf(_T(" "));
                }
                _tprintf(_T("\n"));
            }
        }
    }
    _tprintf(_T("%s %zu moves\n"), GetColour(c), moves);
}

inline int GetCost(const Piece p)
{
    switch (p)
    {
    case Piece::Pawn:       return 100;
    case Piece::Rook:       return 500;
    case Piece::Knight:     return 300;
    case Piece::Bishop:     return 300;
    case Piece::King:       return INT_MAX / 2;
    case Piece::Queen:      return 900;
    default:                return 0;
    }
}

struct Move
{
    int cost;
    Pos from;
    Pos to;
    int moves;
};

// TODO Eliminate returning to previous board states
Move GetBestMove(const Board& b, std::set<Board>& bs, const Colour c, const int depth)
{
    bs.insert(b);
    const Colour otherc = OtherColour(c);

    Move mv1 = { INT_MIN, Pos::none, Pos::none, 0 };
    if (depth < 0)
        return mv1;

    for (const Piece pc : AllPieces)
    {
        const PieceDef pd = { c, pc };
        const std::set<Pos> ps = b.GetPieces(pd);
        for (const auto p : ps)
        {
            const std::set<Pos> m = GetMoves(b, p);
            for (const auto mp : m)
            {
                ++mv1.moves;
                int mvcost = INT_MIN;

                if (depth > 0)
                {
                    _ASSERTE(b.GetPiece(p) == pd);
                    const Board sb = DoMove(b, p, mp);

                    const bool found = bs.find(sb) != bs.end();

                    if (!found)
                    {
                        const Move smv = GetBestMove(sb, bs, otherc, depth - 1);
                        mv1.moves += smv.moves;

                        if (smv.cost == INT_MIN)
                            mvcost = GetCost(b.GetPiece(mp).p);
                        else
                            mvcost = GetCost(b.GetPiece(mp).p) - smv.cost;
                    }
                }
                else
                    mvcost = GetCost(b.GetPiece(mp).p);

                if (mvcost > mv1.cost)
                {
                    mv1.cost = mvcost;
                    mv1.from = p;
                    mv1.to = mp;
                }
            }
        }
    }

    // if cost is INT_MIN then there are no moves, game over ???

    return mv1;
}

void test_moves(const Piece pce, const size_t moves, const bool print = false)
{
    Board b;
    const Pos p = { 4, 6 };
    b.SetPiece(p, { Colour::White, pce });
    const std::set<Pos> m = GetMoves(b, p);
    if (print) PrintBoard(b, p, m);
    ASSERT_EQUAL(m.size(), moves);
}

void test_moves_blocked(const Piece pce, const size_t moves, const Colour blockc, const Pos block, const bool print = false)
{
    Board b;
    const Pos p = { 4, 6 };
    b.SetPiece(p, { Colour::White, pce });
    b.SetPiece(p + block, { blockc, Piece::Pawn });
    const std::set<Pos> m = GetMoves(b, p);
    if (print) PrintBoard(b, p, m);
    ASSERT_EQUAL(m.size(), moves);
}

void test_moves_pawn(const bool en_pass, const bool attack, const int blocked, const size_t moves, const bool print = false)
{
    Board b;
    const Pos p = { 4, en_pass ? 6: 5 };
    b.SetPiece(p, { Colour::White, Piece::Pawn });
    if (attack)
    {
        b.SetPiece(p + Pos({ -1, -1 }), { Colour::Black, Piece::Pawn });
        b.SetPiece(p + Pos({ 1, -1 }), { Colour::Black, Piece::Pawn });
    }
    if (blocked != 0)
        b.SetPiece(p + Pos({ 0, -blocked }), { Colour::Black, Piece::Pawn });
    const std::set<Pos> m = GetMoves(b, p);
    if (print) PrintBoard(b, p, m);
    ASSERT_EQUAL(m.size(), moves);
}

void test_move_check(const Piece pce, const size_t moves, const bool print = false)
{
    Board b;
    const Pos p = { 4, 4 };
    b.SetPiece(p, { Colour::White, pce });
    b.SetPiece(p + Pos({ -2, 0 }), { Colour::White, Piece::King });
    b.SetPiece(p + Pos({ 2, 0 }), { Colour::Black, Piece::Rook });

    const std::set<Pos> m = GetMoves(b, p);
    if (print) PrintBoard(b, p, m);
    ASSERT_EQUAL(m.size(), moves);
}

void test_move_check2(const Piece pce, const size_t moves, const bool print = false)
{
    Board b;
    const Pos p = { 4, 4 };
    b.SetPiece(p, { Colour::Black, pce });
    b.SetPiece(p + Pos({ -2, 0 }), { Colour::White, Piece::King });
    b.SetPiece(p + Pos({ 2, 0 }), { Colour::Black, Piece::Rook });
    b.SetPiece(p + Pos({ 0, 2 }), { Colour::White, Piece::Pawn });

    const std::set<Pos> m = GetMoves(b, p);
    if (print) PrintBoard(b, p, m);
    ASSERT_EQUAL(m.size(), moves);
}

void test_move_castle(const Colour c, const bool print = false)
{
    Board b;
    const int row = c == Colour::White ? Board::Height - 1 : 0;
    const Pos p = { 4, row };
    b.SetPiece(p, { c, Piece::King });
    b.SetPiece(Pos({ 0, row }), { c, Piece::Rook });
    b.SetPiece(Pos({ Board::Width - 1, row }), { c, Piece::Rook });

    const std::set<Pos> m = GetMoves(b, p);
    if (print) PrintBoard(b, p, m);
    ASSERT_EQUAL(m.size(), 7);
}

void test_move_en_passant(const Colour c, const bool print = false)
{
    Board b;
    const int row = c == Colour::White ? 3 : Board::Height - 4;
    const int dir = c == Colour::White ? -1 : 1;
    const Pos p = { 5, row };
    b.SetPiece(p, { c, Piece::Pawn });
    const Pos q = { 4, row + 2 * dir };
    b.SetPiece(q, { OtherColour(c), Piece::Pawn });

    b = DoMove(b, q, q + Pos({ 0, -2 * dir }));
    _ASSERT(b.m_enPassant != Pos::none);

    const std::set<Pos> m = GetMoves(b, p);
    if (print) PrintBoard(b, p, m);
    ASSERT_EQUAL(m.size(), 2);
}

void test_move_castle_check(const Colour c, const int col, const int moves, const bool print = false)
{
    Board b;
    const int row = c == Colour::White ? Board::Height - 1 : 0;
    const Pos p = { 4, row };
    b.SetPiece(p, { c, Piece::King });
    b.SetPiece(Pos({ 0, row }), { c, Piece::Rook });
    b.SetPiece(Pos({ Board::Width - 1, row }), { c, Piece::Rook });
    b.SetPiece(Pos({ col, 4 }), { OtherColour(c), Piece::Rook });

    const std::set<Pos> m = GetMoves(b, p);
    if (print) PrintBoard(b, p, m);
    ASSERT_EQUAL(m.size(), moves);
}

void test()
{
    test_moves(Piece::Pawn, 2);
    test_moves(Piece::Rook, 14);
    test_moves(Piece::Knight, 6);
    test_moves(Piece::Bishop, 9);
    test_moves(Piece::Queen, 23);
    test_moves(Piece::King, 8);

    test_moves_blocked(Piece::Pawn, 0, Colour::Black, Pos({ 0, -1 }));
    test_moves_blocked(Piece::Pawn, 1, Colour::Black, Pos({ 0, -2 }));
    test_moves_blocked(Piece::Pawn, 3, Colour::Black, Pos({ 1, -1 }));
    test_moves_blocked(Piece::Rook, 10, Colour::Black, Pos({ 0, -2 }));
    test_moves_blocked(Piece::Knight, 6, Colour::Black, Pos({ 1, -2 }));
    test_moves_blocked(Piece::Bishop, 8, Colour::Black, Pos({ 2, -2 }));
    test_moves_blocked(Piece::Queen, 22, Colour::Black, Pos({ 2, -2 }));
    test_moves_blocked(Piece::King, 7, Colour::Black, Pos({ 2, -2 }));
    test_moves_blocked(Piece::King, 7, Colour::Black, Pos({ 2, -1 }));
    test_moves_blocked(Piece::King, 8, Colour::Black, Pos({ 1, -1 }));

    test_moves_blocked(Piece::Pawn, 0, Colour::White, Pos({ 0, -1 }));
    test_moves_blocked(Piece::Pawn, 1, Colour::White, Pos({ 0, -2 }));
    test_moves_blocked(Piece::Pawn, 2, Colour::White, Pos({ 1, -1 }));
    test_moves_blocked(Piece::Rook, 9, Colour::White, Pos({ 0, -2 }));
    test_moves_blocked(Piece::Knight, 5, Colour::White, Pos({ 1, -2 }));
    test_moves_blocked(Piece::Bishop, 7, Colour::White, Pos({ 2, -2 }));
    test_moves_blocked(Piece::Queen, 21, Colour::White, Pos({ 2, -2 }));
    test_moves_blocked(Piece::King, 8, Colour::White, Pos({ 2, -2 }));
    test_moves_blocked(Piece::King, 8, Colour::White, Pos({ 2, -1 }));
    test_moves_blocked(Piece::King, 7, Colour::White, Pos({ 1, -1 }));

    test_moves_pawn(false, false, 0, 1);
    test_moves_pawn(true, false, 0, 2);
    test_moves_pawn(false, true, 0, 3);
    test_moves_pawn(true, true, 0, 4);

    test_moves_pawn(false, false, 1, 0);
    test_moves_pawn(true, false, 1, 0);
    test_moves_pawn(false, true, 1, 2);
    test_moves_pawn(true, true, 1, 2);

    test_move_check(Piece::Pawn, 0);
    test_move_check(Piece::Rook, 3);
    test_move_check(Piece::Knight, 0);
    test_move_check(Piece::Bishop, 0);
    test_move_check(Piece::Queen, 3);

    test_move_check2(Piece::Pawn, 1);
    test_move_check2(Piece::Rook, 9);
    test_move_check2(Piece::Knight, 8);
    test_move_check2(Piece::Bishop, 13);
    test_move_check2(Piece::Queen, 22);

    test_move_en_passant(Colour::White);
    test_move_en_passant(Colour::Black);

    test_move_castle(Colour::White);
    test_move_castle(Colour::Black);

    test_move_castle_check(Colour::White, 4, 4);
    test_move_castle_check(Colour::Black, 4, 4);
    test_move_castle_check(Colour::White, 3, 4);
    test_move_castle_check(Colour::Black, 3, 4);
    test_move_castle_check(Colour::White, 2, 6);
    test_move_castle_check(Colour::Black, 2, 6);

    //test_known_move_stats(true);
    //test_peterellisjones();
}

int _tmain(const int argc, const TCHAR* const argv[])
{
    _tprintf(_T("Chess\n"));

#ifdef _DEBUG
    test();
#else
    test_known_move_stats(true);
#endif

    if (false)
    {
        //Board bfen = Board::CreateFromFen(_T("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
        Colour colour = Colour::White;
        Board bfen = Board::CreateFromFen(_T("8/ppp3p1/8/8/3p4/5Q2/1ppp2K1/brk4n w - - 11 7"), colour);
        PrintBoard(bfen);
        PrintBoardMoves(bfen, colour);
    }

#if 0
    Board b = Board::Create();
    //Pos p;
#elif 0
    Board b;
    Pos p = { 5, 2 };
    b.SetPiece(p, { Colour::White, Piece::King });
    b.SetPiece({ 5, 5 }, { Colour::Black, Piece::Pawn });
#elif 0
    Board b;
    Pos p = { 5, 2 };
    b.SetPiece(p, { Colour::White, Piece::King });
    b.SetPiece({ 5, 5 }, { Colour::Black, Piece::Rook });
    b.SetPiece({ 7, 2 }, { Colour::Black, Piece::Rook });
#elif 0
    Board b;
    Pos p = { 5, 2 };
    b.SetPiece(p, { Colour::Black, Piece::King });
    b.SetPiece({ 5, 5 }, { Colour::White, Piece::Rook });
#else
    Board b;
    Pos p = { 4, 1 };
    b.SetPiece(p, { Colour::Black, Piece::King });
    b.SetPiece({ 5, 5 }, { Colour::White, Piece::Rook });
#endif
    PrintBoard(b);

    int prevmoves = 0;
    for (int depth = 0; depth < 4; ++depth)
    {
        const clock_t start = clock();
        std::set<Board> sb;
        Move mv = GetBestMove(b, sb, Colour::White, depth);
        const clock_t end = clock();

        _tprintf(_T("depth %d, move from { %d, %d } to { %d, %d } moves %d  %d, time %f\n"), depth, mv.from.x, mv.from.y, mv.to.x, mv.to.y, mv.moves, mv.moves - prevmoves, float(end - start)/CLOCKS_PER_SEC);
        prevmoves = mv.moves;
    }
    _tprintf(_T("\n"));

    return EXIT_SUCCESS;
}
