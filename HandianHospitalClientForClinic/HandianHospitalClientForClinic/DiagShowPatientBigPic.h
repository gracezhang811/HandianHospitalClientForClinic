#pragma once

#include "duilib.h"



class CDiagShowPatientBigPicWnd : public WindowImplBase
{
protected:
	virtual ~CDiagShowPatientBigPicWnd() {};

public:
	CDiagShowPatientBigPicWnd() { };
	virtual LPCTSTR		GetWindowClassName() const { return _T("ShowPatientBigPicFrame"); };
	virtual CDuiString	GetSkinFolder() { return _T("skin"); }
	virtual CDuiString	GetSkinFile() { return _T("DiagShowPatientBigPic.xml"); }
	virtual void		OnFinalMessage(HWND hWnd) { delete this; }
	virtual LRESULT		OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void		Init(HWND hWndParent, POINT ptPos);
	void		InitData(BSTR webUrl);
	void		Notify(TNotifyUI& msg);
	LRESULT		HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CActiveXUI*				p_showPatientPicActiveXUI;
};

