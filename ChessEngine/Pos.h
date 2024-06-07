#pragma once

struct Pos
{
    int x;
    int y;

    static const Pos none;

    bool operator==(const Pos p) const
    {
        return x == p.x && y == p.y;
    }
    bool operator!=(const Pos p) const
    {
        return x != p.x || y != p.y;
    }
    bool operator<(const Pos p) const
    {
        return x == p.x ? y < p.y : x < p.x;
    }
    Pos operator+(const Pos p) const
    {
        return { x + p.x, y + p.y };
    }
    Pos& operator+=(const Pos p)
    {
        x += p.x;
        y += p.y;
        return *this;
    }
};
