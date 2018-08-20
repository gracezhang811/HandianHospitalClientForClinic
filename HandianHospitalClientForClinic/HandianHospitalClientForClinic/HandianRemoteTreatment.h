#pragma once

#include "Resource.h"
#include "duilib.h"
#include "zhumu_sdk.h"
#include "def.h"
#include "HDHttpFunction.h"
#include "HttpUploadFiles.h"
#include "DiagToRecordConsultation.h"
#include "DiagModifyClincInfoHint.h"
#include "DiagShowPatientBigPic.h"
#include "ListItemButtonUI.h"
#include "AVPlayer.h"
#include "vlc.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "myvoice.h"
#ifdef __cplusplus
}
#endif

class CAuthServiceMgr;
class CMeetingServiceMgr;


class CDuiFrameWnd : public WindowImplBase
{
public:
	CDuiFrameWnd();
	virtual ~CDuiFrameWnd();

public:
	virtual LPCTSTR		GetWindowClassName() const { return _T("DUIMainFrame"); };
	virtual CDuiString	GetSkinFile() { return _T("MainPage.xml"); };
	virtual CDuiString	GetSkinFolder() { return _T("../skin"); };

	virtual void		InitWindow();
	virtual LRESULT		HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/);
	void				InitSimpleData();
	LRESULT				OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT				OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void				OpenFileDialog(CButtonUI* PicControl, unsigned __int64 filesize);
	virtual CControlUI* CreateControl(LPCTSTR pstrClassName);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void		OnClick(TNotifyUI& msg);
	virtual void		OnSelectChanged(TNotifyUI &msg);
	virtual void		OnItemSelect(TNotifyUI &msg);
	virtual void		OnItemButtonClick(TNotifyUI &msg);

private:
	void				InitAllControls();
	void				ResetAllControls();
	bool				SDKAuth();
	bool				SDKJoinMeeting();

private:
	int					HttpLogin(LPCTSTR username, LPCTSTR pwd);
	int					HttpGetExpertList();
	void				HttpGetAppointmentList(LPCTSTR curl, int pagenum);
	void				HttpGetRecordList(LPCTSTR curl, int pagenum);
	void				HttpGetCurrentAppointmentList(LPCTSTR curl);
	void				HttpGetUnPayInfoList(LPCTSTR curl, int pagenum);
	void				HttpGetAllPayInfoList(LPCTSTR curl, int pagenum);
	void				HttpGetExpertWorkTimeList();
	int					HttpGetUserInfo();
	int					HttpGetBillInfo();
	void				HttpGetAppointmentDetail(LPCTSTR appointmentnumber);
	int					HttpGetUserPayStatus();
	void				HttpGetFreeTime(LPCTSTR expert, LPCTSTR choosedate);
	void				HttpGetFreeTimeOnEditAM(LPCTSTR expert, LPCTSTR choosedate);
	int					HttpCreateAppointment(CStringA poststr);
	int					HttpCancelAppointment(LPCTSTR appointmentnumber);
	int					HttpGetAppointmentUpdate(LPCTSTR appointmentnumber);
	int					HttpUpdateAppointment(CStringA poststr);
	void				HttpGetRecordDetail(LPCTSTR appointmentnumber);
	void				HttpGetRecordUpdate(LPCTSTR appointmentnumber);
	int					HttpUpdateRecord(CStringA poststr);
	int					HttpGetExpertInfo();
	int					HttpGetClinicInfo();
	int					HttpGetUserInfoUpdate();
	int					HttpGetClinicInfoUpdate();
	int					HttpCheckClinicUpdatePermit();
	int					HttpUpdateUserInfo(CStringA poststr);
	int					HttpUpdateClinicInfo(CStringA poststr);
	int					HttpSignUpUser(CStringA poststr);
	int					HttpSignUpClinic(CStringA poststr);
	int					HttpSendCode(LPCTSTR mobile);
	int					HttpResetPassword(CStringA poststr);
	int					HttpGetMeetingNumber(LPCTSTR appointmentnumber);
	//int					HttpSendMeetingStartMsg(CStringA poststr);
	Json::Value			HttpUploadClinicIMG(CStringW path);
	Json::Value			HttpUploadPatientIMG(CStringW path);
	void				HDWriteLog(CStringA logmsg);


private:
	CAuthServiceMgr*		m_pAuthServiceMgr;
	CMeetingServiceMgr*		m_pMeetingServiceMgr;
	CAVPlayer*				m_pAVPlayer;
	bool					m_bSDKInit;
//	bool					m_bZhumuAuth;

private:
	CDiagToRecordConsultationWnd*	m_pToRecordConsultationWnd;
	CDiagModifyClincInfoFrameWnd*	m_pModifyClincInfoFrameWnd;
	CDiagShowPatientBigPicWnd*		m_pShowPatientBigPicWnd;

private:
	CContainerUI*			m_containerTitleUI;
	CContainerUI*			m_containerLoginUI;
	CContainerUI*			m_containerResetPwdUI;
	CContainerUI*			m_containerSignUpUI;
	CContainerUI*			m_containerSignInfoUI;
	CContainerUI*			m_containerReSignInfoUI;
	CContainerUI*			m_containerAuditingUI;

	CContainerUI*			m_containerMainpageUI;

	CContainerUI*			m_diagCancelApppontmentUI;
	CContainerUI*			m_diagArrearageUI;

	CContainerUI*			m_containerAppointmentListUI;
	CContainerUI*			m_containerToAppointUI;
	CContainerUI*			m_containerAppointmentDetailUI;
	CContainerUI*			m_containerEditAppointmentUI;

	CContainerUI*			m_containerPriceOfFeeOnToAMUI;
	CContainerUI*			m_containerPriceOfScoreOnToAMUI;
	CContainerUI*			m_containerPriceOfFeeOnEditAMUI;
	CContainerUI*			m_containerPriceOfScoreOnEditAMUI;

	CTileLayoutUI*			m_tileFreeTimeOfMorningAppointUI;
	CTileLayoutUI*			m_tileFreeTimeOfAfternoonAppointUI;
	CTileLayoutUI*			m_tileFreeTimeOfNightAppointUI;
	CTileLayoutUI*			m_tileFreeTimeOfEditMorningAppointUI;
	CTileLayoutUI*			m_tileFreeTimeOfEditAfternoonAppointUI;
	CTileLayoutUI*			m_tileFreeTimeOfEditNightAppointUI;

	CHorizontalLayoutUI*	m_layFreeTimeOfMorningAppointUI;
	CHorizontalLayoutUI*	m_layeFreeTimeOfAfternoonAppointUI;
	CHorizontalLayoutUI*	m_layFreeTimeOfNightAppointUI;
	CHorizontalLayoutUI*	m_layFreeTimeOfMorningAppointOnEditAMUI;
	CHorizontalLayoutUI*	m_layeFreeTimeOfAfternoonAppointOnEditAMUI;
	CHorizontalLayoutUI*	m_layFreeTimeOfNightAppointOnEditAMUI;

	CTileLayoutUI*			m_tileExpertInfoUI;

	CContainerUI*			m_containerRecordListUI;
	CContainerUI*			m_containerRecordDetailUI;
	CContainerUI*			m_containerEditRecordUI;

	CContainerUI*			m_containerShowClinicInfoUI;
	CContainerUI*			m_containerEditClinicInfoUI;
	CContainerUI*			m_containerShowUserInfoUI;
	CContainerUI*			m_containerEditUserInfoUI;

	CLabelUI*				m_labTitleClinicNameUI;
	CLabelUI*				m_labTitleClinicImageUI;

	CTabLayoutUI*           m_tabMainControlUI;
	CTabLayoutUI*           m_tabPatientInfoControlUI;
	CTabLayoutUI*           m_tabPatientInfoOnDetailControlUI;
	CTabLayoutUI*           m_tabPatientInfoOnEditControlUI;
	CTabLayoutUI*           m_tabPatientInfoOnRecordControlUI;
	CTabLayoutUI*           m_tabPatientInfoOnEditRecordControlUI;
	CTabLayoutUI*           m_tabPayTypeControlUI;
	CTabLayoutUI*           m_tabDoctorInfoTypeControlUI;
	CTabLayoutUI*           m_tabClinicInfoControlUI;

	CListUI*				m_listAppointmentUI;
	CListUI*				m_listRecordUI;
	CListUI*				m_listCAppointmentUI;
	CListUI*				m_listDoctorWorkInfoUI;
	CListUI*				m_listUnpayUI;
	CListUI*				m_listAllpayUI;

	CButtonUI*				m_btnCloseUI;
	CButtonUI*				m_btnWndMinUI;
	CButtonUI*				m_btnCloseEXEUI;
	CButtonUI*				m_btnLoginUI;

	CButtonUI*				m_btnUploadLicenseUI;
	CButtonUI*				m_btnUploadPhotoUI;
	CButtonUI*				m_btnUploadDoctorLicenseUI;

	CButtonUI*				m_btnReUploadLicenseUI;
	CButtonUI*				m_btnReUploadPhotoUI;
	CButtonUI*				m_btnReUploadDoctorLicenseUI;

	CButtonUI*				m_btnUploadNewLicenseUI;
	CButtonUI*				m_btnUploadNewPhotoUI;
	CButtonUI*				m_btnUploadNewDoctorLicenseUI;

	CButtonUI*				m_btnUploadPatientPic1UI;
	CButtonUI*				m_btnUploadPatientPic2UI;
	CButtonUI*				m_btnUploadPatientPic3UI;
	CButtonUI*				m_btnUploadNewPatientPic1UI;
	CButtonUI*				m_btnUploadNewPatientPic2UI;
	CButtonUI*				m_btnUploadNewPatientPic3UI;
	CButtonUI*				m_btnShowPatientPic1UIOnAMDetail;
	CButtonUI*				m_btnShowPatientPic2UIOnAMDetail;
	CButtonUI*				m_btnShowPatientPic3UIOnAMDetail;
	CButtonUI*				m_btnShowPatientPic1UIOnRDDetail;
	CButtonUI*				m_btnShowPatientPic2UIOnRDDetail;
	CButtonUI*				m_btnShowPatientPic3UIOnRDDetail;

	CButtonUI*				m_btnPlayAudio;
	CButtonUI*				m_btnPauseAudio;
	CButtonUI*				m_btnStopAudio;

	CComboUI*				m_cboChoosePayTypeUI;
	CComboUI*				m_cboEditChoosePayTypeUI;
	CComboUI*				m_cboChooseExpertOnSearchAppoint;
	CComboUI*				m_cboChooseExpertOnToAppoint;
	CComboUI*				m_cboChooseExpertOnEditAppoint;
	CComboUI*				m_cboChooseExpertOnSearchRecord;

	CActiveXUI*				pActiveXUI;

	CEditUI*				m_editMobileOnLoginUI;
	CEditUI*				m_editPWDOnLoginUI;

	CDateTimeUI*			m_dataChooseOnSearchAppoint;
	CDateTimeUI*			m_dataChooseOnToAppoint;
	CDateTimeUI*			m_dataChooseOnEditAppoint;
	CDateTimeUI*			m_dataChooseOnSearchRecord;

	COptionUI*				m_optionCheckAppointment;
	COptionUI*				m_optionConsultationRecord;
	COptionUI*				m_optionCurrentConsultation;
	COptionUI*				m_optionPaymentRecord;
	COptionUI*				m_optionDoctorInfo;
	COptionUI*				m_optionClinicInfo;

	COptionUI*				m_optionPatientConditionPageOnToAM;
	COptionUI*				m_optionPatientPicPageOnToAM;
	COptionUI*				m_optionPatientConditionPageOnShowAM;
	COptionUI*				m_optionPatientPicPageOnShowAM;
	COptionUI*				m_optionPatientConditionPageOnEditAM;
	COptionUI*				m_optionPatientPicPageOnEditAM;

	COptionUI*				m_optionPatientConditionPageOnShowRecord;
	COptionUI*				m_optionPatientPicPageOnShowRecord;
	COptionUI*				m_optionPatientConditionPageOnEditRecord;
	COptionUI*				m_optionPatientPicPageOnEditRecord;

	COptionUI*				m_optionUnPay;
	COptionUI*				m_optionAllPay;
	COptionUI*				m_optionBill;
	COptionUI*				m_optionWeixinCode;

	COptionUI*				m_optionExpertTileInfo;
	COptionUI*				m_optionDoctorWorkTime;

	COptionUI*				m_optionUserInfo;
	COptionUI*				m_optionClinicSignInfo;

	COptionUI*				m_optionChooseMaleOnEditRecord;
	COptionUI*				m_optionChooseFemaleOnEditRecord;

	CLabelUI*				m_labWaitMeetingStart;
	//CLabelUI*				m_labPatientConditionHintOnEditAM;
	//CLabelUI*				m_labPatientConditionHintOnToAM;
	//CLabelUI*				m_labPatientImgHintOnEditAM;
	//CLabelUI*				m_labPatientImgHintOnToAM;

	CLabelUI*				m_labPageNumOnAppoint;
	CLabelUI*				m_labCurrentPageNumOnAppoint;
	CLabelUI*				m_labPageNumOnRecord;
	CLabelUI*				m_labCurrentPageNumOnRecord;
	CLabelUI*				m_labPageNumOnUnPay;
	CLabelUI*				m_labCurrentPageNumOnUnPay;
	CLabelUI*				m_labPageNumOnAllPay;
	CLabelUI*				m_labCurrentPageNumOnAllPay;

	CActiveXUI*				p_showCodeImageActiveXUI;

};
