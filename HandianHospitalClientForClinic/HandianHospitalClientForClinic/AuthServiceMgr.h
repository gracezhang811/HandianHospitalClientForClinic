#pragma once
#include "zhumu_sdk.h"
#include "AuthService_interface.h"


class CAuthServiceMgr :
	public ZHUMU_SDK_NAMESPACE::IAuthServiceEvent
{
public:
	CAuthServiceMgr();
	~CAuthServiceMgr();

	bool Init(ZHUMU_SDK_NAMESPACE::InitParam* initParam);
	bool UnInit();	
	bool SDKAuth(ZHUMU_SDK_NAMESPACE::AuthParam& authParam);
	bool Login(ZHUMU_SDK_NAMESPACE::LoginParam& param);
	bool LogOut();

	virtual void onAuthenticationReturn(ZHUMU_SDK_NAMESPACE::AuthResult ret);
	virtual void onLoginRet(ZHUMU_SDK_NAMESPACE::LOGINSTATUS status, ZHUMU_SDK_NAMESPACE::IAccountInfo* pAccountInfo);
	virtual void onLogout();

private:
	wchar_t* AnsiToUnicode(const char* szStr);
	inline char* UnicodeToAnsi(const wchar_t* szStr);

private:
	ZHUMU_SDK_NAMESPACE::IAuthService* m_pAuthService;
	bool m_bInited;
	bool m_bLogined;
public:
	bool m_bauthed;
};

