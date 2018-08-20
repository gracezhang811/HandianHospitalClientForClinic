#pragma once

#include "Resource.h"
#include "duilib.h"
#include "def.h"
#include "json.h"
#include "RWinHttp.h"
#include "stdafx.h"

Json::Value			HDWinHttpGet(LPCTSTR curl);
Json::Value 		HDWinHttpPost(LPCTSTR curl, CStringA postdata);
Json::Value 		HDWinHttpGetWithToken(LPCTSTR curl, CStringA token);
Json::Value 		HDWinHttpPostWithToken(LPCTSTR curl, CStringA postdata, CStringA token);
int					HDWinHttpDownloadFile(LPCTSTR curl, LPCTSTR path);
//Json::Value			HDWinHttpUploadFile(LPCTSTR curl, CStringA token, CStringW path);

//char*				HeaderInfoTrans(char* szBuf, DWORD* pdwSize);
wchar_t*			AnsiToUnicode(const char* szStr);
inline char*		UnicodeToAnsi(const wchar_t* szStr);
wchar_t*			UTF8ToWString(const char* lpcszString);
std::string			WStringToUTF8(const wchar_t* lpwcszWString);