#pragma once
#include "Rad/Window.h"

class DoubleBufWindow : public Window
{
protected:
    ~DoubleBufWindow()
    {
        if (m_hbmp)
            DeleteBitmap(m_hbmp);
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override
    {
        LRESULT ret = 0;
        switch (uMsg)
        {
            HANDLE_MSG(WM_ERASEBKGND, OnEraseBkgnd);
            HANDLE_MSG(WM_PAINT, OnPaint);
            HANDLE_MSG(WM_SIZE, OnSize);
        }

        if (!IsHandled())
            ret = Window::HandleMessage(uMsg, wParam, lParam);

        return ret;
    }

    BOOL OnEraseBkgnd(HDC hdc)
    {
        return FALSE;
    }

    void OnPaint()
    {
        RECT rcClient;
        CHECK_LE(GetClientRect(*this, &rcClient));

        PAINTSTRUCT ps;
        BeginPaint(*this, &ps);
        HDC hDC = ps.hdc;
        ps.hdc = CreateCompatibleDC(hDC);
        HBITMAP hOldBmp = SelectBitmap(ps.hdc, m_hbmp);
        HBRUSH hBrush = (HBRUSH) GetClassLongPtr(*this, GCLP_HBRBACKGROUND);
        if (hBrush)
            FillRect(ps.hdc, &ps.rcPaint, hBrush);
        OnDraw(&ps);
        BitBlt(hDC, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, ps.hdc, 0, 0, SRCCOPY);
        SelectBitmap(ps.hdc, hOldBmp);
        DeleteDC(ps.hdc);
        ps.hdc = hDC;
        EndPaint(*this, &ps);
    }

    void OnSize(UINT state, int cx, int cy)
    {
        SetHandled(false);

        if (m_hbmp)
            DeleteBitmap(m_hbmp);

        HDC hDC = GetDC(*this);
        m_hbmp = CreateCompatibleBitmap(hDC, cx, cy);
        ReleaseDC(*this, hDC);
    }

private:
    HBITMAP m_hbmp = NULL;
};
