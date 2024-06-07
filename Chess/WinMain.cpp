#define STRICT
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
//#include <objbase.h>
#include <commctrl.h>
#include <gdiplus.h>

HINSTANCE g_hInstance = NULL;
HACCEL g_hAccelTable = NULL;
HWND g_hWndAccel = NULL;
HWND g_hWndDlg = NULL;

//#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

bool Run(_In_ const LPCTSTR lpCmdLine, _In_ const int nShowCmd);

int DoMessageLoop()
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if ((g_hWndDlg == NULL || !IsDialogMessage(g_hWndDlg, &msg))
            && (g_hAccelTable == NULL || !TranslateAccelerator(g_hWndAccel, g_hAccelTable, &msg)))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return int(msg.wParam);
}

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
    int ret = 0;
    g_hInstance = hInstance;
#ifdef _OBJBASE_H_  // from objbase.h
    if (SUCCEEDED(CoInitialize(nullptr)))
#endif
    {
#ifdef _INC_COMMCTRL    // from commctrl.h
        InitCommonControls();
#endif
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        if (Run(lpCmdLine, nShowCmd))
            ret = DoMessageLoop();

        Gdiplus::GdiplusShutdown(gdiplusToken);
#ifdef _OBJBASE_H_
        CoUninitialize();
#endif
    }
    return ret;
}
