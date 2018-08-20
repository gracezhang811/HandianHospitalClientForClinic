#pragma once
#include "duilib.h"


class CDiagModifyClincInfoFrameWnd : public WindowImplBase
{
protected:
	virtual ~CDiagModifyClincInfoFrameWnd() {};

public:
	CDiagModifyClincInfoFrameWnd() { };
	virtual LPCTSTR		GetWindowClassName() const { return _T("DiagModifyClincInfoFrame"); };
	virtual CDuiString	GetSkinFolder() { return _T("skin"); }
	virtual CDuiString	GetSkinFile() { return _T("DiagModifyClinicInfo.xml"); }
	virtual void		OnFinalMessage(HWND hWnd) { delete this; }
	virtual LRESULT		OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void		Init(HWND hWndParent, POINT ptPos);
	void		Notify(TNotifyUI& msg);
	LRESULT		HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};



