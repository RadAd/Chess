#include "Moves.h"

#include "Board.h"

namespace
{
    inline bool CanMove(const Board& b, const Pos from, const Pos to)
    {
        // TODO This could be done faster
        const std::set<Pos> mv = GetMoves(b, from, false);
        return mv.find(to) != mv.end();
    }

    bool InsertIf(std::set<Pos>& m, const Board& b, const Pos from, const Pos to, const Colour c, const bool DoCheckCheck)
    {
        _ASSERTE(Board::Valid(from));
        _ASSERTE(c == Colour::None || c == OtherColour(b.GetPiece(from).c));
        if (!Board::Valid(to))
            return false;
        const PieceDef pd = b.GetPiece(to);
        if (pd.c != c)
            return false;
        if (DoCheckCheck && IsInCheck(DoMove(b, from, to), b.GetPiece(from).c))
            return true;    // Can't make this move but keep checking

        m.insert(to);
        return true;
    }

    inline bool InsertIfEnPassant(std::set<Pos>& m, const Board& b, const Pos from, const Pos to, const Colour c, const bool DoCheckCheck)
    {
        if (to == b.m_enPassant)
            return InsertIf(m, b, from, to, Colour::None, DoCheckCheck);
        else
            return InsertIf(m, b, from, to, c, DoCheckCheck);
    }

    bool InsertIfNot(std::set<Pos>& m, const Board& b, const Pos from, const Pos to, const Colour c, const bool DoCheckCheck)
    {
        _ASSERTE(Board::Valid(from));
        if (!Board::Valid(to))
            return false;
        const PieceDef pd = b.GetPiece(to);
        if (pd.c == c)
            return false;
        if (DoCheckCheck && IsInCheck(DoMove(b, from, to), c))
            return false;

        m.insert(to);
        return true;
    }

    void InsertDir(std::set<Pos>& m, const Board& b, const Pos from, const Pos dir, const Colour otherc, const bool DoCheckCheck)
    {
        _ASSERTE(Board::Valid(from));
        for (Pos to = from + dir; Board::Valid(to); to += dir)
            if (!InsertIf(m, b, from, to, Colour::None, DoCheckCheck))
            {
                InsertIf(m, b, from, to, otherc, DoCheckCheck);
                break;
            }
    }
}

std::set<Pos> GetMoves(const Board& b, const Pos p, const bool DoCheckCheck)
{
    static const Pos straights[] = { Pos({ 1, 0 }), Pos({ -1, 0 }), Pos({ 0, 1 }), Pos({ 0, -1 }) };
    static const Pos diagnals[] = { Pos({ 1, 1 }), Pos({ -1, 1 }), Pos({ 1, -1 }), Pos({ -1, -1 }) };

    std::set<Pos> m;
    const PieceDef pd = b.GetPiece(p);
    _ASSERTE(pd.c != Colour::None);

    const int dir = pd.c == Colour::White ? -1 : 1;
    const Colour otherc = OtherColour(pd.c);
    switch (pd.p)
    {
    case Piece::Pawn:
        if (InsertIf(m, b, p, p + Pos({ 0, dir }), Colour::None, DoCheckCheck) && ((pd.c == Colour::White && p.y == Board::Height - 2) || (pd.c == Colour::Black && p.y == 1)))
            InsertIf(m, b, p, p + Pos({ 0, 2 * dir }), Colour::None, DoCheckCheck);
        InsertIfEnPassant(m, b, p, p + Pos({ 1, dir }), otherc, DoCheckCheck);
        InsertIfEnPassant(m, b, p, p + Pos({ -1, dir }), otherc, DoCheckCheck);
        break;
    case Piece::Rook:
        for (const Pos o : straights)
            InsertDir(m, b, p, o, otherc, DoCheckCheck);
        break;
    case Piece::Knight:
        for (const Pos o : { Pos({ 1, -2 }), Pos({ -1, -2 }), Pos({ 1, 2 }), Pos({ -1, 2 }), Pos({ 2, 1 }), Pos({ 2, -1 }), Pos({ -2, 1 }), Pos({ -2, -1 }) })
            InsertIfNot(m, b, p, p + o, pd.c, DoCheckCheck);
        break;
    case Piece::Bishop:
        for (const Pos o : diagnals)
            InsertDir(m, b, p, o, otherc, DoCheckCheck);
        break;
    case Piece::King:
        for (const Pos o : straights)
            InsertIfNot(m, b, p, p + o, pd.c, DoCheckCheck);
        for (const Pos o : diagnals)
            InsertIfNot(m, b, p, p + o, pd.c, DoCheckCheck);

        {   // Castling
            const int row = pd.c == Colour::White ? Board::Height - 1 : 0;
            if (p == Pos({ 4, row }) && (!DoCheckCheck || !IsInCheck(b, pd.c)))
            {
                bool c1 = IsInCheck(DoMove(b, p, p + Pos({ -1, 0 })), pd.c);
                if (b.GetPiece(Pos({ 0, row })) == PieceDef({ pd.c, Piece::Rook })
                    && b.GetPiece(Pos({ 1, row })) == PieceDef::empty
                    && b.GetPiece(Pos({ 2, row })) == PieceDef::empty
                    && b.GetPiece(Pos({ 3, row })) == PieceDef::empty
                    && (!DoCheckCheck || !IsInCheck(DoMove(b, p, p + Pos({ -1, 0 })), pd.c))) // TODO AND hasn't moved
                    InsertIf(m, b, p, p + Pos({ -2, 0 }), Colour::None, DoCheckCheck);
                if (b.GetPiece(Pos({ Board::Width - 1, row })) == PieceDef({ pd.c, Piece::Rook })
                    && b.GetPiece(Pos({ 5, row })) == PieceDef::empty
                    && b.GetPiece(Pos({ 6, row })) == PieceDef::empty
                    && (!DoCheckCheck || !IsInCheck(DoMove(b, p, p + Pos({ 1, 0 })), pd.c))) // TODO AND hasn't moved
                    InsertIf(m, b, p, p + Pos({ 2, 0 }), Colour::None, DoCheckCheck);
            }
        }
        break;
    case Piece::Queen:
        for (const Pos o : straights)
            InsertDir(m, b, p, o, otherc, DoCheckCheck);
        for (const Pos o : diagnals)
            InsertDir(m, b, p, o, otherc, DoCheckCheck);
        break;
    }

    return m;
}

std::set<Pos> GetThreats(const Board& b, const Pos po)
{
    _ASSERT(Board::Valid(po));
    std::set<Pos> threats;

    auto piece = b.GetPiece(po);
    if (piece == PieceDef::empty)
        return threats;

    const Colour otherc = OtherColour(piece.c);

    for (const Piece p : AllPieces)
    {
        const PieceDef pd = { otherc, p };
        const std::set<Pos> ps = b.GetPieces(pd);
        for (const auto p : ps)
        {
            if (CanMove(b, po, p))
                threats.insert(p);
        }
    }

    return threats;
}

std::set<Pos> GetCheckThreats(const Board& b, const Colour c)
{
    _ASSERTE(c != Colour::None);
    std::set<Pos> threats;

    auto kings = b.GetPieces({ c, Piece::King });
    if (kings.empty())
        return threats;
    _ASSERTE(kings.size() == 1);

    const Pos pk = *kings.begin();
    const Colour otherc = OtherColour(c);

    for (const Piece p : AllPieces)
    {
        const PieceDef pd = { otherc, p };
        const std::set<Pos> ps = b.GetPieces(pd);
        for (const auto p : ps)
        {
            if (CanMove(b, p, pk))
                threats.insert(p);
        }
    }

    return threats;
}

Board DoMove(const Board& bo, const Pos from, const Pos to, bool* fCastle)
{
    _ASSERT(Board::Valid(from));
    _ASSERT(Board::Valid(to));
    Board b = bo;

    const PieceDef pd = b.GetPiece(from);
    _ASSERT(pd != PieceDef::empty);
    b.SetPiece(from, PieceDef::empty);
    b.SetPiece(to, pd);

    const int row = pd.c == Colour::White ? Board::Height - 1 : 0;
    const int dir = pd.c == Colour::White ? -1 : 1;
    if (pd.p == Piece::King && from == Pos({ 4, row }))
    {   // Castle
        if (from + Pos({ -2, 0 }) == to)
        {
            b = DoMove(b, Pos({ 0, row }), from + Pos({ -1, 0 }));
            if (fCastle) *fCastle = true;
        }
        else if (from + Pos({ 2, 0 }) == to)
        {
            b = DoMove(b, Pos({ Board::Width - 1, row }), from + Pos({ 1, 0 }));
            if (fCastle) *fCastle = true;
        }
    }
    else if (pd.p == Piece::Pawn && to == b.m_enPassant)
    {
        const Pos pawn = to + Pos({ 0, -dir });
        _ASSERT(b.GetPiece(pawn) == PieceDef({ OtherColour(pd.c), Piece::Pawn }));
        b.SetPiece(pawn, PieceDef::empty);
    }

    if (pd.p == Piece::Pawn && from.x == to.x && from.y == (row + 1 * dir) && to.y == (row + 3 * dir))
    {   // En Passant initial move
        b.m_enPassant = Pos({ from.x , row + 2 * dir });
    }
    else
    {
        b.m_enPassant = Pos::none;
    }

    return b;
}

bool IsInCheckMate(const Board& b, const Colour c)
{
    _ASSERTE(IsInCheck(b, c));
    for (const Piece p : AllPieces)
    {
        const PieceDef pd = { c, p };
        const std::set<Pos> ps = b.GetPieces(pd);
        for (const auto p : ps)
        {
            const std::set<Pos> mv = GetMoves(b, p);
            if (!mv.empty())
                return false;
        }
    }
    return true;
}
