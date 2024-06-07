#include "Board.h"

#include <tchar.h>

#include "MathUtils.h"

const PieceDef PieceDef::empty = { Colour::None, Piece::None };

Board Board::Create()
{
    Board b;
    for (int x = 0; x < Board::Width; ++x)
    {
        b.SetPiece({ x, 1 }, { Colour::Black, Piece::Pawn });
        b.SetPiece({ x, Board::Height - 2 }, { Colour::White, Piece::Pawn });
    }

    int x = 0;
    for (const Piece p : { Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King, Piece::Bishop, Piece::Knight, Piece::Rook })
    {
        b.SetPiece({ x, 0 }, { Colour::Black, p });
        b.SetPiece({ x, Board::Height - 1 }, { Colour::White, p });
        ++x;
    }

    return b;
}

// https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
Board Board::CreateFromFen(const std::tstring& fen, Colour& active)
{
    Board b;

    std::tstring::const_iterator it = fen.begin();
    Pos p = { 0, 0 };
    while (it != fen.end() && *it != _T(' '))
    {
        const TCHAR c = *it++;
        PieceDef pd = PieceDef::empty;
        switch (c)
        {
        case _T('p'): pd.c = Colour::Black; pd.p = Piece::Pawn; break;
        case _T('r'): pd.c = Colour::Black; pd.p = Piece::Rook; break;
        case _T('n'): pd.c = Colour::Black; pd.p = Piece::Knight; break;
        case _T('b'): pd.c = Colour::Black; pd.p = Piece::Bishop; break;
        case _T('q'): pd.c = Colour::Black; pd.p = Piece::Queen; break;
        case _T('k'): pd.c = Colour::Black; pd.p = Piece::King; break;
        case _T('P'): pd.c = Colour::White; pd.p = Piece::Pawn; break;
        case _T('R'): pd.c = Colour::White; pd.p = Piece::Rook; break;
        case _T('N'): pd.c = Colour::White; pd.p = Piece::Knight; break;
        case _T('B'): pd.c = Colour::White; pd.p = Piece::Bishop; break;
        case _T('Q'): pd.c = Colour::White; pd.p = Piece::Queen; break;
        case _T('K'): pd.c = Colour::White; pd.p = Piece::King; break;
        case _T('/'): _ASSERT(p.x == 8); p.x = 0; ++p.y; break;
        case _T('1'): case _T('2'): case _T('3'): case _T('4'): case _T('5'): case _T('6'): case _T('7'): case _T('8'): p.x += (c - _T('0')); break;
        default: _ASSERT(false); break;
        }

        if (pd != PieceDef::empty)
        {
            b.SetPiece(p, pd);
            ++p.x;
        }
    }

    while (it != fen.end() && *it == _T(' '))
        it++;

    active = it == fen.end() || *it++ == _T('w') ? Colour::White : Colour::Black;

    while (it != fen.end() && *it == _T(' '))
        it++;

    // TODO Castling state
    while (it != fen.end() && *it != _T(' '))
        ++it;

    while (it != fen.end() && *it == _T(' '))
        it++;

    // En passant
    if (*it == _T('-'))
    {
        ++it;
        b.m_enPassant = Pos::none;
    }
    else
    {
        b.m_enPassant.x = *it - 'a';
        ++it;
        b.m_enPassant.y = 8 - (*it - '0');
        ++it;
    }
    _ASSERT(*it == _T(' '));

    while (it != fen.end() && *it == _T(' '))
        it++;

    // TODO Halfmove clock
    while (it != fen.end() && *it != _T(' '))
        ++it;

    while (it != fen.end() && *it == _T(' '))
        it++;

    // TODO Fullmove number
    while (it != fen.end() && *it != _T(' '))
        ++it;

    _ASSERT(it == fen.end());

    return b;
}

PieceDef Board::GetPiece(const Pos p) const
{
    const int idx = ToBoardIndex(p);
    if (idx >= 0)
    {
        for (const Colour c : { Colour::Black, Colour::White })
            for (const Piece p : AllPieces)
            {
                if (pieces(c, p).test(idx))
                    return { c, p };
            }
    }
    return PieceDef::empty;
}

void Board::SetPiece(const Pos p, const PieceDef pd)
{
    const int idx = ToBoardIndex(p);

    {
        const PieceDef pd2 = GetPiece(p);
        if (pd2 != PieceDef::empty)
            pieces(pd2).set(idx, false);
        _ASSERTE(GetPiece(p) == PieceDef::empty);
    }

    if (pd != PieceDef::empty)
        pieces(pd).set(idx, true);
}

std::set<Pos> Board::GetPieces(const PieceDef pd) const
{
    std::set<Pos> ps;

    const auto& bs = pieces(pd);

    for (auto n = bs.to_ullong(), lsb2 = lsb(n); n != 0; n &= ~lsb2, lsb2 = lsb(n))
    {
        const int idx = log2_64(lsb2);
        _ASSERTE(bs.test(idx));
        const Pos p = FromBoardIndex(idx);
        _ASSERTE(idx == ToBoardIndex(p));
        _ASSERTE(GetPiece(p) == pd);

        ps.insert(p);
    }

    return ps;
}
