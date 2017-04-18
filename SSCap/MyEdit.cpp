// MyEdit.cpp : implementation file
// Download by http://www.codefans.net

#include "stdafx.h"
#include "MyEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// CMyEdit


CMyEdit::CMyEdit()
{
	m_bMove=TRUE;	
}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	//{{AFX_MSG_MAP(CMyEdit)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyEdit message handlers




int CMyEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	return 0;
}

void CMyEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	
	
	// TODO: Add your message handler code here and/or call default
	//MessageBox("Mouse");
//***********************************************************************************
	
	CRect rect;
	GetWindowRect(&rect);

	int xScreen = GetSystemMetrics(SM_CXSCREEN);
	//int ySCreen = GetSystemMetrics(SM_CYSCREEN);

	if(m_bMove)
	{
		//移动到左上角
		MoveWindow(10,10,rect.Width(),rect.Height());
		m_bMove=FALSE;
	}
	else
	{
		//移动到右上角
		MoveWindow(xScreen-180,10,rect.Width(),rect.Height());
		m_bMove=TRUE;
	}
//**************************************************************************************
	CEdit::OnMouseMove(nFlags, point);
}

void CMyEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
//**********************************************************************************
   //隐藏光标提示符,其实也用不着
	this->HideCaret();
//*********************************************************************************
	// TODO: Add your message handler code here	
}

HBRUSH CMyEdit::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	// TODO: Change any attributes of the DC here
	
	// TODO: Return a non-NULL brush if the parent's handler should not be called
//***************************************************************************************
	//设置文字背景透明
	pDC->SetBkMode(TRANSPARENT);
//**************************************************************************************
	return NULL;
}

BOOL CMyEdit::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
//**************************************************************************************
    
	//取消文字复选
	this->SetSel(0,0);
	
	//设置位图背景
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_BITMAP_CAPTURE_BACK);

	BITMAP bmp;
	bitmap.GetBitmap(&bmp);

	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);

	dcCompatible.SelectObject(&bitmap);

	CRect rect;
	GetClientRect(&rect);

	//pDC->SetBkMode(TRANSPARENT);
	pDC->BitBlt(0,0,rect.Width(),rect.Height(),&dcCompatible,0,0,SRCCOPY);
	return TRUE;
//*********************************************************************************
	//CEdit::OnEraseBkgnd(pDC);
}
