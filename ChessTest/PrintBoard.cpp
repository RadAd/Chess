#include "PrintBoard.h"

const TCHAR* GetColour(Colour c)
{
    switch (c)
    {
    default:
    case Colour::None: return _T("None");
    case Colour::White: return _T("White");
    case Colour::Black: return _T("Black");
    }
}

TCHAR GetIcon(const Piece p)
{
    switch (p)
    {
    case Piece::Pawn:       return _T('p');
    case Piece::Rook:       return _T('r');
    case Piece::Knight:     return _T('k');
    case Piece::Bishop:     return _T('b');
    case Piece::King:       return _T('K');
    case Piece::Queen:      return _T('Q');
    default:                return _T('.');
    }
}

void PrintBoard(const Board& b, const Pos ph, const std::set<Pos>& m)
{
    const int bg[] = { 40, 47 };
    const int fg[] = { 35, 36 };

    Pos p;
    for (p.y = 0; p.y < Board::Height; ++p.y)
    {
        for (p.x = 0; p.x < Board::Width; ++p.x)
        {
            const PieceDef pd = b.GetPiece(p);
            _tprintf(_T("\x1b[%dm\x1b[%dm %c "), (m.find(p) != m.end()) ? 41 : p == ph ? 43 : bg[(p.x + p.y) % 2], fg[int(pd.c)], GetIcon(pd.p));
        }
        _tprintf(_T("\x1b[0m\n"));
    }
    _tprintf(_T("\n"));
}
