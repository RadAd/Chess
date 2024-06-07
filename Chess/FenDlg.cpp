#include "FenDlg.h"

#include "Rad/Windowxx.h"

BOOL FenDlg::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
    //ShowWindow(*this, SW_SHOW);
    return TRUE;
}

void FenDlg::OnClose()
{
    if (IsModal())
        EndDialog(*this, IDCANCEL);
    else
        DestroyWindow(*this);
}

void FenDlg::OnCommand(int id, HWND hWndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
        GetDlgItemText(*this, IDC_FEN, m_fen, ARRAYSIZE(m_fen));
    case IDCANCEL:
        EndDialog(*this, id);
        break;

    default:
        SetHandled(false);
        break;
    }
}

LRESULT FenDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 0;
    switch (uMsg)
    {
        HANDLE_MSG(WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(WM_CLOSE, OnClose);
        HANDLE_MSG(WM_COMMAND, OnCommand);
    }

    return IsHandled() ? ret : Dialog::HandleMessage(uMsg, wParam, lParam);
}
