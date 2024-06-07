#pragma once
#include <Windows.h>
#include <CommCtrl.h>

#ifdef __cplusplus
extern "C"
{
#endif

inline HWND ListView_Create(HWND hParent, DWORD dwStyle, int id)
{
    return CreateWindow(
        STATUSCLASSNAME,
        nullptr,
        dwStyle,
        0, 0, 0, 0,
        hParent,
        (HMENU) (INT_PTR) id,
        NULL,
        NULL);
}

inline void StatusBar_Reposition(HWND hWndStatusBar)
{
    SendMessage(hWndStatusBar, WM_SIZE, 0, 0);
}

inline BOOL StatusBar_SetText(HWND hWndStatusBar, LPCTSTR pStr, UINT index = 0, UINT flags = 0)
{
    return (BOOL) SendMessage(hWndStatusBar, SB_SETTEXT, index | flags, (LPARAM) pStr);
}

inline BOOL StatusBar_SetParts(HWND hWndStatusBar, CONST INT* pParts, UINT nCount)
{
    return (BOOL) SendMessage(hWndStatusBar, SB_SETPARTS, nCount, (LPARAM) pParts);
}

#ifdef __cplusplus
}
#endif
