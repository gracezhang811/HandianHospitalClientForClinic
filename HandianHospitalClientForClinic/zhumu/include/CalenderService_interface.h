#ifndef _ZHUMU_CALENDERSERVICE_INTERFACE_H_
#define _ZHUMU_CALENDERSERVICE_INTERFACE_H_
#include <windows.h>
#include "zhumu_sdk_def.h"
#include <list>
using namespace std;


/// \brief Zhumu SDK Namespace
/// 
///
namespace ZHUMU_SDK_NAMESPACE {


class ICalenderEvent
{
public:
	virtual UINT64 GetMeetingNum() = 0;
	virtual const wchar_t* GetEventID() = 0;
	virtual const wchar_t* GetSummary() = 0;
	virtual const wchar_t* GetStartTime() = 0;
	virtual const wchar_t* GetEndTime() = 0;
	virtual const wchar_t* GetCreatorEmail() = 0;
	virtual const wchar_t* GetCreatorName() = 0;
	virtual const wchar_t* GetDescription() = 0;
};
enum CalenderResult
{
	CALENDER_SUCCESS,
	CALENDER_AUTH_ERROR,
	CALENDER_WEBAPI_ERROR,
	CALENDER_UNKNOW,
};

/// \brief Calender Service Callback Event
///
class ICalenderServiceEvent
{
public:
	virtual void onCalenderEventsNotifcation(CalenderResult result, list<ICalenderEvent* >* plstEvents, const wchar_t* content) = 0;
};

/*! \struct tagCalenderAuthParam
    \brief SDK Calender Authentication Parameter.
    A more detailed struct description.
*/
typedef struct tagCalenderAuthParam
{
	typedef struct tagGoogleCalAuth
	{
		const wchar_t* authCode;
	}GoogleCalAuth;
	typedef struct tagOutlookExchageAuth
	{
		const wchar_t* exchangeResEmail;		///< email address of resource in Exchange system
		const wchar_t* exchangeEMSURL;	///< URL of Exchange Web Service
		const wchar_t* exchangePassword;	///< password of the email in Exchange system, not password of the work email in Zhumu system
	}OutlookExchageAuth;
	ICalenderServiceEvent* pEvent;
	const wchar_t* userEmail;
	union
	{
		GoogleCalAuth googleAuth;
		OutlookExchageAuth exchangeAuth;
	};
	bool isGoogleCal;
	tagCalenderAuthParam()
	{
		pEvent = NULL;
		isGoogleCal = false;
	}
}CalenderAuthParam;

/// \brief Calender Service Interface
class ICalenderService
{
public:
	virtual SDKError Init(CalenderAuthParam& authParam) = 0;
	virtual SDKError Uninit() = 0;
	virtual SDKError RetrieveCalEvents(int uElapse, bool bStop) = 0;
};
///
};

#endif