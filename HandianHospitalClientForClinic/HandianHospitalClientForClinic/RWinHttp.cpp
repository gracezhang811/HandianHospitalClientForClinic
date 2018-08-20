#include "stdafx.h"
#include "RWinHttp.h"


RWinHttp::RWinHttp()
{
}


RWinHttp::~RWinHttp()
{
}


HINTERNET RWinHttp::OpenSession(LPCWSTR userAgent = 0)
{
#ifdef USE_WINHTTP
	return WinHttpOpen(userAgent, NULL, NULL, NULL, NULL);;
#else
	return InternetOpen(userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#endif
}

HINTERNET RWinHttp::Connect(HINTERNET hSession, LPCWSTR serverAddr, int portNo)
{
#ifdef USE_WINHTTP
	return WinHttpConnect(hSession, serverAddr, (INTERNET_PORT)portNo, 0);
#else
	return InternetConnect(hSession, serverAddr, portNo, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
#endif
}

HINTERNET RWinHttp::OpenRequest(HINTERNET hConnect, LPCWSTR verb, LPCWSTR objectName, int scheme)
{
	DWORD flags = 0;
#ifdef USE_WINHTTP
	if (scheme == INTERNET_SCHEME_HTTPS) {
		flags |= WINHTTP_FLAG_SECURE;
	}

	return WinHttpOpenRequest(hConnect, verb, objectName, NULL, NULL, NULL, flags);

#else
	if (scheme == INTERNET_SCHEME_HTTPS) {
		flags |= INTERNET_FLAG_SECURE;
	}

	return HttpOpenRequest(hConnect, verb, objectName, NULL, NULL, NULL, flags, 0);
#endif
}

BOOL RWinHttp::AddRequestHeaders(HINTERNET hRequest, LPCWSTR header)
{
	SIZE_T len = lstrlenW(header);
#ifdef USE_WINHTTP
	return WinHttpAddRequestHeaders(hRequest, header, DWORD(len), WINHTTP_ADDREQ_FLAG_ADD);
#else
	return HttpAddRequestHeaders(hRequest, header, DWORD(len), HTTP_ADDREQ_FLAG_ADD);
#endif
}

BOOL RWinHttp::SendRequest(HINTERNET hRequest, const void* body, DWORD size)
{
#ifdef USE_WINHTTP
	return WinHttpSendRequest(hRequest, 0, 0, const_cast<void*>(body), size, size, 0);
#else
	return HttpSendRequest(hRequest, 0, 0, const_cast<void*>(body), size);
#endif
}

BOOL RWinHttp::EndRequest(HINTERNET hRequest)
{
#ifdef USE_WINHTTP
	return WinHttpReceiveResponse(hRequest, 0);
#else
	// if you use HttpSendRequestEx to send request then use HttpEndRequest in here!
	return TRUE;
#endif
}

BOOL RWinHttp::QueryInfo(HINTERNET hRequest, int queryId, char* szBuf, DWORD* pdwSize)
{
#ifdef USE_WINHTTP
	return WinHttpQueryHeaders(hRequest, (DWORD)queryId, 0, szBuf, pdwSize, 0);
#else
	return HttpQueryInfo(hRequest, queryId, szBuf, pdwSize, 0);
#endif
}

BOOL RWinHttp::ReadData(HINTERNET hRequest, void* buffer, DWORD length, DWORD* cbRead)
{
#ifdef USE_WINHTTP
	return WinHttpReadData(hRequest, buffer, length, cbRead);
#else
	return InternetReadFile(hRequest, buffer, length, cbRead);
#endif
}

BOOL RWinHttp::WriteData(HINTERNET hRequest, void* buffer, DWORD length, DWORD* dwWritten)
{
#ifdef USE_WINHTTP
	return WinHttpWriteData(hRequest, buffer, length, dwWritten);
#else
	return InternetWriteFile(hRequest, buffer, length, dwWritten);
#endif
}

void RWinHttp::CloseInternetHandle(HINTERNET hInternet)
{
	if (hInternet)
	{
#ifdef USE_WINHTTP
		WinHttpCloseHandle(hInternet);
#else
		InternetCloseHandle(hInternet);
#endif
	}
}