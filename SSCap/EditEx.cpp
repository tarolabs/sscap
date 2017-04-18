// EditEx.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"
#include "EditEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CEditEx

IMPLEMENT_DYNAMIC(CEditEx, CEdit)

CEditEx::CEditEx()
{

}

CEditEx::~CEditEx()
{
}


BEGIN_MESSAGE_MAP(CEditEx, CEdit)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// CEditEx message handlers
BOOL CEditEx::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		BOOL b = GetKeyState(VK_CONTROL) & 0x80;

		if( b 
			&& (pMsg->wParam=='a'
			|| pMsg->wParam=='A')
			)
		{
			SetSel(0,-1);

			return TRUE;
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void CEditEx::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	SetSel(0,-1);

	//CEdit::OnLButtonDblClk(nFlags, point);
}
