// AddSSFromLink.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"

#include "SocketBase.h"
#include "SocksClient.h"
#include "SSClient.h"
#include "Listener.h"
#include "SSCapDlg.h"

#include "AddSSFromLink.h"
#include "afxdialogex.h"
#include "BaseDef.h"
#include "Utils.h"
#include "UIListCtrl.h"
#include <string>
#include <vector>
using namespace std;

// CAddSSFromLink dialog
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CAddSSFromLink, CDialogEx)

CAddSSFromLink::CAddSSFromLink(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddSSFromLink::IDD, pParent)
{
	m_nAddedNodes = 0;
}

CAddSSFromLink::~CAddSSFromLink()
{
}

void CAddSSFromLink::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, m_editLinks);
}


BEGIN_MESSAGE_MAP(CAddSSFromLink, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddSSFromLink::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddSSFromLink message handlers


void CAddSSFromLink::OnBnClickedOk()
{
	CString strLinks;
	m_editLinks.GetWindowText( strLinks );

	strLinks.Trim();

	if( strLinks .IsEmpty() )
	{
		AfxMessageBox( lm_u82u16_s( _("Input shadowsocks server link please.")));
		return;
	}
	strLinks.Replace( _T("\r\n"), _T("\n"));

	wstring sLinks = wstring( strLinks.GetBuffer( ) );
	vector<wstring> results;

	SplitStringW( sLinks,wstring(_T("\n")), results , false );

	CSSConfigInfo *pCfg = GetConfigInfo();

	for( int i = 0 ; i < results.size(); i ++ )
	{
		char *pAnsiLink = lm_u2a_s( (wchar_t*)results[i].c_str() );
		string link = string( pAnsiLink );

		CSSNodeInfo *pNode = pCfg->AddNodeFromLink( link );
		if( pNode != NULL )
		{
			CListCtrl *pList = GetSSListContainer();
			UILC_AddItem(pList,pNode );

			m_nAddedNodes ++;
		}
	}

	CDialogEx::OnOK();
}


BOOL CAddSSFromLink::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText( lm_u82u16_s( _("Add new shadowsocks server from ss link.") ) );
	GetDlgItem( IDC_STATIC_ADD_SS_FROM_LINK_TIP )->SetWindowText( lm_u82u16_s( _("Input some shadowsocks server link below( per ss link per line )")));
	GetDlgItem( IDOK )->SetWindowText( lm_u82u16_s( _("OK") ) );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
