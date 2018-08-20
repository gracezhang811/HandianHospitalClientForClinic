#include "stdafx.h"
#include "DiagModifyClincInfoHint.h"

LRESULT  CDiagModifyClincInfoFrameWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Close();
	bHandled = FALSE;
	return 0;
}

void CDiagModifyClincInfoFrameWnd::Init(HWND hWndParent, POINT ptPos)
{
	Create(hWndParent, _T("DiagModifyClincInfoFrame"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	::ClientToScreen(hWndParent, &ptPos);
	::SetWindowPos(*this, NULL, ptPos.x, ptPos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void	CDiagModifyClincInfoFrameWnd::Notify(TNotifyUI& msg) {
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("ButtonConfirmModifyClinicInfoHint"))
		{
			Close();
			return;
		}
	}
}

LRESULT CDiagModifyClincInfoFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL    bHandled = TRUE;

	switch (uMsg)
	{
	case WM_KILLFOCUS:
		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		break;
	default:
		bHandled = FALSE;
	}

	if (bHandled || m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
	{
		return lRes;
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}










