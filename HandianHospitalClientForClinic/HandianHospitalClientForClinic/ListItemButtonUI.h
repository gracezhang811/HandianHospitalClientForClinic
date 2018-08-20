#pragma once
#include "duilib.h"
#include "Resource.h"


class CListItemButtonUI :
	public CButtonUI
{
public:
	CListItemButtonUI(void);
	~CListItemButtonUI(void);
	virtual void DoEvent(TEventUI& event);
private:
};

