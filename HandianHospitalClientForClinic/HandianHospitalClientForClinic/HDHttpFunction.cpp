#include "stdafx.h"
#include "HDHttpFunction.h"

char szBuf[BUF_SIZE];
char szLog[LOG_SIZE] = { 0 };

int	HDWinHttpDownloadFile(LPCTSTR curl, LPCTSTR path)
{
	URL_INFO url_info = { 0 };
	URL_COMPONENTSW url = { 0 };
	url.dwStructSize = sizeof(url);
	url.lpszExtraInfo = url_info.szExtraInfo;
	url.lpszHostName = url_info.szHostName;
	url.lpszPassword = url_info.szPassword;
	url.lpszScheme = url_info.szScheme;
	url.lpszUrlPath = url_info.szUrlPath;
	url.lpszUserName = url_info.szUserName;

	url.dwExtraInfoLength =
		url.dwHostNameLength =
		url.dwPasswordLength =
		url.dwSchemeLength =
		url.dwUrlPathLength =
		url.dwUserNameLength = 512;

	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	OutputDebugStringW(path);
	OutputDebugStringW(L"\r\n");
	if (!(WinHttpCrackUrl(curl, 0, ICU_ESCAPE, &url))) {
		OutputDebugStringA(" WinHttpCrackUrl error \r\n");
	}

	// 创建一个会话
	HINTERNET hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
	if (hSession == NULL) {
		OutputDebugStringA(" WinHttpOpen error \r\n");
	}
	BYTE *pBuffer = NULL;
	DWORD dwReadBytes, dwSizeDW = sizeof(dwSizeDW), dwContentSize, dwIndex = 0;
	// 创建一个连接
	HINTERNET hConnect = WinHttpConnect(hSession, url.lpszHostName, url.nPort, 0);
	if (hConnect == NULL) {
		OutputDebugStringA(" WinHttpConnect error \r\n");
	}
	// 创建一个请求，先查询内容的大小
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"HEAD", url.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
	if (hRequest == NULL) {
		OutputDebugStringA(" WinHttpOpenRequest error \r\n");
	}
	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		OutputDebugStringA(" WinHttpSendRequest error \r\n");
	}
	if (!WinHttpReceiveResponse(hRequest, 0)) {
		OutputDebugStringA(" WinHttpReceiveResponse error \r\n");
	}
	if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwContentSize, &dwSizeDW, &dwIndex)) {
		OutputDebugStringA(" WinHttpQueryHeaders error \r\n");
	}
	//WinHttpCloseHandle(hRequest);

	// 创建一个请求，获取数据
	hRequest = WinHttpOpenRequest(hConnect, L"GET", url.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		OutputDebugStringA(" WinHttpSendRequest error \n");
		return -1;
	}
	if (!WinHttpReceiveResponse(hRequest, 0)) {
		OutputDebugStringA(" WinHttpReceiveResponse error \n");
		return -1;
	}
	pBuffer = new BYTE[dwContentSize];
	ZeroMemory(pBuffer, dwContentSize);
	if (!WinHttpReadData(hRequest, pBuffer, dwContentSize, &dwReadBytes)) {
		OutputDebugStringA(" WinHttpReadData error \n");
		return -1;
	}              
	
	HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		OutputDebugStringA(" CreateFileW error \n");
		return -1;
	}
	else {
		if (!WriteFile(hFile, pBuffer, dwContentSize, &dwReadBytes, NULL)) {
			OutputDebugStringA(" WriteFile error \n");
			return -1;
		}
		CloseHandle(hFile);
	}
	
	delete pBuffer;

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	return 1;
}

/*
Json::Value HDWinHttpUploadFile(LPCTSTR curl, CStringA token, CStringW path)
{
	Json::Value errormesg;
	errormesg["code"] = -1;
	errormesg["message"] = WStringToUTF8(L"网络错误。");

	HINTERNET hSession = 0;
	HINTERNET hConnect = 0;
	HINTERNET hRequest = 0;
	CStringW strHeaderToken(L"Authorization: Bearer ");
	strHeaderToken += token;

	RWinHttp* pWinHttp = new RWinHttp();

	// Test data
	CrackedUrl crackedUrl(curl);

	// Open session.
	hSession = pWinHttp->OpenSession(L"Duilib WebBrowser");
	if (hSession == NULL) {
		printf("Error:Open session!\n");
		sprintf_s(szLog, "Error:Open session");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Connect.
	hConnect = pWinHttp->Connect(hSession, crackedUrl.GetHostName(), crackedUrl.GetPort());
	if (hConnect == NULL) {
		sprintf_s(szLog, "Error:Connect failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Open request.
	//hRequest = pWinHttp->OpenRequest(hConnect, L"POST", crackedUrl.GetPath(), crackedUrl.GetScheme());
	hRequest = pWinHttp->OpenRequest(hConnect, L"POST", L"v1/upload/img?type=clinic", crackedUrl.GetScheme());
	if (hRequest == NULL) {
		sprintf_s(szLog, "Error:OpenRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}
	//OutputDebugStringW(crackedUrl.GetPath());
	//OutputDebugStringW(L"\r\n");
	CStringW m_wstrNewHeader = L"Content-Type: multipart/form-data; boundary=";
	m_wstrNewHeader += BOUNDARYPART;
	m_wstrNewHeader += L"\r\n";

	// Add request header.
	if (!pWinHttp->AddRequestHeaders(hRequest, m_wstrNewHeader)) {
		sprintf_s(szLog, "Error:AddRequestHeaders failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Add request header.
	if (!pWinHttp->AddRequestHeaders(hRequest, strHeaderToken)) {
		sprintf_s(szLog, "Error:AddRequestHeaders token failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	CStringW m_wstrBlockStart = L"--";
	m_wstrBlockStart += BOUNDARYPART;
	m_wstrBlockStart += L"\r\n";

	CStringW m_wstrBlockEnd = L"--";
	m_wstrBlockEnd += L"\r\n";
	m_wstrBlockEnd += BOUNDARYPART;
	m_wstrBlockEnd += L"\r\n";

	CStringW wstrUploadFileHeader;
	wstrUploadFileHeader += m_wstrBlockStart;
	//wstrUploadFileHeader += L"Content-Disposition: form-data; name=\"Filedata\"; filename=\"E:\\workspace\\HandianHospitalClientForClinic\\skin\\defaulthead.jpg\"\r\n";
	//wstrUploadFileHeader += L"Content-Type:application/octet-stream\r\n\r\n";
	//wstrUploadFileHeader += L"\r\n";
	wstrUploadFileHeader += L"Content-Disposition: form-data; name=\"";
	wstrUploadFileHeader += L"FileData";
	wstrUploadFileHeader += L"\";";
	wstrUploadFileHeader += L" filename=\"";
	wstrUploadFileHeader += path;
	wstrUploadFileHeader += L"\"\r\n";
	wstrUploadFileHeader += L"Content-Type:application/octet-stream\r\n\r\n";
	wstrUploadFileHeader += m_wstrBlockEnd;

	// Send post data.可以直接写入
	
	if (!pWinHttp->SendRequest(hRequest, wstrUploadFileHeader, wstrUploadFileHeader.GetLength())) {
		sprintf_s(szLog, "Error:SendRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}
	
	

	// End request
	if (!pWinHttp->EndRequest(hRequest)) {
		sprintf_s(szLog, "Error:EndRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	//char szBuf[BUF_SIZE];
	DWORD dwSize = 0;
	szBuf[0] = 0;

	// read data.
	for (;;) {
		dwSize = BUF_SIZE;
		if (pWinHttp->ReadData(hRequest, szBuf, dwSize, &dwSize) == FALSE) {
			OutputDebugStringA("ReadData false.\n");
			break;
		}

		if (dwSize <= 0) {
			OutputDebugStringA("dwSize = 0.\n");
			break;
		}

		szBuf[dwSize] = 0;
		OutputDebugStringW((LPCWSTR)UTF8ToWString(szBuf));
	}

	pWinHttp->CloseInternetHandle(hRequest);
	pWinHttp->CloseInternetHandle(hConnect);
	pWinHttp->CloseInternetHandle(hSession);

	delete pWinHttp;

	Json::Reader reader;
	Json::Value root;
	reader.parse(szBuf, root, false);
	return root;
}
*/

/*
char* HeaderInfoTrans(char* szBuf, DWORD* pdwSize)
{
	char sStr[HEADINFO_SIZE];
	int len = *pdwSize;
	int j = 0;
	for (int i = 0; i<len; ++i)
	{
		if (szBuf[i] != '\0')
		{
			sStr[j] = szBuf[i];
			j++;
		}
	}
	sStr[j] = 0;

	return sStr;
}
*/

wchar_t* AnsiToUnicode(const char* szStr)
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
inline char* UnicodeToAnsi(const wchar_t* szStr)
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

wchar_t* UTF8ToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode;
	pUnicode = new wchar_t[unicodeLen + 1];
	memset((void*)pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	return pUnicode;
}



std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}


Json::Value HDWinHttpGet(LPCTSTR curl) {
	//char szLog[LOG_SIZE] = { 0 };
	Json::Value errormesg;
	errormesg["code"] = -1;
	errormesg["message"] = WStringToUTF8(L"网络错误。");
	Json::Reader reader;
	Json::Value root;

	HINTERNET hSession = 0;
	HINTERNET hConnect = 0;
	HINTERNET hRequest = 0;
	CStringW strHeader(L"Content-type: application/x-www-form-urlencoded\r\n");

	RWinHttp* pWinHttp = new RWinHttp();

	// Test data
	CrackedUrl crackedUrl(curl);

	// Open session.
	hSession = pWinHttp->OpenSession(L"Duilib WebBrowser");
	if (hSession == NULL) {
		sprintf_s(szLog, "Error:Open session.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	// Connect.
	hConnect = pWinHttp->Connect(hSession, crackedUrl.GetHostName(), crackedUrl.GetPort());
	if (hConnect == NULL) {
		sprintf_s(szLog, "Error:Connect failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	// Open request.
	hRequest = pWinHttp->OpenRequest(hConnect, L"GET", crackedUrl.GetRestPart(), crackedUrl.GetScheme());
	if (hRequest == NULL) {
		sprintf_s(szLog, "Error:OpenRequest failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	// Add request header.
	if (!pWinHttp->AddRequestHeaders(hRequest, strHeader)) {
		sprintf_s(szLog, "Error:AddRequestHeaders failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	// Send data.
	if (!pWinHttp->SendRequest(hRequest, NULL, NULL)) {
		sprintf_s(szLog, "Error:SendRequest failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	// End request
	if (!pWinHttp->EndRequest(hRequest)) {
		sprintf_s(szLog, "Error:EndRequest failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	DWORD dwSize = 0;
	szBuf[0] = 0;

	// Read data.
	for (;;) {
		dwSize = BUF_SIZE;
		if (pWinHttp->ReadData(hRequest, szBuf, dwSize, &dwSize) == FALSE) {
			break;
		}

		if (dwSize <= 0) {
			break;
		}

		szBuf[dwSize] = 0;
		OutputDebugStringW((LPCWSTR)UTF8ToWString(szBuf));
	}

	pWinHttp->CloseInternetHandle(hRequest);
	pWinHttp->CloseInternetHandle(hConnect);
	pWinHttp->CloseInternetHandle(hSession);

	delete pWinHttp;

	reader.parse(szBuf, root, false);
	return root;
}


Json::Value HDWinHttpPost(LPCTSTR curl, CStringA postdata)
{
	Json::Value errormesg;
	errormesg["code"] = -1;
	errormesg["message"] = WStringToUTF8(L"网络错误。");
	Json::Reader reader;
	Json::Value root;
	HINTERNET hSession = 0;
	HINTERNET hConnect = 0;
	HINTERNET hRequest = 0;
	CStringW strHeader(L"Content-type: application/x-www-form-urlencoded\r\n");

	RWinHttp* pWinHttp = new RWinHttp();

	// Test data
	CrackedUrl crackedUrl(curl);
	//CStringA strPostData("value1=10&value2=14");

	// Open session.
	hSession = pWinHttp->OpenSession(L"Duilib WebBrowser");
	if (hSession == NULL) {
		printf("Error:Open session!\n");
		sprintf_s(szLog, "Error:Open session");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Connect.
	hConnect = pWinHttp->Connect(hSession, crackedUrl.GetHostName(), crackedUrl.GetPort());
	if (hConnect == NULL) {
		sprintf_s(szLog, "Error:Connect failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Open request.
	hRequest = pWinHttp->OpenRequest(hConnect, L"POST", crackedUrl.GetPath(), crackedUrl.GetScheme());
	if (hRequest == NULL) {
		sprintf_s(szLog, "Error:OpenRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Add request header.
	if (!pWinHttp->AddRequestHeaders(hRequest, strHeader)) {
		sprintf_s(szLog, "Error:AddRequestHeaders failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Send post data.
	if (!pWinHttp->SendRequest(hRequest, (const char*)postdata, postdata.GetLength())) {
		sprintf_s(szLog, "Error:SendRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// End request
	if (!pWinHttp->EndRequest(hRequest)) {
		sprintf_s(szLog, "Error:EndRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	//char szBuf[BUF_SIZE];
	DWORD dwSize = 0;
	szBuf[0] = 0;

	// read data.
	for (;;) {
		dwSize = BUF_SIZE;
		if (pWinHttp->ReadData(hRequest, szBuf, dwSize, &dwSize) == FALSE) {
			OutputDebugStringA("ReadData false.\n");
			break;
		}

		if (dwSize <= 0) {
			OutputDebugStringA("dwSize = 0.\n");
			break;
		}

		szBuf[dwSize] = 0;
		OutputDebugStringW(AnsiToUnicode(szBuf));
	}

	pWinHttp->CloseInternetHandle(hRequest);
	pWinHttp->CloseInternetHandle(hConnect);
	pWinHttp->CloseInternetHandle(hSession);

	delete pWinHttp;
	reader.parse(szBuf, root, false);
	return root;
}


Json::Value HDWinHttpGetWithToken(LPCTSTR curl, CStringA token) {
	//char szLog[LOG_SIZE] = { 0 };
	Json::Value errormesg;
	errormesg["code"] = -1;
	errormesg["message"] = WStringToUTF8(L"网络错误。");
	Json::Reader reader;
	Json::Value root;

	HINTERNET hSession = 0;
	HINTERNET hConnect = 0;
	HINTERNET hRequest = 0;
	CStringW strHeader(L"Content-type: application/x-www-form-urlencoded\r\n");
	CStringW strHeaderToken(L"Authorization: Bearer ");
	strHeaderToken += token;

	RWinHttp* pWinHttp = new RWinHttp();

	// Test data
	CrackedUrl crackedUrl(curl);

	// Open session.
	hSession = pWinHttp->OpenSession(L"Duilib WebBrowser");
	if (hSession == NULL) {
		sprintf_s(szLog, "Error:Open session.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
//		delete szBuf;
		return errormesg;
	}

	// Connect.
	hConnect = pWinHttp->Connect(hSession, crackedUrl.GetHostName(), crackedUrl.GetPort());
	if (hConnect == NULL) {
		sprintf_s(szLog, "Error:Connect failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	// Open request.
	hRequest = pWinHttp->OpenRequest(hConnect, L"GET", crackedUrl.GetRestPart(), crackedUrl.GetScheme());
	if (hRequest == NULL) {
		sprintf_s(szLog, "Error:OpenRequest failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	// Add request header.
	if (!pWinHttp->AddRequestHeaders(hRequest, strHeader)) {
		sprintf_s(szLog, "Error:AddRequestHeaders failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}


	// Add request header.
	if (!pWinHttp->AddRequestHeaders(hRequest, strHeaderToken)) {
		sprintf_s(szLog, "Error:AddRequestHeaders token failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Send data.
	if (!pWinHttp->SendRequest(hRequest, NULL, NULL)) {
		sprintf_s(szLog, "Error:SendRequest failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	// End request
	if (!pWinHttp->EndRequest(hRequest)) {
		sprintf_s(szLog, "Error:EndRequest failed.\r\n");
		OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szLog));
		return errormesg;
	}

	DWORD dwSize = 0;
	szBuf[0] = 0;

	// read data.
	for (;;) {
		dwSize = BUF_SIZE;
		if (pWinHttp->ReadData(hRequest, szBuf, dwSize, &dwSize) == FALSE) {
			OutputDebugStringA("ReadData false.\n");
			break;
		}

		if (dwSize <= 0) {
			OutputDebugStringA("dwSize = 0.\n");
			break;
		}

		szBuf[dwSize] = 0;
		OutputDebugStringW((LPCWSTR)UTF8ToWString(szBuf));
		//OutputDebugStringA((LPCSTR)(LPTSTR)AnsiToUnicode(szBuf));
	}

	pWinHttp->CloseInternetHandle(hRequest);
	pWinHttp->CloseInternetHandle(hConnect);
	pWinHttp->CloseInternetHandle(hSession);

	delete pWinHttp;

	reader.parse(szBuf, root, false);
	return root;
}


Json::Value HDWinHttpPostWithToken(LPCTSTR curl, CStringA postdata, CStringA token)
{
	Json::Value errormesg;
	errormesg["code"] = -1;
	errormesg["message"] = WStringToUTF8(L"网络错误。");
	Json::Reader reader;
	Json::Value root;

	HINTERNET hSession = 0;
	HINTERNET hConnect = 0;
	HINTERNET hRequest = 0;
	CStringW strHeader(L"Content-type: application/x-www-form-urlencoded\r\n");
	CStringW strHeaderToken(L"Authorization: Bearer ");
	strHeaderToken += token;

	RWinHttp* pWinHttp = new RWinHttp();

	// Test data
	CrackedUrl crackedUrl(curl);

	// Open session.
	hSession = pWinHttp->OpenSession(L"Duilib WebBrowser");
	if (hSession == NULL) {
		printf("Error:Open session!\n");
		sprintf_s(szLog, "Error:Open session");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Connect.
	hConnect = pWinHttp->Connect(hSession, crackedUrl.GetHostName(), crackedUrl.GetPort());
	if (hConnect == NULL) {
		sprintf_s(szLog, "Error:Connect failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Open request.
	hRequest = pWinHttp->OpenRequest(hConnect, L"POST", crackedUrl.GetPath(), crackedUrl.GetScheme());
	if (hRequest == NULL) {
		sprintf_s(szLog, "Error:OpenRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Add request header.
	if (!pWinHttp->AddRequestHeaders(hRequest, strHeader)) {
		sprintf_s(szLog, "Error:AddRequestHeaders failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Add request header.
	if (!pWinHttp->AddRequestHeaders(hRequest, strHeaderToken)) {
		sprintf_s(szLog, "Error:AddRequestHeaders token failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// Send post data.
	if (!pWinHttp->SendRequest(hRequest, (const char*)postdata, postdata.GetLength())) {
		sprintf_s(szLog, "Error:SendRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	// End request
	if (!pWinHttp->EndRequest(hRequest)) {
		sprintf_s(szLog, "Error:EndRequest failed");
		OutputDebugStringA(szLog);
		return errormesg;
	}

	//char szBuf[BUF_SIZE];
	DWORD dwSize = 0;
	szBuf[0] = 0;

	// read data.
	for (;;) {
		dwSize = BUF_SIZE;
		if (pWinHttp->ReadData(hRequest, szBuf, dwSize, &dwSize) == FALSE) {
			OutputDebugStringA("ReadData false.\n");
			break;
		}

		if (dwSize <= 0) {
			OutputDebugStringA("dwSize = 0.\n");
			break;
		}

		szBuf[dwSize] = 0;
		OutputDebugStringW((LPCWSTR)UTF8ToWString(szBuf));
	}

	pWinHttp->CloseInternetHandle(hRequest);
	pWinHttp->CloseInternetHandle(hConnect);
	pWinHttp->CloseInternetHandle(hSession);

	delete pWinHttp;

	reader.parse(szBuf, root, false);
	return root;
}
