#pragma once

#include <windows.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>

#define USE_WINHTTP    //Comment this line to user wininet.
#ifdef USE_WINHTTP
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#endif

// CrackedUrl
class CrackedUrl {
	int m_scheme;
	CStringW m_host;
	int m_port;
	CStringW m_path;
	CStringW m_extrainfo;
	CStringW m_restpart;

public:
	CrackedUrl(LPCWSTR url)
	{
		URL_COMPONENTS uc = { 0 };
		uc.dwStructSize = sizeof(uc);

		const DWORD BUF_LEN = 256;

		WCHAR host[BUF_LEN];
		uc.lpszHostName = host;
		uc.dwHostNameLength = BUF_LEN;

		WCHAR path[BUF_LEN];
		uc.lpszUrlPath = path;
		uc.dwUrlPathLength = BUF_LEN;

		WCHAR extra[BUF_LEN];
		uc.lpszExtraInfo = extra;
		uc.dwExtraInfoLength = BUF_LEN;

#ifdef USE_WINHTTP
		if (!WinHttpCrackUrl(url, 0, ICU_ESCAPE, &uc)) {
			printf("Error:WinHttpCrackUrl failed!\n");
		}

#else
		if (!InternetCrackUrl(url, 0, ICU_ESCAPE, &uc)) {
			printf("Error:InternetCrackUrl failed!\n");
		}
#endif
		m_scheme = uc.nScheme;
		m_host = host;
		m_port = uc.nPort;
		m_path = path;
		m_extrainfo = extra;

		TCHAR CURLRest[256]; //L"countries?format=json"
		wsprintf(CURLRest, _T("%s%s"), m_path, m_extrainfo);
		m_restpart = CURLRest;
	}

	int GetScheme() const
	{
		return m_scheme;
	}

	LPCWSTR GetHostName() const
	{
		return m_host;
	}

	int GetPort() const
	{
		return m_port;
	}

	LPCWSTR GetPath() const
	{
		return m_path;
	}

	LPCWSTR GetExtraInfo() const
	{
		return m_extrainfo;
	}

	LPCWSTR GetRestPart() const
	{
		return m_restpart;
	}

	static CStringA UrlEncode(const char* p)
	{
		if (p == 0) {
			return CStringA();
		}

		CStringA buf;

		for (;;) {
			int ch = (BYTE)(*(p++));
			if (ch == '\0') {
				break;
			}

			if (isalnum(ch) || ch == '_' || ch == '-' || ch == '.') {
				buf += (char)ch;
			}
			else if (ch == ' ') {
				buf += '+';
			}
			else {
				char c[16];
				wsprintfA(c, "%%%02X", ch);
				buf += c;
			}
		}

		return buf;
	}
};


class RWinHttp
{
public:
	RWinHttp();
	~RWinHttp();

public:
	HINTERNET OpenSession(LPCWSTR userAgent);
	HINTERNET Connect(HINTERNET hSession, LPCWSTR serverAddr, int portNo);
	HINTERNET OpenRequest(HINTERNET hConnect, LPCWSTR verb, LPCWSTR objectName, int scheme);
	BOOL AddRequestHeaders(HINTERNET hRequest, LPCWSTR header);
	BOOL SendRequest(HINTERNET hRequest, const void* body, DWORD size);
	BOOL EndRequest(HINTERNET hRequest);
	BOOL QueryInfo(HINTERNET hRequest, int queryId, char* szBuf, DWORD* pdwSize);
	BOOL ReadData(HINTERNET hRequest, void* buffer, DWORD length, DWORD* cbRead);
	BOOL WriteData(HINTERNET hRequest, void* buffer, DWORD length, DWORD* dwWritten);
	void CloseInternetHandle(HINTERNET hInternet);

};

