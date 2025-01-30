#pragma once

#include <set>

struct Pos;
class Board;
enum class Colour;

std::set<Pos> GetMoves(const Board& b, const Pos p, const bool DoCheckCheck = true);
std::set<Pos> GetThreats(const Board& b, const Pos p);
std::set<Pos> GetCheckThreats(const Board& b, const Colour c);
Board DoMove(const Board& b, const Pos from, const Pos to, bool* fCastle = nullptr);

inline bool IsInCheck(const Board& b, const Colour c)
{
    const auto ct = GetCheckThreats(b, c);
    const bool InCheck = !ct.empty();
    return InCheck;
}

bool IsInCheckMate(const Board& b, const Colour c);
