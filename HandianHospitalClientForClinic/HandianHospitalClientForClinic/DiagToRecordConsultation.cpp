#include "stdafx.h"
#include "DiagToRecordConsultation.h"

CStringA tokendiag("");
CStringA editrecordappointmentnodiag("");
CStringW rtmessage1("");

void CDiagToRecordConsultationWnd::HttpGetRecordUpdate(LPCTSTR appointmentnumber, CStringA token) {
	CStringW curl = USER_UPDATE_RECORD_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get record info update success \n ");
		temp = UTF8ToWString(root["data"]["expert_name"].asCString());
		m_PaintManager.FindControl(_T("LabelExpertNameOnToRecord"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabelStartTimeOnToRecord"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabelEndTimeOnToRecord"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientDescOnToRecord"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "") && (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			path1 += L"-pic1-";
			//int index = temp.ReverseFind('/');
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnToRecord"))->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnToRecord"))->SetBkImage(path1);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnToRecord"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		if ((root["data"]["patient_img2_thumb"] != "") && (root["data"]["patient_img2_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img2_thumb"].asCString());
			CStringW path2 = L"..\\hdtemp\\";
			path2 += appointmentnumber;
			path2 += L"-pic2-";
			//int index = temp.ReverseFind('/');
			path2 += temp.Mid(60);
			if (PathFileExists(path2) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnToRecord"))->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnToRecord"))->SetBkImage(path2);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnToRecord"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		if ((root["data"]["patient_img3_thumb"] != "") && (root["data"]["patient_img3_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img3_thumb"].asCString());
			CStringW path3 = L"..\\hdtemp\\";
			path3 += appointmentnumber;
			//int index = temp.ReverseFind('/');
			path3 += L"-pic3-";
			path3 += temp.Mid(60);
			if (PathFileExists(path3) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnToRecord"))->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnToRecord"))->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnToRecord"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientNameOnToRecord"))->SetText(temp);

		int age = root["data"]["patient_age"].asInt();
		if (age != 0) {
			char aa[20];
			sprintf_s(aa, "%d", age);
			CStringW bb = aa;
			m_PaintManager.FindControl(_T("EditPatientAgeOnToRecord"))->SetText(bb);
		}
		int sex = root["data"]["patient_gender"].asInt();
		if (sex == 1) {
			m_optionChooseMaleOnToRecord->Selected(true);
			m_optionChooseFemaleOnToRecord->Selected(false);
		}
		else {
			m_optionChooseMaleOnToRecord->Selected(false);
			m_optionChooseFemaleOnToRecord->Selected(true);
		}

		if (root["data"]["patient_mobile"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_mobile"].asCString());
			m_PaintManager.FindControl(_T("EditPatientPhoneOnToRecord"))->SetText(temp);
		}

		if (root["data"]["patient_idcard"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_idcard"].asCString());
			m_PaintManager.FindControl(_T("EditPatientIDOnToRecord"))->SetText(temp);

		}

		if (root["data"]["expert_diagnosis"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_diagnosis"].asCString());
			m_PaintManager.FindControl(_T("EditDiagnosisDescOnToRecord"))->SetText(temp);
		}
		if (root["data"]["expert_advise"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_advise"].asCString());
			m_PaintManager.FindControl(_T("LabelExpertAdviseOnToRecord"))->SetText(temp);
		}
	}
	else {
		OutputDebugStringA("get record info update failed \n ");
	}
}


int CDiagToRecordConsultationWnd::HttpUpdateRecord(CStringA poststr, CStringA token) {
	OutputDebugStringA(poststr);

	Json::Value root = HDWinHttpPostWithToken(USER_UPDATE_RECORD_SUBMIT_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("update record success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("update record failed \n ");
		rtmessage1 = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}

LRESULT  CDiagToRecordConsultationWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Close();
	bHandled = FALSE;
	return 0;
}

void CDiagToRecordConsultationWnd::InitAllControls() {
	m_tabPatientInfoOnToRecordControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnToRecord")));
	m_optionChooseMaleOnToRecord = (COptionUI*)m_PaintManager.FindControl(_T("CheckPatientmaleOnToRecord"));
	m_optionChooseFemaleOnToRecord = (COptionUI*)m_PaintManager.FindControl(_T("CheckPatientFemaleOnToRecord"));
	m_optionPatientConditionPageOnToRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnToRecord")));
	m_optionPatientPicPageOnToRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnToRecord")));
	m_richeditDiagnosisDescOnToRecord = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("EditDiagnosisDescOnToRecord")));
	m_btnStartVoiceOnToRecord = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonStartVoiceOnToRecord")));
	m_btnEndVoiceOnToRecord = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonEndVoiceOnToRecord")));

}

void CDiagToRecordConsultationWnd::ResetAllControls() {
	m_tabPatientInfoOnToRecordControlUI = NULL;
	m_optionChooseMaleOnToRecord = NULL;
	m_optionChooseFemaleOnToRecord = NULL;
	m_optionPatientConditionPageOnToRecord = NULL;
	m_optionPatientPicPageOnToRecord = NULL;
	m_richeditDiagnosisDescOnToRecord = NULL;
	m_btnStartVoiceOnToRecord = NULL;
	m_btnEndVoiceOnToRecord = NULL;
}


LRESULT CDiagToRecordConsultationWnd::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ResetAllControls();
	Close();
	bHandled = FALSE;
	return 0;
}

LRESULT CDiagToRecordConsultationWnd::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ResetAllControls();
	Close();
	bHandled = FALSE;
	return 0;
}

void CDiagToRecordConsultationWnd::InitRecordData(LPCTSTR appointmentnumber, CStringA token)
{
	tokendiag = token;
	editrecordappointmentnodiag = appointmentnumber;
	HttpGetRecordUpdate(appointmentnumber, token);
}

void CDiagToRecordConsultationWnd::Init(HWND hWndParent, POINT ptPos)
{
	Create(NULL, _T("诊断记录"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	//Create(hWndParent, _T("RecordConsulltationFrame"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	::ClientToScreen(hWndParent, &ptPos);
	::SetWindowPos(*this, NULL, ptPos.x, ptPos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	m_pVoiceResult = (char*)malloc(4096);//BUFFER_SIZE
	memset(m_pVoiceResult, 0, 4096);
	InitAllControls();
}

void	CDiagToRecordConsultationWnd::Notify(TNotifyUI& msg) {
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("ButtonSubmitRecordOnDiag"))
		{
			CStringW giagnosis = m_richeditDiagnosisDescOnToRecord->GetText();
			CStringW paage = m_PaintManager.FindControl(_T("EditPatientAgeOnToRecord"))->GetText();
			CStringW pamobile = m_PaintManager.FindControl(_T("EditPatientPhoneOnToRecord"))->GetText();
			CStringW paidcard = m_PaintManager.FindControl(_T("EditPatientIDOnToRecord"))->GetText();

			CStringA poststr = "source=clinic";
			poststr += "&appointment_no=";
			poststr += editrecordappointmentnodiag;
			poststr += "&patient_gender=";
			if (m_optionChooseMaleOnToRecord->IsSelected()) {
				poststr += "1";
			}
			else {
				poststr += "2";
			}
			if (paage != _T("")) {
				poststr += "&patient_age=";
				poststr += WStringToUTF8(paage).c_str();
			}
			if (pamobile != _T("")) {
				poststr += "&patient_mobile=";
				poststr += WStringToUTF8(pamobile).c_str();
			}
			if (paidcard != _T("")) {
				poststr += "&patient_idcard=";
				poststr += WStringToUTF8(paidcard).c_str();
			}
			if (giagnosis != _T("")) {
				poststr += "&expert_diagnosis=";
				poststr += WStringToUTF8(giagnosis).c_str();
				if (HttpUpdateRecord(poststr, tokendiag)) {
					Close();
				}
				else {
					::MessageBox(*this, rtmessage1, _T("提示"), 0);
				}				
			}
			else {
				::MessageBox(*this, _T("缺少诊断信息！"), _T("提示"), 0);
			}		
			return;
		}
		if (msg.pSender->GetName() == _T("ButtonCancelSubmitRecordOnDiag"))
		{
			Close();
			return;
		}
		if (msg.pSender->GetName() == _T("ButtonCloseRecordDiag"))
		{
			Close();
			return;
		}
		if (msg.pSender == m_btnStartVoiceOnToRecord)
		{			
			RecognizeVoice(&m_pVoiceResult);
			m_btnStartVoiceOnToRecord->SetTextColor(0xFFe3e4e8);
			m_btnStartVoiceOnToRecord->SetEnabled(false);
			m_btnEndVoiceOnToRecord->SetTextColor(0xFF313131);
			m_btnEndVoiceOnToRecord->SetEnabled(true);
		}
		if (msg.pSender == m_btnEndVoiceOnToRecord)
		{
			CStringW rtmessage("语音识别结束");
			::MessageBox(*this, rtmessage, _T("提示"), 0);
			CStringW temp = m_richeditDiagnosisDescOnToRecord->GetText();
			CStringW temp1 = (LPCTSTR)AnsiToUnicode(m_pVoiceResult);
			temp += temp1;
			m_richeditDiagnosisDescOnToRecord->SetText(temp);
			OutputDebugStringW((LPCWSTR)AnsiToUnicode(m_pVoiceResult));
			memset(m_pVoiceResult, 0, 4096);
			m_btnEndVoiceOnToRecord->SetTextColor(0xFFe3e4e8);
			m_btnEndVoiceOnToRecord->SetEnabled(false);
			m_btnStartVoiceOnToRecord->SetTextColor(0xFF313131);
			m_btnStartVoiceOnToRecord->SetEnabled(true);
		}


	}
	if (msg.sType == _T("selectchanged"))
	{
		if (msg.pSender->GetName() == _T("OptionPatientConditionPageOnToRecord")) {
			m_tabPatientInfoOnToRecordControlUI->SelectItem(0);
			m_optionPatientConditionPageOnToRecord->SetBkColor(0xffffffff);
			m_optionPatientPicPageOnToRecord->SetBkColor(0x00000000);
		}
			
		else if (msg.pSender->GetName() == _T("OptionPatientPicPageOnToRecord")) {
			m_tabPatientInfoOnToRecordControlUI->SelectItem(1);
			m_optionPatientConditionPageOnToRecord->SetBkColor(0x00000000);
			m_optionPatientPicPageOnToRecord->SetBkColor(0xffffffff);
		}
			
	}
}



LRESULT CDiagToRecordConsultationWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_RETURN:
		case VK_ESCAPE://拦截ESC退出界面
			bHandled = true;
			return 0;
		default:
			break;
		}
	}
	return FALSE;
}

LRESULT CDiagToRecordConsultationWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL    bHandled = TRUE;

	switch (uMsg)
	{
	case WM_CLOSE:
		lRes = OnClose(uMsg, wParam, lParam, bHandled);
		break;

	case WM_DESTROY:
		lRes = OnDestroy(uMsg, wParam, lParam, bHandled);
		break;

	case WM_KILLFOCUS:
		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		break;
	default:
		bHandled = FALSE;
	}
	/*
	if (bHandled || m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
	{
		return lRes;
	}
	*/
	return __super::HandleMessage(uMsg, wParam, lParam);
}









