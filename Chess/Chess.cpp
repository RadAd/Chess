
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <CommCtrl.h>
#include <gdiplus.h>
//#include <tchar.h>
//#include <strsafe.h>

#include "Rad/Window.h"
#include "Rad/Dialog.h"
#include "Rad/Windowxx.h"
#include "Rad/WinError.h"
#include "DoubleBufWindow.h"
#include "StatusBarPlus.h"

#include "FenDlg.h"
#include "WindowsUtils.h"
#include "resource.h"

#include <algorithm>
#include <memory>
#include <map>

#include "../ChessEngine/Board.h"
#include "../ChessEngine/Moves.h"

// https://commons.wikimedia.org/wiki/File:Chess_Pieces_Sprite.svg
// https://www.behance.net/gallery/10018309/Chess-Artwork-Pieces-and-Board-Art-Assets

extern HINSTANCE g_hInstance;

std::tstring toStr(const Pos p)
{
    TCHAR s[5] = TEXT("--");
    s[0] = TEXT('a') + p.x;
    s[1] = TEXT('0') + (Board::Height - p.y);
    return s;
}

#define ID_STATUS 100

inline Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype)
{
    const HRSRC hrSrc = FindResourceW(hMod, resid, restype);     // get the handle to the resource
    if (!hrSrc)
        return nullptr;

    const DWORD dwResourceSize = SizeofResource(hMod, hrSrc);
    if (dwResourceSize <= 0)
        return nullptr;

    const HGLOBAL hGlobalResource = LoadResource(hMod, hrSrc); // load it
    if (!hGlobalResource)
        return nullptr;

    const LPCVOID imagebytes = LockResource(hGlobalResource); // get a pointer to the file bytes
    if (!imagebytes)
        return nullptr;

    // copy image bytes into a real hglobal memory handle
    const HGLOBAL hGlobal = ::GlobalDup(GHND, imagebytes, dwResourceSize);
    if (!hGlobal)
        return nullptr;

    IStream* pStream = nullptr;
    const HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
    if (FAILED(hr))
    {
        GlobalFree(hGlobal);
        return nullptr;
    }

    // pStream now owns the global handle and will invoke GlobalFree on release
    Gdiplus::Bitmap* pBmp = new Gdiplus::Bitmap(pStream);
    pStream->Release();
    return pBmp;
}

class RootWindow : public DoubleBufWindow
{
public:
    friend WindowManager<RootWindow>;
    struct Class : public MainClass
    {
        static LPCTSTR ClassName() { return TEXT("RadChess"); }

        static void GetWndClass(WNDCLASS& cs)
        {
            MainClass::GetWndClass(cs);
            //cs.hbrBackground = GetStockBrush(BLACK_BRUSH);
            cs.hbrBackground = CreateSolidBrush(RGB(128, 128, 128));
            cs.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON1));
        }
    };
public:
    static RootWindow* Create() { return WindowManager<RootWindow>::Create(NULL, TEXT("Chess")); }

protected:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    BOOL OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnDestroy();
    void OnSize(UINT state, int cx, int cy);
    void OnMouseMove(int x, int y, UINT keyFlags);
    void OnLButtonDown(int x, int y, UINT keyFlags);
    void OnLButtonUp(int x, int y, UINT keyFlags);
    void OnContextMenu(HWND hWndContext, UINT xPos, UINT yPos);
    void OnCommand(int id, HWND hWndCtl, UINT codeNotify);

    void OnDraw(const PAINTSTRUCT* pps) const override
    {
        RECT rcStatus;
        CHECK_LE(GetChildWindowRect(m_hStatus, &rcStatus));
        RECT rcClient;
        CHECK_LE(GetClientRect(*this, &rcClient));
        rcClient.bottom -= Height(rcStatus);

        const int width = std::min(Width(rcClient) / Board::Width, Height(rcClient) / Board::Height);
        const int height = width;
        const int offsetx = (Width(rcClient) - (width * Board::Width)) / 2;
        const int offsety = (Height(rcClient) - (height * Board::Height)) / 2;

        Gdiplus::Graphics g(pps->hdc);
        Gdiplus::SolidBrush b1(Gdiplus::Color(233, 237, 204));
        Gdiplus::SolidBrush b2(Gdiplus::Color(119, 153, 84));
        Gdiplus::SolidBrush bselected(Gdiplus::Color(187, 202, 42));
        Gdiplus::SolidBrush bmove(Gdiplus::Color(245, 245, 104));
        Gdiplus::SolidBrush bthreat(Gdiplus::Color(245, 0, 0));

        const bool bCapture = GetCapture() == *this;

        Pos p;
        for (p.y = 0; p.y < Board::Height; ++p.y)
            for (p.x = 0; p.x < Board::Width; ++p.x)
            {
                const Gdiplus::Rect r(offsetx + p.x * width, offsety + p.y * height, width, height);
                g.FillRectangle(p == m_mouse ? &bselected
                            : m_threats.find(p) != m_threats.end() ? &bthreat
                            : m_moves.find(p) != m_moves.end() ? &bmove
                            : (p.x + p.y) % 2 == 0 ? &b1
                            : &b2, r);
            }
        auto itMovingPiece = m_cbmp.end();
        for (p.y = 0; p.y < Board::Height; ++p.y)
            for (p.x = 0; p.x < Board::Width; ++p.x)
            {
                const PieceDef pd = m_board.GetPiece(p);
                auto itPiece = m_cbmp.find(pd);
                if (itPiece != m_cbmp.end())
                    if (!bCapture || p != m_mouse)
                    {
                        const Gdiplus::Point pt(offsetx + p.x * width, offsety + p.y * height);
                        g.DrawCachedBitmap(itPiece->second.get(), pt.X, pt.Y);
                    }
                    else
                    {
                        itMovingPiece = itPiece;
                    }
            }
        if (itMovingPiece != m_cbmp.end())
        {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(*this, &pt);
            g.DrawCachedBitmap(itMovingPiece->second.get(), pt.x - width / 2, pt.y - height / 2);
        }
    }

    Pos GetBoardPos(int x, int y) const
    {
        RECT rcStatus;
        CHECK_LE(GetChildWindowRect(m_hStatus, &rcStatus));
        RECT rcClient;
        CHECK_LE(GetClientRect(*this, &rcClient));
        rcClient.bottom -= Height(rcStatus);

        const int width = std::min(Width(rcClient) / Board::Width, Height(rcClient) / Board::Height);
        const int height = width;
        const int offsetx = (Width(rcClient) - (width * Board::Width)) / 2;
        const int offsety = (Height(rcClient) - (height * Board::Height)) / 2;

        if ((x - offsetx) < 0)
            x -= width;
        if ((y - offsety) < 0)
            y -= height;

        return { (x - offsetx) / width, (y - offsety) / height };
    }

    void SetColour(Colour colour)
    {
        m_colour = colour;
        StatusBar_SetText(m_hStatus, m_colour == Colour::White ? TEXT("White") : TEXT("Black"), 2);
        StatusBar_SetText(m_hStatus, IsInCheck(m_board, m_colour) ? IsInCheckMate(m_board, m_colour) ? TEXT("Checkmate") : TEXT("Check") : TEXT(""), 3);
    }

    Board m_board = Board::Create();
    Colour m_colour = Colour::White;
    Pos m_mouse;
    std::set<Pos> m_moves;
    std::set<Pos> m_threats;

    std::unique_ptr<Gdiplus::Bitmap> m_bmp;
    std::map<PieceDef, std::unique_ptr<Gdiplus::CachedBitmap>> m_cbmp;

    HWND m_hStatus = NULL;
};

BOOL RootWindow::OnCreate(const LPCREATESTRUCT lpCreateStruct)
{
    //m_bmp.reset(Gdiplus::Bitmap::FromResource(g_hInstance, MAKEINTRESOURCE(IDB_PNG1)));
    m_bmp.reset(LoadImageFromResource(g_hInstance, MAKEINTRESOURCE(IDB_PNG3), L"PNG"));
    //m_bmp.reset(Gdiplus::Bitmap::FromFile(L"Res\\270px-Chess_Pieces_Sprite.svg.png"));
    _ASSERTE(m_bmp);

    m_hStatus = ListView_Create(*this, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE, ID_STATUS);
    StatusBar_SetText(m_hStatus, TEXT("Ready"), 0, SBT_NOBORDERS);

    return TRUE;
}

void RootWindow::OnDestroy()
{
    PostQuitMessage(0);
}

void RootWindow::OnSize(const UINT state, const int cx, const int cy)
{
    if (cx <= 0 || cy <= 0)
        return;

    DoubleBufWindow::OnSize(state, cx, cy);

    StatusBar_Reposition(m_hStatus);
    RECT rcStatus;
    CHECK_LE(GetChildWindowRect(m_hStatus, &rcStatus));

    const int parts_size[] = { -1, 100, 100, 100 };
    int parts_offset[ARRAYSIZE(parts_size)] = {};
    int offset = Width(rcStatus);
    for (int i = ARRAYSIZE(parts_size) - 1; i >= 0; --i)
    {
        parts_offset[i] = offset;
        offset -= parts_size[i] >= 0 ? parts_size[i] : offset;
    }
    StatusBar_SetParts(m_hStatus, parts_offset, ARRAYSIZE(parts_offset));
    SetColour(m_colour);

    const int width = std::min(cx / Board::Width, cy / Board::Height);
    const int height = width;

    const Gdiplus::PixelFormat pf = m_bmp->GetPixelFormat();
    Gdiplus::Bitmap tmpbmp(width * 6, height * 2, pf);

    {
        Gdiplus::Graphics gbmp(&tmpbmp);
        //gbmp.ScaleTransform(scalewidth, scaleheight);
        gbmp.DrawImage(m_bmp.get(), 0, 0, width * 6, height * 2);
    }

    Gdiplus::Graphics g(*this);
    int y = 0;
    for (auto c : { Colour::White, Colour::Black })
    {
        int x = 5;
        for (auto p : AllPieces)
        {
            std::unique_ptr<Gdiplus::Bitmap> subbmp(tmpbmp.Clone(x * width, y * height, width, height, pf));
            _ASSERTE(subbmp);
            m_cbmp[{ c, p }].reset(new Gdiplus::CachedBitmap(subbmp.get(), &g));
            --x;
        }
        ++y;
    }

    CHECK_LE(InvalidateRect(*this, nullptr, TRUE));
}

void RootWindow::OnMouseMove(int x, int y, UINT keyFlags)
{
    const Pos p = GetBoardPos(x, y);
    if (GetCapture() != *this)
    {
        if (m_mouse != p)
        {
            m_mouse = p;
            //if (m_board.GetPiece(p).c != Colour::None)
            if (m_board.GetPiece(p).c == m_colour)
                m_moves = GetMoves(m_board, p);
            else
                m_moves.clear();
            StatusBar_SetText(m_hStatus, m_board.Valid(m_mouse) ? toStr(m_mouse).c_str() : TEXT(""), 1);
            CHECK_LE(InvalidateRect(*this, nullptr, TRUE));
        }
    }
    else
    {
        if (Board::Valid(p))
        {
            const Board nb = DoMove(m_board, m_mouse, p);
            m_threats = GetThreats(nb, p);
            StatusBar_SetText(m_hStatus, m_board.Valid(p) ? (toStr(m_mouse) + TEXT(" -> ") + toStr(p)).c_str() : toStr(m_mouse).c_str(), 1);
        }
        else
        {
            m_threats.clear();
        }
        CHECK_LE(InvalidateRect(*this, nullptr, TRUE));
    }
}

void RootWindow::OnLButtonDown(int x, int y, UINT keyFlags)
{
    SetCapture(*this);
    CHECK_LE(InvalidateRect(*this, nullptr, TRUE));
}

void RootWindow::OnLButtonUp(int x, int y, UINT keyFlags)
{
    if (GetCapture() == *this)
    {
        const Pos p = GetBoardPos(x, y);
        if (m_mouse != p && (keyFlags & MK_CONTROL || m_moves.find(p) != m_moves.end()))
        {
            m_board = DoMove(m_board, m_mouse, p);
            SetColour(OtherColour(m_colour));
            m_moves.clear();
        }
        m_threats.clear();
        CHECK_LE(InvalidateRect(*this, nullptr, TRUE));
    }
    ReleaseCapture();
}

void RootWindow::OnContextMenu(HWND hWndContext, UINT xPos, UINT yPos)
{
    HMENU hMenu = LoadPopupMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENU1));
    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, xPos, yPos, 0, *this, nullptr);
    DestroyMenu(hMenu);
}

void RootWindow::OnCommand(int id, HWND hWndCtl, UINT codeNotify)
{
    switch (id)
    {
    case ID_GAME_RESTART:
        m_board = Board::Create();
        SetColour(Colour::White);
        CHECK_LE(InvalidateRect(*this, nullptr, TRUE));
        break;

    case ID_GAME_FEN:
    {
        FenDlg fendlg;
        if (fendlg.DoModal(*this) == IDOK)
        {
            m_board = Board::CreateFromFen(fendlg.m_fen, m_colour);
            SetColour(m_colour);
            CHECK_LE(InvalidateRect(*this, nullptr, TRUE));
        }
        break;
    }
    }
}

LRESULT RootWindow::HandleMessage(const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    LRESULT ret = 0;
    switch (uMsg)
    {
        HANDLE_MSG(WM_CREATE, OnCreate);
        HANDLE_MSG(WM_DESTROY, OnDestroy);
        HANDLE_MSG(WM_SIZE, OnSize);
        HANDLE_MSG(WM_MOUSEMOVE, OnMouseMove);
        HANDLE_MSG(WM_LBUTTONDOWN, OnLButtonDown);
        HANDLE_MSG(WM_LBUTTONUP, OnLButtonUp);
        HANDLE_MSG(WM_CONTEXTMENU, OnContextMenu);
        HANDLE_MSG(WM_COMMAND, OnCommand);
    }

    if (!IsHandled())
        ret = DoubleBufWindow::HandleMessage(uMsg, wParam, lParam);

    return ret;
}

bool Run(_In_ const LPCTSTR lpCmdLine, _In_ const int nShowCmd)
{
    RadLogInitWnd(NULL, "Chess", L"Chess");

    //CHECK_LE_RET(Register<MainClass>(), false);
    CHECK_LE_RET(Register<RootWindow::Class>(), false);

    RootWindow* prw = RootWindow::Create();
    CHECK_LE_RET(prw != nullptr, false);

    RadLogInitWnd(*prw, nullptr, nullptr);
    ShowWindow(*prw, nShowCmd);

    return true;
}
