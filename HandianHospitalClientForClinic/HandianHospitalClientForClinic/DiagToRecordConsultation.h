#pragma once

#include "Resource.h"
#include "duilib.h"
#include "def.h"
#include "json.h"
#include "HDHttpFunction.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "myvoice.h"
#ifdef __cplusplus
}
#endif

class CDiagToRecordConsultationWnd : public WindowImplBase
{
protected:
	virtual ~CDiagToRecordConsultationWnd() {};

public:
	CDiagToRecordConsultationWnd() { };
	virtual LPCTSTR		GetWindowClassName() const { return _T("RecordConsulltationFrame"); };
	virtual CDuiString	GetSkinFolder() { return _T("skin"); }
	virtual CDuiString	GetSkinFile() { return _T("DiagRecordConsulltation.xml"); }
	virtual void		OnFinalMessage(HWND hWnd) { delete this; }
	virtual LRESULT		OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void				Init(HWND hWndParent, POINT ptPos);
	void				Notify(TNotifyUI& msg);
	LRESULT				HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT				MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
	void				InitRecordData(LPCTSTR appointmentnumber, CStringA token);
	LRESULT				OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT				OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	void				HttpGetRecordUpdate(LPCTSTR appointmentnumber, CStringA token);
	int					HttpUpdateRecord(CStringA poststr, CStringA token);

private:
	void				InitAllControls();
	void				ResetAllControls();

private:
	CTabLayoutUI*           m_tabPatientInfoOnToRecordControlUI;
	COptionUI*				m_optionChooseMaleOnToRecord;
	COptionUI*				m_optionChooseFemaleOnToRecord;
	COptionUI*				m_optionPatientConditionPageOnToRecord;
	COptionUI*				m_optionPatientPicPageOnToRecord;
	CRichEditUI*			m_richeditDiagnosisDescOnToRecord;
	CButtonUI*				m_btnStartVoiceOnToRecord;
	CButtonUI*				m_btnEndVoiceOnToRecord;
	char*					m_pVoiceResult;
};

