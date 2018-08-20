#include "stdafx.h"
#include "MeetingServiceMgr.h"
#include "zhumu_sdk_def.h"

CMeetingServiceMgr::CMeetingServiceMgr()
{
	m_strCamera = _T("");
	m_bInited = false;
	isinmeetingmow = false;
}

CMeetingServiceMgr::~CMeetingServiceMgr()
{
}

wchar_t* CMeetingServiceMgr::AnsiToUnicode(const char* szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}

//将宽字节wchar_t*转化为单字节char*  
inline char* CMeetingServiceMgr::UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}

bool CMeetingServiceMgr::Init()
{
	if (m_bInited)
		return true;


	//ZHUMU_SDK_NAMESPACE::SDKError aaa = ZHUMU_SDK_NAMESPACE::ZHUMU_CreateMeetingService(&m_pMeetingService);
	//if (aaa != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS) {
	//	wchar_t NumberChar[100];
	//	ZeroMemory(NumberChar, 100 * sizeof(wchar_t));
	//	wsprintfW(NumberChar, L"error type = %d\n", aaa);
	//	OutputDebugStringW(NumberChar);
	//	return false;
	//}
	
	if (ZHUMU_SDK_NAMESPACE::ZHUMU_CreateMeetingService(&m_pMeetingService) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;
		
	if (m_pMeetingService == NULL)
		return false;

	if (m_pMeetingService->SetEvent(this) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	if (ZHUMU_SDK_NAMESPACE::ZHUMU_CreateSettingService(&m_pSettingService) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	if (m_pSettingService == NULL)
		return false;

	m_pSettingService->EnableAutoJoinAudio(true);

	ZHUMU_SDK_NAMESPACE::IMeetingConfiguration* pConfigCtrl = m_pMeetingService->GetMeetingConfiguration();
	if (NULL != pConfigCtrl) {
		pConfigCtrl->SetBottomFloatToolbarWndVisibility(true);
		pConfigCtrl->SetSharingToolbarVisibility(true);
	}

	m_bInited = true;

	return true;
}

bool CMeetingServiceMgr::UnInit()
{
	if (!m_bInited)
		return true;

	m_bInited = false;

	if (ZHUMU_SDK_NAMESPACE::ZHUMU_DestroyMeetingService(m_pMeetingService) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	if (ZHUMU_SDK_NAMESPACE::ZHUMU_DestroySettingService(m_pSettingService) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	return true;
}

bool CMeetingServiceMgr::IsInMeeting()
{
	bool bInMeeting(true);
	ZHUMU_SDK_NAMESPACE::MeetingStatus status = m_pMeetingService->GetMeetingStatus();

	if (status == ZHUMU_SDK_NAMESPACE::MEETING_STATUS_IDLE ||
		status == ZHUMU_SDK_NAMESPACE::MEETING_STATUS_ENDED)
	{
		bInMeeting = false;
	}

	return bInMeeting;
}

bool CMeetingServiceMgr::Start(ZHUMU_SDK_NAMESPACE::StartParam& startParam)
{
	if (m_pMeetingService == NULL)
		return false;

	if (!IsInMeeting())
	{
		if (m_pMeetingService->Start(startParam) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
			return false;
	}

	return true;
}

bool CMeetingServiceMgr::Join(ZHUMU_SDK_NAMESPACE::JoinParam& joinParam)
{
	if (m_pMeetingService == NULL)
		return false;

	if (!IsInMeeting())
	{
		if ((m_pMeetingService->Join(joinParam)) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS) {
			isinmeetingmow = false;
			return false;
		}
		//if (m_pMeetingService->Join(joinParam) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
			//return false;
	}
	isinmeetingmow = true;
	return true;
}

bool CMeetingServiceMgr::Leave()
{
	if (m_pMeetingService == NULL)
		return false;

	if (IsInMeeting())
	{
		if (m_pMeetingService->Leave(ZHUMU_SDK_NAMESPACE::LEAVE_MEETING) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
			return false;
	}

	return true;
}

bool CMeetingServiceMgr::End()
{
	if (m_pMeetingService == NULL)
		return false;

	if (IsInMeeting())
	{
		if (m_pMeetingService->Leave(ZHUMU_SDK_NAMESPACE::END_MEETING) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
			return false;
	}

	return true;
}

void CMeetingServiceMgr::onMeetingStatusChanged(ZHUMU_SDK_NAMESPACE::MeetingStatus status, int iResult)
{
	char szLog[MAX_PATH] = { 0 };
	sprintf_s(szLog, "meeting status:%d, details result:%d\r\n", status, iResult);
	//OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));

	if (ZHUMU_SDK_NAMESPACE::MEETING_STATUS_INMEETING != status)
	{
		std::wstring g_lstCamera;
		g_lstCamera = L"";
		isinmeetingmow = false;
		//InvalidateRect(g_demoWnd, NULL, TRUE);
		sprintf_s(szLog, "meeting status: NOT MEETING_STATUS_INMEETING");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
	}
}

void CMeetingServiceMgr::onRecording2MP4Done(bool bsuccess, int iResult, const wchar_t* szPath)
{
	char szLog[MAX_PATH] = { 0 };
	sprintf_s(szLog, "recording to mp4:%d, details result:%d\r\n", bsuccess, iResult);
	//OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
	//OutputDebugStringW(szPath);
}

void CMeetingServiceMgr::onRecording2MP4Processing(int iPercentage)
{
	char szLog[MAX_PATH] = { 0 };
	sprintf_s(szLog, "recording to mp4 processing:%d%%\r\n", iPercentage);
	//OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
}

void CMeetingServiceMgr::onRecordingStatus(ZHUMU_SDK_NAMESPACE::RecordingStatus status)
{

}

void CMeetingServiceMgr::onUserJoin(list<unsigned int>* lstUserID, const wchar_t* strUserList)
{
	if (m_pMeetingService)
	{
		for (list<unsigned int > ::iterator i = lstUserID->begin(); i != lstUserID->end(); ++i)
		{
			int userId = *i;
			ZHUMU_SDK_NAMESPACE::IUserInfo* pUserInfo = m_pMeetingService->GetUserByUserID(userId);
			if (pUserInfo)
			{
				//OutputDebugStringA("new user join:\r\n");
				OutputDebugStringW(pUserInfo->GetUserName());
				//OutputDebugStringA("\r\n");
				char szUserID[64] = { 0 };
				sprintf_s(szUserID, "%d\r\n", pUserInfo->GetUserID());
				//OutputDebugStringA(szUserID);
			}
		}
	}
}

void CMeetingServiceMgr::onUserLeft(list<unsigned int>* lstUserID, const wchar_t* strUserList)
{
	if (m_pMeetingService)
	{
		for (list<unsigned int > ::iterator i = lstUserID->begin(); i != lstUserID->end(); ++i)
		{
			int userId = *i;
			{
				char szTmp[MAX_PATH] = { 0 };
				sprintf_s(szTmp, "user(id:%d) left the meeting\r\n", userId);
				//OutputDebugStringA(szTmp);
			}
		}
	}
}

void CMeetingServiceMgr::onRemoteControlStatus(ZHUMU_SDK_NAMESPACE::RemoteControlStatus status, unsigned int userId)
{
	char szLog[MAX_PATH] = { 0 };
	sprintf_s(szLog, "onRemoteControlStatus:status=%d, userid=%d\r\n", status, userId);
	//OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));

	if (ZHUMU_SDK_NAMESPACE::Remote_Control_I_Can_Request_Control_Who == status)
	{
		if (m_pMeetingService)
			m_pMeetingService->GetMeetingRemoteController()->GiveRemoteControlTo(userId);
	}
}

void CMeetingServiceMgr::onSharingStatus(ZHUMU_SDK_NAMESPACE::SharingStatus status, unsigned int userId)
{
	char szLog[MAX_PATH] = { 0 };
	sprintf_s(szLog, "onSharingStatus:status=%d, userid=%d\r\n", status, userId);
	//OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
}

void CMeetingServiceMgr::onUserAudioStatusChange(list<ZHUMU_SDK_NAMESPACE::IUserAudioStatus* >* lstAudioStatusChange, const wchar_t* strAudioStatusList = NULL)
{
	int size = lstAudioStatusChange->size();
	for (list<ZHUMU_SDK_NAMESPACE::IUserAudioStatus* >::iterator i = lstAudioStatusChange->begin(); i != lstAudioStatusChange->end(); )
	{
		ZHUMU_SDK_NAMESPACE::IUserAudioStatus* audioStatus = *i;
		char sz[MAX_PATH] = { 0 };
		sprintf_s(sz, "userid:%d, status:%d\r\n", audioStatus->GetUserId(), audioStatus->GetStatus());
		delete *i;
		i = lstAudioStatusChange->erase(i);
		//OutputDebugStringA(sz);
	}
}

void CMeetingServiceMgr::onChatMsgNotifcation(ZHUMU_SDK_NAMESPACE::IChatMsgInfo* chatMsg, const wchar_t* ccc)
{
	if (NULL == chatMsg)
		return;

	wchar_t szTmp[1024] = { 0 };
	wsprintf(szTmp, _T("onChatMsgNotifcation:  %d:%s-->%d:%s::::%s\r\n"), chatMsg->GetSenderUserId(), chatMsg->GetSenderDisplayName(),
		chatMsg->GetReceiverUserId(), chatMsg->GetReceiverDisplayName(), chatMsg->GetContent());
	OutputDebugStringW(szTmp);
}

void CMeetingServiceMgr::onUserVideoStatusChange(unsigned int userId, ZHUMU_SDK_NAMESPACE::VideoStatus status)
{
	wchar_t szTmp[1024] = { 0 };
	wsprintf(szTmp, _T("onUserVideoStatusChange: userId: %d, VideoStatus: %d\r\n"), userId, status);
	OutputDebugStringW(szTmp);
}

void CMeetingServiceMgr::onHostChangeNotification(unsigned int userId)
{
	wchar_t szTmp[1024] = { 0 };
	wsprintf(szTmp, _T("onHostChangeNotification: userId: %d\r\n"), userId);
	OutputDebugStringW(szTmp);
}

void CMeetingServiceMgr::onSpotlightVideoChangeNotification(bool bSpotlight, unsigned int userid)
{
	wchar_t szTmp[1024] = { 0 };
	wsprintf(szTmp, _T("onSpotlightVideoChangeNotification: Spotlight:%d, userId: %d\r\n"), bSpotlight, userid);
	OutputDebugStringW(szTmp);
}

void CMeetingServiceMgr::onRecordPriviligeChanged(bool bCanRec)
{
	wchar_t szTmp[1024] = { 0 };
	wsprintf(szTmp, _T("onRecordPriviligeChanged: CanRec:%d\r\n"), bCanRec);
	OutputDebugStringW(szTmp);
}

void CMeetingServiceMgr::onLowOrRaiseHandStatusChanged(bool bLow, unsigned int userid)
{
	wchar_t szTmp[1024] = { 0 };
	wsprintf(szTmp, _T("onLowOrRaiseHandStatusChanged: Low:%d, userId: %d\r\n"), bLow, userid);
	OutputDebugStringW(szTmp);
}