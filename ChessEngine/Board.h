#pragma once
#include <bitset>
#include <set>

#include "TString.h"

#include "Pos.h"

enum class Colour
{
    Black,
    White,
    None
};

inline Colour OtherColour(const Colour c)
{
    _ASSERTE(c != Colour::None);
    return c == Colour::White ? Colour::Black : Colour::White;
}

enum class Piece
{
    Pawn,
    Rook,
    Knight,
    Bishop,
    Queen,
    King,
    None
};

static const Piece AllPieces[] = { Piece::Pawn, Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen, Piece::King };

struct PieceDef
{
    Colour c;
    Piece p;

    bool operator==(const PieceDef pd) const
    {
        return c == pd.c && p == pd.p;
    }
    bool operator!=(const PieceDef pd) const
    {
        return c != pd.c || p != pd.p;
    }
    bool operator<(const PieceDef pd) const
    {
        if (c == pd.c)
            return p < pd.p;
        else
            return c < pd.c;
    }

    static const PieceDef empty;
};

class Board
{
public:
    static const int Width = 8;
    static const int Height = 8;

    static Board Create();

    // https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
    static Board CreateFromFen(const std::tstring& fen, Colour& active);

    inline static bool Valid(const Pos p)
    {
        if (p.x < 0 || p.x >= Width)
            return false;
        if (p.y < 0 || p.y >= Height)
            return false;
        return true;
    }

    bool operator==(const Board& other) const
    {
        return m_pieces == other.m_pieces && m_enPassant == other.m_enPassant;
    }
    bool operator<(const Board& other) const
    {
        return m_pieces == other.m_pieces ? m_enPassant < other.m_enPassant : m_pieces < other.m_pieces;
    }

    PieceDef GetPiece(const Pos p) const;

    void SetPiece(const Pos p, const PieceDef pd);

    std::set<Pos> GetPieces(const PieceDef pd) const;

    Pos m_enPassant = Pos::none;    // The tile skipped over

private:
    static const int Size = Width * Height;

    std::bitset<Size> m_pieces[int(Colour::None)][int(Piece::None)];

    const std::bitset<Size>& pieces(const Colour c, const Piece p) const { return m_pieces[int(c)][int(p)]; }
    std::bitset<Size>& pieces(const Colour c, const Piece p) { return m_pieces[int(c)][int(p)]; }

    const std::bitset<Size>& pieces(PieceDef pd) const { return pieces(pd.c, pd.p); }
    std::bitset<Size>& pieces(PieceDef pd) { return pieces(pd.c, pd.p); }

    inline static int ToBoardIndex(const Pos p)
    {
        if (!Valid(p))
            return -1;
        return p.y * Width + p.x;
    }

    inline static Pos FromBoardIndex(const int idx)
    {
        if (idx < 0 && idx >= Size)
            return Pos::none;
        return { idx % Width, idx / Height };
    }
};
