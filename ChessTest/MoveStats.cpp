#include <tchar.h>

#include "../ChessEngine/Board.h"
#include "../ChessEngine/Moves.h"

#include "PrintBoard.h"
#include "Utils.h"

#include <ctime>

struct Stats
{
    size_t moves;
    size_t captures;
    size_t checks;
    size_t castles;
    size_t checkmates;

    Stats& operator+=(const Stats s)
    {
        moves += s.moves;
        captures += s.captures;
        checks += s.checks;
        castles += s.castles;
        checkmates += s.checkmates;
        return *this;
    }
    Stats& operator-=(const Stats s)
    {
        moves -= s.moves;
        captures -= s.captures;
        checks -= s.checks;
        castles -= s.castles;
        checkmates -= s.checkmates;
        return *this;
    }
    Stats operator-(const Stats s) const
    {
        Stats r(*this);
        r -= s;
        return r;
    }
    bool operator==(const Stats s) const
    {
        return moves == s.moves
            and captures == s.captures
            and checks == s.checks
            and castles == s.castles
            and checkmates == s.checkmates;
    }
};

Stats GetMoveStats(const Board& b, const Colour c, const int depth, const bool print = false)
{
    const Colour otherc = OtherColour(c);

    if (depth < 0)
        return {};

    Stats stats = {};
    for (const Piece p : AllPieces)
    {
        const PieceDef pd = { c, p };
        const std::set<Pos> ps = b.GetPieces(pd);
        for (const auto p : ps)
        {
            const std::set<Pos> m = GetMoves(b, p);
            stats.moves += m.size();

            if (print and depth == 0 and !m.empty())
            {
                _tprintf(_T("  piece %c  { %d, %d } -> "), GetIcon(pd.p), p.x, p.y);
                for (const auto mp : m)
                {
                    _tprintf(_T(" { %d, %d } "), mp.x, mp.y);
                }
                _tprintf(_T("\n"));
            }

            for (const auto mp : m)
            {
                if (b.GetPiece(mp) != PieceDef::empty)
                    ++stats.captures;
                bool fCastle = false;
                const Board sb = DoMove(b, p, mp, &fCastle);
                if (fCastle)
                    ++stats.castles;

                if (IsInCheck(sb, otherc))
                {
                    ++stats.checks;
                    if (IsInCheckMate(sb, otherc))
                        ++stats.checkmates;
                }

                if (depth > 0)
                {
                    const Stats thisstats = GetMoveStats(sb, otherc, depth - 1, print);
                    if (print and depth == 1)
                    {
                        PrintBoard(sb);
                        _tprintf(_T("  moves %zu, captures %zu\n"), thisstats.moves, thisstats.captures);
                    }
                    stats += thisstats;
                }
            }
        }
    }

    return stats;
}

Stats GetMoveStats(const std::tstring& fen, const int depth, const bool print = false)
{
    Colour c = Colour::White;
    Board b = Board::CreateFromFen(fen, c);
    return GetMoveStats(b, c, depth, print);
}

void test_known_move_stats(const bool print = false)
{
    Board b = Board::Create();

    // From https://youtu.be/U4ogK0MIzqk?t=608
    // https://en.wikipedia.org/wiki/Shannon_number
    // https://www.chessprogramming.org/Perft_Results
    const Stats depth_count[] = {
        {         20 },
        {        400 },
        {       8902,    34,    12 },
        {     197281,  1576,   469, 0,   8 },
        {    4865609, 82719, 27351, 0, 347 },
        {  119060324 },
        //3195901860,
    };

    if (print)  _tprintf(_T("%5s %8s %8s %8s %8s %10s %8s\n"), _T("Depth"), _T("Moves"), _T("Captures"), _T("Checks"), _T("Castles"), _T("Checkmates"), _T("Time"));
    Stats prevstats = {};
    //const int depth = 3;
    for (int depth = 0; depth < 5; ++depth)
    {
        const clock_t start = clock();
        const Stats stats = GetMoveStats(b, Colour::White, depth);
        const clock_t end = clock();

        const Stats difftsats = stats - prevstats;

        if (print)  _tprintf(_T("%5d %8zu %8zu %8zu %8zu %10zu %8.3f\n"), depth,
            difftsats.moves,
            difftsats.captures,
            difftsats.checks,
            difftsats.castles,
            difftsats.checkmates,
            float(end - start) / CLOCKS_PER_SEC);
        ASSERT_EQUAL(difftsats.moves,       depth_count[depth].moves);
        ASSERT_EQUAL(difftsats.captures,    depth_count[depth].captures);
        ASSERT_EQUAL(difftsats.checks,      depth_count[depth].checks);
        ASSERT_EQUAL(difftsats.castles,     depth_count[depth].castles);
        ASSERT_EQUAL(difftsats.checkmates,  depth_count[depth].checkmates);
        prevstats = stats;
    }
    if (print)  _tprintf(_T("\n"));

    if (false)
    {
        Board b;
        b.SetPiece(Pos({ 4, Board::Height - 1 }), { Colour::White, Piece::King });
        b.SetPiece(Pos({ 4, Board::Height - 2 }), { Colour::White, Piece::Rook });
        b.SetPiece(Pos({ 4, 1 }), { Colour::Black, Piece::King });

        PrintBoard(b);
        const Stats stats = GetMoveStats(b, Colour::White, 0, true);
        _tprintf(_T("%8s %8s %8s %8s %10s\n"), _T("Moves"), _T("Captures"), _T("Checks"), _T("Castles"), _T("Checkmates"));
        _tprintf(_T("%8zu %8zu %8zu %8zu %8zu\n"), stats.moves, stats.captures, stats.checks, stats.castles, stats.checkmates);
        _tprintf(_T("\n"));
    }
}

Stats test_peterellisjones_GetMoveStats(const std::tstring& fen, const int depth)
{
    _tprintf(_T("%s\n"), fen.c_str());
    const Stats prev = depth > 0 ? GetMoveStats(fen, depth - 1) : Stats({});
    const Stats stats = GetMoveStats(fen, depth);
    return stats - prev;
}

void test_peterellisjones()
{
    //https://gist.github.com/peterellisjones/8c46c28141c162d1d8a0f0badbc9cff9
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2"), 0).moves, 8);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3"), 0).moves, 8);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2"), 0).moves, 19);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2"), 0).moves, 5);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2"), 0).moves, 44);
    //ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9"), 0).moves, 39);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4"), 0).moves, 9);
    //ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"), 2).moves, 62379);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"), 2).moves, 89890);
    //ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1"), 5).moves, 1134888);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1"), 5).moves, 1440467);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("5k2/8/8/8/8/8/8/4K2R w K - 0 1"), 5).moves, 661072);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1"), 5).moves, 803711);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1"), 3).moves, 1274206);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1"), 3).moves, 1720476);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1"), 5).moves, 3821001);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1"), 4).moves, 1004658);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("4k3/1P6/8/8/8/8/K7/8 w - - 0 1"), 5).moves, 217342);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("8/P1k5/K7/8/8/8/8/8 w - - 0 1"), 5).moves, 92683);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("K1k5/8/P7/8/8/8/8/8 w - - 0 1"), 5).moves, 2217);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("8/k1P5/8/1K6/8/8/8/8 w - - 0 1"), 6).moves, 567584);
    ASSERT_EQUAL(test_peterellisjones_GetMoveStats(_T("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1"), 3).moves, 23527);
}
