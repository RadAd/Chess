#pragma once
#include <tchar.h>

#include "../ChessEngine/Board.h"

const TCHAR* GetColour(Colour c);
TCHAR GetIcon(const Piece p);
void PrintBoard(const Board& b, const Pos ph = Pos::none, const std::set<Pos>& m = std::set<Pos>());
