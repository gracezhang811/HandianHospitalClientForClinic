#pragma once
#include "zhumu_sdk.h"
#include "MeetingService_interface.h"


class CMeetingServiceMgr :
	public ZHUMU_SDK_NAMESPACE::IMeetingServiceEvent
{
public:
	CMeetingServiceMgr();
	~CMeetingServiceMgr();

	bool Init();
	bool UnInit();
	bool IsInMeeting();
	bool Start(ZHUMU_SDK_NAMESPACE::StartParam& startParam);
	bool Join(ZHUMU_SDK_NAMESPACE::JoinParam& joinParam);
	bool Leave();
	bool End();

	virtual void onMeetingStatusChanged(ZHUMU_SDK_NAMESPACE::MeetingStatus status, int iResult);
	virtual void onRecording2MP4Done(bool bsuccess, int iResult, const wchar_t* szPath);
	virtual void onRecording2MP4Processing(int iPercentage);
	void onRecordingStatus(ZHUMU_SDK_NAMESPACE::RecordingStatus status);
	virtual void onUserJoin(list<unsigned int>* lstUserID, const wchar_t* strUserList);
	virtual void onUserLeft(list<unsigned int>* lstUserID, const wchar_t* strUserList);
	void onRemoteControlStatus(ZHUMU_SDK_NAMESPACE::RemoteControlStatus status, unsigned int userId);
	void onSharingStatus(ZHUMU_SDK_NAMESPACE::SharingStatus status, unsigned int userId);
	void onUserAudioStatusChange(list<ZHUMU_SDK_NAMESPACE::IUserAudioStatus* >* lstAudioStatusChange, const wchar_t* strAudioStatusList);
	void onChatMsgNotifcation(ZHUMU_SDK_NAMESPACE::IChatMsgInfo* chatMsg, const wchar_t* ccc);
	void onUserVideoStatusChange(unsigned int userId, ZHUMU_SDK_NAMESPACE::VideoStatus status);
	void onHostChangeNotification(unsigned int userId);
	void onSpotlightVideoChangeNotification(bool bSpotlight, unsigned int userid);
	void onRecordPriviligeChanged(bool bCanRec);
	void onLowOrRaiseHandStatusChanged(bool bLow, unsigned int userid);

private:
	wchar_t* AnsiToUnicode(const char* szStr);
	inline char* UnicodeToAnsi(const wchar_t* szStr);

private:
	ZHUMU_SDK_NAMESPACE::IMeetingService* m_pMeetingService;
	ZHUMU_SDK_NAMESPACE::ISettingService* m_pSettingService;
	std::wstring m_strCamera;
	bool m_bInited;

public:
	bool isinmeetingmow;
};

