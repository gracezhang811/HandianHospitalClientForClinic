#include "stdafx.h"
#include "ListItemButtonUI.h"


CListItemButtonUI::CListItemButtonUI(void)
{
}


CListItemButtonUI::~CListItemButtonUI(void)
{
}

void CListItemButtonUI::DoEvent(TEventUI& event)
{
	if (event.Type == UIEVENT_BUTTONUP) {
		/*
		event.lParam = GetParent()->GetParent()->GetParent()->GetTag();		
		if (GetName() == _T("ButtonCheckAppointDetail")) {
			event.wParam = 1;
		}
		*/
		m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMBUTTONCLICK, event.lParam,
			0);
	}
	CButtonUI::DoEvent(event);
}
