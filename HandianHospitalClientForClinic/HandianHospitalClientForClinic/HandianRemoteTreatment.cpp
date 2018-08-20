#include "stdafx.h"
#include "HandianRemoteTreatment.h"
#include "AuthServiceMgr.h"
#include "MeetingServiceMgr.h"
#include "RWinHttp.h"
#include "json.h"
#include "HttpClientSyn.h"


CStringW ClinicUserName = L"诊所";
CStringA token("");
Json::Value expertlist;
int expertnum;
CStringA editappointmentno("");
CStringA editrecordappointmentno("");
CStringW rtmessage("");
CStringW zhumukey("");
CStringW zhumusecret("");
CStringW meetingnum;
//int inmeeting = 0;
int inplaying = 0;
int doctorinfohtml = 0;
int startmeetingfailed = 0;

int appointlisttotalpagenum = 1;
int appointlistcurrentpagenum = 1;
CStringW appointsearchtext = L"";
int recordlisttotalpagenum = 1;
int recordlistcurrentpagenum = 1;
CStringW recordsearchtext = L"";
int unpaylisttotalpagenum = 1;
int unpaylistcurrentpagenum = 1;
int allpaylisttotalpagenum = 1;
int allpaylistcurrentpagenum = 1;

HANDLE hFile;

DUI_BEGIN_MESSAGE_MAP(CDuiFrameWnd, WindowImplBase)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMSELECT, OnItemSelect)
DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, OnSelectChanged)
DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMBUTTONCLICK, OnItemButtonClick)
DUI_END_MESSAGE_MAP()

CDuiFrameWnd::CDuiFrameWnd()
{
	ResetAllControls();
	m_pAuthServiceMgr = new CAuthServiceMgr();
	m_pMeetingServiceMgr = new CMeetingServiceMgr();	
	m_pAVPlayer = new CAVPlayer();
	m_bSDKInit = false;
}

CDuiFrameWnd::~CDuiFrameWnd() {
	if (m_pAuthServiceMgr)
	{
		delete m_pAuthServiceMgr;
		m_pAuthServiceMgr = NULL;
	}
	if (m_pMeetingServiceMgr)
	{
		delete m_pMeetingServiceMgr;
		m_pMeetingServiceMgr = NULL;
	}
	if (m_pAVPlayer)
	{
		delete m_pAVPlayer;
		m_pAVPlayer = NULL;
	}
	m_bSDKInit = false;
};


void CDuiFrameWnd::OpenFileDialog(CButtonUI* PicControl, unsigned __int64 filesize)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH] = _T("");

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = *this;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = STR_FILE_FILTER;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	PicControl->SetUserData(L"");
	if (GetOpenFileName(&ofn))
	{
		std::vector<wstring> vctString(1, szFile);
		HANDLE handle = CreateFile(vctString[0].c_str(), FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (handle != INVALID_HANDLE_VALUE)
		{
			unsigned __int64 size = GetFileSize(handle, NULL);		
			if (size <= filesize) {
				PicControl->SetBkImage(vctString[0].c_str());
				PicControl->SetUserData(vctString[0].c_str());
				PicControl->SetTextColor(0xffffffff);
			}
			else {
				::MessageBox(*this, _T("选择图像大小超过限制。"), _T("提示"), 0);
			}

		}
		OutputDebugStringW(vctString[0].c_str());
	}
}

void CDuiFrameWnd::HDWriteLog(CStringA logmsg) 
{
	DWORD dwWritenSize = 0;
	BOOL bRet = WriteFile(hFile, logmsg, logmsg.GetLength(), &dwWritenSize, NULL);
	if (!bRet)
	{
		OutputDebugStringA("WriteFile 写文件失败\r\n");
	}
	//刷新文件缓冲区  
	FlushFileBuffers(hFile);
}

bool CDuiFrameWnd::SDKAuth()
{
	std::wstring strWebDomain = TEST_ZOOM_WEBDOMAIN;
	std::wstring strKey = TEST_ZOOM_KEY;
	std::wstring strSecret = TEST_ZOOM_SECRET;
	if (strWebDomain.length() <= 0 || strKey.length() <= 0 || strSecret.length() <= 0)
		return false;

	if (!m_bSDKInit)
	{
		ZHUMU_SDK_NAMESPACE::InitParam initParam;
		initParam.strWebDomain = strWebDomain.c_str();
		if (m_pAuthServiceMgr)
			m_bSDKInit = m_pAuthServiceMgr->Init(&initParam);
			if (!m_bSDKInit)
			{
				return false;
			}

			ZHUMU_SDK_NAMESPACE::AuthParam authParam;
			authParam.appKey = strKey.c_str();
			authParam.appSecret = strSecret.c_str();

			return m_pAuthServiceMgr->SDKAuth(authParam);
	}
	else {
		return false;
	}
}


bool CDuiFrameWnd::SDKJoinMeeting() {
	if (!m_pMeetingServiceMgr->Init())
	{
		OutputDebugStringA("MeetingServiceInit失败！\n");
		m_labWaitMeetingStart->SetVisible(false);
		startmeetingfailed = 1;
		CStringA logmsg = "初始化会议失败。\r\n";
		HDWriteLog(logmsg);
		return false;
	}
	else {
		OutputDebugStringA("MeetingServiceInit成功\n");
		ZHUMU_SDK_NAMESPACE::JoinParam joinParam;
		//joinParam.userType = ZHUMU_SDK_NAMESPACE::SDK_UT_NORMALUSER;
		//ZHUMU_SDK_NAMESPACE::JoinParam4NormalUser& normalParam = joinParam.param.normaluserJoin;
		joinParam.userType = ZHUMU_SDK_NAMESPACE::SDK_UT_APIUSER;
		ZHUMU_SDK_NAMESPACE::JoinParam4APIUser& normalParam = joinParam.param.apiuserJoin;

		std::wstring strMeetingNumber = meetingnum;
		normalParam.meetingNumber = _wtoi64(strMeetingNumber.c_str());
		normalParam.userName = ClinicUserName;
		normalParam.isDirectShareDesktop = false;///direct share desktop or not when you start meeting
		normalParam.hDirectShareAppWnd = NULL;///direct share window or not when you start meeting
		//bool bJoin = m_pMeetingServiceMgr->Join(joinParam);
		if (m_pMeetingServiceMgr->Join(joinParam)) {
			OutputDebugStringA("加入会议！\n");	
			CStringA logmsg = "加入会议成功。\r\n";
			HDWriteLog(logmsg);
			m_labWaitMeetingStart->SetVisible(false);
			//inmeeting = 1;
			startmeetingfailed = 0;
			return true;
		}
		else {
			OutputDebugStringA("加入会议失败！！\n");
			CStringA logmsg = "加入会议失败。\r\n";
			HDWriteLog(logmsg);
			m_labWaitMeetingStart->SetVisible(false);
			startmeetingfailed = 1;
			return false;
		}
	}
}

void CDuiFrameWnd::InitAllControls() {
	m_containerTitleUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_toptitle")));
	m_containerLoginUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_login")));
	m_containerResetPwdUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_resetpwd")));
	m_containerSignUpUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_signup")));
	m_containerSignInfoUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_signinfo")));
	m_containerReSignInfoUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_resigninfo")));
	m_containerAuditingUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_auditing")));

	m_containerMainpageUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_mainpage")));

	m_diagCancelApppontmentUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("diagcancelappointment")));
	m_diagArrearageUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("diagarrearage")));

	m_containerAppointmentListUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_appointmentlist")));
	m_containerToAppointUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_toappoint")));
	m_containerAppointmentDetailUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_appointmentdetail")));
	m_containerEditAppointmentUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_editappointment")));
		
	m_tileFreeTimeOfMorningAppointUI = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(_T("FreeTimeOfMorningAppoint")));
	m_tileFreeTimeOfAfternoonAppointUI = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(_T("FreeTimeOfAfternoonAppoint")));
	m_tileFreeTimeOfNightAppointUI = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(_T("FreeTimeOfNightAppoint")));
	m_tileFreeTimeOfEditMorningAppointUI = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(_T("FreeTimeOfEditMorningAppoint")));
	m_tileFreeTimeOfEditAfternoonAppointUI = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(_T("FreeTimeOfEditAfternoonAppoint")));
	m_tileFreeTimeOfEditNightAppointUI = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(_T("FreeTimeOfEditNightAppoint")));

	m_layFreeTimeOfMorningAppointUI = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("LayOfMorningTimeOnToAM")));
	m_layeFreeTimeOfAfternoonAppointUI = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("LayOfAfternoonTileOnToAM")));
	m_layFreeTimeOfNightAppointUI = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("LayOfnightTimeOnToAM")));
	m_layFreeTimeOfMorningAppointOnEditAMUI = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("LayOfMorningTimeOnEditAM")));
	m_layeFreeTimeOfAfternoonAppointOnEditAMUI = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("LayOfAfternoonTileOnEditAM")));
	m_layFreeTimeOfNightAppointOnEditAMUI = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("LayOfnightTimeOnEditAM")));

	m_tileExpertInfoUI = static_cast<CTileLayoutUI*>(m_PaintManager.FindControl(_T("TileExpertInfo")));

	m_containerRecordListUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_recordlist")));
	m_containerRecordDetailUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_recorddetail")));
	m_containerEditRecordUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_editrecord")));

	m_containerShowClinicInfoUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_showclinicinfo")));
	m_containerEditClinicInfoUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_editclinicinfo")));
	m_containerShowUserInfoUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_showuserinfo")));
	m_containerEditUserInfoUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_edituserinfo")));

	m_btnCloseUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonClose")));
	m_btnCloseEXEUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonCloseEXE")));
	m_btnWndMinUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("minbtn")));

	m_btnLoginUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonLogin")));
	
	m_btnUploadLicenseUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadLicense")));
	m_btnUploadPhotoUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadPhoto")));
	m_btnUploadDoctorLicenseUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadDoctorLicense")));
	
	m_btnReUploadLicenseUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonReUploadLicense")));
	m_btnReUploadPhotoUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonReUploadPhoto")));
	m_btnReUploadDoctorLicenseUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonReUploadDoctorLicense")));
	
	m_btnUploadNewLicenseUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadNewLicense")));
	m_btnUploadNewPhotoUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadNewPhoto")));
	m_btnUploadNewDoctorLicenseUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadNewDoctorLicense")));

	m_labTitleClinicNameUI = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelTitleClinicName")));
	m_labTitleClinicImageUI = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelTitleClinicImage")));

	m_tabMainControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchOperationType")));
	m_tabPatientInfoControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoType")));
	m_tabPatientInfoOnDetailControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnDetail")));
	m_tabPatientInfoOnEditControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnEdit")));
	m_tabPatientInfoOnRecordControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnRecord")));
	m_tabPatientInfoOnEditRecordControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnEditRecord")));
	m_tabPayTypeControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPayInfoType")));
	m_tabDoctorInfoTypeControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchDoctorInfoType")));
	m_tabClinicInfoControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchClinicInfoType")));

	m_listAppointmentUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListAppointments")));
	m_listRecordUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListRecord")));
	m_listCAppointmentUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListCAppointment")));
	m_listDoctorWorkInfoUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListDoctorWorkTime")));
	m_listUnpayUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListUnpay")));
	m_listAllpayUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListAllpay")));

	m_btnUploadPatientPic1UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadPatientPic1")));
	m_btnUploadPatientPic2UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadPatientPic2")));
	m_btnUploadPatientPic3UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadPatientPic3")));
	m_btnUploadNewPatientPic1UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadNewPatientPic1")));
	m_btnUploadNewPatientPic2UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadNewPatientPic2")));
	m_btnUploadNewPatientPic3UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonUploadNewPatientPic3")));

	m_btnShowPatientPic1UIOnAMDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic1")));
	m_btnShowPatientPic2UIOnAMDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic2")));
	m_btnShowPatientPic3UIOnAMDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic3")));
	m_btnShowPatientPic1UIOnRDDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic1OnRecord")));
	m_btnShowPatientPic2UIOnRDDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic2OnRecord")));
	m_btnShowPatientPic3UIOnRDDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic3OnRecord")));

	m_btnPlayAudio = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonPlayMp3Record")));
	m_btnPauseAudio = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonPauseMp3Record")));
	m_btnStopAudio = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonStopMp3Record")));

	m_cboChooseExpertOnSearchAppoint = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("ComboChooseExpertOnSearchAppoint")));
	m_cboChooseExpertOnToAppoint = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("ComboChooseExpertOnToAppoint")));
	m_cboChooseExpertOnEditAppoint = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("ComboChooseExpertOnEditAppoint")));
	m_cboChooseExpertOnSearchRecord = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("ComboChooseExpertOnSearchRecord")));

	pActiveXUI = static_cast<CActiveXUI*>(m_PaintManager.FindControl(_T("DocterInfoPage")));

	m_editMobileOnLoginUI = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("EditMobileOnLogin")));
	m_editPWDOnLoginUI = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("EditPWDOnLogin")));

	m_dataChooseOnSearchAppoint = static_cast<CDateTimeUI*>(m_PaintManager.FindControl(_T("DateChooseOnSearchAppoint")));
	m_dataChooseOnToAppoint = static_cast<CDateTimeUI*>(m_PaintManager.FindControl(_T("DateChooseOnToAppoint")));
	m_dataChooseOnEditAppoint = static_cast<CDateTimeUI*>(m_PaintManager.FindControl(_T("DateChooseOnEditAppoint")));
	m_dataChooseOnSearchRecord = static_cast<CDateTimeUI*>(m_PaintManager.FindControl(_T("DateChooseOnSearchRecord")));

	m_optionCheckAppointment = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("CheckAppointment")));
	m_optionConsultationRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("ConsultationRecord")));
	m_optionCurrentConsultation = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("CurrentConsultation")));
	m_optionPaymentRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("PaymentRecord")));
	m_optionDoctorInfo = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("DoctorInfo")));
	m_optionClinicInfo = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("ClinicInfo")));

	m_optionChooseMaleOnEditRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("CheckPatientMaleOnRecordEdit")));
	m_optionChooseFemaleOnEditRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("CheckPatientFemaleOnRecordEdit")));

	m_optionPatientConditionPageOnToAM = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPage")));
	m_optionPatientPicPageOnToAM = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPage")));
	m_optionPatientConditionPageOnShowAM = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnDetail")));
	m_optionPatientPicPageOnShowAM = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnDetail")));
	m_optionPatientConditionPageOnEditAM = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnEdit")));
	m_optionPatientPicPageOnEditAM = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnEdit")));

	m_optionPatientConditionPageOnShowRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnRecord")));
	m_optionPatientPicPageOnShowRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnRecord")));
	m_optionPatientConditionPageOnEditRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnEditRecord")));
	m_optionPatientPicPageOnEditRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnEditRecord")));

	m_optionUnPay = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionUnPay")));
	m_optionAllPay = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionAllPay")));
	m_optionBill = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionBill")));
	m_optionWeixinCode = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionWeixinCode")));

	m_optionExpertTileInfo = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionDoctorInfo")));
	m_optionDoctorWorkTime = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionDoctorWorkTime")));

	m_optionUserInfo = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionUserInfoPage")));
	m_optionClinicSignInfo = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionClinicInfoPage")));

	m_labWaitMeetingStart = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelWaitMeetingStart")));
	//m_labPatientConditionHintOnToAM = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPatientConditionHintOnToAM")));
	//m_labPatientConditionHintOnEditAM = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPatientConditionHintOnEditAM")));
	//m_labPatientImgHintOnEditAM = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPatientImgHintOnEditAM")));
	//m_labPatientImgHintOnToAM = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPatientImgHintOnToAM")));

	m_labPageNumOnAppoint = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPageNumOnAppoint")));
	m_labCurrentPageNumOnAppoint = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelCurrentPageNumOnAppoint")));
	m_labPageNumOnRecord = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPageNumOnRecord")));
	m_labCurrentPageNumOnRecord = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelCurrentPageNumOnRecord")));
	m_labPageNumOnUnPay = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPageNumOnUnPay")));
	m_labCurrentPageNumOnUnPay = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelCurrentPageNumOnUnPay")));
	m_labPageNumOnAllPay = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPageNumOnAllPay")));
	m_labCurrentPageNumOnAllPay = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelCurrentPageNumOnAllPay")));

	p_showCodeImageActiveXUI = static_cast<CActiveXUI*>(m_PaintManager.FindControl(_T("CodeImage")));
}

void CDuiFrameWnd::InitWindow() {
	InitAllControls();
};


void CDuiFrameWnd::ResetAllControls() {
	m_containerTitleUI = NULL;
	m_containerLoginUI = NULL;
	m_containerAuditingUI = NULL;
	m_containerResetPwdUI = NULL;
	m_containerSignUpUI = NULL;
	m_containerSignInfoUI = NULL;
	m_containerReSignInfoUI = NULL;

	m_containerMainpageUI = NULL;

	m_diagCancelApppontmentUI = NULL;
	m_diagArrearageUI = NULL;

	m_containerAppointmentListUI = NULL;
	m_containerToAppointUI = NULL;
	m_containerAppointmentDetailUI = NULL;
	m_containerEditAppointmentUI = NULL;

	m_tileFreeTimeOfMorningAppointUI = NULL;
	m_tileFreeTimeOfAfternoonAppointUI = NULL;
	m_tileFreeTimeOfNightAppointUI = NULL;
	m_tileFreeTimeOfEditMorningAppointUI = NULL;
	m_tileFreeTimeOfEditAfternoonAppointUI = NULL;
	m_tileFreeTimeOfEditNightAppointUI = NULL;

	m_layFreeTimeOfMorningAppointUI = NULL;
	m_layeFreeTimeOfAfternoonAppointUI = NULL;
	m_layFreeTimeOfNightAppointUI = NULL;
	m_layFreeTimeOfMorningAppointOnEditAMUI = NULL;
	m_layeFreeTimeOfAfternoonAppointOnEditAMUI = NULL;
	m_layFreeTimeOfNightAppointOnEditAMUI = NULL;

	m_tileExpertInfoUI = NULL;

	m_containerRecordListUI = NULL;
	m_containerRecordDetailUI = NULL;
	m_containerEditRecordUI = NULL;

	m_containerShowClinicInfoUI = NULL;
	m_containerEditClinicInfoUI = NULL;
	m_containerShowUserInfoUI = NULL;
	m_containerEditUserInfoUI = NULL;

	m_btnCloseUI = NULL;
	m_btnCloseEXEUI = NULL;
	m_btnLoginUI = NULL;
	m_btnWndMinUI = NULL;

	m_btnReUploadLicenseUI = NULL;
	m_btnReUploadPhotoUI = NULL;
	m_btnReUploadDoctorLicenseUI = NULL;

	m_btnUploadLicenseUI = NULL;
	m_btnUploadPhotoUI = NULL;
	m_btnUploadDoctorLicenseUI = NULL;

	m_btnUploadNewLicenseUI = NULL;
	m_btnUploadNewPhotoUI = NULL;
	m_btnUploadNewDoctorLicenseUI = NULL;

	m_labTitleClinicNameUI = NULL;
	m_labTitleClinicImageUI = NULL;

	m_tabMainControlUI = NULL;
	m_tabPatientInfoControlUI = NULL;
	m_tabPatientInfoOnDetailControlUI = NULL;
	m_tabPatientInfoOnEditControlUI = NULL;
	m_tabPatientInfoOnRecordControlUI = NULL;
	m_tabPatientInfoOnEditRecordControlUI = NULL;
	m_tabPayTypeControlUI = NULL;
	m_tabDoctorInfoTypeControlUI = NULL;
	m_tabClinicInfoControlUI = NULL;

	m_listAppointmentUI = NULL;
	m_listRecordUI = NULL;
	m_listCAppointmentUI = NULL;
	m_listDoctorWorkInfoUI = NULL;
	m_listUnpayUI = NULL;
	m_listAllpayUI = NULL;

	m_btnUploadPatientPic1UI = NULL;
	m_btnUploadPatientPic2UI = NULL;
	m_btnUploadPatientPic3UI = NULL;
	m_btnUploadNewPatientPic1UI = NULL;
	m_btnUploadNewPatientPic2UI = NULL;
	m_btnUploadNewPatientPic3UI = NULL;

	m_btnShowPatientPic1UIOnAMDetail = NULL;
	m_btnShowPatientPic2UIOnAMDetail = NULL;
	m_btnShowPatientPic3UIOnAMDetail = NULL;
	m_btnShowPatientPic1UIOnRDDetail = NULL;
	m_btnShowPatientPic2UIOnRDDetail = NULL;
	m_btnShowPatientPic3UIOnRDDetail = NULL;

	m_btnPlayAudio = NULL;
	m_btnPauseAudio = NULL;
	m_btnStopAudio = NULL;

	m_cboChooseExpertOnSearchAppoint = NULL;
	m_cboChooseExpertOnToAppoint = NULL;
	m_cboChooseExpertOnEditAppoint = NULL;
	m_cboChooseExpertOnSearchRecord = NULL;

	pActiveXUI = NULL;
	
	m_editMobileOnLoginUI = NULL;
	m_editPWDOnLoginUI = NULL;
	m_dataChooseOnSearchAppoint = NULL;
	m_dataChooseOnToAppoint = NULL;
	m_dataChooseOnEditAppoint = NULL;
	m_dataChooseOnSearchRecord = NULL;

	m_optionCheckAppointment = NULL;
	m_optionConsultationRecord = NULL;
	m_optionCurrentConsultation = NULL;
	m_optionPaymentRecord = NULL;
	m_optionDoctorInfo = NULL;
	m_optionClinicInfo = NULL;

	m_optionChooseMaleOnEditRecord = NULL;
	m_optionChooseFemaleOnEditRecord = NULL;

	m_optionPatientConditionPageOnToAM = NULL;
	m_optionPatientPicPageOnToAM = NULL;
	m_optionPatientConditionPageOnShowAM = NULL;
	m_optionPatientPicPageOnShowAM = NULL;
	m_optionPatientConditionPageOnEditAM = NULL;
	m_optionPatientPicPageOnEditAM = NULL;

	m_optionPatientConditionPageOnShowRecord = NULL;
	m_optionPatientPicPageOnShowRecord = NULL;
	m_optionPatientConditionPageOnEditRecord = NULL;
	m_optionPatientPicPageOnEditRecord = NULL;

	m_optionUnPay = NULL;
	m_optionAllPay = NULL;
	m_optionBill = NULL;
	m_optionWeixinCode = NULL;

	m_optionExpertTileInfo = NULL;
	m_optionDoctorWorkTime = NULL;

	m_optionUserInfo = NULL;
	m_optionClinicSignInfo = NULL;

	m_labWaitMeetingStart = NULL;
	//m_labPatientConditionHintOnEditAM = NULL;
	//m_labPatientConditionHintOnToAM = NULL;
	//m_labPatientImgHintOnEditAM = NULL;
	//m_labPatientImgHintOnToAM = NULL;

	m_labPageNumOnAppoint = NULL;
	m_labCurrentPageNumOnAppoint = NULL;
	m_labPageNumOnRecord = NULL;
	m_labCurrentPageNumOnRecord = NULL;
	m_labPageNumOnUnPay = NULL;
	m_labCurrentPageNumOnUnPay = NULL;
	m_labPageNumOnAllPay = NULL;
	m_labCurrentPageNumOnAllPay = NULL;

	p_showCodeImageActiveXUI = NULL;
}

void CDuiFrameWnd::InitSimpleData() {
	m_dataChooseOnSearchAppoint->SetText(_T(""));
	m_dataChooseOnSearchRecord->SetText(_T(""));

	hFile = CreateFileW(L"..\\hdlog.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA("create file error!\n");
		CloseHandle(hFile);
	}
	else {
		SetFilePointer(hFile, NULL, NULL, FILE_END);

		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char aa[30];
		sprintf_s(aa, "%4d-%02d-%02d %02d:%02d:%02d   ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
		HDWriteLog(aa);

		CStringA logmsg= "初始化程序.\r\n";
		HDWriteLog(logmsg);
	}
	InitVoice();
	if (SDKAuth()) {
		OutputDebugStringA("auth SDK success \n ");
		CStringA logmsg = "会议身份验证成功。\r\n";
		HDWriteLog(logmsg);
	}
	else {
		OutputDebugStringA("Auth SDK failed!\n ");
		CStringA logmsg = "会议身份验证失败。\r\n";
		HDWriteLog(logmsg);
	}
};


LRESULT CDuiFrameWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
			case VK_RETURN:
			case VK_ESCAPE://拦截ESC退出界面
				bHandled = true;
				return 0;
			default:
				break;
		}
	}
	return FALSE;
}


LRESULT CDuiFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	if (uMsg == WM_CLOSE)
	{
		OnClose(uMsg, wParam, lParam, bHandled);
		return lRes;
	}
	else if (uMsg == WM_DESTROY)
	{
		OnDestroy(uMsg, wParam, lParam, bHandled);
		return lRes;
	}
/*
	if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
	{
		return lRes;
	}
	*/
	return __super::HandleMessage(uMsg, wParam, lParam);

}

LRESULT CDuiFrameWnd::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	OutputDebugStringA("close frame \n ");
	ResetAllControls();
	bHandled = FALSE;
	m_pAuthServiceMgr->UnInit();
	m_pMeetingServiceMgr->UnInit();
	UninitVoice();
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
	system("del ..\\hdtemp\\*.* /a /s /f /q");
	::PostQuitMessage(0);
	return 0;
}

LRESULT CDuiFrameWnd::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	OutputDebugStringA("destroy frame \n ");
	ResetAllControls();
	bHandled = FALSE;
	m_pAuthServiceMgr->UnInit();
	m_pMeetingServiceMgr->UnInit();
	UninitVoice();
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
	system("del ..\\hdtemp\\*.*  /a /s /f /q");
	::PostQuitMessage(0);
	return 0;
}


CControlUI* CDuiFrameWnd::CreateControl(LPCTSTR pstrClassName)
{
	CDuiString     strXML;
	CDialogBuilder builder;

	if (_tcsicmp(pstrClassName, _T("Appointment")) == 0)
	{
		strXML = _T("Appointment.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("ConsultationRecord")) == 0)
	{
		strXML = _T("ConsultationRecord.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("CurrentConsultation")) == 0)
	{
		strXML = _T("CurrentConsultation.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("PaymentRecord")) == 0)
	{
		strXML = _T("PaymentRecord.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("DoctorInfo")) == 0)
	{
		strXML = _T("DoctorInfo.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("ClinicInfo")) == 0)
	{
		strXML = _T("ClinicInfo.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("DiagCancelApppontment")) == 0)
	{
		strXML = _T("DiagCancelApppontment.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("DiagArrearage")) == 0)
	{
		strXML = _T("DiagArrearage.xml");
	}
	else 	if (_tcsicmp(pstrClassName, _T("ListItemButton")) == 0) {
		return  new CListItemButtonUI();
	}
	
	if (!strXML.IsEmpty())
	{
		CControlUI* pUI = builder.Create(strXML.GetData(), NULL, NULL, &m_PaintManager, NULL); // 这里必须传入m_PaintManager，不然子XML不能使用默认滚动条等信息。
		return pUI;
	}
	else {
		return NULL;
	}
}

int CDuiFrameWnd::HttpLogin(LPCTSTR username, LPCTSTR pwd) {
	LPCTSTR curl = LOGIN_URL;
	CStringA postdata = "";
	postdata += "username=";
	postdata += username;
	postdata += "&password=";
	postdata += pwd;
	postdata += "&grant_type=";
	postdata += GRANT_TYPE;
	postdata += "&client_id=";
	postdata += CLIENT_ID;
	postdata += "&client_secret=";
	postdata += CLIENT_SECRET;
	postdata += "&type=";
	postdata += LOGIN_TYPE;
	CStringW temp = L"";

	Json::Value root = HDWinHttpPost(curl, postdata);

	if (root["code"] == 0) {
		OutputDebugStringA("login success \n ");
		token = root["data"]["access_token"].asCString();
		
		return 1;
	}
	else if (root["code"] == -1) {
		::MessageBox(*this, _T("网络错误！"), _T("提示"), 0);
		return -1;
	}
	else {
		::MessageBox(*this, _T("登录失败，请检查用户名和密码！"), _T("提示"), 0);
		return -1;
	}
}

int CDuiFrameWnd::HttpGetExpertList() {
	Json::Value root = HDWinHttpGetWithToken(EXPERT_LIST_URL, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get expert list success \n ");
		int size = root["data"].size();
		CStringW temp;
		Json::Value newexpert;
		for (int i = 0; i < size; ++i) {
			newexpert["name"] = root["data"][i]["name"];
			newexpert["id"] = root["data"][i]["id"];
			newexpert["uuid"] = root["data"][i]["user_uuid"];
			newexpert["fee_by_money"] = root["data"][i]["fee_by_money"];
			expertlist.append(newexpert);
		}
		return size;
	}
	else {
		OutputDebugStringA("get expert list failed \n ");
		return -1;
	}
}

void CDuiFrameWnd::HttpGetFreeTime(LPCTSTR expert, LPCTSTR choosedate) {
	CStringW curl = L"";
	curl += EXPERT_FREE_TIME_URL;
	curl += L"?expert_uuid=";
	curl += expert;
	curl += L"&date=";
	curl += choosedate;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");

	Json::Value root = HDWinHttpGetWithToken(curl, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get expert free time success \n ");			
		CStringW temp;			
		if (!(root["data"]["morning"].isNull())) {
			int size = root["data"]["morning"].size();
			if (size > 5) {
				m_tileFreeTimeOfMorningAppointUI->SetMinHeight(112);
				m_tileFreeTimeOfMorningAppointUI->SetMaxHeight(112);
				m_layFreeTimeOfMorningAppointUI->SetMaxHeight(112);
				m_layFreeTimeOfMorningAppointUI->SetMinHeight(112);
			}
			else {
				m_tileFreeTimeOfMorningAppointUI->SetMinHeight(56);
				m_tileFreeTimeOfMorningAppointUI->SetMaxHeight(56);
				m_layFreeTimeOfMorningAppointUI->SetMaxHeight(56);
				m_layFreeTimeOfMorningAppointUI->SetMinHeight(56);
			}
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CVerticalLayoutUI* pLine = (CVerticalLayoutUI*)(builder.Create(_T("AppointmentTimeItem.xml"), (UINT)0, this));
				if (pLine != NULL)
				{
					m_tileFreeTimeOfMorningAppointUI->Add(pLine);
					temp = UTF8ToWString(root["data"]["morning"][i].asCString());
					pLine->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"))->SetText(temp);
					pLine->SetUserData(temp);
					pLine->SetTag(i);
				}
			}
		}
			
		if (!(root["data"]["afternoon"].isNull())) {
			int size = root["data"]["afternoon"].size();
			if (size > 5) {
				m_tileFreeTimeOfAfternoonAppointUI->SetMinHeight(112);
				m_tileFreeTimeOfAfternoonAppointUI->SetMaxHeight(112);
				m_layeFreeTimeOfAfternoonAppointUI->SetMaxHeight(112);
				m_layeFreeTimeOfAfternoonAppointUI->SetMinHeight(112);
			}
			else {
				m_tileFreeTimeOfAfternoonAppointUI->SetMinHeight(56);
				m_tileFreeTimeOfAfternoonAppointUI->SetMaxHeight(56);
				m_layeFreeTimeOfAfternoonAppointUI->SetMaxHeight(56);
				m_layeFreeTimeOfAfternoonAppointUI->SetMinHeight(56);
			}
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CVerticalLayoutUI* pLine = (CVerticalLayoutUI*)(builder.Create(_T("AppointmentTimeItem.xml"), (UINT)0, this));

				if (pLine != NULL)
				{
					m_tileFreeTimeOfAfternoonAppointUI->Add(pLine);
					temp = UTF8ToWString(root["data"]["afternoon"][i].asCString());
					pLine->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"))->SetText(temp);
					pLine->SetUserData(temp);
					pLine->SetTag(10+i);
				}
			}
		}
		if (!(root["data"]["evening"].isNull())) {
			int size = root["data"]["evening"].size();
			if (size > 5) {
				m_tileFreeTimeOfNightAppointUI->SetMinHeight(90);
				m_tileFreeTimeOfNightAppointUI->SetMaxHeight(90);
				m_layFreeTimeOfNightAppointUI->SetMaxHeight(112);
				m_layFreeTimeOfNightAppointUI->SetMinHeight(112);
			}
			else {
				m_tileFreeTimeOfNightAppointUI->SetMinHeight(45);
				m_tileFreeTimeOfNightAppointUI->SetMaxHeight(45);
				m_layFreeTimeOfNightAppointUI->SetMaxHeight(56);
				m_layFreeTimeOfNightAppointUI->SetMinHeight(56);
			}
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CVerticalLayoutUI* pLine = (CVerticalLayoutUI*)(builder.Create(_T("AppointmentTimeItem.xml"), (UINT)0, this));

				if (pLine != NULL)
				{
					m_tileFreeTimeOfNightAppointUI->Add(pLine);
					temp = UTF8ToWString(root["data"]["evening"][i].asCString());
					pLine->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"))->SetText(temp);
					pLine->SetUserData(temp);
					pLine->SetTag(20+i);
				}
			}
		}

	}
	else if(root["code"].asInt() == 20902){
		::MessageBox(*this, _T("无空闲时段"), _T("提示"), 0);
	}
	else {
		OutputDebugStringA("get free time  failed \n ");
	}

}


void CDuiFrameWnd::HttpGetFreeTimeOnEditAM(LPCTSTR expert, LPCTSTR choosedate) {
	CStringW curl = L"";
	curl += EXPERT_FREE_TIME_URL;
	curl += L"?expert_uuid=";
	curl += expert;
	curl += L"&date=";
	curl += choosedate;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");

	Json::Value root = HDWinHttpGetWithToken(curl, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get expert free time on edit success \n ");
		CStringW temp;
		if (!(root["data"]["morning"].isNull())) {
			int size = root["data"]["morning"].size();
			if (size > 5) {
				m_tileFreeTimeOfEditMorningAppointUI->SetMinHeight(112);
				m_tileFreeTimeOfEditMorningAppointUI->SetMaxHeight(112);
				m_layFreeTimeOfMorningAppointOnEditAMUI->SetMinHeight(131);
				m_layFreeTimeOfMorningAppointOnEditAMUI->SetMaxHeight(131);
			}
			else {
				m_tileFreeTimeOfEditMorningAppointUI->SetMinHeight(56);
				m_tileFreeTimeOfEditMorningAppointUI->SetMaxHeight(56);
				m_layFreeTimeOfMorningAppointOnEditAMUI->SetMinHeight(75);
				m_layFreeTimeOfMorningAppointOnEditAMUI->SetMaxHeight(75);
			}
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CVerticalLayoutUI* pLine = (CVerticalLayoutUI*)(builder.Create(_T("AppointmentTimeItem.xml"), (UINT)0, this));
				if (pLine != NULL)
				{
					m_tileFreeTimeOfEditMorningAppointUI->Add(pLine);
					temp = UTF8ToWString(root["data"]["morning"][i].asCString());
					pLine->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"))->SetText(temp);
					pLine->SetUserData(temp);
					pLine->SetTag(i);
				}
			}
		}

		if (!(root["data"]["afternoon"].isNull())) {
			int size = root["data"]["afternoon"].size();
			if (size > 5) {
				m_tileFreeTimeOfEditAfternoonAppointUI->SetMinHeight(112);
				m_tileFreeTimeOfEditAfternoonAppointUI->SetMaxHeight(112);
				m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMinHeight(131);
				m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMaxHeight(131);
			}
			else {
				m_tileFreeTimeOfEditAfternoonAppointUI->SetMinHeight(56);
				m_tileFreeTimeOfEditAfternoonAppointUI->SetMaxHeight(56);
				m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMinHeight(75);
				m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMaxHeight(75);
			}
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CVerticalLayoutUI* pLine = (CVerticalLayoutUI*)(builder.Create(_T("AppointmentTimeItem.xml"), (UINT)0, this));

				if (pLine != NULL)
				{
					m_tileFreeTimeOfEditAfternoonAppointUI->Add(pLine);
					temp = UTF8ToWString(root["data"]["afternoon"][i].asCString());
					pLine->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"))->SetText(temp);
					pLine->SetUserData(temp);
					pLine->SetTag(10+i);
				}
			}
		}
		if (!(root["data"]["evening"].isNull())) {
			int size = root["data"]["evening"].size();
			if (size > 5) {
				m_tileFreeTimeOfEditNightAppointUI->SetMinHeight(112);
				m_tileFreeTimeOfEditNightAppointUI->SetMaxHeight(112);
				m_layFreeTimeOfNightAppointOnEditAMUI->SetMinHeight(112);
				m_layFreeTimeOfNightAppointOnEditAMUI->SetMaxHeight(112);
			}
			else {
				m_tileFreeTimeOfEditNightAppointUI->SetMinHeight(56);
				m_tileFreeTimeOfEditNightAppointUI->SetMaxHeight(56);
				m_layFreeTimeOfNightAppointOnEditAMUI->SetMinHeight(56);
				m_layFreeTimeOfNightAppointOnEditAMUI->SetMaxHeight(56);
			}
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CVerticalLayoutUI* pLine = (CVerticalLayoutUI*)(builder.Create(_T("AppointmentTimeItem.xml"), (UINT)0, this));

				if (pLine != NULL)
				{
					m_tileFreeTimeOfEditNightAppointUI->Add(pLine);
					temp = UTF8ToWString(root["data"]["evening"][i].asCString());
					pLine->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"))->SetText(temp);
					pLine->SetUserData(temp);
					pLine->SetTag(20+i);
				}
			}
		}

	}
	else if (root["code"].asInt() == 20902) {
		::MessageBox(*this, _T("无空闲时段"), _T("提示"), 0);
	}
	else {
		OutputDebugStringA("get edit free time  failed \n ");
	}
}


void CDuiFrameWnd::HttpGetAppointmentList(LPCTSTR curl, int pagenum) {
	CStringW pagecount;
	pagecount.Format(_T("%d"), pagenum);
	CStringW url = curl;
	url += L"&page=";
	url += pagecount;
	OutputDebugStringW(url);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(url, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get appointment list success \n ");
		int size = root["data"].size();
		CStringW temp;
		appointlisttotalpagenum = root["totalPage"].asInt();
		appointlistcurrentpagenum = pagenum;

		int totalpagenum1 = root["totalPage"].asInt();
		char aa[20];
		sprintf_s(aa, "%d", totalpagenum1);
		CStringW totalpagenum2 = aa;
		m_labPageNumOnAppoint->SetText(totalpagenum2);

		sprintf_s(aa, "%d", pagenum);
		CStringW cpagenum = aa;
		m_labCurrentPageNumOnAppoint->SetText(cpagenum);

		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				/*
				CStringW appointno_temp = root["data"][i]["appointment_no"].asCString();
				CStringA strA(appointno_temp.GetBuffer(0));
				string s = strA.GetBuffer(0);
				const char* pc = s.c_str();
				unsigned __int64 appnum = atol(pc);
				*/

				//unsigned __int64 appnum = root["data"][i]["appointment_no"].asInt64();
				//sprintf_s(aa, "%llu", appnum);
				//CStringW appnostr = aa;
				CStringW appnostr = root["data"][i]["appointment_no"].asCString();

				if (root["data"][i]["status"] == 1) {
					CDialogBuilder builder;
					CListContainerElementUI* pLine;
					pLine = (CListContainerElementUI*)(builder.Create(_T("AppointmentLine.xml"), (UINT)0, this));
					if (pLine != NULL)
					{
						pLine->SetUserData(appnostr);
						m_listAppointmentUI->Add(pLine);
						CLabelUI* pAppointmentTime = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnAMLine"));
						temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
						pAppointmentTime->SetText(temp);

						CLabelUI* pAppointmentDoctor = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentDoctorOnAMLine"));
						temp = UTF8ToWString(root["data"][i]["expert"]["name"].asCString());
						pAppointmentDoctor->SetText(temp);

						CLabelUI* pAppointmentPatient = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentPatientOnAMLine"));
						temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
						pAppointmentPatient->SetText(temp);

						CLabelUI* pAppointmentStatus = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentStatusOnAMLine"));
						pAppointmentStatus->SetText(_T("预约中"));
					}
				}
				else {
					CDialogBuilder builder;
					CListContainerElementUI* pLine;
					pLine = (CListContainerElementUI*)(builder.Create(_T("AppointmentLine_Done.xml"), (UINT)0, this));
					if (pLine != NULL)
					{
						pLine->SetUserData(appnostr);
						m_listAppointmentUI->Add(pLine);
						CLabelUI* pAppointmentTime = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnAMLine"));
						temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
						pAppointmentTime->SetText(temp);

						CLabelUI* pAppointmentDoctor = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentDoctorOnAMLine"));
						temp = UTF8ToWString(root["data"][i]["expert"]["name"].asCString());
						pAppointmentDoctor->SetText(temp);

						CLabelUI* pAppointmentPatient = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentPatientOnAMLine"));
						temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
						pAppointmentPatient->SetText(temp);

						CLabelUI* pAppointmentStatus = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentStatusOnAMLine"));
						if (root["data"][i]["status"] == 2)
							pAppointmentStatus->SetText(_T("预约成功"));
						else if (root["data"][i]["status"] == 3)
							pAppointmentStatus->SetText(_T("预约取消"));
						else if (root["data"][i]["status"] == 4)
							pAppointmentStatus->SetText(_T("平台取消"));
					}
				}
			}
		}

	}
	else {
		OutputDebugStringA("get appointment list failed \n ");
	}
}




void CDuiFrameWnd::HttpGetCurrentAppointmentList(LPCTSTR curl) {
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get current appointment list success \n ");
		int size = root["data"].size();
		CStringW temp;
		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				//unsigned __int64 appnum = root["data"][i]["appointment_no"].asInt64();
				//char aa[20];
				//sprintf_s(aa, "%llu", appnum);
				//CStringW appnostr = aa;
				CStringW appnostr = root["data"][i]["appointment_no"].asCString();
				if (root["data"][i]["dx_status"].asInt() <= 1) {
					CDialogBuilder builder;
					CListContainerElementUI* pLine;
					pLine = (CListContainerElementUI*)(builder.Create(_T("CurrentConsultationLine.xml"), (UINT)0, this));
					if (pLine != NULL)
					{
						pLine->SetUserData(appnostr);
						m_listCAppointmentUI->Add(pLine);

						CLabelUI* pCAppointmentTime = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
						pCAppointmentTime->SetText(temp);

						CLabelUI* pCAppointmentDoctor = (CLabelUI*)pLine->FindSubControl(_T("LabelExpertNameOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["expert"]["name"].asCString());
						pCAppointmentDoctor->SetText(temp);

						CLabelUI* pCAppointmentPatient = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientNameOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
						pCAppointmentPatient->SetText(temp);

						CControlUI* pCAppointmentExpertImg = (CControlUI*)pLine->FindSubControl(_T("ControlExpertImgOnCAMLine"));
						if (root["data"][i]["expert"]["head_img"].isString()) {
							temp = UTF8ToWString(root["data"][i]["expert"]["head_img"].asCString());

							CStringW path1 = L"..\\hdtemp\\";
							path1 += appnostr;
							path1 += temp.Mid(55);
							if (PathFileExists(path1) == TRUE) {
								OutputDebugStringA("\r\n file exist.\n");
								pCAppointmentExpertImg->SetBkImage(path1);
							}
							else {
								OutputDebugStringA("\r\n file not exist.\n");
								if (HDWinHttpDownloadFile(temp, path1) > 0) {
									pCAppointmentExpertImg->SetBkImage(path1);
									OutputDebugStringA("download success.\n");
								}
								else {
									pCAppointmentExpertImg->SetBkImage(L"bkimage/downloadimagefailed-1.png");
								}
							}
						}
					}
				}
				else if (root["data"][i]["dx_status"].asInt() == 2) {
					CDialogBuilder builder;
					CListContainerElementUI* pLine;
					pLine = (CListContainerElementUI*)(builder.Create(_T("CurrentConsultationLine_Done.xml"), (UINT)0, this));
					if (pLine != NULL)
					{
						pLine->SetUserData(appnostr);
						m_listCAppointmentUI->Add(pLine);

						CLabelUI* pCAppointmentTime = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
						pCAppointmentTime->SetText(temp);

						CLabelUI* pCAppointmentDoctor = (CLabelUI*)pLine->FindSubControl(_T("LabelExpertNameOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["expert"]["name"].asCString());
						pCAppointmentDoctor->SetText(temp);

						CLabelUI* pCAppointmentPatient = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientNameOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
						pCAppointmentPatient->SetText(temp);

						CControlUI* pCAppointmentExpertImg = (CControlUI*)pLine->FindSubControl(_T("ControlExpertImgOnCAMLine"));
						if (root["data"][i]["expert"]["head_img"].isString()) {
							temp = UTF8ToWString(root["data"][i]["expert"]["head_img"].asCString());
							CStringW path1 = L"..\\hdtemp\\";
							path1 += appnostr;
							path1 += temp.Mid(55);
							if (PathFileExists(path1) == TRUE) {
								OutputDebugStringA("\r\n file exist.\n");
								pCAppointmentExpertImg->SetBkImage(path1);
							}
							else {
								OutputDebugStringA("\r\n file not exist.\n");
								if (HDWinHttpDownloadFile(temp, path1) > 0) {
									pCAppointmentExpertImg->SetBkImage(path1);
									OutputDebugStringA("download success.\n");
								}
								else {
									pCAppointmentExpertImg->SetBkImage(L"bkimage/downloadimagefailed-1.png");
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		OutputDebugStringA("get current appointment list failed \n ");
	}

}



void CDuiFrameWnd::HttpGetRecordList(LPCTSTR curl, int pagenum) {
	CStringW pagecount;
	pagecount.Format(_T("%d"), pagenum);
	CStringW url = curl;
	url += L"&page=";
	url += pagecount;
	OutputDebugStringW(url);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(url, token);

	if (root["code"] == 0) {
		OutputDebugStringA("get record list success \n ");
		int size = root["data"].size();
		CStringW temp;

		recordlisttotalpagenum = root["totalPage"].asInt();
		recordlistcurrentpagenum = pagenum;

		int totalpagenum1 = root["totalPage"].asInt();
		char aa[20];
		sprintf_s(aa, "%d", totalpagenum1);
		CStringW totalpagenum2 = aa;
		m_labPageNumOnRecord->SetText(totalpagenum2);

		sprintf_s(aa, "%d", pagenum);
		CStringW cpagenum = aa;
		m_labCurrentPageNumOnRecord->SetText(cpagenum);

		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				//unsigned __int64 appnum = root["data"][i]["appointment_no"].asInt64();
				//sprintf_s(aa, "%llu", appnum);
				//CStringW appnostr = aa;
				CStringW appnostr = root["data"][i]["appointment_no"].asCString();
					if (root["data"][i]["change_status"].asInt() == 1) {
						CDialogBuilder builder;
						CListContainerElementUI* pLine;
						pLine = (CListContainerElementUI*)(builder.Create(_T("ConsultationRecordLine.xml"), (UINT)0, this));
						if (pLine != NULL)
						{
							pLine->SetUserData(appnostr);
							m_listRecordUI->Add(pLine);
							CLabelUI* pAppointmentTime1 = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnRecordLine"));
							temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
							pAppointmentTime1->SetText(temp);

							CLabelUI* pRecordExpert = (CLabelUI*)pLine->FindSubControl(_T("LabelExpertNameOnRecordLine"));
							temp = UTF8ToWString(root["data"][i]["expert_name"].asCString());
							pRecordExpert->SetText(temp);

							CLabelUI* pRecordPatientName = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientNameOnRecordLine"));
							temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
							pRecordPatientName->SetText(temp);

							CLabelUI* pRecordPatientDesc = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientDescOnRecordLine"));
							temp = UTF8ToWString(root["data"][i]["patient_description"].asCString());
							pRecordPatientDesc->SetText(temp);
						}
					}
					else {
						CDialogBuilder builder;
						CListContainerElementUI* pLine;
						pLine = (CListContainerElementUI*)(builder.Create(_T("ConsultationRecordLine_Done.xml"), (UINT)0, this));
						if (pLine != NULL)
						{
							pLine->SetUserData(appnostr);
							m_listRecordUI->Add(pLine);
							CLabelUI* pAppointmentTime1 = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnRecordLine"));
							temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
							pAppointmentTime1->SetText(temp);

							CLabelUI* pRecordExpert = (CLabelUI*)pLine->FindSubControl(_T("LabelExpertNameOnRecordLine"));
							temp = UTF8ToWString(root["data"][i]["expert_name"].asCString());
							pRecordExpert->SetText(temp);

							CLabelUI* pRecordPatientName = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientNameOnRecordLine"));
							temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
							pRecordPatientName->SetText(temp);

							CLabelUI* pRecordPatientDesc = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientDescOnRecordLine"));
							temp = UTF8ToWString(root["data"][i]["patient_description"].asCString());
							pRecordPatientDesc->SetText(temp);
						}
					}
			}
		}
	}
	else {
		OutputDebugStringA("get record list failed \n ");
	}
}


void CDuiFrameWnd::HttpGetUnPayInfoList(LPCTSTR curl, int pagenum) {
	CStringW pagecount;
	pagecount.Format(_T("%d"), pagenum);
	CStringW url = curl;
	url += L"&page=";
	url += pagecount;
	OutputDebugStringW(url);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(url, token);

	if (root["code"] == 0) {
		OutputDebugStringA("get pay info list success \n ");
		int size = root["data"].size();
		CStringW temp;

		unpaylisttotalpagenum = root["totalPage"].asInt();
		unpaylistcurrentpagenum = pagenum;

		int totalpagenum1 = root["totalPage"].asInt();
		char ee[20];
		sprintf_s(ee, "%d", totalpagenum1);
		CStringW totalpagenum2 = ee;
		m_labPageNumOnUnPay->SetText(totalpagenum2);

		sprintf_s(ee, "%d", pagenum);
		CStringW cpagenum = ee;
		m_labCurrentPageNumOnUnPay->SetText(cpagenum);

		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CListContainerElementUI* pLine;
				pLine = (CListContainerElementUI*)(builder.Create(_T("PaymentRecordLine.xml"), (UINT)0, this));
				if (pLine != NULL)
				{
					m_listUnpayUI->Add(pLine);
					CLabelUI* pMeetingStartTimeOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelMeetingStartTimeOnPayLine"));
					temp = UTF8ToWString(root["data"][i]["real_starttime"].asCString());
					pMeetingStartTimeOnPayLine->SetText(temp);

					CLabelUI* pMeetingEndTimeOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelMeetingEndTimeOnPayLine"));
					temp = UTF8ToWString(root["data"][i]["real_endtime"].asCString());
					pMeetingEndTimeOnPayLine->SetText(temp);

					CLabelUI* pExpertNameOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelExpertNameOnPayLine"));
					temp = UTF8ToWString(root["data"][i]["expert_name"].asCString());
					pExpertNameOnPayLine->SetText(temp);

					CLabelUI* pPatientNameOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientNameOnPayLine"));
					temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
					pPatientNameOnPayLine->SetText(temp);

					CLabelUI* pPayTypeOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelPayTypeOnPayLine"));
					if (root["data"][i]["pay_status"].asInt() == 0) {
						pPayTypeOnPayLine->SetText(_T("待支付"));
					}
					else {
						pPayTypeOnPayLine->SetText(_T("已支付"));
					}				

					CLabelUI* pCostOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelCostOnPayLine"));
					int realfee = root["data"][i]["real_fee"].asInt();
					char ff[20];
					sprintf_s(ff, "%d", realfee);
					CStringW bb = ff;
					pCostOnPayLine->SetText(bb);
				}
			}
		}
	}
	else {
		OutputDebugStringA("get unpay info list failed \n ");
	}
}


void CDuiFrameWnd::HttpGetAllPayInfoList(LPCTSTR curl, int pagenum) {
	CStringW pagecount;
	pagecount.Format(_T("%d"), pagenum);
	CStringW url = curl;
	url += L"&page=";
	url += pagecount;
	OutputDebugStringW(url);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(url, token);

	if (root["code"] == 0) {
		OutputDebugStringA("get pay info list success \n ");
		int size = root["data"].size();
		CStringW temp;

		allpaylisttotalpagenum = root["totalPage"].asInt();
		allpaylistcurrentpagenum = pagenum;

		int totalpagenum1 = root["totalPage"].asInt();
		char ee[20];
		sprintf_s(ee, "%d", totalpagenum1);
		CStringW totalpagenum2 = ee;
		m_labPageNumOnUnPay->SetText(totalpagenum2);

		sprintf_s(ee, "%d", pagenum);
		CStringW cpagenum = ee;
		m_labCurrentPageNumOnUnPay->SetText(cpagenum);

		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CListContainerElementUI* pLine;
				pLine = (CListContainerElementUI*)(builder.Create(_T("PaymentRecordLine.xml"), (UINT)0, this));
				if (pLine != NULL)
				{
					m_listAllpayUI->Add(pLine);
					CLabelUI* pMeetingStartTimeOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelMeetingStartTimeOnPayLine"));
					temp = UTF8ToWString(root["data"][i]["real_starttime"].asCString());
					pMeetingStartTimeOnPayLine->SetText(temp);

					CLabelUI* pMeetingEndTimeOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelMeetingEndTimeOnPayLine"));
					temp = UTF8ToWString(root["data"][i]["real_endtime"].asCString());
					pMeetingEndTimeOnPayLine->SetText(temp);

					CLabelUI* pExpertNameOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelExpertNameOnPayLine"));
					temp = UTF8ToWString(root["data"][i]["expert_name"].asCString());
					pExpertNameOnPayLine->SetText(temp);

					CLabelUI* pPatientNameOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientNameOnPayLine"));
					temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
					pPatientNameOnPayLine->SetText(temp);

					CLabelUI* pPayTypeOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelPayTypeOnPayLine"));
					if (root["data"][i]["pay_status"].asInt() == 0) {
						pPayTypeOnPayLine->SetText(_T("待支付"));
					}
					else {
						pPayTypeOnPayLine->SetText(_T("已支付"));
					}

					CLabelUI* pCostOnPayLine = (CLabelUI*)pLine->FindSubControl(_T("LabelCostOnPayLine"));
					int realfee = root["data"][i]["real_fee"].asInt();
					char ff[20];
					sprintf_s(ff, "%d", realfee);
					CStringW bb = ff;
					pCostOnPayLine->SetText(bb);
				}
			}
		}
	}
	else {
		OutputDebugStringA("get unpay info list failed \n ");
	}
}

int CDuiFrameWnd::HttpGetBillInfo() {
	Json::Value root = HDWinHttpGetWithToken(BILL_URL, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get bill info list success \n ");
		CStringW temp;
		char aa[20];
		temp = UTF8ToWString(root["data"]["start_day"].asCString());
		m_PaintManager.FindControl(_T("LabelBillStartTime"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["end_day"].asCString());
		m_PaintManager.FindControl(_T("LabelBillEndTime"))->SetText(temp);

		if (root["data"]["unpayAmount"].isInt()) {
			int cc = root["data"]["unpayAmount"].asInt();			
			sprintf_s(aa, "%d", cc);
			temp = aa;
		}
		else if(root["data"]["unpayAmount"].isString()){
			temp = UTF8ToWString(root["data"]["unpayAmount"].asCString());
		}		
		temp += L"元";		
		m_PaintManager.FindControl(_T("LabelBillToPay"))->SetText(temp);

		if (root["data"]["payedAmount"].isInt()) {
			int cc = root["data"]["payedAmount"].asInt();
			sprintf_s(aa, "%d", cc);
			temp = aa;
		}
		else if (root["data"]["payedAmount"].isString()) {
			temp = UTF8ToWString(root["data"]["payedAmount"].asCString());
		}
		temp += L"元";
		m_PaintManager.FindControl(_T("LabelBillPaid"))->SetText(temp);
		return 0;
	}
	else {
		OutputDebugStringA("get bill info list failed \n ");
		return -1;
	}
}



int CDuiFrameWnd::HttpGetExpertInfo() {
	Json::Value root = HDWinHttpGetWithToken(EXPERT_LIST_URL, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get expert info success \n ");
		int size = root["data"].size();
		CStringW temp;
		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CVerticalLayoutUI* pLine;
				pLine = (CVerticalLayoutUI*)(builder.Create(_T("ExpertInfoItem.xml"), (UINT)0, this));
				if (pLine != NULL)
				{
					m_tileExpertInfoUI->Add(pLine);

					CLabelUI* pExpertName = (CLabelUI*)pLine->FindSubControl(_T("LabelExpertNameOnExpertInfoItem"));
					if (root["data"][i]["name"].isString()) {
						temp = UTF8ToWString(root["data"][i]["name"].asCString());
						pExpertName->SetText(temp);
					}
					

					CRichEditUI* pExpertSkill = (CRichEditUI*)pLine->FindSubControl(_T("LabelExpertJobOnExpertInfoItem"));
					if (root["data"][i]["skill"].isString()) {
						temp = UTF8ToWString(root["data"][i]["skill"].asCString());
						pExpertSkill->SetText(temp);
					}
					

					CControlUI* pExpertImg = (CControlUI*)pLine->FindSubControl(_T("ControlExpertImgOnExpertInfoItem"));
					if (root["data"][i]["url"].isString()) {
						CStringW temp1 = UTF8ToWString(root["data"][i]["url"].asCString());
						pExpertImg->GetParent()->SetUserData(temp1);
					}

					if (root["data"][i]["head_img_thumb"].isString()) {
						temp = UTF8ToWString(root["data"][i]["head_img_thumb"].asCString());
						CStringW path1 = L"..\\hdtemp\\";
						path1 += L"expert-";
						path1 += temp.Mid(55);
						if (PathFileExists(path1) == TRUE) {
							OutputDebugStringA("\r\n file exist.\n");
							pExpertImg->SetBkImage(path1);
						}
						else {
							OutputDebugStringA("\r\n file not exist.\n");
							if (HDWinHttpDownloadFile(temp, path1) > 0) {
								pExpertImg->SetBkImage(path1);
								OutputDebugStringA("download success.\n");
							}
							else {
								pExpertImg->SetBkImage(L"bkimage/downloadimagefailed-1.png");
							}
						}
					}

				}
			}
		}
		return 1;
	}
	else {
		OutputDebugStringA("get expert info failed \n ");
		return -1;
	}

}


void CDuiFrameWnd::HttpGetExpertWorkTimeList() {
	Json::Value root = HDWinHttpGetWithToken(EXPERT_LIST_URL, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get expert list success \n ");
		int size = root["data"].size();
		CStringW temp;
		if (size >= 1) {
			//bool temp11 = root["data"][0]["free_time"]["1"].isArray();
			for (int i = 0; i < size; ++i) {
				CDialogBuilder builder;
				CListContainerElementUI* pLine;
				pLine = (CListContainerElementUI*)(builder.Create(_T("DoctorWorkTimeLine.xml"), (UINT)0, this));
				if (pLine != NULL)
				{
					m_listDoctorWorkInfoUI->Add(pLine);
					CVerticalLayoutUI* pExpertNameLay = (CVerticalLayoutUI*)pLine->FindSubControl(_T("container_expertname"));
					CLabelUI* pExpertName = (CLabelUI*)pExpertNameLay->FindSubControl(_T("ExpertNameOnExpertWTLine"));
					CControlUI *pExpertImg = (CControlUI*)pExpertNameLay->FindSubControl(_T("ControlExpertImgOnExpertTimeLine"));

					if (root["data"][i]["head_img_thumb"].isString()) {
						temp = UTF8ToWString(root["data"][i]["head_img_thumb"].asCString());
						CStringW path1 = L"..\\hdtemp\\";
						path1 += L"expert-";
						path1 += temp.Mid(55);
						if (PathFileExists(path1) == TRUE) {
							OutputDebugStringA("\r\n file exist.\n");
							pExpertImg->SetBkImage(path1);
						}
						else {
							OutputDebugStringA("\r\n file not exist.\n");
							if (HDWinHttpDownloadFile(temp, path1) > 0) {
								pExpertImg->SetBkImage(path1);
								OutputDebugStringA("download success.\n");
							}
							else {
								pExpertImg->SetBkImage(L"bkimage/downloadimagefailed-1.png");
							}
						}
					}

					temp = UTF8ToWString(root["data"][i]["name"].asCString());
					pExpertName->SetText(temp);
					if (root["data"][i]["free_time"].size() > 0) {
						if (root["data"][i]["free_time"]["1"].isNull()) {						
							int size1 = root["data"][i]["free_time"]["1"].size();
							for (int j = 0; j < size1; ++j) {
								CVerticalLayoutUI* pExpertWorkTime = (CVerticalLayoutUI*)pLine->FindSubControl(_T("ItemOfMonday"));
								CLabelUI* pExpertWorkTimeItem = (CLabelUI*)pExpertWorkTime->GetItemAt(j);
								temp = UTF8ToWString(root["data"][i]["free_time"]["1"][j].asCString());
								pExpertWorkTimeItem->SetText(temp);
							}
						
						}
						
						if (!(root["data"][i]["free_time"]["2"].isNull())) {
							int size1 = root["data"][i]["free_time"]["2"].size();
							for (int j = 0; j < size1; ++j) {
								CVerticalLayoutUI* pExpertWorkTime = (CVerticalLayoutUI*)pLine->GetItemAt(2);
								CLabelUI* pExpertWorkTimeItem = (CLabelUI*)pExpertWorkTime->GetItemAt(j);
								temp = UTF8ToWString(root["data"][i]["free_time"]["2"][j].asCString());
								pExpertWorkTimeItem->SetText(temp);
							}
						}
						if (!(root["data"][i]["free_time"]["3"].isNull())) {
							int size1 = root["data"][i]["free_time"]["3"].size();
							for (int j = 0; j < size1; ++j) {
								CVerticalLayoutUI* pExpertWorkTime = (CVerticalLayoutUI*)pLine->GetItemAt(3);
								CLabelUI* pExpertWorkTimeItem = (CLabelUI*)pExpertWorkTime->GetItemAt(j);
								temp = UTF8ToWString(root["data"][i]["free_time"]["3"][j].asCString());
								pExpertWorkTimeItem->SetText(temp);
							}
						}
						if (!(root["data"][i]["free_time"]["4"].isNull())) {
							int size1 = root["data"][i]["free_time"]["4"].size();
							for (int j = 0; j < size1; ++j) {
								CVerticalLayoutUI* pExpertWorkTime = (CVerticalLayoutUI*)pLine->GetItemAt(4);
								CLabelUI* pExpertWorkTimeItem = (CLabelUI*)pExpertWorkTime->GetItemAt(j);
								temp = UTF8ToWString(root["data"][i]["free_time"]["4"][j].asCString());
								pExpertWorkTimeItem->SetText(temp);
							}
						}
						if (!(root["data"][i]["free_time"]["5"].isNull())) {
							int size1 = root["data"][i]["free_time"]["5"].size();
							for (int j = 0; j < size1; ++j) {
								CVerticalLayoutUI* pExpertWorkTime = (CVerticalLayoutUI*)pLine->GetItemAt(5);
								CLabelUI* pExpertWorkTimeItem = (CLabelUI*)pExpertWorkTime->GetItemAt(j);
								temp = UTF8ToWString(root["data"][i]["free_time"]["5"][j].asCString());
								pExpertWorkTimeItem->SetText(temp);
							}
						}
						if (!(root["data"][i]["free_time"]["6"].isNull())) {
							int size1 = root["data"][i]["free_time"]["6"].size();
							for (int j = 0; j < size1; ++j) {
								CVerticalLayoutUI* pExpertWorkTime = (CVerticalLayoutUI*)pLine->GetItemAt(6);
								CLabelUI* pExpertWorkTimeItem = (CLabelUI*)pExpertWorkTime->GetItemAt(j);
								temp = UTF8ToWString(root["data"][i]["free_time"]["6"][j].asCString());
								pExpertWorkTimeItem->SetText(temp);
							}
						}
						if (!(root["data"][i]["free_time"]["0"].isNull())) {
							int size1 = root["data"][i]["free_time"]["0"].size();
							for (int j = 0; j < size1; ++j) {
								CVerticalLayoutUI* pExpertWorkTime = (CVerticalLayoutUI*)pLine->GetItemAt(7);
								CLabelUI* pExpertWorkTimeItem = (CLabelUI*)pExpertWorkTime->GetItemAt(j);
								temp = UTF8ToWString(root["data"][i]["free_time"]["0"][j].asCString());
								pExpertWorkTimeItem->SetText(temp);
							}
						}
					}
				}
			}
		}
	}
	else {
		OutputDebugStringA("get record list failed \n ");
	}

}

int CDuiFrameWnd::HttpGetUserInfo() {
	Json::Value root = HDWinHttpGetWithToken(USER_INFO_URL, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get user info success \n ");			
		if (root["data"]["clinic"].isNull()) {
			return 9;
		}
		else {
			if (root["data"]["clinic"]["verify_status"] == 1) {
				return 1;
			}
			else if (root["data"]["clinic"]["verify_status"] == 2) {
				CStringW temp = UTF8ToWString(root["data"]["clinic"]["name"].asCString());
				m_labTitleClinicNameUI->SetText(temp);
				ClinicUserName = temp;
				return 2;
			}
			else if (root["data"]["clinic"]["verify_status"] == 3) {
				CStringW temp = L"";
				if (!(root["data"]["clinic"]["verify_reason"].isNull())) {
					OutputDebugStringW(L"1111111\n");
					temp = UTF8ToWString(root["data"]["clinic"]["verify_reason"].asCString());
					m_PaintManager.FindControl(_T("LabelClinicInfoErrorReasonOnResignClinic"))->SetText(temp);
				}
				if (root["data"]["clinic"]["name"].isString()) {
					temp = UTF8ToWString(root["data"]["clinic"]["name"].asCString());
					m_PaintManager.FindControl(_T("EditClinicNameOnResignClinic"))->SetText(temp);
				}
				if (root["data"]["clinic"]["address"].isString()) {
					temp = UTF8ToWString(root["data"]["clinic"]["address"].asCString());
					m_PaintManager.FindControl(_T("EditClinicAddrOnResignClinic"))->SetText(temp);
				}
				if (root["data"]["clinic"]["tel"].isString()) {
					temp = UTF8ToWString(root["data"]["clinic"]["tel"].asCString());
					m_PaintManager.FindControl(_T("EditClinicPhoneOnResignClinic"))->SetText(temp);
				}
				if (root["data"]["clinic"]["chief"].isString()) {
					temp = UTF8ToWString(root["data"]["clinic"]["chief"].asCString());
					m_PaintManager.FindControl(_T("EditCorporationNameOnResignClinic"))->SetText(temp);
				}
				if (root["data"]["clinic"]["idcard"].isString()) {
					temp = UTF8ToWString(root["data"]["clinic"]["idcard"].asCString());
					m_PaintManager.FindControl(_T("EditCorporationIDOnResignClinic"))->SetText(temp);
				}

				if (root["data"]["clinic"]["Business_license_img"].isString()) {
					temp = UTF8ToWString(root["data"]["clinic"]["Business_license_img"].asCString());
					CStringW path1 = L"";
					if (temp.Right(3) == L"jpg") {
						path1 = L"..\\hdtemp\\cliniclisence.jpg";
					}
					else if (temp.Right(3) == L"png") {
						path1 = L"..\\hdtemp\\cliniclisence.png";
					}
					if (PathFileExists(path1) == TRUE) {
						OutputDebugStringA("\r\n file exist.\n");
						m_btnReUploadLicenseUI->SetBkImage(path1);
					}
					else {
						OutputDebugStringA("\r\n file not exist.\n");
						if (HDWinHttpDownloadFile(temp, path1) > 0) {
							m_btnReUploadLicenseUI->SetBkImage(path1);
							OutputDebugStringA("download success.\n");
						}
						else {
							m_btnReUploadLicenseUI->SetBkImage(L"bkimage/downloadimagefailed-2.png");
						}
					}
					m_btnReUploadLicenseUI->SetUserData(temp);					
				}

				if (root["data"]["clinic"]["local_img"].isString()) {
					temp = UTF8ToWString(root["data"]["clinic"]["local_img"].asCString());
					CStringW path2 = L"";
					if (temp.Right(3) == L"jpg") {
						path2 = L"..\\hdtemp\\photo.jpg";
					}
					else if (temp.Right(3) == L"png") {
						path2 = L"..\\hdtemp\\photo.png";
					}
					if (PathFileExists(path2) == TRUE) {
						OutputDebugStringA("\r\n file exist.\n");
						m_btnReUploadPhotoUI->SetBkImage(path2);
					}
					else {
						OutputDebugStringA("\r\n file not exist.\n");
						if (HDWinHttpDownloadFile(temp, path2) > 0) {
							m_btnReUploadPhotoUI->SetBkImage(path2);
							OutputDebugStringA("download success.\n");
						}
						else {
							m_btnReUploadPhotoUI->SetBkImage(L"bkimage/downloadimagefailed-2.png");
						}
					}
					m_btnReUploadPhotoUI->SetUserData(temp);
				}

				if (root["data"]["clinic"]["doctor_certificate_img"].isString()) {
					temp = UTF8ToWString(root["data"]["clinic"]["doctor_certificate_img"].asCString());
					CStringW path3 = L"";
					if (temp.Right(3) == L"jpg") {
						path3 = L"..\\hdtemp\\certificatev.jpg";
					}
					else if (temp.Right(3) == L"png") {
						path3 = L"..\\hdtemp\\certificatev.png";
					}
					if (PathFileExists(path3) == TRUE) {
						OutputDebugStringA("\r\n file exist.\n");
						m_btnReUploadDoctorLicenseUI->SetBkImage(path3);
					}
					else {
						OutputDebugStringA("\r\n file not exist.\n");
						if (HDWinHttpDownloadFile(temp, path3) > 0) {
							m_btnReUploadDoctorLicenseUI->SetBkImage(path3);
							OutputDebugStringA("download success.\n");
						}
						else {
							m_btnReUploadDoctorLicenseUI->SetBkImage(L"bkimage/downloadimagefailed-2.png");
						}
					}
					m_btnReUploadDoctorLicenseUI->SetUserData(temp);
				}
				return 3;
			}
			else {
				return -1;
			}
		}
	}
	else {
		OutputDebugStringA("get user info failed \n ");
		return -1;
	}
}


int CDuiFrameWnd::HttpGetClinicInfo() {
	Json::Value root = HDWinHttpGetWithToken(USER_INFO_URL, token);
	CStringW temp;
	if (root["code"] == 0) {
		OutputDebugStringA("get clinic info success \n ");
			if (root["data"]["clinic"]["verify_status"] == 2) {
				temp = UTF8ToWString(root["data"]["username"].asCString());
				m_PaintManager.FindControl(_T("LabelUserNameOnUserInfo"))->SetText(temp);

				temp = UTF8ToWString(root["data"]["email"].asCString());
				m_PaintManager.FindControl(_T("LabelUserEmailOnUserInfo"))->SetText(temp);

				temp = UTF8ToWString(root["data"]["clinic"]["name"].asCString());
				m_PaintManager.FindControl(_T("LabelClinicNameOnClinicInfo"))->SetText(temp);

				temp = UTF8ToWString(root["data"]["clinic"]["address"].asCString());
				m_PaintManager.FindControl(_T("LabelClinicAddrOnClinicInfo"))->SetText(temp);

				temp = UTF8ToWString(root["data"]["clinic"]["tel"].asCString());
				m_PaintManager.FindControl(_T("LabelClinicTelOnClinicInfo"))->SetText(temp);
					
				temp = UTF8ToWString(root["data"]["clinic"]["chief"].asCString());
				m_PaintManager.FindControl(_T("LabelClinicChiefOnClinicInfo"))->SetText(temp);

				temp = UTF8ToWString(root["data"]["clinic"]["idcard"].asCString());
				m_PaintManager.FindControl(_T("LabelClinicIDCardOnClinicInfo"))->SetText(temp);

				temp = UTF8ToWString(root["data"]["clinic"]["Business_license_img"].asCString());
				CStringW path1 = L"..\\hdtemp\\cliniclisence.";
				if (temp.Right(3) == L"jpg") {
					path1 += L"jpg";
				}
				else if (temp.Right(3) == L"png") {
					path1 += L"png";
				}
				if (PathFileExists(path1) == TRUE) {
					OutputDebugStringA("\r\n file exist.\n");
					m_PaintManager.FindControl(_T("LabelClinicBusinessLisenceOnClinicInfo"))->SetBkImage(path1);
				}
				else {
					OutputDebugStringA("\r\n file not exist.\n");
					if (HDWinHttpDownloadFile(temp, path1) > 0) {
						m_PaintManager.FindControl(_T("LabelClinicBusinessLisenceOnClinicInfo"))->SetBkImage(path1);
						OutputDebugStringA("download success.\n");
					}
					else {
						m_PaintManager.FindControl(_T("LabelClinicBusinessLisenceOnClinicInfo"))->SetBkImage(L"bkimage/downloadimagefailed-2.png");
					}
				}
			
				temp = UTF8ToWString(root["data"]["clinic"]["local_img"].asCString());
				CStringW path2 = L"..\\hdtemp\\photo.";
				if (temp.Right(3) == L"jpg") {
					path2 += L"jpg";
				}
				else if (temp.Right(3) == L"png") {
					path2 += L"png";
				}
				if (PathFileExists(path2) == TRUE) {
					OutputDebugStringA("\r\n file exist.\n");
					m_PaintManager.FindControl(_T("LabelClinicLocalPhotoOnClinicInfo"))->SetBkImage(path2);
				}
				else {
					OutputDebugStringA("\r\n file not exist.\n");
					if (HDWinHttpDownloadFile(temp, path2) > 0) {
						m_PaintManager.FindControl(_T("LabelClinicLocalPhotoOnClinicInfo"))->SetBkImage(path2);
						OutputDebugStringA("download success.\n");
					}
					else {
						m_PaintManager.FindControl(_T("LabelClinicLocalPhotoOnClinicInfo"))->SetBkImage(L"bkimage/downloadimagefailed-2.png");
					}
				}


				temp = UTF8ToWString(root["data"]["clinic"]["doctor_certificate_img"].asCString());
				CStringW path3 = L"..\\hdtemp\\certificatev.";
				if (temp.Right(3) == L"jpg") {
					path3 += L"jpg";
				}
				else if (temp.Right(3) == L"png") {
					path3 += L"png";
				}
				if (PathFileExists(path3) == TRUE) {
					OutputDebugStringA("\r\n file exist.\n");
					m_PaintManager.FindControl(_T("LabelClinicDoctorLisenceOnClinicInfo"))->SetBkImage(path3);
				}
				else {
					OutputDebugStringA("\r\n file not exist.\n");
					if (HDWinHttpDownloadFile(temp, path3) > 0) {
						m_PaintManager.FindControl(_T("LabelClinicDoctorLisenceOnClinicInfo"))->SetBkImage(path3);
						OutputDebugStringA("download success.\n");
					}
					else {
						m_PaintManager.FindControl(_T("LabelClinicDoctorLisenceOnClinicInfo"))->SetBkImage(L"bkimage/downloadimagefailed-2.png");
					}
				}
				return 2;
			}
			else {
				return -1;
			}
	}
	else {
		OutputDebugStringA("get clinic info failed \n ");
		return -1;
	}
}

int CDuiFrameWnd::HttpGetUserInfoUpdate() {
	Json::Value root = HDWinHttpGetWithToken(USER_INFO_URL, token);
	CStringW temp;
	if (root["code"] == 0) {
		OutputDebugStringA("get user info update success \n ");
		if (root["data"]["clinic"]["verify_status"] == 2) {
			temp = UTF8ToWString(root["data"]["username"].asCString());
			m_PaintManager.FindControl(_T("EditUserNameOnUserInfo"))->SetText(temp);

			temp = UTF8ToWString(root["data"]["email"].asCString());
			m_PaintManager.FindControl(_T("EditUserEmailOnUserInfo"))->SetText(temp);
		}
		return 1;
	}
	else {
		OutputDebugStringA("get user info update failed \n ");
		return -1;
	}
}


int CDuiFrameWnd::HttpCheckClinicUpdatePermit() {
	Json::Value root = HDWinHttpGetWithToken(USER_UPDATE_CLINUCINFO_PERMIT_URL, token);
	CStringW temp;
	if (root["code"] == 0) {
		OutputDebugStringA("get clinic info change permit success \n ");
		return 1;

	}
	else {
		OutputDebugStringA("get user clinic info change permit failed \n ");
		return -1;
	}
}


int CDuiFrameWnd::HttpGetClinicInfoUpdate() {
	Json::Value root = HDWinHttpGetWithToken(USER_INFO_URL, token);
	CStringW temp;
	if (root["code"] == 0) {
		OutputDebugStringA("get clinic info update success \n ");
		if (root["data"]["clinic"]["verify_status"] == 2) {
			temp = UTF8ToWString(root["data"]["clinic"]["name"].asCString());
			m_PaintManager.FindControl(_T("EditClinicNameOnClinicInfo"))->SetText(temp);

			temp = UTF8ToWString(root["data"]["clinic"]["address"].asCString());
			m_PaintManager.FindControl(_T("EditClinicAddrOnClinicInfo"))->SetText(temp);

			temp = UTF8ToWString(root["data"]["clinic"]["tel"].asCString());
			m_PaintManager.FindControl(_T("EditClinicTelOnClinicInfo"))->SetText(temp);

			temp = UTF8ToWString(root["data"]["clinic"]["chief"].asCString());
			m_PaintManager.FindControl(_T("EditClinicChiefOnClinicInfo"))->SetText(temp);

			temp = UTF8ToWString(root["data"]["clinic"]["idcard"].asCString());
			m_PaintManager.FindControl(_T("EditClinicIDCardOnClinicInfo"))->SetText(temp);

			temp = UTF8ToWString(root["data"]["clinic"]["Business_license_img"].asCString());
			m_btnUploadNewLicenseUI->SetUserData(temp);
			if (temp.Right(3) == L"jpg") {
				m_btnUploadNewLicenseUI->SetBkImage(L"..\\hdtemp\\cliniclisence.jpg");
			}
			else if(temp.Right(3) == L"png"){
				m_btnUploadNewLicenseUI->SetBkImage(L"..\\hdtemp\\cliniclisence.png");
			}
			
			temp = UTF8ToWString(root["data"]["clinic"]["local_img"].asCString());
			m_btnUploadNewPhotoUI->SetUserData(temp);
			if (temp.Right(3) == L"jpg") {
				m_btnUploadNewPhotoUI->SetBkImage(L"..\\hdtemp\\photo.jpg");
			}
			else if (temp.Right(3) == L"png") {
				m_btnUploadNewPhotoUI->SetBkImage(L"..\\hdtemp\\photo.png");
			}
			
			temp = UTF8ToWString(root["data"]["clinic"]["doctor_certificate_img"].asCString());
			m_btnUploadNewDoctorLicenseUI->SetUserData(temp);
			if (temp.Right(3) == L"jpg") {
				m_btnUploadNewDoctorLicenseUI->SetBkImage(L"..\\hdtemp\\certificatev.jpg");
			}
			else if (temp.Right(3) == L"png") {
				m_btnUploadNewDoctorLicenseUI->SetBkImage(L"..\\hdtemp\\certificatev.png");
			}
		}
		return 1;
	}
	else {
		OutputDebugStringA("get clinic info update failed \n ");
		return -1;
	}
}


int CDuiFrameWnd::HttpCreateAppointment(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(USER_CREATE_APPOINTMENT_URL, poststr, token);
	if (root["code"] == 0) {
		OutputDebugStringA("create appointment success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("create appointment failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}



int CDuiFrameWnd::HttpUpdateAppointment(CStringA poststr) {
	OutputDebugStringA(poststr);

	Json::Value root = HDWinHttpPostWithToken(USER_UPDATE_APPOINTMENT_SUBMIT_URL, poststr, token);
	if (root["code"] == 0) {
		OutputDebugStringA("update appointment success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("update appointment failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}

}


int CDuiFrameWnd::HttpGetUserPayStatus() {
	Json::Value root = HDWinHttpGetWithToken(USER_PAY_STATUS_URL, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get user pay status success \n ");
		int num = root["data"]["nums"].asInt();
		return num;
	}
	else {
		OutputDebugStringA("get user pay status failed \n ");
		return -1;
	}

}

void CDuiFrameWnd::HttpGetAppointmentDetail(LPCTSTR appointmentnumber) {
	CStringW curl = APPOINTMENT_DETAIL_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get appointment info success \n ");
		temp = UTF8ToWString(root["data"]["expert"]["name"].asCString());
		m_PaintManager.FindControl(_T("LabalExpertOnAMDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["order_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabalStartTimeOnAMDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["order_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabalEndTimeOnAMDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("LabalPatientOnAMDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("LabalIllnessInfoOnAMDetail"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "")&& (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1"].asCString());
			m_btnShowPatientPic1UIOnAMDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			OutputDebugStringA("----patient_img1.\n");
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			path1 += L"-pic1-";
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE){
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic1UIOnAMDetail->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					OutputDebugStringA("download success.\n");
					m_btnShowPatientPic1UIOnAMDetail->SetBkImage(path1);
				}
				else {
					m_btnShowPatientPic1UIOnAMDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
			
		}
		else {
			m_btnShowPatientPic1UIOnAMDetail->SetVisible(false);
		}

		if ((root["data"]["patient_img2_thumb"] != "") && (root["data"]["patient_img2_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img2"].asCString());
			m_btnShowPatientPic2UIOnAMDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img2_thumb"].asCString());
			CStringW path2 = L"..\\hdtemp\\";
			path2 += appointmentnumber;
			//int index = temp.ReverseFind('\/');
			path2 += L"-pic2-";
			path2 += temp.Mid(60);
			if (PathFileExists(path2) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic2UIOnAMDetail->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					m_btnShowPatientPic2UIOnAMDetail->SetBkImage(path2);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic2UIOnAMDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}

		}
		else {
			m_btnShowPatientPic2UIOnAMDetail->SetVisible(false);
		}

		if ((root["data"]["patient_img3_thumb"] != "") && (root["data"]["patient_img3_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img3"].asCString());
			m_btnShowPatientPic3UIOnAMDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img3_thumb"].asCString());
			CStringW path3 = L"..\\hdtemp\\";
			path3 += appointmentnumber;
			path3 += L"-pic3-";
			//int index = temp.ReverseFind('\/');
			path3 += temp.Mid(60);
			if (PathFileExists(path3) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic3UIOnAMDetail->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_btnShowPatientPic3UIOnAMDetail->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic3UIOnAMDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}
		else {
			m_btnShowPatientPic3UIOnAMDetail->SetVisible(false);
		}

		int cost = root["data"]["order_fee"].asInt();
		char aa[20];
		sprintf_s(aa, "%d", cost);
		CStringW thecost = aa;
		m_PaintManager.FindControl(_T("LabalCostFeeOnAMDetail"))->SetText(thecost);
	}
	else {
		OutputDebugStringA("get appointment info failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
	}
}



int CDuiFrameWnd::HttpGetAppointmentUpdate(LPCTSTR appointmentnumber) {
	CStringW curl = USER_UPDATE_APPOINTMENT_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get appointment update info success \n ");

		unsigned __int64 appnum = root["data"]["appointment_no"].asInt64();
		char aa[20];
		sprintf_s(aa, "%llu", appnum);
		editappointmentno = aa;

		temp = UTF8ToWString(root["data"]["expert"]["name"].asCString());
		m_PaintManager.FindControl(_T("LabalExpertOnEditAM"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["expert_uuid"].asCString());
		m_PaintManager.FindControl(_T("LabalExpertOnEditAM"))->SetUserData(temp);

		temp = UTF8ToWString(root["data"]["order_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabalStartTimeOnEditAM"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["order_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabalEndTimeOnEditAM"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("EditPatientNameOnEditAM"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("EditPatientDescOnEditAM"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "") && (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1"].asCString());
			m_btnUploadNewPatientPic1UI->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			//int index = temp.ReverseFind('/');
			path1 += L"-pic1-";
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnUploadNewPatientPic1UI->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					m_btnUploadNewPatientPic1UI->SetBkImage(path1);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnUploadNewPatientPic1UI->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		if ((root["data"]["patient_img2_thumb"] != "") && (root["data"]["patient_img2_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img2"].asCString());
			m_btnUploadNewPatientPic2UI->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img2_thumb"].asCString());
			CStringW path2 = L"..\\hdtemp\\";
			path2 += appointmentnumber;
			//int index = temp.ReverseFind('/');
			path2 += L"-pic2-";
			path2 += temp.Mid(60);
			if (PathFileExists(path2) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnUploadNewPatientPic2UI->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					m_btnUploadNewPatientPic2UI->SetBkImage(path2);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnUploadNewPatientPic2UI->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		if ((root["data"]["patient_img3_thumb"] != "") && (root["data"]["patient_img3_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img3"].asCString());
			m_btnUploadNewPatientPic3UI->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img3_thumb"].asCString());
			CStringW path3 = L"..\\hdtemp\\";
			path3 += appointmentnumber;
			path3 += L"-pic3-";
			//int index = temp.ReverseFind('/');
			path3 += temp.Mid(60);
			if (PathFileExists(path3) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnUploadNewPatientPic3UI->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_btnUploadNewPatientPic3UI->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnUploadNewPatientPic3UI->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}

		}

		int cost = root["data"]["order_fee"].asInt();
		sprintf_s(aa, "%d", cost);
		CStringW thecost = aa;
		m_PaintManager.FindControl(_T("LabelCostMoneyOnEditAM"))->SetText(thecost);
		
		return 1;
	}
	else {
		OutputDebugStringA("get appointment update info failed \n ");
		return -1;
	}
}

int	CDuiFrameWnd::HttpCancelAppointment(LPCTSTR appointmentnumber)
{	
	CStringA poststr = "appointment_no=";
	poststr += appointmentnumber;
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");

	Json::Value root = HDWinHttpPostWithToken(USER_CANCEL_APPOINTMENT_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("cancel appointment success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("cancel appointment failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}

}



void CDuiFrameWnd::HttpGetRecordDetail(LPCTSTR appointmentnumber) {
	CStringW curl = RECORD_DETAIL_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get record info success \n ");
		temp = UTF8ToWString(root["data"]["expert_name"].asCString());
		m_PaintManager.FindControl(_T("LabelExpertNameOnRecordDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabelStartTimeOnRecordDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabelEndTimeOnRecordDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientDescNameOnRecordDetail"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "") && (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1"].asCString());
			m_btnShowPatientPic1UIOnRDDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			//int index = temp.ReverseFind('/');
			path1 += L"-pic1-";
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic1UIOnRDDetail->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					m_btnShowPatientPic1UIOnRDDetail->SetBkImage(path1);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic1UIOnRDDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}
		else {
			m_btnShowPatientPic1UIOnRDDetail->SetVisible(false);
		}

		if ((root["data"]["patient_img2_thumb"] != "") && (root["data"]["patient_img2_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img2"].asCString());
			m_btnShowPatientPic2UIOnRDDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img2_thumb"].asCString());
			CStringW path2 = L"..\\hdtemp\\";
			path2 += appointmentnumber;
			path2 += L"-pic2-";
			//int index = temp.ReverseFind('/');
			path2 += temp.Mid(60);
			if (PathFileExists(path2) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic2UIOnRDDetail->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					m_btnShowPatientPic2UIOnRDDetail->SetBkImage(path2);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic2UIOnRDDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}	
		}
		else {
			m_btnShowPatientPic2UIOnRDDetail->SetVisible(false);
		}

		if ((root["data"]["patient_img3_thumb"] != "") && (root["data"]["patient_img3_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img3"].asCString());
			m_btnShowPatientPic3UIOnRDDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img3_thumb"].asCString());
			CStringW path3 = L"..\\hdtemp\\";
			path3 += appointmentnumber;
			path3 += L"-pic3-";
			//int index = temp.ReverseFind('/');
			path3 += temp.Mid(60);
			if (PathFileExists(path3) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic3UIOnRDDetail->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_btnShowPatientPic3UIOnRDDetail->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic3UIOnRDDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}
		else {
			m_btnShowPatientPic3UIOnRDDetail->SetVisible(false);
		}

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientNameOnRecordDetail"))->SetText(temp);

		int age = root["data"]["patient_age"].asInt();
		if (age != 0) {
			char aa[20];
			sprintf_s(aa, "%d", age);
			CStringW bb = aa;
			m_PaintManager.FindControl(_T("LabelPatientAgeOnRecordDetail"))->SetText(bb);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientAgeOnRecordDetail"))->SetText(L"");
		}

		int sex = root["data"]["patient_gender"].asInt();
		if (sex == 2) {
			m_PaintManager.FindControl(_T("LabelPatientSexOnRecordDetail"))->SetText(_T("女"));
		}
		else if (sex == 1) {
			m_PaintManager.FindControl(_T("LabelPatientSexOnRecordDetail"))->SetText(_T("男"));
		}

		if (root["data"]["patient_mobile"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_mobile"].asCString());
			m_PaintManager.FindControl(_T("LabelPatientPhoneOnRecordDetail"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientPhoneOnRecordDetail"))->SetText(L"");
		}

		if (root["data"]["patient_idcard"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_idcard"].asCString());
			m_PaintManager.FindControl(_T("LabelPatientIDOnRecordDetail"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientIDOnRecordDetail"))->SetText(L"");
		}

		if (root["data"]["expert_diagnosis"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_diagnosis"].asCString());
			m_PaintManager.FindControl(_T("LabelDiagnosisDescOnRecordDetail"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelDiagnosisDescOnRecordDetail"))->SetText(L"");
		}

		if (root["data"]["expert_advise"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_advise"].asCString());
			m_PaintManager.FindControl(_T("LabelExpertAdviseOnRecordDetail"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelExpertAdviseOnRecordDetail"))->SetText(L"");
		}
		CStringW mp3url = root["data"]["audio_url"].asCString();
		m_PaintManager.FindControl(_T("ButtonPlayMp3Record"))->SetUserData(mp3url);
	}
	else {
		OutputDebugStringA("get record info failed \n ");
	}

}



void CDuiFrameWnd::HttpGetRecordUpdate(LPCTSTR appointmentnumber) {
	CStringW curl = USER_UPDATE_RECORD_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get record info update success \n ");

		unsigned __int64 appnum = root["data"]["appointment_no"].asInt64();
		char aa[20];
		sprintf_s(aa, "%llu", appnum);
		editrecordappointmentno = aa;

		temp = UTF8ToWString(root["data"]["expert_name"].asCString());
		m_PaintManager.FindControl(_T("LabelExpertNameOnRecordEdit"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabelStartTimeOnRecordEdit"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabelEndTimeOnRecordEdit"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientDescOnRecordEdit"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "") && (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			path1 += L"-pic1-";
			//int index = temp.ReverseFind('/');
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnRecordEdit"))->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnRecordEdit"))->SetBkImage(path1);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnRecordEdit"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		if ((root["data"]["patient_img2_thumb"] != "") && (root["data"]["patient_img2_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img2_thumb"].asCString());
			CStringW path2 = L"..\\hdtemp\\";
			path2 += appointmentnumber;
			path2 += L"-pic2-";
			//int index = temp.ReverseFind('/');
			path2 += temp.Mid(60);
			if (PathFileExists(path2) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnRecordEdit"))->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnRecordEdit"))->SetBkImage(path2);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnRecordEdit"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		if ((root["data"]["patient_img3_thumb"] != "") && (root["data"]["patient_img3_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img3_thumb"].asCString());
			CStringW path3 = L"..\\hdtemp\\";
			path3 += appointmentnumber;
			//int index = temp.ReverseFind('/');
			path3 += L"-pic3-";
			path3 += temp.Mid(60);
			if (PathFileExists(path3) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnRecordEdit"))->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnRecordEdit"))->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnRecordEdit"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientNameOnRecordEdit"))->SetText(temp);

		int age = root["data"]["patient_age"].asInt();
		if (age != 0) {
			char aa[20];
			sprintf_s(aa, "%d", age);
			CStringW bb = aa;
			m_PaintManager.FindControl(_T("EditPatientAgeOnRecordEdit"))->SetText(bb);
		}
		else {
			m_PaintManager.FindControl(_T("EditPatientAgeOnRecordEdit"))->SetText(L"");
		}

		int sex = root["data"]["patient_gender"].asInt();
		if (sex == 1) {
			m_optionChooseMaleOnEditRecord->Selected(true);
			m_optionChooseFemaleOnEditRecord->Selected(false);
		}
		else {
			m_optionChooseMaleOnEditRecord->Selected(false);
			m_optionChooseFemaleOnEditRecord->Selected(true);
		}

		if (root["data"]["patient_mobile"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_mobile"].asCString());
			m_PaintManager.FindControl(_T("EditPatientPhoneOnRecordEdit"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("EditPatientPhoneOnRecordEdit"))->SetText(L"");
		}

		if (root["data"]["patient_idcard"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_idcard"].asCString());
			m_PaintManager.FindControl(_T("EditPatientIDOnRecordEdit"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("EditPatientIDOnRecordEdit"))->SetText(L"");
		}

		if (root["data"]["expert_diagnosis"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_diagnosis"].asCString());
			m_PaintManager.FindControl(_T("EditDiagnosisDescOnRecordEdit"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("EditDiagnosisDescOnRecordEdit"))->SetText(L"");
		}

		if (root["data"]["expert_advise"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_advise"].asCString());
			m_PaintManager.FindControl(_T("LabelExpertAdviseOnRecordEdit"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelExpertAdviseOnRecordEdit"))->SetText(L"");
		}
	}
	else {
		OutputDebugStringA("get record info update failed \n ");
	}
}


int CDuiFrameWnd::HttpUpdateRecord(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(USER_UPDATE_RECORD_SUBMIT_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("update record success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("update record failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}


int CDuiFrameWnd::HttpUpdateUserInfo(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(USER_UPDATE_USERINFO_SUBMIT_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("update user info success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("update user info failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}


int CDuiFrameWnd::HttpUpdateClinicInfo(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(USER_UPDATE_CLINICINFO_SUBMIT_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("update clinic info success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("update clinic info failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}


int CDuiFrameWnd::HttpSignUpUser(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPost(SIGN_UP_URL, poststr);

	if (root["code"] == 0) {
		OutputDebugStringA("sign up user success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("sign up user failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}


int CDuiFrameWnd::HttpSignUpClinic(CStringA poststr) {
	//OutputDebugStringA(poststr);
	//OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(SIGN_UP_CLINIC_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("sign up clinic success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("sign up clinic failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}



int CDuiFrameWnd::HttpResetPassword(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPost(RESET_PASSWORD_URL, poststr);

	if (root["code"] == 0) {
		OutputDebugStringA("reset pwd success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("reset pwd failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}



int CDuiFrameWnd::HttpSendCode(LPCTSTR mobile) {
	CStringW curl = SEND_CODE_URL;
	curl += L"?mobile=";
	curl += mobile;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGet(curl);

	if (root["code"] == 0) {
		OutputDebugStringA("send code success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("send code failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}

int CDuiFrameWnd::HttpGetMeetingNumber(LPCTSTR appointmentnumber) {

	CStringW curl = GET_MEETINT_NUMBER_URL;
	curl += L"?appointment_no=";
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		OutputDebugStringA("get meeting number  success \n ");
		//zhumukey = UTF8ToWString(root["data"]["app_key"].asCString());
		//zhumusecret = UTF8ToWString(root["data"]["app_secret"].asCString());
		meetingnum = UTF8ToWString(root["data"]["meeting_number"].asCString());
		return 1;
	}
	else {
		OutputDebugStringA("get meeting number  failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}

Json::Value CDuiFrameWnd::HttpUploadClinicIMG(CStringW path) {
	CStringW curl = UPLOAD_CLINIC_IMG_URL;
	VecStParam aa;
	CStringW filekey = L"Filedata";
	Json::Reader reader;
	Json::Value root;
	Json::Value errormesg;
	errormesg["code"] = -1;
	errormesg["message"] = WStringToUTF8(L"网络错误。");
	CHttpUploadFiles* m_pUploadFiles = new CHttpUploadFiles();
	m_pUploadFiles->token1 = token;
	BOOL result = m_pUploadFiles->TransDataToServer(curl.GetString(), aa,
		path.GetString(), filekey.GetString());
	if (result) {
		reader.parse(m_pUploadFiles->resultdata, root, false);
		delete m_pUploadFiles;
		return root;
	}
	else {
		OutputDebugStringA("upload clinic img failed \n ");
		delete m_pUploadFiles;
		return errormesg;
	}
}

Json::Value CDuiFrameWnd::HttpUploadPatientIMG(CStringW path) {
	CStringW curl = UPLOAD_CLINIC_IMG_URL;
	VecStParam aa;
	CStringW filekey = L"Filedata";
	Json::Reader reader;
	Json::Value root;
	Json::Value errormesg;
	errormesg["code"] = -1;
	errormesg["message"] = WStringToUTF8(L"网络错误。");
	CHttpUploadFiles* m_pUploadFiles = new CHttpUploadFiles();
	m_pUploadFiles->token1 = token;
	BOOL result = m_pUploadFiles->TransDataToServer(curl.GetString(), aa,
		path.GetString(), filekey.GetString());
	if (result) {
		reader.parse(m_pUploadFiles->resultdata, root, false);
		delete m_pUploadFiles;
		return root;
	}
	else {
		OutputDebugStringA("upload clinic img failed \n ");
		delete m_pUploadFiles;
		return errormesg;
	}
}


void CDuiFrameWnd::OnClick(TNotifyUI &msg) {

	if (msg.pSender == m_btnLoginUI)
	{	
		CStringA logmsg = "登录。\r\n";
		HDWriteLog(logmsg);
		CString username = m_editMobileOnLoginUI->GetText();
		CString pwd = m_editPWDOnLoginUI->GetText();
		if (HttpLogin(username, pwd) > 0) {
			CStringW temp = L"";
			int userstatus = HttpGetUserInfo();
			if (userstatus == 1) {
				m_containerLoginUI->SetVisible(false);
				m_containerAuditingUI->SetVisible(true);
				m_containerTitleUI->SetBkImage(L"bkimage/top-bg.png");
				m_containerResetPwdUI->SetVisible(false);
				m_containerSignUpUI->SetVisible(false);
				m_containerSignInfoUI->SetVisible(false);
				m_containerReSignInfoUI->SetVisible(false);
				m_diagArrearageUI->SetVisible(false);
				m_diagCancelApppontmentUI->SetVisible(false);
				m_containerMainpageUI->SetVisible(false);
			}
			else if (userstatus == 2) {
				m_containerLoginUI->SetVisible(false);
				m_containerAuditingUI->SetVisible(false);
				m_containerResetPwdUI->SetVisible(false);
				m_containerSignUpUI->SetVisible(false);
				m_containerSignInfoUI->SetVisible(false);
				m_diagArrearageUI->SetVisible(false);
				m_diagCancelApppontmentUI->SetVisible(false);
				m_containerTitleUI->SetBkImage(L"bkimage/top-bg.png");
				m_labTitleClinicImageUI->SetVisible(true);
				m_containerMainpageUI->SetVisible(true);

				m_optionDoctorInfo->Selected(true);
				m_optionDoctorInfo->SetBkImage(L"Option/expertinfo_selected.png");
				HttpGetExpertInfo();
				doctorinfohtml = 1;
				m_optionExpertTileInfo->Selected(true);
				m_tabDoctorInfoTypeControlUI->SelectItem(0);
				m_optionExpertTileInfo->SetBkImage(L"bkimage/option_bk1.png");
				m_optionDoctorWorkTime->SetBkImage(L"bkimage/option_bk2.png");
				m_listDoctorWorkInfoUI->RemoveAll();
				HttpGetExpertWorkTimeList();
				expertnum = HttpGetExpertList();
				/*
				m_optionCheckAppointment->Selected(true);
				m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_selected.png");
				
				if (expertnum > 0) {
					for (int i = 0; i < expertnum; ++i) {
						CListLabelElementUI* pLine = new CListLabelElementUI;
						temp = UTF8ToWString(expertlist[i]["name"].asCString());
						if (pLine != NULL)
						{
							m_cboChooseExpertOnSearchAppoint->Add(pLine);
							pLine->SetText(temp);
							pLine->SetTag(i);
						}
					}
				}
				HttpGetAppointmentList(APPOINTMENT_LIST_URL, 1);
				*/
			}
			else if (userstatus == 3) {
				m_containerLoginUI->SetVisible(false);
				m_containerAuditingUI->SetVisible(false);
				m_containerResetPwdUI->SetVisible(false);
				m_containerSignUpUI->SetVisible(false);
				m_containerSignInfoUI->SetVisible(false);
				m_containerReSignInfoUI->SetVisible(true);
				m_diagArrearageUI->SetVisible(false);
				m_diagCancelApppontmentUI->SetVisible(false);
				m_containerMainpageUI->SetVisible(false);
			}
			else if (userstatus == 9) {
				m_containerLoginUI->SetVisible(false);
				m_containerAuditingUI->SetVisible(false);
				m_containerResetPwdUI->SetVisible(false);
				m_containerSignUpUI->SetVisible(false);
				m_containerSignInfoUI->SetVisible(true);
				m_containerReSignInfoUI->SetVisible(false);
				m_diagArrearageUI->SetVisible(false);
				m_diagCancelApppontmentUI->SetVisible(false);
				m_containerMainpageUI->SetVisible(false);
			}
		}
		
	}
	else if (msg.pSender == m_btnWndMinUI)
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		return;
	}
	else if (msg.pSender == m_btnCloseEXEUI) {
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char aa[30];
		sprintf_s(aa, "%4d-%02d-%02d %02d:%02d:%02d   ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
		HDWriteLog(aa);

		CStringA logmsg = "退出程序。\r\n";
		HDWriteLog(logmsg);
		Close();
	}
	else if (msg.pSender == m_btnCloseUI) {
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char aa[30];
		sprintf_s(aa, "%4d-%02d-%02d %02d:%02d:%02d   ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
		HDWriteLog(aa);

		CStringA logmsg = "关闭程序。\r\n";
		HDWriteLog(logmsg);
		Close();
	}
	else if (msg.pSender->GetName()== _T("ButtonToResetPwd")) {
		CStringA logmsg = "进入重置密码页面。\r\n";
		HDWriteLog(logmsg);
		m_containerLoginUI->SetVisible(false);
		m_containerMainpageUI->SetVisible(false);
		m_containerAuditingUI->SetVisible(false);
		m_containerResetPwdUI->SetVisible(true);
		m_containerSignUpUI->SetVisible(false);
		m_containerSignInfoUI->SetVisible(false);
		m_containerReSignInfoUI->SetVisible(false);
		m_diagArrearageUI->SetVisible(false);
		m_diagCancelApppontmentUI->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("ButtonBackToLoginOnResetPwd")) {
		CStringA logmsg = "返回登录页面。\r\n";
		HDWriteLog(logmsg);
		m_containerLoginUI->SetVisible(true);
		m_containerMainpageUI->SetVisible(false);
		m_containerAuditingUI->SetVisible(false);
		m_containerResetPwdUI->SetVisible(false);
		m_containerSignUpUI->SetVisible(false);
		m_containerSignInfoUI->SetVisible(false);
		m_containerReSignInfoUI->SetVisible(false);
		m_diagArrearageUI->SetVisible(false);
		m_diagCancelApppontmentUI->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("ButtonBackToLoginOnSignUp")) {
		CStringA logmsg = "返回登录页面。\r\n";
		HDWriteLog(logmsg);
		m_containerLoginUI->SetVisible(true);
		m_containerMainpageUI->SetVisible(false);
		m_containerAuditingUI->SetVisible(false);
		m_containerResetPwdUI->SetVisible(false);
		m_containerSignUpUI->SetVisible(false);
		m_containerSignInfoUI->SetVisible(false);
		m_containerReSignInfoUI->SetVisible(false);
		m_diagArrearageUI->SetVisible(false);
		m_diagCancelApppontmentUI->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("ButtonSendResetVerificationOnResetPWD")) {
		CStringA logmsg = "重置密码页面获取验证码。\r\n";
		HDWriteLog(logmsg);
		CStringW mobile = m_PaintManager.FindControl(_T("EditPhoneOnResetPwd"))->GetText();;
		if (HttpSendCode(mobile) > 0) {

		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonResetPwd")) {
		CStringA logmsg = "重置密码。\r\n";
		HDWriteLog(logmsg);
		CStringW mobile = m_PaintManager.FindControl(_T("EditPhoneOnResetPwd"))->GetText();
		CStringW code = m_PaintManager.FindControl(_T("EditVerificationOnResetPwd"))->GetText();
		CStringW passwd = m_PaintManager.FindControl(_T("EditNewPwdOnResetPwd"))->GetText();
		CStringW repasswd = m_PaintManager.FindControl(_T("EditRepetPwdOnResetPwd"))->GetText();

		CStringA poststr = "mobile=";
		poststr += WStringToUTF8(mobile).c_str();
		poststr += "&code=";
		poststr += WStringToUTF8(code).c_str();
		poststr += "&password=";
		poststr += WStringToUTF8(passwd).c_str();
		poststr += "&password_confirm=";
		poststr += WStringToUTF8(repasswd).c_str();

		if (HttpResetPassword(poststr) > 0) {
			::MessageBox(*this, _T("修改成功！"), _T("提示"), 0);
			m_containerLoginUI->SetVisible(true);
			m_containerMainpageUI->SetVisible(false);
			m_containerAuditingUI->SetVisible(false);
			m_containerResetPwdUI->SetVisible(false);
			m_containerSignUpUI->SetVisible(false);
			m_containerSignInfoUI->SetVisible(false);
			m_diagArrearageUI->SetVisible(false);
			m_diagCancelApppontmentUI->SetVisible(false);
		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}

	}
	else if (msg.pSender->GetName() == _T("ButtonToSignUp")) {
		CStringA logmsg = "进入注册页面。\r\n";
		HDWriteLog(logmsg);
		m_containerLoginUI->SetVisible(false);
		m_containerMainpageUI->SetVisible(false);
		m_containerAuditingUI->SetVisible(false);
		m_containerResetPwdUI->SetVisible(false);
		m_containerSignUpUI->SetVisible(true);
		m_containerSignInfoUI->SetVisible(false);
		m_diagArrearageUI->SetVisible(false);
		m_diagCancelApppontmentUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonSendSignupVerificationOnSignUp")) {
		CStringA logmsg = "注册页面获取验证码。\r\n";
		HDWriteLog(logmsg);
		CStringW mobile = m_PaintManager.FindControl(_T("EditPhoneOnSignUpUser"))->GetText();;
		if(HttpSendCode(mobile) > 0){

		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}
	}
	else if (msg.pSender->GetName() == _T("ButtonSignUp")) {
		CStringA logmsg = "注册用户。\r\n";
		HDWriteLog(logmsg);
		CStringW mobile = m_PaintManager.FindControl(_T("EditPhoneOnSignUpUser"))->GetText();
		CStringW code = m_PaintManager.FindControl(_T("EditVerificationOnSignUpUser"))->GetText();
		CStringW username = m_PaintManager.FindControl(_T("EditNameOnSignUpUser"))->GetText();
		CStringW email = m_PaintManager.FindControl(_T("EditEmailOnSignUpUser"))->GetText();
		CStringW passwd = m_PaintManager.FindControl(_T("EditPwdOnSignUpUser"))->GetText();
		CStringW repasswd = m_PaintManager.FindControl(_T("EditRepetPwdOnSignUpUser"))->GetText();
		
		CStringA poststr = "mobile=";
		poststr += WStringToUTF8(mobile).c_str();
		poststr += "&code=";
		poststr += WStringToUTF8(code).c_str();
		if (username != L"") {
			poststr += "&username=";
			poststr += WStringToUTF8(username).c_str();
		}
		if (poststr != L"") {
			poststr += "&email=";
			poststr += WStringToUTF8(email).c_str();
		}

		poststr += "&password=";
		poststr += WStringToUTF8(passwd).c_str();
		poststr += "&password_confirm=";
		poststr += WStringToUTF8(repasswd).c_str();

		if (HttpSignUpUser(poststr) > 0) {
			if (HttpLogin(mobile, passwd) > 0) {
				m_containerLoginUI->SetVisible(false);
				m_containerMainpageUI->SetVisible(false);
				m_containerAuditingUI->SetVisible(false);
				m_containerResetPwdUI->SetVisible(false);
				m_containerSignUpUI->SetVisible(false);
				m_containerSignInfoUI->SetVisible(true);
				m_diagArrearageUI->SetVisible(false);
				m_diagCancelApppontmentUI->SetVisible(false);
			}			
		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonSignInfo")) {
		CStringA logmsg = "注册诊所。\r\n";
		HDWriteLog(logmsg);
		CStringW name = m_PaintManager.FindControl(_T("EditClinicNameOnSignUpClinic"))->GetText();
		CStringW addr = m_PaintManager.FindControl(_T("EditClinicAddrOnSignUpClinic"))->GetText();
		CStringW tel = m_PaintManager.FindControl(_T("EditClinicPhoneOnSignUpClinic"))->GetText();
		CStringW chief = m_PaintManager.FindControl(_T("EditCorporationNameOnSignUpClinic"))->GetText();
		CStringW idcard = m_PaintManager.FindControl(_T("EditCorporationIDOnSignUpClinic"))->GetText();

		CStringA poststr = "name=";
		poststr += WStringToUTF8(name).c_str();;
		poststr += "&address=";
		poststr += WStringToUTF8(addr).c_str();;
		poststr += "&tel=";
		poststr += WStringToUTF8(tel).c_str(); tel;
		poststr += "&chief=";
		poststr += WStringToUTF8(chief).c_str();
		poststr += "&idcard=";
		poststr += WStringToUTF8(idcard).c_str();
		poststr += "&Business_license_img=";

		CStringW path1 = m_btnUploadLicenseUI->GetUserData();
		Json::Value result1 = HttpUploadClinicIMG(path1);
		if (result1["code"] == 0) {
			poststr += result1["data"]["path"].asCString();
			OutputDebugStringA("uploaf file 1 success.\n");
		}
		poststr += "&local_img=";
		CStringW path2 = m_btnUploadPhotoUI->GetUserData();
		Json::Value result2 = HttpUploadClinicIMG(path2);
		if (result2["code"] == 0) {
			poststr += result2["data"]["path"].asCString();
			OutputDebugStringA("uploaf file 2 success.\n");
		}
		poststr += "&doctor_certificate_img=";
		CStringW path3 = m_btnUploadDoctorLicenseUI->GetUserData();
		Json::Value result3 = HttpUploadClinicIMG(path3);
		if (result3["code"] == 0) {
			poststr += result3["data"]["path"].asCString();
			OutputDebugStringA("uploaf file 3 success.\n");
		}

		if ((result1["code"] == 0) && (result2["code"] == 0) && (result3["code"] == 0)) {
			OutputDebugStringW(L"----signup.\r\n");
			OutputDebugStringA(poststr);
			if (HttpSignUpClinic(poststr) > 0) {
				m_containerLoginUI->SetVisible(false);
				m_containerMainpageUI->SetVisible(false);
				m_containerAuditingUI->SetVisible(true);
				m_containerTitleUI->SetBkImage(L"bkimage/top-bg.png");
				m_containerResetPwdUI->SetVisible(false);
				m_containerSignUpUI->SetVisible(false);
				m_containerSignInfoUI->SetVisible(false);
				m_containerReSignInfoUI->SetVisible(false);
				m_diagArrearageUI->SetVisible(false);
				m_diagCancelApppontmentUI->SetVisible(false);
			}
			else {
				::MessageBox(*this, rtmessage, _T("提示"), 0);
			}
		}
		else {
			::MessageBox(*this, _T("上传图像失败！"), _T("提示"), 0);
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonReSignInfo")) {
		CStringA logmsg = "重新录入诊所信息。\r\n";
		HDWriteLog(logmsg);
		CStringW name = m_PaintManager.FindControl(_T("EditClinicNameOnResignClinic"))->GetText();
		CStringW addr = m_PaintManager.FindControl(_T("EditClinicAddrOnResignClinic"))->GetText();
		CStringW tel = m_PaintManager.FindControl(_T("EditClinicPhoneOnResignClinic"))->GetText();
		CStringW chief = m_PaintManager.FindControl(_T("EditCorporationNameOnResignClinic"))->GetText();
		CStringW idcard = m_PaintManager.FindControl(_T("EditCorporationIDOnResignClinic"))->GetText();

		CStringA poststr = "name=";
		poststr += WStringToUTF8(name).c_str();;
		poststr += "&address=";
		poststr += WStringToUTF8(addr).c_str();;
		poststr += "&tel=";
		poststr += WStringToUTF8(tel).c_str(); tel;
		poststr += "&chief=";
		poststr += WStringToUTF8(chief).c_str();;
		poststr += "&idcard=";
		poststr += WStringToUTF8(idcard).c_str();;
		poststr += "&Business_license_img=";
		int imgready = 0;
		CStringW temp = m_btnReUploadLicenseUI->GetUserData();
		if (temp.Left(4) == L"http") {
			poststr += temp.Mid(30);
			imgready++;
		}
		else {
			Json::Value result1 = HttpUploadClinicIMG(temp);
			if (result1["code"] == 0) {
				poststr += result1["data"]["path"].asCString();
				imgready++;
				OutputDebugStringA("uploaf file 1 success.\n");
			}
		}
		poststr += "&local_img=";
		temp = m_btnReUploadPhotoUI->GetUserData();
		if (temp.Left(4) == L"http") {
			poststr += temp.Mid(30);
			imgready++;
		}
		else {
			Json::Value result2 = HttpUploadClinicIMG(temp);
			if (result2["code"] == 0) {
				poststr += result2["data"]["path"].asCString();
				OutputDebugStringA("uploaf file 1 success.\n");
				imgready++;
			}
		}
		poststr += "&doctor_certificate_img=";
		temp = m_btnReUploadDoctorLicenseUI->GetUserData();
		if (temp.Left(4) == L"http") {
			poststr += temp.Mid(30);
			imgready++;
		}
		else {
			Json::Value result3 = HttpUploadClinicIMG(temp);
			if (result3["code"] == 0) {
				poststr += result3["data"]["path"].asCString();
				OutputDebugStringA("uploaf file 1 success.\n");
				imgready++;
			}
		}
		if (imgready++ == 3) {
			if (HttpUpdateClinicInfo(poststr) > 0) {
				m_containerLoginUI->SetVisible(false);
				m_containerMainpageUI->SetVisible(false);
				m_containerAuditingUI->SetVisible(true);
				m_containerTitleUI->SetBkImage(L"bkimage/top-bg.png");
				m_containerResetPwdUI->SetVisible(false);
				m_containerSignUpUI->SetVisible(false);
				m_containerSignInfoUI->SetVisible(false);
				m_containerReSignInfoUI->SetVisible(false);
				m_diagArrearageUI->SetVisible(false);
				m_diagCancelApppontmentUI->SetVisible(false);
			}
			else {
				::MessageBox(*this, rtmessage, _T("提示"), 0);
			}
		}
		else {
			::MessageBox(*this, _T("上传图像失败！"), _T("提示"), 0);
		}
	}

	//Appointment part
	else if (msg.pSender->GetName() == _T("ButtonSearchAppointment")) {
		CStringA logmsg = "搜索预约。\r\n";
		HDWriteLog(logmsg);
		CStringW choosedate = m_dataChooseOnSearchAppoint->GetText();
		int expertindex = m_cboChooseExpertOnSearchAppoint->GetCurSel();
		CStringW curl = APPOINTMENT_LIST_URL;
		appointsearchtext = L"";

		if (expertindex >= 0){
			CStringW expertuuid = UTF8ToWString(expertlist[expertindex]["uuid"].asCString());
			curl += L"&expert_uuid=";
			curl += expertuuid;
			appointsearchtext += L"&expert_uuid=";
			appointsearchtext += expertuuid;
		}
		if (choosedate != _T("")) {
			curl += L"&date=";
			curl += choosedate;
			appointsearchtext += L"&date=";
			appointsearchtext += choosedate;
		}
		OutputDebugStringW(curl);
		OutputDebugStringW(L"\r\n");
		m_listAppointmentUI->RemoveAll();
		HttpGetAppointmentList(curl, 1);
	}

	else if (msg.pSender->GetName() == _T("ButtonMakeAppointment")) {
		CStringA logmsg = "预约诊断。\r\n";
		HDWriteLog(logmsg);
		if (expertnum > 0) {
			m_cboChooseExpertOnToAppoint->RemoveAll();
			for (int i = 0; i < expertnum; ++i) {
				CListLabelElementUI* pLine = new CListLabelElementUI;
				CStringW temp = UTF8ToWString(expertlist[i]["name"].asCString());
				if (pLine != NULL)
				{
					m_cboChooseExpertOnToAppoint->Add(pLine);
					pLine->SetText(temp);
					pLine->SetTag(i);
				}
			}
		}
		m_tabPatientInfoControlUI->SelectItem(0);
		m_optionPatientConditionPageOnToAM->Selected(true);		
		m_optionPatientConditionPageOnToAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnToAM->SetBkColor(0x00000000);
		m_tileFreeTimeOfMorningAppointUI->RemoveAll();
		m_tileFreeTimeOfAfternoonAppointUI->RemoveAll();
		m_tileFreeTimeOfNightAppointUI->RemoveAll();
		m_tileFreeTimeOfMorningAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfAfternoonAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfNightAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfMorningAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfAfternoonAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfNightAppointUI->SetMinHeight(1);
		m_layFreeTimeOfMorningAppointUI->SetMaxHeight(56);
		m_layFreeTimeOfMorningAppointUI->SetMinHeight(56);
		m_layeFreeTimeOfAfternoonAppointUI->SetMaxHeight(56);
		m_layeFreeTimeOfAfternoonAppointUI->SetMinHeight(56);
		m_layFreeTimeOfNightAppointUI->SetMaxHeight(56);
		m_layFreeTimeOfNightAppointUI->SetMinHeight(56);
		m_PaintManager.FindControl(_T("EditPatientNameOnToAM"))->SetText(NULL);
		m_PaintManager.FindControl(_T("EditPatientDescOnToAM"))->SetText(L"");
		m_btnUploadPatientPic1UI->SetBkImage(L"");
		m_btnUploadPatientPic2UI->SetBkImage(L"");
		m_btnUploadPatientPic3UI->SetBkImage(L"");

		int unpaynum = HttpGetUserPayStatus();
		if (unpaynum > 0) {
			//处于未结清状态弹出提示
			m_diagArrearageUI->SetVisible(true);
		}
		else if (unpaynum == 0) {
				m_containerAppointmentListUI->SetVisible(false);
				m_containerToAppointUI->SetVisible(true);
				m_containerAppointmentDetailUI->SetVisible(false);
				m_containerEditAppointmentUI->SetVisible(false);
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonReturnToAppointListOnToAppoint")) {
		CStringA logmsg = "返回预约列表。\r\n";
		HDWriteLog(logmsg);
		m_btnShowPatientPic1UIOnAMDetail->SetVisible(true);
		m_btnShowPatientPic2UIOnAMDetail->SetVisible(true);
		m_btnShowPatientPic3UIOnAMDetail->SetVisible(true);
		m_containerAppointmentListUI->SetVisible(true);
		m_containerToAppointUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(false);
		m_containerEditAppointmentUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonSearchFreeTimeOnToAM")) {
		CStringA logmsg = "预约页面查看空闲时间。\r\n";
		HDWriteLog(logmsg);
		CStringW choosedate = m_dataChooseOnToAppoint->GetText();
		int expertindex = m_cboChooseExpertOnToAppoint->GetCurSel();
		CStringW expertuuid;
		if (expertindex >= 0) {
			expertuuid = UTF8ToWString(expertlist[expertindex]["uuid"].asCString());
			m_tileFreeTimeOfMorningAppointUI->RemoveAll();
			m_tileFreeTimeOfAfternoonAppointUI->RemoveAll();
			m_tileFreeTimeOfNightAppointUI->RemoveAll();
			m_tileFreeTimeOfMorningAppointUI->SetMaxHeight(1);
			m_tileFreeTimeOfMorningAppointUI->SetMinHeight(1);
			m_tileFreeTimeOfAfternoonAppointUI->SetMaxHeight(1);
			m_tileFreeTimeOfAfternoonAppointUI->SetMinHeight(1);
			m_tileFreeTimeOfNightAppointUI->SetMaxHeight(1);
			m_tileFreeTimeOfNightAppointUI->SetMinHeight(1);
			m_layFreeTimeOfMorningAppointUI->SetMaxHeight(56);
			m_layFreeTimeOfMorningAppointUI->SetMinHeight(56);
			m_tileFreeTimeOfNightAppointUI->SetMaxHeight(56);
			m_tileFreeTimeOfNightAppointUI->SetMinHeight(56);
			m_tileFreeTimeOfAfternoonAppointUI->SetMaxHeight(56);
			m_tileFreeTimeOfAfternoonAppointUI->SetMinHeight(56);
			HttpGetFreeTime(expertuuid, choosedate);
		}
		else {
			::MessageBox(*this, _T("请选择专家！"), _T("提示"), 0);
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonSubmitAppointment")) {
		CStringA logmsg = "确认预约。\r\n";
		HDWriteLog(logmsg);
		int expertindex = m_cboChooseExpertOnToAppoint->GetCurSel();
		CStringA expertuuid;
		if (expertindex >= 0) {
			expertuuid = UTF8ToWString(expertlist[expertindex]["uuid"].asCString());
		}
		else {
			::MessageBox(*this, _T("请选择专家！"), _T("提示"), 0);
			return;
		}
		CStringA choosedate = m_dataChooseOnToAppoint->GetText();
		
		//bool isstart = false;
		int choosedtimenum = 0;
		CStringW starttime;
		CStringW endtime;

		int num1 = m_tileFreeTimeOfMorningAppointUI->GetCount();
		for (int i = 0; i < num1; ++i) {
			CVerticalLayoutUI* timeitem = (CVerticalLayoutUI*)m_tileFreeTimeOfMorningAppointUI->GetItemAt(i);
			CCheckBoxUI* timechoose = (CCheckBoxUI*)timeitem->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"));
			if (timechoose->GetCheck() == true) {
				choosedtimenum++;
				starttime = timeitem->GetUserData().Left(5);
				endtime = timeitem->GetUserData().Right(5);
			}
		}

		int num2 = m_tileFreeTimeOfAfternoonAppointUI->GetCount();
		for (int i = 0; i < num2; ++i) {
			CVerticalLayoutUI* timeitem = (CVerticalLayoutUI*)m_tileFreeTimeOfAfternoonAppointUI->GetItemAt(i);
			CCheckBoxUI* timechoose = (CCheckBoxUI*)timeitem->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"));
			if (timechoose->GetCheck() == true) {
				choosedtimenum++;
				starttime = timeitem->GetUserData().Left(5);
				endtime = timeitem->GetUserData().Right(5);
			}
		}

		int num3 = m_tileFreeTimeOfNightAppointUI->GetCount();
		for (int i = 0; i < num3; ++i) {
			CVerticalLayoutUI* timeitem = (CVerticalLayoutUI*)m_tileFreeTimeOfNightAppointUI->GetItemAt(i);
			CCheckBoxUI* timechoose = (CCheckBoxUI*)timeitem->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"));
			if (timechoose->GetCheck() == true) {
				choosedtimenum++;
				starttime = timeitem->GetUserData().Left(5);
				endtime = timeitem->GetUserData().Right(5);
			}
		}

		if (choosedtimenum == 0) {
			::MessageBox(*this, _T("请选择时间!"), _T("提示"), 0);
			return;
		}
		else if(choosedtimenum > 1){
			::MessageBox(*this, _T("只能选择一个时段!"), _T("提示"), 0);
			return;
		}
		else {

		}
		CStringA thestarttime = choosedate;
		thestarttime += " ";
		thestarttime += starttime;
		thestarttime += ":00";
		CStringA theendtime = choosedate;
		theendtime += " ";
		theendtime += endtime;
		theendtime += ":00";
		CStringW patientname = m_PaintManager.FindControl(_T("EditPatientNameOnToAM"))->GetText();
		if (patientname == "") {
			::MessageBox(*this, _T("请填写患者信息！"), _T("提示"), 0);
			return;
		}
		CStringW patientdesc = m_PaintManager.FindControl(_T("EditPatientDescOnToAM"))->GetText();
		if (patientdesc == "") {
			::MessageBox(*this, _T("请填写主述！"), _T("提示"), 0);
			return;
		}

		CStringA poststr = "expert_uuid=";
		poststr += expertuuid;
		poststr += "&order_starttime=";
		poststr += thestarttime;
		poststr += "&order_endtime=";
		poststr += theendtime;
		poststr += "&patient_name=";
		poststr += WStringToUTF8(patientname).c_str();
		poststr += "&patient_description=";
		poststr += WStringToUTF8(patientdesc).c_str(); 
		poststr += "&pay_type=2";

		CStringW path1 = m_btnUploadPatientPic1UI->GetUserData();
		if (path1 != L"") {
			Json::Value result1 = HttpUploadPatientIMG(path1);
			if (result1["code"] == 0) {
				poststr += L"&patient_img1=";
				poststr += result1["data"]["path"].asCString();
				OutputDebugStringA("uploaf file 1 success.\n");
			}
		}

		CStringW path2 = m_btnUploadPatientPic2UI->GetUserData();
		if (path2 != L"") {
			Json::Value result2 = HttpUploadPatientIMG(path2);
			if (result2["code"] == 0) {
				poststr += L"&patient_img2=";
				poststr += result2["data"]["path"].asCString();
				OutputDebugStringA("uploaf file 2 success.\n");
			}
		}

		CStringW path3 = m_btnUploadPatientPic3UI->GetUserData();
		if (path3 != L"") {
			Json::Value result3 = HttpUploadPatientIMG(path3);
			if (result3["code"] == 0) {
				poststr += L"&patient_img3=";
				poststr += result3["data"]["path"].asCString();
				OutputDebugStringA("uploaf file 3 success.\n");
			}
		}

		if (HttpCreateAppointment(poststr) > 0) {
			m_listAppointmentUI->RemoveAll();
			HttpGetAppointmentList(APPOINTMENT_LIST_URL,1);
			m_containerAppointmentListUI->SetVisible(true);
			m_containerToAppointUI->SetVisible(false);
			m_containerAppointmentDetailUI->SetVisible(false);
			m_containerEditAppointmentUI->SetVisible(false);
		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}
	}
	else if (msg.pSender->GetName() == _T("ButtonCancelSubmitAppoint")) {
		CStringA logmsg = "取消确认预约。\r\n";
		HDWriteLog(logmsg);
		m_containerAppointmentListUI->SetVisible(true);
		m_containerToAppointUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(false);
		m_containerEditAppointmentUI->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("ButtonReturnToAppointmentList")) {
		CStringA logmsg = "返回预约列表。\r\n";
		HDWriteLog(logmsg);
		m_containerAppointmentListUI->SetVisible(true);
		m_containerToAppointUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(false);
		m_containerEditAppointmentUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonSearchFreeTimeOnEditAM")) {
		CStringA logmsg = "修改预约页面查找空闲时间。\r\n";
		HDWriteLog(logmsg);
		CStringW choosedate = m_dataChooseOnEditAppoint->GetText();
		int expertindex = m_cboChooseExpertOnEditAppoint->GetCurSel();
		CStringW expertuuid;
		if (expertindex >= 0) {
			expertuuid = UTF8ToWString(expertlist[expertindex]["uuid"].asCString());
			m_tileFreeTimeOfEditMorningAppointUI->RemoveAll();
			m_tileFreeTimeOfEditAfternoonAppointUI->RemoveAll();
			m_tileFreeTimeOfEditNightAppointUI->RemoveAll();
			m_tileFreeTimeOfEditMorningAppointUI->SetMaxHeight(1);
			m_tileFreeTimeOfEditMorningAppointUI->SetMinHeight(1);
			m_tileFreeTimeOfEditAfternoonAppointUI->SetMaxHeight(1);
			m_tileFreeTimeOfEditAfternoonAppointUI->SetMinHeight(1);
			m_tileFreeTimeOfEditNightAppointUI->SetMaxHeight(1);
			m_tileFreeTimeOfEditNightAppointUI->SetMinHeight(1);
			m_layFreeTimeOfMorningAppointOnEditAMUI->SetMinHeight(75);
			m_layFreeTimeOfMorningAppointOnEditAMUI->SetMaxHeight(75);
			m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMinHeight(75);
			m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMaxHeight(75);
			m_layFreeTimeOfNightAppointOnEditAMUI->SetMinHeight(56);
			m_layFreeTimeOfNightAppointOnEditAMUI->SetMaxHeight(56);
			HttpGetFreeTimeOnEditAM(expertuuid, choosedate);

		}
		else {
			::MessageBox(*this, _T("请选择专家！"), _T("提示"), 0);
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonEditBasicInfoOnEditAM")) {
		CStringA logmsg = "修改预约页面编辑基础信息。\r\n";
		HDWriteLog(logmsg);
		m_PaintManager.FindControl(_T("container_showexpertinfooneditam"))->SetVisible(false);
		m_PaintManager.FindControl(_T("container_editexpertinfooneditam"))->SetVisible(true);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetVisible(true);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetVisible(true);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetVisible(true);
		m_PaintManager.FindControl(_T("ButtonReturnToBasicInfoOnEditAM"))->SetVisible(true);
		m_PaintManager.FindControl(_T("ButtonEditBasicInfoOnEditAM"))->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonReturnToBasicInfoOnEditAM")) {
		CStringA logmsg = "修改预约页面取消编辑基础信息。\r\n";
		HDWriteLog(logmsg);
		m_PaintManager.FindControl(_T("container_showexpertinfooneditam"))->SetVisible(true);
		m_PaintManager.FindControl(_T("container_editexpertinfooneditam"))->SetVisible(false);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetVisible(false);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetVisible(false);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetVisible(false);
		m_tileFreeTimeOfEditMorningAppointUI->RemoveAll();
		m_tileFreeTimeOfEditAfternoonAppointUI->RemoveAll();
		m_tileFreeTimeOfEditNightAppointUI->RemoveAll();
		m_tileFreeTimeOfEditMorningAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditMorningAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfEditAfternoonAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditAfternoonAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfEditNightAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditNightAppointUI->SetMinHeight(1);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetMinHeight(75);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetMaxHeight(75);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMinHeight(75);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMaxHeight(75);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetMinHeight(56);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetMaxHeight(56);
		m_PaintManager.FindControl(_T("ButtonEditBasicInfoOnEditAM"))->SetVisible(true);
		m_PaintManager.FindControl(_T("ButtonReturnToBasicInfoOnEditAM"))->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonSubmitEditAppointment")) {
		CStringA logmsg = "修改预约页面确认修改预约。\r\n";
		HDWriteLog(logmsg);
		CStringA expertuuid = "";
		CStringA thestarttime = "";
		CStringA theendtime = "";
		if (m_PaintManager.FindControl(_T("container_showexpertinfooneditam"))->IsVisible()) {
			expertuuid = m_PaintManager.FindControl(_T("LabalExpertOnEditAM"))->GetUserData();
			thestarttime = m_PaintManager.FindControl(_T("LabalStartTimeOnEditAM"))->GetText();
			theendtime = m_PaintManager.FindControl(_T("LabalEndTimeOnEditAM"))->GetText();
		}
		else {
			int expertindex = m_cboChooseExpertOnEditAppoint->GetCurSel();
			if (expertindex >= 0) {
				expertuuid = UTF8ToWString(expertlist[expertindex]["uuid"].asCString());
			}
			else {
				::MessageBox(*this, _T("请选择专家！"), _T("提示"), 0);
				return;
			}
			CStringA choosedate = m_dataChooseOnEditAppoint->GetText();
			int choosedtimenum = 0;
			CStringW starttime;
			CStringW endtime;

			int num1 = m_tileFreeTimeOfEditMorningAppointUI->GetCount();
			for (int i = 0; i < num1; ++i) {
				CVerticalLayoutUI* timeitem = (CVerticalLayoutUI*)m_tileFreeTimeOfEditMorningAppointUI->GetItemAt(i);
				CCheckBoxUI* timechoose = (CCheckBoxUI*)timeitem->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"));
				if (timechoose->GetCheck() == true) {
					choosedtimenum++;
					starttime = timeitem->GetUserData().Left(5);
					endtime = timeitem->GetUserData().Right(5);
				}
			}

			int num2 = m_tileFreeTimeOfEditAfternoonAppointUI->GetCount();
			for (int i = 0; i < num2; ++i) {
				CVerticalLayoutUI* timeitem = (CVerticalLayoutUI*)m_tileFreeTimeOfEditAfternoonAppointUI->GetItemAt(i);
				CCheckBoxUI* timechoose = (CCheckBoxUI*)timeitem->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"));
				if (timechoose->GetCheck() == true) {
					choosedtimenum++;
					starttime = timeitem->GetUserData().Left(5);
					endtime = timeitem->GetUserData().Right(5);
				}
			}

			int num3 = m_tileFreeTimeOfEditNightAppointUI->GetCount();
			for (int i = 0; i < num3; ++i) {
				CVerticalLayoutUI* timeitem = (CVerticalLayoutUI*)m_tileFreeTimeOfEditNightAppointUI->GetItemAt(i);
				CCheckBoxUI* timechoose = (CCheckBoxUI*)timeitem->FindSubControl(_T("CheckChooseFreeTimeOnTimeItem"));
				if (timechoose->GetCheck() == true) {
					choosedtimenum++;
					starttime = timeitem->GetUserData().Left(5);
					endtime = timeitem->GetUserData().Right(5);
				}
			}

			if (choosedtimenum == 0) {
				::MessageBox(*this, _T("请选择时间！"), _T("提示"), 0);
				return;
			}
			else if (choosedtimenum > 1) {
				::MessageBox(*this, _T("只能选择一个时段！"), _T("提示"), 0);
				return;

			}

			thestarttime = choosedate;
			thestarttime += " ";
			thestarttime += starttime;
			thestarttime += ":00";
			theendtime = choosedate;
			theendtime += " ";
			theendtime += endtime;
			theendtime += ":00";
		}


		CStringW patientname = m_PaintManager.FindControl(_T("EditPatientNameOnEditAM"))->GetText();
		if (patientname == "") {
			::MessageBox(*this, _T("请填写患者信息！"), _T("提示"), 0);
			return;
		}
		CStringW patientdesc = m_PaintManager.FindControl(_T("EditPatientDescOnEditAM"))->GetText();
		if (patientdesc == "") {
			::MessageBox(*this, _T("请填写主述！"), _T("提示"), 0);
			return;
		}

		CStringA poststr = "expert_uuid=";
		poststr += expertuuid;
		poststr += "&appointment_no=";
		poststr += editappointmentno;
		poststr += "&order_starttime=";
		poststr += thestarttime;
		poststr += "&order_endtime=";
		poststr += theendtime;
		poststr += "&patient_name=";
		poststr += WStringToUTF8(patientname).c_str();

		CStringW temp1 = m_btnUploadNewPatientPic1UI->GetUserData();
		if (temp1 != L"") {
			poststr += "&patient_img1=";
			if (temp1.Left(4) == L"http") {			
				poststr += temp1.Mid(30);
			}
			else {
				OutputDebugStringW(temp1);
				Json::Value result1 = HttpUploadPatientIMG(temp1);
				if (result1["code"] == 0) {
					OutputDebugStringA("uploaf file 1 success.\n");
					OutputDebugStringW(UTF8ToWString(result1["data"]["path"].asCString()));
					poststr += result1["data"]["path"].asCString();
				}
			}
		}


		CStringW temp2 = m_btnUploadNewPatientPic2UI->GetUserData();
		if (temp2 != L"") {
			poststr += "&patient_img2=";
			if (temp2.Left(4) == L"http") {				
				poststr += temp2.Mid(30);
			}
			else {
				OutputDebugStringW(temp2);
				Json::Value result2 = HttpUploadPatientIMG(temp2);
				if (result2["code"] == 0) {
					poststr += result2["data"]["path"].asCString();
					OutputDebugStringW(UTF8ToWString(result2["data"]["path"].asCString()));
					OutputDebugStringA("uploaf file 2 success.\n");
				}
			}
		}

		CStringW temp3 = m_btnUploadNewPatientPic3UI->GetUserData();
		if (temp3 != L"") {
			poststr += "&patient_img3=";
			if (temp3.Left(4) == L"http") {				
				poststr += temp3.Mid(30);
			}
			else {
				OutputDebugStringW(temp3);
				Json::Value result3 = HttpUploadPatientIMG(temp3);
				if (result3["code"] == 0) {
					OutputDebugStringW(UTF8ToWString(result3["data"]["path"].asCString()));
					poststr += result3["data"]["path"].asCString();
				}
			}
		}
		poststr += "&patient_description=";
		poststr += WStringToUTF8(patientdesc).c_str();
		poststr += "&pay_type=2";

		if (HttpUpdateAppointment(poststr) > 0) {
			m_listAppointmentUI->RemoveAll();
			HttpGetAppointmentList(APPOINTMENT_LIST_URL, 1);
			m_containerAppointmentListUI->SetVisible(true);
			m_containerToAppointUI->SetVisible(false);
			m_containerAppointmentDetailUI->SetVisible(false);
			m_containerEditAppointmentUI->SetVisible(false);
		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}
	}
	else if (msg.pSender->GetName() == _T("ButtonCancelSubmitEditAppoint")) {
		CStringA logmsg = "修改预约页面取消修改预约。\r\n";
		HDWriteLog(logmsg);
		m_containerAppointmentListUI->SetVisible(true);
		m_containerToAppointUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(false);
		m_containerEditAppointmentUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonReturnToAppointListOnEditAM")) {
		CStringA logmsg = "返回预约列表。\r\n";
		HDWriteLog(logmsg);
		m_containerAppointmentListUI->SetVisible(true);
		m_containerToAppointUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(false);
		m_containerEditAppointmentUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonReturnToAppointmentListOnAMDetail")) {
		CStringA logmsg = "返回预约列表。\r\n";
		HDWriteLog(logmsg);
		m_containerAppointmentListUI->SetVisible(true);
		m_containerToAppointUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(false);
		m_containerEditAppointmentUI->SetVisible(false);
	}

	//record part
	else if (msg.pSender->GetName() == _T("ButtonSearchRecord")) {
		CStringA logmsg = "查找诊断记录。\r\n";
		HDWriteLog(logmsg);
		CStringW choosedate = m_dataChooseOnSearchRecord->GetText();
		CStringW patientname = m_PaintManager.FindControl(_T("EditPatientNameOnSearchRecord"))->GetText();
		int expertindex = m_cboChooseExpertOnSearchRecord->GetCurSel();
		CStringW curl = RECORD_LIST_URL;
		recordsearchtext = L"";

		if (expertindex >= 0) {
			CStringW expertuuid = UTF8ToWString(expertlist[expertindex]["uuid"].asCString());
			curl += L"&expert_uuid=";
			curl += expertuuid;
			recordsearchtext += L"&expert_uuid=";
			recordsearchtext += expertuuid;
		}
		if (choosedate != _T("")) {
			curl += L"&date=";
			curl += choosedate;
			recordsearchtext += L"&date=";
			recordsearchtext += choosedate;
		}
		if (patientname != _T("")) {
			curl += L"&patient_name=";
			curl += patientname;
			recordsearchtext += L"&date=";
			recordsearchtext += choosedate;
		}
		OutputDebugStringW(curl);
		OutputDebugStringW(L"\r\n");
		m_listRecordUI->RemoveAll();
		HttpGetRecordList(curl, 1);
	}

	else if (msg.pSender->GetName() == _T("ButtonReturnToRecordList")) {
		CStringA logmsg = "返回诊断列表。\r\n";
		HDWriteLog(logmsg);
		if (inplaying == 1) {
			m_pAVPlayer->Stop();
			inplaying = 0;

			m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play.png");
			m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended.png");
			m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
			m_btnPlayAudio->SetVisible(true);
			m_btnPlayAudio->SetEnabled(true);
			m_btnPauseAudio->SetVisible(false);
			m_btnPauseAudio->SetEnabled(true);
			m_btnStopAudio->SetVisible(false);
		}		
		m_btnShowPatientPic1UIOnRDDetail->SetVisible(true);
		m_btnShowPatientPic2UIOnRDDetail->SetVisible(true);
		m_btnShowPatientPic3UIOnRDDetail->SetVisible(true);
		m_containerRecordListUI->SetVisible(true);
		m_containerRecordDetailUI->SetVisible(false);
		m_containerEditRecordUI->SetVisible(false);
	}

	else if (msg.pSender  == m_btnPlayAudio) {
		CStringA logmsg = "播放音频。\r\n";
		HDWriteLog(logmsg);
		if (inplaying == 0) {
			CStringW temp = m_btnPlayAudio->GetUserData();
			if (temp == L"") {
				::MessageBox(*this, _T("音频尚未生成。"), _T("提示"), 0);
			}
			else {
				temp += L"&token=";
				temp += token;
				CStringA path1 = temp;
				if (m_pAVPlayer->Play((LPCSTR)path1)) {
					m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play-no.png");
					m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended.png");
					m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
					m_btnPlayAudio->SetEnabled(false);
					m_btnPauseAudio->SetVisible(true);
					m_btnStopAudio->SetVisible(true);
					inplaying = 1;
				}
				else {
					::MessageBox(*this, _T("播放失败。"), _T("提示"), 0);
				}
			}
		}
		else if (inplaying == 1) {
			m_pAVPlayer->Play();
			m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play-no.png");
			m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended.png");
			m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
			m_btnPlayAudio->SetEnabled(false);
			m_btnPauseAudio->SetEnabled(true);
		}		
	}

	else if (msg.pSender == m_btnPauseAudio) {
		CStringA logmsg = "暂停播放音频。\r\n";
		HDWriteLog(logmsg);
		if (inplaying == 1) {
			m_pAVPlayer->Pause();
			m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play.png");
			m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended-no.png");
			m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
			m_btnPlayAudio->SetEnabled(true);
			m_btnPauseAudio->SetEnabled(false);
		}		
	}

	else if (msg.pSender == m_btnStopAudio) {
		CStringA logmsg = "停止播放音频。\r\n";
		HDWriteLog(logmsg);
		if (inplaying == 1) {
			m_pAVPlayer->Stop();
			m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play.png");
			m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended.png");
			m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
			m_btnPlayAudio->SetVisible(true);
			m_btnPlayAudio->SetEnabled(true);
			m_btnPauseAudio->SetVisible(false);
			m_btnPauseAudio->SetEnabled(true);
			m_btnStopAudio->SetVisible(false);
			inplaying = 0;
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonSubmitEditRecord")) {
		CStringA logmsg = "确认修改诊断记录。\r\n";
		HDWriteLog(logmsg);
		CStringW giagnosis = m_PaintManager.FindControl(_T("EditDiagnosisDescOnRecordEdit"))->GetText();
		CStringW paage = m_PaintManager.FindControl(_T("EditPatientAgeOnRecordEdit"))->GetText();
		CStringW pamobile = m_PaintManager.FindControl(_T("EditPatientPhoneOnRecordEdit"))->GetText();
		CStringW paidcard = m_PaintManager.FindControl(_T("EditPatientIDOnRecordEdit"))->GetText();
		
		CStringA poststr = "source=clinic";
		poststr += "&appointment_no=";
		//CStringA poststr = "appointment_no=";
		poststr += editrecordappointmentno;
		poststr += "&patient_gender=";
		if (m_optionChooseMaleOnEditRecord->IsSelected()) {
			poststr += "1";
		}
		else {
			poststr += "2";
		}
		if (paage != _T("")) {
			poststr += "&patient_age=";
			poststr += WStringToUTF8(paage).c_str();
		}
		if (pamobile != _T("")) {
			poststr += "&patient_mobile=";
			poststr += WStringToUTF8(pamobile).c_str();
		}
		if (paidcard != _T("")) {
			poststr += "&patient_idcard=";
			poststr += WStringToUTF8(paidcard).c_str();
		}
		if (giagnosis != _T("")) {
			poststr += "&expert_diagnosis=";
			poststr += WStringToUTF8(giagnosis).c_str();
			if (HttpUpdateRecord(poststr) > 0) {
				m_containerRecordListUI->SetVisible(true);
				m_containerRecordDetailUI->SetVisible(false);
				m_containerEditRecordUI->SetVisible(false);
			}
			else {
				::MessageBox(*this, rtmessage, _T("提示"), 0);
			}
		}
		else {
			::MessageBox(*this, _T("缺少诊断信息！"), _T("提示"), 0);
		}

	}
	else if (msg.pSender->GetName() == _T("ButtonCancelSubmitEditRecord")) {
		CStringA logmsg = "取消修改诊断记录。\r\n";
		HDWriteLog(logmsg);
		m_containerRecordListUI->SetVisible(true);
		m_containerRecordDetailUI->SetVisible(false);
		m_containerEditRecordUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonReturnToRecordListOnEditRD")) {
		CStringA logmsg = "返回诊断列表。\r\n";
		HDWriteLog(logmsg);
		m_containerRecordListUI->SetVisible(true);
		m_containerRecordDetailUI->SetVisible(false);
		m_containerEditRecordUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonEditUserInfo")) {
		CStringA logmsg = "修改用户信息。\r\n";
		HDWriteLog(logmsg);
		m_containerShowUserInfoUI->SetVisible(false);
		m_containerEditUserInfoUI->SetVisible(true);
		HttpGetUserInfoUpdate();
	}
	else if (msg.pSender->GetName() == _T("ButtonReferUserInfo")) {
		CStringA logmsg = "确认修改用户信息。\r\n";
		HDWriteLog(logmsg);
		CStringW newusername = m_PaintManager.FindControl(_T("EditUserNameOnUserInfo"))->GetText();
		CStringW newemail = m_PaintManager.FindControl(_T("EditUserEmailOnUserInfo"))->GetText();
		CStringA poststr = "username=";
		poststr += WStringToUTF8(newusername).c_str();
		poststr += "&email=";
		poststr += WStringToUTF8(newemail).c_str();
		if (HttpUpdateUserInfo(poststr) > 0) {
			HttpGetClinicInfo();
			m_containerShowUserInfoUI->SetVisible(true);
			m_containerEditUserInfoUI->SetVisible(false);
		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0 );
		}
		
	}
	else if (msg.pSender->GetName() == _T("ButtonCancelReferUserInfo")) {
		CStringA logmsg = "取消修改用户信息。\r\n";
		HDWriteLog(logmsg);
		m_containerShowUserInfoUI->SetVisible(true);
		m_containerEditUserInfoUI->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("ButtonEditClinicInfo")) {
		CStringA logmsg = "修改诊所信息。\r\n";
		HDWriteLog(logmsg);
		if (HttpCheckClinicUpdatePermit() < 0) {
			POINT pt = { 320, 191 };
			m_pModifyClincInfoFrameWnd = new CDiagModifyClincInfoFrameWnd();
			m_pModifyClincInfoFrameWnd->Init(*this, pt);
			m_pModifyClincInfoFrameWnd->ShowModal();
		}
		else {
			HttpGetClinicInfoUpdate();
			m_containerShowClinicInfoUI->SetVisible(false);
			m_containerEditClinicInfoUI->SetVisible(true);
		}
	}
	else if (msg.pSender->GetName() == _T("ButtonReferClinicInfo")) {
		CStringA logmsg = "确认修改诊所信息。\r\n";
		HDWriteLog(logmsg);
		CStringW newclinicname = m_PaintManager.FindControl(_T("EditClinicNameOnClinicInfo"))->GetText();
		CStringW newaddr = m_PaintManager.FindControl(_T("EditClinicAddrOnClinicInfo"))->GetText();
		CStringW newtel = m_PaintManager.FindControl(_T("EditClinicTelOnClinicInfo"))->GetText();
		CStringW newchief = m_PaintManager.FindControl(_T("EditClinicChiefOnClinicInfo"))->GetText();
		CStringW newidcard = m_PaintManager.FindControl(_T("EditClinicIDCardOnClinicInfo"))->GetText();
		CStringA poststr = "name=";
		poststr += WStringToUTF8(newclinicname).c_str();
		poststr += "&address=";
		poststr += WStringToUTF8(newaddr).c_str();
		poststr += "&tel=";
		poststr += WStringToUTF8(newtel).c_str();
		poststr += "&chief=";
		poststr += WStringToUTF8(newchief).c_str();
		poststr += "&idcard=";
		poststr += WStringToUTF8(newidcard).c_str();
		poststr += "&Business_license_img=";

		int imgready = 0;
		CStringW temp = m_btnUploadNewLicenseUI->GetUserData();
		if (temp.Left(4) == L"http") {
			poststr += temp.Mid(30);
			imgready++;
		}
		else {
			Json::Value result1 = HttpUploadClinicIMG(temp);
			if (result1["code"] == 0) {
				poststr += result1["data"]["path"].asCString();
				imgready++;
				OutputDebugStringA("uploaf file 1 success.\n");
			}
		}
		poststr += "&local_img=";
		temp = m_btnUploadNewPhotoUI->GetUserData();
		if (temp.Left(4) == L"http") {
			poststr += temp.Mid(30);
			imgready++;
		}
		else {
			Json::Value result2 = HttpUploadClinicIMG(temp);
			if (result2["code"] == 0) {
				poststr += result2["data"]["path"].asCString();
				OutputDebugStringA("uploaf file 1 success.\n");
				imgready++;
			}
		}
		poststr += "&doctor_certificate_img=";
		temp = m_btnUploadNewDoctorLicenseUI->GetUserData();
		if (temp.Left(4) == L"http") {
			poststr += temp.Mid(30);
			imgready++;
		}
		else {
			Json::Value result3 = HttpUploadClinicIMG(temp);
			if (result3["code"] == 0) {
				poststr += result3["data"]["path"].asCString();
				OutputDebugStringA("uploaf file 1 success.\n");
				imgready++;
			}
		}
		if (imgready++ == 3) {
			if (HttpUpdateClinicInfo(poststr) > 0) {
				m_containerShowClinicInfoUI->SetVisible(true);
				m_containerEditClinicInfoUI->SetVisible(false);
				m_containerLoginUI->SetVisible(false);
				m_containerMainpageUI->SetVisible(false);
				m_containerAuditingUI->SetVisible(true);
				m_containerTitleUI->SetBkImage(L"bkimage/top-bg.png");
				m_containerResetPwdUI->SetVisible(false);
				m_containerSignUpUI->SetVisible(false);
				m_containerSignInfoUI->SetVisible(false);
				m_diagArrearageUI->SetVisible(false);
				m_diagCancelApppontmentUI->SetVisible(false);
			}
			else {
				::MessageBox(*this, rtmessage, _T("提示"), 0);
			}
		}
	}
	else if (msg.pSender->GetName() == _T("ButtonCancelReferClinicInfo")) {
		CStringA logmsg = "取消修改诊所信息。\r\n";
		HDWriteLog(logmsg);
		m_containerShowClinicInfoUI->SetVisible(true);
		m_containerEditClinicInfoUI->SetVisible(false);
	}


	else if (msg.pSender->GetName() == _T("ButtonConfrimUnpayHint")) {
		CStringA logmsg = "确认欠费弹窗。\r\n";
		HDWriteLog(logmsg);
		m_diagArrearageUI->SetVisible(false);
		m_containerAppointmentListUI->SetVisible(false);
		m_containerToAppointUI->SetVisible(true);
		m_containerAppointmentDetailUI->SetVisible(false);
		m_containerEditAppointmentUI->SetVisible(false);
	}


	else if (msg.pSender->GetName() == _T("ButtonConfirmCancelAppoint")) {
		CStringA logmsg = "确认取消预约。\r\n";
		HDWriteLog(logmsg);
		m_diagCancelApppontmentUI->SetVisible(false);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetParent()->GetParent()->GetUserData();
		if (HttpCancelAppointment(appointmentnumber) > 0) {
			m_listAppointmentUI->RemoveAll();
			HttpGetAppointmentList(APPOINTMENT_LIST_URL, 1);
		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), NULL);

		}
	}
	else if (msg.pSender->GetName() == _T("ButtonNotCancelAppoint")) {
		CStringA logmsg = "不取消预约。\r\n";
		HDWriteLog(logmsg);
		m_diagCancelApppontmentUI->SetVisible(false);
	}


	else if (msg.pSender == m_btnUploadLicenseUI) {
		CStringA logmsg = "选择诊所执照。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadLicenseUI, CLINIC_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadPhotoUI) {
		CStringA logmsg = "选择诊所实景照片。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadPhotoUI, CLINIC_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadDoctorLicenseUI) {
		CStringA logmsg = "选择诊所医生执照。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadDoctorLicenseUI, CLINIC_PIC_SIZE);
	}
	else if (msg.pSender == m_btnReUploadLicenseUI) {
		CStringA logmsg = "重新选择诊所执照。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnReUploadLicenseUI, CLINIC_PIC_SIZE);
	}
	else if (msg.pSender == m_btnReUploadPhotoUI) {
		CStringA logmsg = "重新选择诊所实景照片。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnReUploadPhotoUI, CLINIC_PIC_SIZE);
	}
	else if (msg.pSender == m_btnReUploadDoctorLicenseUI) {
		CStringA logmsg = "重新选择诊所实景医生执照。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnReUploadDoctorLicenseUI, CLINIC_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadNewLicenseUI) {
		CStringA logmsg = "修改诊所信息页面更新诊所执照。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadNewLicenseUI, CLINIC_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadNewPhotoUI) {
		CStringA logmsg = "修改诊所信息页面更新诊所实景照片。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadNewPhotoUI, CLINIC_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadNewDoctorLicenseUI) {
		CStringA logmsg = "修改诊所信息页面更新诊所医生执照。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadNewDoctorLicenseUI, CLINIC_PIC_SIZE);
	}


	else if (msg.pSender == m_btnUploadPatientPic1UI) {
		CStringA logmsg = "选择患者病情图像1。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadPatientPic1UI, PATIENT_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadPatientPic2UI) {
		CStringA logmsg = "选择患者病情图像2。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadPatientPic2UI, PATIENT_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadPatientPic3UI) {
		CStringA logmsg = "选择患者病情图像3。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadPatientPic3UI, PATIENT_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadNewPatientPic1UI) {
		CStringA logmsg = "更新选择患者病情图像1。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadNewPatientPic1UI, PATIENT_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadNewPatientPic2UI) {
		CStringA logmsg = "更新选择患者病情图像2。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadNewPatientPic2UI, PATIENT_PIC_SIZE);
	}
	else if (msg.pSender == m_btnUploadNewPatientPic3UI) {
		CStringA logmsg = "更新选择患者病情图像3。\r\n";
		HDWriteLog(logmsg);
		OpenFileDialog(m_btnUploadNewPatientPic3UI, PATIENT_PIC_SIZE);
	}

	else if (msg.pSender == m_btnShowPatientPic1UIOnAMDetail) {
		CStringA logmsg = "预约详情页面查看患者大图1。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}

	else if (msg.pSender == m_btnShowPatientPic2UIOnAMDetail) {
		CStringA logmsg = "预约详情页面查看患者大图2。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender == m_btnShowPatientPic3UIOnAMDetail) {
		CStringA logmsg = "预约详情页面查看患者大图3。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender== m_btnShowPatientPic1UIOnRDDetail) {
		CStringA logmsg = "诊断记录详情页面查看患者大图1。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender == m_btnShowPatientPic2UIOnRDDetail) {
		CStringA logmsg = "诊断记录详情页面查看患者大图2。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender == m_btnShowPatientPic1UIOnRDDetail) {
		CStringA logmsg = "诊断记录详情页面查看患者大图3。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonFirstPageOfApppont")) {
		CStringW curl = APPOINTMENT_LIST_URL;
		curl += appointsearchtext;
		m_listAppointmentUI->RemoveAll();
		HttpGetAppointmentList(curl, 1);
	}

	else if (msg.pSender->GetName() == _T("ButtonEndPageOfApppont")) {
		CStringW curl = APPOINTMENT_LIST_URL;
		curl += appointsearchtext;
		m_listAppointmentUI->RemoveAll();
		HttpGetAppointmentList(curl, appointlisttotalpagenum);
	}

	else if (msg.pSender->GetName() == _T("ButtonPreviousPageOfApppont")) {
		if (appointlistcurrentpagenum > 1) {
			CStringW curl = APPOINTMENT_LIST_URL;
			curl += appointsearchtext;
			m_listAppointmentUI->RemoveAll();
			int pagenum = appointlistcurrentpagenum - 1;
			HttpGetAppointmentList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonNextPageOfApppont")) {
		if (appointlistcurrentpagenum < appointlisttotalpagenum) {
			CStringW curl = APPOINTMENT_LIST_URL;
			curl += appointsearchtext;
			m_listAppointmentUI->RemoveAll();
			int pagenum = appointlistcurrentpagenum + 1;
			HttpGetAppointmentList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonFirstPageOfRecord")) {
		CStringW curl = RECORD_LIST_URL;
		curl += recordsearchtext;
		m_listRecordUI->RemoveAll();
		HttpGetRecordList(curl, 1);
	}

	else if (msg.pSender->GetName() == _T("ButtonEndPageOfRecord")) {
		CStringW curl = RECORD_LIST_URL;
		curl += recordsearchtext;
		m_listRecordUI->RemoveAll();
		HttpGetRecordList(curl, recordlisttotalpagenum);
	}

	else if (msg.pSender->GetName() == _T("ButtonPreviousPageOfRecord")) {
		if (recordlistcurrentpagenum > 1) {
			CStringW curl = RECORD_LIST_URL;
			curl += recordsearchtext;
			int pagenum = recordlistcurrentpagenum - 1;
			m_listRecordUI->RemoveAll();
			HttpGetRecordList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonNextPageOfRecord")) {
		if (recordlistcurrentpagenum < recordlisttotalpagenum) {
			CStringW curl = RECORD_LIST_URL;
			curl += recordsearchtext;
			int pagenum = recordlistcurrentpagenum + 1;
			m_listRecordUI->RemoveAll();
			HttpGetRecordList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonFirstPageOfUnPay")) {
		m_listUnpayUI->RemoveAll();
		CStringW curl = UNPAY_LIST_URL;
		HttpGetUnPayInfoList(curl, 1);
	}

	else if (msg.pSender->GetName() == _T("ButtonEndPageOfUnPay")) {
		m_listUnpayUI->RemoveAll();
		CStringW curl = UNPAY_LIST_URL;
		HttpGetUnPayInfoList(curl, unpaylisttotalpagenum);
	}

	else if (msg.pSender->GetName() == _T("ButtonPreviousPageOfUnPay")) {
		if (unpaylistcurrentpagenum > 1) {
			int pagenum = unpaylistcurrentpagenum - 1;
			m_listUnpayUI->RemoveAll();
			CStringW curl = UNPAY_LIST_URL;
			HttpGetUnPayInfoList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonNextPageOfUnPay")) {
		if (unpaylistcurrentpagenum < unpaylisttotalpagenum) {
			int pagenum = unpaylistcurrentpagenum + 1;
			m_listUnpayUI->RemoveAll();
			CStringW curl = UNPAY_LIST_URL;
			HttpGetUnPayInfoList(curl, pagenum);
		}
		else {
		}
	}


	else if (msg.pSender->GetName() == _T("ButtonFirstPageOfAllPay")) {
		m_listAllpayUI->RemoveAll();
		CStringW curl = ALLPAY_LIST_URL;
		HttpGetAllPayInfoList(curl, 1);
	}

	else if (msg.pSender->GetName() == _T("ButtonEndPageOfAllPay")) {
		m_listAllpayUI->RemoveAll();
		CStringW curl = ALLPAY_LIST_URL;
		HttpGetAllPayInfoList(curl, unpaylisttotalpagenum);
	}

	else if (msg.pSender->GetName() == _T("ButtonPreviousPageOfAllPay")) {
		if (allpaylistcurrentpagenum > 1) {
			int pagenum = allpaylistcurrentpagenum - 1;
			m_listAllpayUI->RemoveAll();
			CStringW curl = ALLPAY_LIST_URL;
			HttpGetAllPayInfoList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonNextPageOfAllPay")) {
		if (allpaylistcurrentpagenum < allpaylisttotalpagenum) {
			int pagenum = allpaylistcurrentpagenum + 1;
			m_listAllpayUI->RemoveAll();
			CStringW curl = ALLPAY_LIST_URL;
			HttpGetAllPayInfoList(curl, pagenum);
		}
		else {
		}
	}
}


void CDuiFrameWnd::OnItemButtonClick(TNotifyUI &msg) {
	/*
	wchar_t NumberChar[100];
	ZeroMemory(NumberChar, 100 * sizeof(wchar_t));
	wsprintfW(NumberChar, L"item index = %d\n", msg.wParam);
	OutputDebugStringW(NumberChar);
	OutputDebugStringA("-----click appointment list item \n ");
	*/
	if (msg.pSender->GetName() == _T("ButtonCheckAppointDetail"))
	{
		CStringA logmsg = "查看预约详情。\r\n";
		HDWriteLog(logmsg);
		m_optionPatientConditionPageOnShowAM->Selected(true);
		m_tabPatientInfoOnDetailControlUI->SelectItem(0);
		m_optionPatientConditionPageOnShowAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnShowAM->SetBkColor(0x00000000);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		HttpGetAppointmentDetail(appointmentnumber);
		m_containerAppointmentListUI->SetVisible(false);
		m_containerToAppointUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(true);
		m_containerEditAppointmentUI->SetVisible(false);

	}
	else if (msg.pSender->GetName() == _T("ButtonEditAppoint"))
	{
		CStringA logmsg = "修改预约。\r\n";
		HDWriteLog(logmsg);
		m_optionPatientConditionPageOnEditAM->Selected(true);
		m_tabPatientInfoOnEditControlUI->SelectItem(0);
		m_optionPatientConditionPageOnEditAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnEditAM->SetBkColor(0x00000000);
		if (expertnum > 0) {
			m_cboChooseExpertOnEditAppoint->RemoveAll();
			for (int i = 0; i < expertnum; ++i) {
				CListLabelElementUI* pLine = new CListLabelElementUI;
				CStringW temp = UTF8ToWString(expertlist[i]["name"].asCString());
				if (pLine != NULL)
				{
					m_cboChooseExpertOnEditAppoint->Add(pLine);
					pLine->SetText(temp);
					pLine->SetTag(i);
				}
			}
		}
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		m_containerAppointmentListUI->SetVisible(false);
		m_containerToAppointUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(false);
		m_containerEditAppointmentUI->SetVisible(true);
		m_PaintManager.FindControl(_T("ButtonEditBasicInfoOnEditAM"))->SetVisible(true);
		m_PaintManager.FindControl(_T("ButtonReturnToBasicInfoOnEditAM"))->SetVisible(false);
		m_PaintManager.FindControl(_T("container_showexpertinfooneditam"))->SetVisible(true);
		m_PaintManager.FindControl(_T("container_editexpertinfooneditam"))->SetVisible(false);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetVisible(false);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetVisible(false);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetVisible(false);
		m_tileFreeTimeOfEditMorningAppointUI->RemoveAll();
		m_tileFreeTimeOfEditAfternoonAppointUI->RemoveAll();
		m_tileFreeTimeOfEditAfternoonAppointUI->RemoveAll();
		m_tileFreeTimeOfEditMorningAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditAfternoonAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditAfternoonAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditMorningAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfEditAfternoonAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfEditAfternoonAppointUI->SetMinHeight(1);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetMinHeight(75);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetMaxHeight(75);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMinHeight(75);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMaxHeight(75);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetMinHeight(56);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetMaxHeight(56);
		
		HttpGetAppointmentUpdate(appointmentnumber);
	}
	else if (msg.pSender->GetName() == _T("ButtonCancelAppoint"))
	{
		CStringA logmsg = "取消预约。\r\n";
		HDWriteLog(logmsg);
		m_diagCancelApppontmentUI->SetVisible(true);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		m_diagCancelApppontmentUI->SetUserData(appointmentnumber);
		OutputDebugStringW(L"-----");
		OutputDebugStringW(m_diagCancelApppontmentUI->GetUserData());
	}


	else if (msg.pSender->GetName() == _T("ButtonCheckRecordDetail"))
	{
		CStringA logmsg = "查看诊断记录详情。\r\n";
		HDWriteLog(logmsg);
		m_optionPatientConditionPageOnShowRecord->Selected(true);
		m_tabPatientInfoOnRecordControlUI->SelectItem(0);
		m_optionPatientConditionPageOnShowRecord->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnShowRecord->SetBkColor(0x00000000);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		HttpGetRecordDetail(appointmentnumber);
		m_containerRecordListUI->SetVisible(false);
		m_containerRecordDetailUI->SetVisible(true);
		m_containerEditRecordUI->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("ButtonEditRecord"))
	{
		CStringA logmsg = "修改诊断记录。\r\n";
		HDWriteLog(logmsg);
		m_optionPatientConditionPageOnEditRecord->Selected(true);
		m_tabPatientInfoOnEditRecordControlUI->SelectItem(0);
		m_optionPatientConditionPageOnEditRecord->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnEditRecord->SetBkColor(0x00000000);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		m_containerRecordListUI->SetVisible(false);
		m_containerRecordDetailUI->SetVisible(false);
		m_containerEditRecordUI->SetVisible(true);
		HttpGetRecordUpdate(appointmentnumber);
	}


	else if (msg.pSender->GetName() == _T("ButtonStartZoom"))
	{		
		m_pMeetingServiceMgr->UnInit();
		CStringA logmsg = "开始视频会诊。\r\n";
		HDWriteLog(logmsg);
		//inmeeting = 0;
		startmeetingfailed = 0;
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();

		if (HttpGetMeetingNumber(appointmentnumber) > 0) {
			m_labWaitMeetingStart->SetVisible(true);
			if (m_pAuthServiceMgr->m_bauthed == true) {
				if (m_pMeetingServiceMgr->isinmeetingmow == true) {
					::MessageBox(*this, _T("正在诊疗中。"), _T("提示"), 0);
				}
				else {
					SDKJoinMeeting();
				}					
			}
			else {
				::MessageBox(*this, _T("远程会诊未初始化，请重新启动程序。"), _T("提示"), 0);
			}
		}
		else {
			CListContainerElementUI* cline = (CListContainerElementUI*)msg.pSender->GetParent()->GetParent();
			CStringW appointtime = cline->FindSubControl(_T("LabelAppointmentTimeOnCAMLine"))->GetText();
			CStringW appointminute = appointtime.Mid(11,5);
			OutputDebugStringW(appointminute);
			OutputDebugStringW(L"\r\n");
			if ((appointminute == L"08:00") || (appointminute == L"14:00") || (appointminute == L"19:00")) {
				::MessageBox(*this,_T("专家还未开始会议，请稍候。"), _T("提示"), 0);
			}
			else {
				::MessageBox(*this, rtmessage, _T("提示"), 0);
			}

		}
		
	}	
	else if (msg.pSender->GetName() == _T("ButtonToRecord"))
	{		
		CStringA logmsg = "打开记录诊断新窗口。\r\n";
		HDWriteLog(logmsg);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		POINT pt = { 0, 0 };
		m_pToRecordConsultationWnd = new CDiagToRecordConsultationWnd();
		m_pToRecordConsultationWnd->Init(*this, pt);
		m_pToRecordConsultationWnd->InitRecordData(appointmentnumber, token);
		m_pToRecordConsultationWnd->ShowWindow();
	}

	else if (msg.pSender->GetName() == _T("ButtonExpertDetailOnExpertInfoItem"))
	{
		CStringA logmsg = "打开专家详情新窗口。\r\n";
		HDWriteLog(logmsg);
		OutputDebugStringW(L"open expert detail dialog.\r\n");
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetParent()->GetUserData();
		OutputDebugStringW(temp);
		OutputDebugStringW(L"\r\n");
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonToAppointExpertOnExpertInfoItem"))
	{
		CStringA logmsg = "进入预约页面。\r\n";
		HDWriteLog(logmsg);
		m_dataChooseOnSearchAppoint->SetText(_T(""));
		appointsearchtext = L"";
		m_labWaitMeetingStart->SetVisible(false);
		m_tabMainControlUI->SelectItem(1);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_selected.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_normal.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_normal.png");
		m_optionPaymentRecord->SetBkImage(L"Option/payinfo_normal.png");
		m_optionDoctorInfo->SetBkImage(L"Option/expertinfo_normal.png");
		m_optionClinicInfo->SetBkImage(L"Option/clinicinfo_normal.png");

		if (expertnum > 0) {
			m_cboChooseExpertOnToAppoint->RemoveAll();
			for (int i = 0; i < expertnum; ++i) {
				CListLabelElementUI* pLine = new CListLabelElementUI;
				CStringW temp = UTF8ToWString(expertlist[i]["name"].asCString());
				if (pLine != NULL)
				{
					m_cboChooseExpertOnToAppoint->Add(pLine);
					pLine->SetText(temp);
					pLine->SetTag(i);
				}
			}
		}
		m_tabPatientInfoControlUI->SelectItem(0);
		m_optionPatientConditionPageOnToAM->Selected(true);
		m_optionPatientConditionPageOnToAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnToAM->SetBkColor(0x00000000);
		m_tileFreeTimeOfMorningAppointUI->RemoveAll();
		m_tileFreeTimeOfAfternoonAppointUI->RemoveAll();
		m_tileFreeTimeOfNightAppointUI->RemoveAll();
		m_tileFreeTimeOfMorningAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfAfternoonAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfNightAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfMorningAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfAfternoonAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfNightAppointUI->SetMinHeight(1);
		m_layFreeTimeOfMorningAppointUI->SetMaxHeight(56);
		m_layFreeTimeOfMorningAppointUI->SetMinHeight(56);
		m_layeFreeTimeOfAfternoonAppointUI->SetMaxHeight(56);
		m_layeFreeTimeOfAfternoonAppointUI->SetMinHeight(56);
		m_layFreeTimeOfNightAppointUI->SetMaxHeight(56);
		m_layFreeTimeOfNightAppointUI->SetMinHeight(56);
		m_PaintManager.FindControl(_T("EditPatientNameOnToAM"))->SetText(NULL);
		m_PaintManager.FindControl(_T("EditPatientDescOnToAM"))->SetText(L"");
		m_btnUploadPatientPic1UI->SetBkImage(L"");
		m_btnUploadPatientPic2UI->SetBkImage(L"");
		m_btnUploadPatientPic3UI->SetBkImage(L"");

		int unpaynum = HttpGetUserPayStatus();
		if (unpaynum > 0) {
			//处于未结清状态弹出提示
			m_diagArrearageUI->SetVisible(true);
			m_containerAppointmentListUI->SetVisible(false);
			m_containerToAppointUI->SetVisible(true);
			m_containerAppointmentDetailUI->SetVisible(false);
			m_containerEditAppointmentUI->SetVisible(false);
		}
		else if (unpaynum == 0) {
			m_containerAppointmentListUI->SetVisible(false);
			m_containerToAppointUI->SetVisible(true);
			m_containerAppointmentDetailUI->SetVisible(false);
			m_containerEditAppointmentUI->SetVisible(false);
		}
	}
}


void CDuiFrameWnd::OnItemSelect(TNotifyUI &msg) {
	if (msg.pSender == m_cboChooseExpertOnToAppoint) {
		CStringA logmsg = "预约页面选择专家。\r\n";
		HDWriteLog(logmsg);
		int expertindex = m_cboChooseExpertOnToAppoint->GetCurSel();
		char aa[20];
		int cost = expertlist[expertindex]["fee_by_money"].asInt();
		sprintf_s(aa, "%d", cost);
		CStringW bb = aa;
		m_PaintManager.FindControl(_T("LabelCostMoneyOnToAM"))->SetText(bb);

		m_tileFreeTimeOfMorningAppointUI->RemoveAll();
		m_tileFreeTimeOfAfternoonAppointUI->RemoveAll();
		m_tileFreeTimeOfNightAppointUI->RemoveAll();
		m_tileFreeTimeOfMorningAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfAfternoonAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfNightAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfMorningAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfAfternoonAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfNightAppointUI->SetMinHeight(1);
		m_layFreeTimeOfMorningAppointUI->SetMaxHeight(56);
		m_layFreeTimeOfMorningAppointUI->SetMinHeight(56);
		m_tileFreeTimeOfNightAppointUI->SetMaxHeight(56);
		m_tileFreeTimeOfNightAppointUI->SetMinHeight(56);
		m_tileFreeTimeOfAfternoonAppointUI->SetMaxHeight(56);
		m_tileFreeTimeOfAfternoonAppointUI->SetMinHeight(56);
	}

	if (msg.pSender == m_cboChooseExpertOnEditAppoint) {
		CStringA logmsg = "修改预约页面选择专家。\r\n";
		HDWriteLog(logmsg);
		int expertindex = m_cboChooseExpertOnEditAppoint->GetCurSel();
		char aa[20];
		int cost = expertlist[expertindex]["fee_by_money"].asInt();
		sprintf_s(aa, "%d", cost);
		CStringW bb = aa;
		m_PaintManager.FindControl(_T("LabelCostMoneyOnEditAM"))->SetText(bb);

		m_tileFreeTimeOfEditMorningAppointUI->RemoveAll();
		m_tileFreeTimeOfEditAfternoonAppointUI->RemoveAll();
		m_tileFreeTimeOfEditNightAppointUI->RemoveAll();
		m_tileFreeTimeOfEditMorningAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditAfternoonAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditNightAppointUI->SetMaxHeight(1);
		m_tileFreeTimeOfEditMorningAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfEditAfternoonAppointUI->SetMinHeight(1);
		m_tileFreeTimeOfEditNightAppointUI->SetMinHeight(1);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetMinHeight(75);
		m_layFreeTimeOfMorningAppointOnEditAMUI->SetMaxHeight(75);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMinHeight(75);
		m_layeFreeTimeOfAfternoonAppointOnEditAMUI->SetMaxHeight(75);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetMinHeight(56);
		m_layFreeTimeOfNightAppointOnEditAMUI->SetMaxHeight(56);

	}

}

void CDuiFrameWnd::OnSelectChanged(TNotifyUI &msg) {
	CDuiString name = msg.pSender->GetName();
	if (name == _T("CheckAppointment")) {
		CStringA logmsg = "进入查看预约页面。\r\n";
		HDWriteLog(logmsg);
		m_dataChooseOnSearchAppoint->SetText(_T(""));
		appointsearchtext = L"";
		//m_btnCloseEXEUI->SetVisible(false);
		m_labWaitMeetingStart->SetVisible(false);
		m_tabMainControlUI->SelectItem(1);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_selected.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_normal.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_normal.png");
		m_optionPaymentRecord->SetBkImage(L"Option/payinfo_normal.png");
		m_optionDoctorInfo->SetBkImage(L"Option/expertinfo_normal.png");
		m_optionClinicInfo->SetBkImage(L"Option/clinicinfo_normal.png");
		expertnum = HttpGetExpertList();
		if (expertnum > 0) {
			m_cboChooseExpertOnSearchAppoint->RemoveAll();
			for (int i = 0; i < expertnum; ++i) {
				CListLabelElementUI* pLine = new CListLabelElementUI;
				CStringW temp = UTF8ToWString(expertlist[i]["name"].asCString());
				if (pLine != NULL)
				{
					m_cboChooseExpertOnSearchAppoint->Add(pLine);
					pLine->SetText(temp);
					pLine->SetTag(i);
				}
			}
		}
		m_listAppointmentUI->RemoveAll();
		HttpGetAppointmentList(APPOINTMENT_LIST_URL, 1);
	}		
	else if (name == _T("ConsultationRecord")) {
		CStringA logmsg = "进入诊断记录页面。\r\n";
		HDWriteLog(logmsg);
		m_tabMainControlUI->SelectItem(2);
		//m_btnCloseEXEUI->SetVisible(false);
		m_labWaitMeetingStart->SetVisible(false);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_normal.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_selected.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_normal.png");
		m_optionPaymentRecord->SetBkImage(L"Option/payinfo_normal.png");
		m_optionDoctorInfo->SetBkImage(L"Option/expertinfo_normal.png");
		m_optionClinicInfo->SetBkImage(L"Option/clinicinfo_normal.png");

		m_dataChooseOnSearchRecord->SetText(_T(""));
		recordsearchtext = L"";
		if (expertnum > 0) {
			m_cboChooseExpertOnSearchRecord->RemoveAll();
			for (int i = 0; i < expertnum; ++i) {
				CListLabelElementUI* pLine = new CListLabelElementUI;
				CStringW temp = UTF8ToWString(expertlist[i]["name"].asCString());
				if (pLine != NULL)
				{
					m_cboChooseExpertOnSearchRecord->Add(pLine);
					pLine->SetText(temp);
					pLine->SetTag(i);
				}
			}
		}
		m_listRecordUI->RemoveAll();
		HttpGetRecordList(RECORD_LIST_URL, 1);
	}
		
	else if (name == _T("CurrentConsultation")) {
		CStringA logmsg = "进入当日诊疗页面。\r\n";
		HDWriteLog(logmsg);
		m_tabMainControlUI->SelectItem(3);
		//m_btnCloseEXEUI->SetVisible(false);
		m_labWaitMeetingStart->SetVisible(false);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_normal.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_normal.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_selected.png");
		m_optionPaymentRecord->SetBkImage(L"Option/payinfo_normal.png");
		m_optionDoctorInfo->SetBkImage(L"Option/expertinfo_normal.png");
		m_optionClinicInfo->SetBkImage(L"Option/clinicinfo_normal.png");

		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char aa[20];
		sprintf_s(aa, "%4d-%02d-%02d", sys.wYear, sys.wMonth, sys.wDay);
		CStringW bb = aa;
		m_PaintManager.FindControl(_T("LabelDateUI"))->SetText(bb);
		CStringW curl = CURRENT_APPOINTMENT_LIST_URL;
		curl += L"&date=";
		curl += bb;
		//curl += L"2017-07-17";
		curl += L"&status=2";
		m_listCAppointmentUI->RemoveAll();
		HttpGetCurrentAppointmentList(curl);
	}
	else if (name == _T("PaymentRecord")) {
		CStringA logmsg = "进入支付记录页面。\r\n";
		HDWriteLog(logmsg);
		m_tabMainControlUI->SelectItem(4);
		//m_btnCloseEXEUI->SetVisible(false);
		m_labWaitMeetingStart->SetVisible(false);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_normal.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_normal.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_normal.png");
		m_optionPaymentRecord->SetBkImage(L"Option/payinfo_selected.png");
		m_optionDoctorInfo->SetBkImage(L"Option/expertinfo_normal.png");
		m_optionClinicInfo->SetBkImage(L"Option/clinicinfo_normal.png");

		m_optionUnPay->Selected(true);
		m_tabPayTypeControlUI->SelectItem(0);
		m_optionUnPay->SetBkImage(L"bkimage/option_bk1.png");
		m_optionAllPay->SetBkImage(L"bkimage/option_bk2.png");
		m_optionBill->SetBkImage(L"bkimage/option_bk2.png");
		m_optionWeixinCode->SetBkImage(L"bkimage/option_bk2.png");
		m_listUnpayUI->RemoveAll();
		CStringW curl = UNPAY_LIST_URL;
		HttpGetUnPayInfoList(curl, 1);

	}
		
	else if (name == _T("DoctorInfo")) {
		CStringA logmsg = "进入专家信息页面。\r\n";
		HDWriteLog(logmsg);
		m_tabMainControlUI->SelectItem(0);
		//m_btnCloseEXEUI->SetVisible(false);
		m_labWaitMeetingStart->SetVisible(false);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_normal.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_normal.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_normal.png");
		m_optionPaymentRecord->SetBkImage(L"Option/payinfo_normal.png");
		m_optionDoctorInfo->SetBkImage(L"Option/expertinfo_selected.png");
		m_optionClinicInfo->SetBkImage(L"Option/clinicinfo_normal.png");
		if (doctorinfohtml == 0) {
			HttpGetExpertInfo();
			doctorinfohtml = 1;
		}

		m_optionExpertTileInfo->Selected(true);
		m_tabDoctorInfoTypeControlUI->SelectItem(0);
		m_optionExpertTileInfo->SetBkImage(L"bkimage/option_bk1.png");
		m_optionDoctorWorkTime->SetBkImage(L"bkimage/option_bk2.png");

		m_listDoctorWorkInfoUI->RemoveAll();
		HttpGetExpertWorkTimeList();

	}		
	else if (name == _T("ClinicInfo")) {
		CStringA logmsg = "进入诊所信息页面。\r\n";
		HDWriteLog(logmsg);
		m_tabMainControlUI->SelectItem(5);
		//m_btnCloseEXEUI->SetVisible(true);
		m_labWaitMeetingStart->SetVisible(false);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_normal.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_normal.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_normal.png");
		m_optionPaymentRecord->SetBkImage(L"Option/payinfo_normal.png");
		m_optionDoctorInfo->SetBkImage(L"Option/expertinfo_normal.png");
		m_optionClinicInfo->SetBkImage(L"Option/clinicinfo_selected.png");

		m_tabClinicInfoControlUI->SelectItem(0);
		m_optionUserInfo->Selected(true);
		m_optionUserInfo->SetBkImage(L"bkimage/option_bk1.png");
		m_optionClinicSignInfo->SetBkImage(L"bkimage/option_bk2.png");

		HttpGetClinicInfo();
	}		

	else if (name == _T("OptionPatientConditionPage")) {
		CStringA logmsg = "预约页面进入患者主述tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoControlUI->SelectItem(0);
		m_optionPatientConditionPageOnToAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnToAM->SetBkColor(0x00000000);
		//m_labPatientConditionHintOnToAM->SetVisible(true);
		//m_labPatientImgHintOnToAM->SetVisible(false);
	}
		
	else if (name == _T("OptionPatientPicPage")) {
		CStringA logmsg = "预约页面进入患者图像tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoControlUI->SelectItem(1);
		m_optionPatientConditionPageOnToAM->SetBkColor(0x00000000);
		m_optionPatientPicPageOnToAM->SetBkColor(0xffffffff);
		//m_labPatientConditionHintOnToAM->SetVisible(false);
		//m_labPatientImgHintOnToAM->SetVisible(true);
	}
		

	else if (name == _T("OptionPatientConditionPageOnDetail")) {
		CStringA logmsg = "预约详情页面进入患者主述tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnDetailControlUI->SelectItem(0);
		m_optionPatientConditionPageOnShowAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnShowAM->SetBkColor(0x00000000);
	}
		
	else if (name == _T("OptionPatientPicPageOnDetail")) {
		CStringA logmsg = "预约详情页面进入患者图像tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnDetailControlUI->SelectItem(1);
		m_optionPatientConditionPageOnShowAM->SetBkColor(0x00000000);
		m_optionPatientPicPageOnShowAM->SetBkColor(0xffffffff);
	}
		

	else if (name == _T("OptionPatientConditionPageOnEdit")) {
		CStringA logmsg = "修改预约页面进入患者主述tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnEditControlUI->SelectItem(0);
		m_optionPatientConditionPageOnEditAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnEditAM->SetBkColor(0x00000000);
	}		
	else if (name == _T("OptionPatientPicPageOnEdit")) {
		CStringA logmsg = "修改预约页面进入患者图像tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnEditControlUI->SelectItem(1);
		m_optionPatientConditionPageOnEditAM->SetBkColor(0x00000000);
		m_optionPatientPicPageOnEditAM->SetBkColor(0xffffffff);

	}
		

	else if (name == _T("OptionPatientConditionPageOnRecord")) {
		CStringA logmsg = "诊断记录详情页面进入患者主述tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnRecordControlUI->SelectItem(0);
		m_optionPatientConditionPageOnShowRecord->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnShowRecord->SetBkColor(0x00000000);
	}
		
	else if (name == _T("OptionPatientPicPageOnRecord")) {
		CStringA logmsg = "诊断记录详情页面进入患者图像tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnRecordControlUI->SelectItem(1);
		m_optionPatientConditionPageOnShowRecord->SetBkColor(0x00000000);
		m_optionPatientPicPageOnShowRecord->SetBkColor(0xffffffff);
	}
		

	else if (name == _T("OptionPatientConditionPageOnEditRecord")) {
		CStringA logmsg = "修改诊断记录详情页面进入患者主述tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnEditRecordControlUI->SelectItem(0);
		m_optionPatientConditionPageOnEditRecord->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnEditRecord->SetBkColor(0x00000000);
	}
		
	else if (name == _T("OptionPatientPicPageOnEditRecord")) {
		CStringA logmsg = "修改诊断记录详情页面进入患者图像tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnEditRecordControlUI->SelectItem(1);
		m_optionPatientConditionPageOnEditRecord->SetBkColor(0x00000000);
		m_optionPatientPicPageOnEditRecord->SetBkColor(0xffffffff);
	}
		

	else if (name == _T("OptionUnPay")) {
		CStringA logmsg = "进入未支付tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPayTypeControlUI->SelectItem(0);
		m_listUnpayUI->RemoveAll();
		CStringW curl = UNPAY_LIST_URL;
		HttpGetUnPayInfoList(curl,  1);
		m_optionUnPay->SetBkImage(L"bkimage/option_bk1.png");
		m_optionAllPay->SetBkImage(L"bkimage/option_bk2.png");
		m_optionBill->SetBkImage(L"bkimage/option_bk2.png");
		m_optionWeixinCode->SetBkImage(L"bkimage/option_bk2.png");
	}
		
	else if (name == _T("OptionAllPay")) {
		CStringA logmsg = "进入已支付tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPayTypeControlUI->SelectItem(1);
		m_listAllpayUI->RemoveAll();
		CStringW curl = ALLPAY_LIST_URL;
		HttpGetAllPayInfoList(curl,  1);
		m_optionUnPay->SetBkImage(L"bkimage/option_bk2.png");
		m_optionAllPay->SetBkImage(L"bkimage/option_bk1.png");
		m_optionBill->SetBkImage(L"bkimage/option_bk2.png");
		m_optionWeixinCode->SetBkImage(L"bkimage/option_bk2.png");

	}
		
	else if (name == _T("OptionBill")) {
		CStringA logmsg = "进入账单tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPayTypeControlUI->SelectItem(2);
		m_optionUnPay->SetBkImage(L"bkimage/option_bk2.png");
		m_optionAllPay->SetBkImage(L"bkimage/option_bk2.png");
		m_optionBill->SetBkImage(L"bkimage/option_bk1.png");
		m_optionWeixinCode->SetBkImage(L"bkimage/option_bk2.png");
		HttpGetBillInfo();

	}
	
	else if (name == _T("OptionWeixinCode")) {
		CStringA logmsg = "进入微信付款码tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPayTypeControlUI->SelectItem(3);
		m_optionUnPay->SetBkImage(L"bkimage/option_bk2.png");
		m_optionAllPay->SetBkImage(L"bkimage/option_bk2.png");
		m_optionBill->SetBkImage(L"bkimage/option_bk2.png");
		m_optionWeixinCode->SetBkImage(L"bkimage/option_bk1.png");
		if (p_showCodeImageActiveXUI)
		{
			IWebBrowser2* pWebBrowser = NULL;
			p_showCodeImageActiveXUI->SetDelayCreate(false);
			p_showCodeImageActiveXUI->CreateControl(CLSID_WebBrowser);
			p_showCodeImageActiveXUI->GetControl(IID_IWebBrowser2, (void**)&pWebBrowser);

			OutputDebugStringW(L"get in weixin code img.\r\n");
			CStringW codeaddress = L"http://www.handianzy.com/images/hd_fkm.jpg";
			//CStringW codeaddress = L"http://www.baidu.com";
			BSTR webUrl = codeaddress.AllocSysString();
			if (pWebBrowser != NULL)
			{
				pWebBrowser->Navigate(webUrl, NULL, NULL, NULL, NULL);
				pWebBrowser->Release();
			}
		}
	}

	else if (name == _T("OptionDoctorInfo")) {
		CStringA logmsg = "进入专家信息tab。\r\n";
		HDWriteLog(logmsg);
		m_tabDoctorInfoTypeControlUI->SelectItem(0);
		m_optionExpertTileInfo->SetBkImage(L"bkimage/option_bk1.png");
		m_optionDoctorWorkTime->SetBkImage(L"bkimage/option_bk2.png");
	}

	else if (name == _T("OptionDoctorWorkTime")) {
		CStringA logmsg = "进入专家坐诊时间tab。\r\n";
		HDWriteLog(logmsg);
		m_tabDoctorInfoTypeControlUI->SelectItem(1);
		m_optionExpertTileInfo->SetBkImage(L"bkimage/option_bk2.png");
		m_optionDoctorWorkTime->SetBkImage(L"bkimage/option_bk1.png");
	}
	

	else if (name == _T("OptionUserInfoPage")) {
		CStringA logmsg = "进入用户信息tab。\r\n";
		HDWriteLog(logmsg);
		m_tabClinicInfoControlUI->SelectItem(0);
		m_optionUserInfo->SetBkImage(L"bkimage/option_bk1.png");
		m_optionClinicSignInfo->SetBkImage(L"bkimage/option_bk2.png");
	}

	else if (name == _T("OptionClinicInfoPage")) {
		CStringA logmsg = "进入诊所信息tab。\r\n";
		HDWriteLog(logmsg);
		m_tabClinicInfoControlUI->SelectItem(1);
		m_optionUserInfo->SetBkImage(L"bkimage/option_bk2.png");
		m_optionClinicSignInfo->SetBkImage(L"bkimage/option_bk1.png");
	}


}


