/*!
* \file zhumu_sdk.h
* \brief  Zhumu Windows SDK
*
*/

#include "zhumu_sdk_def.h"
/// \brief Zhumu SDK Namespace
/// 
///
namespace ZHUMU_SDK_NAMESPACE {

	extern "C"
	{
		class IMeetingService;
		class IAuthService;
		class ISettingService;
		class ICalenderService;
		class InitParam;
		class IPreMeetingService;
		class INetworkConnectionHelper;
		class IEmbeddedBrowser;
		/// \brief Initialize Zhumu SDK
		/// \param initParam Initialize Zhumu SDK Parameter
		/// \return If the function succeeds, the return value is SDKErr_Success.
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_InitSDK(ZHUMU_SDK_NAMESPACE::InitParam* initParam);

		/// \brief Create Authentication Service Interface
		/// \param ppAuthService A pointer to a IAuthService* that receives IAuthService Object. 
		/// \return If the function succeeds, the return value is SDKErr_Success, and ppAuthService is not NULL
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_CreateAuthService(IAuthService** ppAuthService);

		/// \brief Destroy Authentication Service Interface
		/// \param pAuthService A pointer to a IAuthService to be destroyed. 
		/// \return If the function succeeds, the return value is SDKErr_Success.
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_DestroyAuthService(IAuthService* pAuthService);

		/// \brief Create Meeting Service Interface
		/// \param ppMeetingService A pointer to a IMeetingService* that receives IMeetingService Object. 
		/// \return If the function succeeds, the return value is SDKErr_Success, and ppMeetingService is not NULL
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_CreateMeetingService(ZHUMU_SDK_NAMESPACE::IMeetingService** ppMeetingService);

		/// \brief Destroy Meeting Service Interface
		/// \param pMeetingService A pointer to a IMeetingService to be destroyed. 
		/// \return If the function succeeds, the return value is SDKErr_Success.
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_DestroyMeetingService(ZHUMU_SDK_NAMESPACE::IMeetingService* pMeetingService);

		/// \brief Create Setting Service Interface
		/// \param ppSettingService A pointer to a ISettingService* that receives ISettingService Object. 
		/// \return If the function succeeds, the return value is SDKErr_Success, and ppSettingService is not NULL
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_CreateSettingService(ZHUMU_SDK_NAMESPACE::ISettingService** ppSettingService);

		/// \brief Destroy Setting Service Interface
		/// \param pSettingService A pointer to a ISettingService to be destroyed. 
		/// \return If the function succeeds, the return value is SDKErr_Success.
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_DestroySettingService(ZHUMU_SDK_NAMESPACE::ISettingService* pSettingService);

		/// \brief Create Pre-Meeting Service Interface
		/// \param ppPreMeetingService A pointer to a IPreMeetingService* that receives IPreMeetingService Object. 
		/// \return If the function succeeds, the return value is SDKErr_Success, and ppCalenderService is not NULL
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_CreatePreMeetingService(IPreMeetingService** ppPreMeetingService);

		/// \brief Destroy Pre-Meeting Service Interface
		/// \param pPreMeetingService A pointer to a IPreMeetingService to be destroyed. 
		/// \return If the function succeeds, the return value is SDKErr_Success.
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_DestroyPreMeetingService(IPreMeetingService* pPreMeetingService);

		/// \brief Create network connection helper Interface
		/// \param ppNetworkHelper A pointer to a INetworkConnectionHelper* that receives INetworkConnectionHelper Object. 
		/// \return If the function succeeds, the return value is SDKErr_Success, and ppNetworkHelper is not NULL
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_CreateNetworkConnectionHelper(INetworkConnectionHelper** ppNetworkHelper);

		/// \brief Destroy network connection helper Interface
		/// \param pNetworkHelper A pointer to a INetworkConnectionHelper to be destroyed. 
		/// \return If the function succeeds, the return value is SDKErr_Success.
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_DestroyNetworkConnectionHelper(INetworkConnectionHelper* pNetworkHelper);

		/// \brief Create Embedded Browser Interface
		/// \param ppEmbeddedBrowser A pointer to a IEmbeddedBrowser* that receives IEmbeddedBrowser Object. 
		/// \param hwnd parent window of the embedded browser. 
		/// \return If the function succeeds, the return value is SDKErr_Success, and ppEmbeddedBrowser is not NULL
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_CreateEmbeddedBrowser(IEmbeddedBrowser** ppEmbeddedBrowser, HWND hwnd);

		/// \brief Destroy Embedded Browser Interface
		/// \param pEmbeddedBrowser A pointer to a IEmbeddedBrowser to be destroyed. 
		/// \return If the function succeeds, the return value is SDKErr_Success.
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_DestroyEmbeddedBrowser(IEmbeddedBrowser* pEmbeddedBrowser);



		/// \brief Clean up Zhumu SDK
		/// \return If the function succeeds, the return value is SDKErr_Success.
		///If the function fails, the return value is not SDKErr_Success. To get extended error information, refer to SDKError enum.
		ZHUMU_SDK_API ZHUMU_SDK_NAMESPACE::SDKError ZHUMU_CleanUPSDK();
	}

};
