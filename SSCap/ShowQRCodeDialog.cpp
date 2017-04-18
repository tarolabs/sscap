// ShowQRCodeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"
#include "ShowQRCodeDialog.h"
#include "afxdialogex.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CShowQRCodeDialog dialog

IMPLEMENT_DYNAMIC(CShowQRCodeDialog, CDialogEx)

CShowQRCodeDialog::CShowQRCodeDialog(HBITMAP hBitmap, CWnd* pParent /*=NULL*/)
	: CDialogEx(CShowQRCodeDialog::IDD, pParent)
{
	m_hQRCodeBithmap = hBitmap;
}

CShowQRCodeDialog::~CShowQRCodeDialog()
{
}

void CShowQRCodeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_STATIC_SHOW_QRCODE, m_staQRCode);
	DDX_Control(pDX, IDC_STATIC_QRCODE, m_staQRCode1);
}


BEGIN_MESSAGE_MAP(CShowQRCodeDialog, CDialogEx)
END_MESSAGE_MAP()


// CShowQRCodeDialog message handlers


BOOL CShowQRCodeDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog(); 

	CRect rcClient;
	
	GetClientRect(&rcClient );
	m_staQRCode1.SetWindowPos( NULL, 0, 0 , rcClient.Width(), rcClient.Height(), SWP_NOMOVE|SWP_SHOWWINDOW );

	m_staQRCode1.SetBitmap( m_hQRCodeBithmap );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
