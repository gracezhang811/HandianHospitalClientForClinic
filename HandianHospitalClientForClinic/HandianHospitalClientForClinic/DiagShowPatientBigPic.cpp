#include "stdafx.h"
#include "DiagShowPatientBigPic.h"



LRESULT  CDiagShowPatientBigPicWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Close();
	bHandled = FALSE;
	return 0;
}

void CDiagShowPatientBigPicWnd::Init(HWND hWndParent, POINT ptPos)
{	
	Create(NULL, _T("²é¿´Í¼Ïñ"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	//Create(hWndParent, _T("RecordConsulltationFrame"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	::ClientToScreen(hWndParent, &ptPos);
	::SetWindowPos(*this, NULL, ptPos.x, ptPos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	p_showPatientPicActiveXUI = static_cast<CActiveXUI*>(m_PaintManager.FindControl(_T("PatientBigPic")));

}


void CDiagShowPatientBigPicWnd::InitData(BSTR webUrl) {
	if (p_showPatientPicActiveXUI)
	{
		IWebBrowser2* pWebBrowser = NULL;
		p_showPatientPicActiveXUI->SetDelayCreate(false);
		p_showPatientPicActiveXUI->CreateControl(CLSID_WebBrowser);
		p_showPatientPicActiveXUI->GetControl(IID_IWebBrowser2, (void**)&pWebBrowser);

		if (pWebBrowser != NULL)
		{
			pWebBrowser->Navigate(webUrl, NULL, NULL, NULL, NULL);
			pWebBrowser->Release();
		}
	}
}
void	CDiagShowPatientBigPicWnd::Notify(TNotifyUI& msg) {
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("ButtonClosePicDiag"))
		{
			Close();
			return;
		}
	}
}

LRESULT CDiagShowPatientBigPicWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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









