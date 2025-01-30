#pragma once
#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

inline HGLOBAL GlobalDup(_In_ UINT uFlags, _In_ LPCVOID pData, _In_ SIZE_T dwBytes)
{
    const HGLOBAL hGlobal = ::GlobalAlloc(uFlags, dwBytes);
    if (!hGlobal)
        return NULL;

    void* pBuffer = ::GlobalLock(hGlobal);
    if (!pBuffer)
    {
        ::GlobalFree(hGlobal);
        return NULL;
    }

    memcpy(pBuffer, pData, dwBytes);
    if (!::GlobalUnlock(hGlobal) && GetLastError() != NO_ERROR)
    {
        ::GlobalFree(hGlobal);
        return NULL;
    }

    return hGlobal;
}

inline HMENU LoadPopupMenu(_In_ HINSTANCE hInstance, _In_ LPCTSTR lpMenuName)
{
    HMENU hMenu = LoadMenu(hInstance, lpMenuName);
    if (!hMenu)
        return NULL;

    HMENU hSubMenu = GetSubMenu(hMenu, 0);
    if (!hSubMenu)
    {
        DestroyMenu(hMenu);
        return NULL;
    }

    if (!RemoveMenu(hMenu, 0, MF_BYPOSITION))
    {
        DestroyMenu(hMenu);
        return NULL;
    }

    DestroyMenu(hMenu);

    return hSubMenu;
}

inline BOOL ScreenToClient(_In_ HWND hWnd, _Inout_ LPRECT lpRect)
{
    if (!ScreenToClient(hWnd, (LPPOINT) lpRect)) return FALSE;
    if (!ScreenToClient(hWnd, (LPPOINT) lpRect + 1)) return FALSE;
    return TRUE;
}

inline BOOL GetChildWindowRect(_In_ HWND hWnd, _Out_ LPRECT lpRect)
{
    if (!GetWindowRect(hWnd, lpRect)) return FALSE;
    HWND hParent = GetParent(hWnd);
    return ScreenToClient(hParent, lpRect);
}

inline LONG Width(const RECT rc)
{
    return rc.right - rc.left;
}

inline LONG Height(const RECT rc)
{
    return rc.bottom - rc.top;
}
