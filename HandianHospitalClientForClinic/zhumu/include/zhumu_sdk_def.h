#ifndef _ZHUMU_SDK_DEF_H_
#define _ZHUMU_SDK_DEF_H_
/*!
* \file zhumu_sdk_def.h
* \brief  zhumu Windows SDK Common Defines File
*
*/

#if defined(_WIN32)
//#define ZHUMU_SDK_API	__declspec(dllexport) 
#ifdef ZHUMU_SDK_API  
#define ZHUMU_SDK_API __declspec(dllexport)  
#else  
#define ZHUMU_SDK_API __declspec(dllimport)  
#endif  
#endif
#include "Windows.h"




//#define USING_ZHUMU_SDK_NAMESPACE using namespace ZHUMU_SDK_NAMESPACE;

/// \brief Zhumu SDK Namespace
/// 
///
namespace ZHUMU_SDK_NAMESPACE {
	/*! \enum SDKError
		\brief Init SDK Parameter.
		A more detailed struct description.
	*/
	enum SDKError
	{
		SDKERR_SUCCESS = 0,///< Success Result
		SDKERR_NO_IMPL,///< Not support this feature now 
		SDKERR_WRONG_USEAGE,///< Wrong useage about this feature 
		SDKERR_INVALID_PARAMETER,///< Wrong parameter 
		SDKERR_MODULE_LOAD_FAILED,///< Load module failed 
		SDKERR_MEMORY_FAILED,///< No memory allocated 
		SDKERR_SERVICE_FAILED,///< Internal service error 
		SDKERR_UNINITIALIZE,///< Not initialize before use 
		SDKERR_UNAUTHENTICATION,///< Not Authentication before use
		SDKERR_NORECORDINGINPROCESS,///< No recording in process
		SDKERR_TRANSCODER_NOFOUND,///< can't find transcoder module
		SDKERR_VIDEO_NOTREADY,///< Video service not ready
		SDKERR_NO_PERMISSION,///< No premission to do this
		SDKERR_UNKNOWN,///< Unknown error 
		SDKERR_OTHER_SDK_INSTANCE_RUNNING,
	};

	enum SDK_LANGUAGE_ID
	{
		LANGUAGE_Unknow = 0,
		LANGUAGE_English,
		LANGUAGE_Chinese_Simplified,
		LANGUAGE_Chinese_Traditional,
		LANGUAGE_Japanese,
		LANGUAGE_Spanish,
		LANGUAGE_German,
		LANGUAGE_French,
	};

	typedef struct tagWndPosition
	{
		int left;
		int top;
		tagWndPosition()
		{
			left = 0;
			top = 0;
		}
	}WndPosition;



};

#endif //ZHUMU_SDK_DEF_H_H_