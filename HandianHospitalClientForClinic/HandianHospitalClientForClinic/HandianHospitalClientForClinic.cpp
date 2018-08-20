// HandianHospitalClientForClinic.cpp : 定义应用程序的入口点。
//
#include "stdafx.h"
#include "HandianHospitalClientForClinic.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	::CoInitialize(NULL);

	CDuiFrameWnd *duiFrame = new CDuiFrameWnd;
	duiFrame->Create(NULL, _T("欢迎登录汉典云诊所"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	duiFrame->InitWindow();
	duiFrame->InitSimpleData();
	duiFrame->CenterWindow();
	duiFrame->ShowModal();
	//duiFrame->ShowWindow();

	delete duiFrame;
	::CoUninitialize();

	return 0;
}