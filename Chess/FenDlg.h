#pragma once

#include "Rad/Dialog.h"

#include "resource.h"

class FenDlg : public Dialog
{
public:
    INT_PTR DoModal(HWND hParent)
    {
        return Dialog::DoModal(this, IDD_FEN, hParent);
    }

    TCHAR m_fen[1024] = TEXT("");

protected:
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
    BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
    void OnClose();
    void OnCommand(int id, HWND hwndCtl, UINT codeNotify);
};
