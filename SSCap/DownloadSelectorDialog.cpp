// DownloadSelectorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"
#include "DownloadSelectorDialog.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "APPConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CDownloadSelectorDialog dialog

IMPLEMENT_DYNAMIC(CDownloadSelectorDialog, CDialogEx)

CDownloadSelectorDialog::CDownloadSelectorDialog(string strVersion, string strChange, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDownloadSelectorDialog::IDD, pParent)
	, m_strVersion( strVersion )
	, m_strChangelog( strChange )
{
	nDownloadSource = 0;
}

CDownloadSelectorDialog::~CDownloadSelectorDialog()
{
}

void CDownloadSelectorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_CHANGELOG, m_richChangelog);
	DDX_Control(pDX, IDCANCEL, m_btnMirror1);
}


BEGIN_MESSAGE_MAP(CDownloadSelectorDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDownloadSelectorDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// CDownloadSelectorDialog message handlers


BOOL CDownloadSelectorDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	char szTip[1024] = {0};
	sprintf_s( szTip, 1024, _("New version v%s was released.\r\nUpdate to new version now?"), m_strVersion.c_str() );
	CString strTip1 = lm_u82u16_s( szTip );

	char szTitle[1024] = {0};
	sprintf_s( szTitle, 1024, _("New Version %s was released"), m_strVersion.c_str() );

	SetWindowText(lm_u82u16_s( szTitle ));
	GetDlgItem( IDC_STATIC_DOWNLOAD_SELECTOR_TIP )->SetWindowText( lm_u82u16_s( _("Select a download mirror")));
	GetDlgItem( IDC_STATIC_DOWNLOAD_SELECTOR_NOTICE )->SetWindowText( strTip1 );
	GetDlgItem( IDC_STATIC_CHANGELOG_TIPS )->SetWindowText( lm_u82u16_s( _("What's new?")));

	m_richChangelog.SetSel(0, -1 );
	m_richChangelog.ReplaceSel( lm_u82u16_s( m_strChangelog.c_str() ) );

	CheckRadioButton( IDC_RADIO_GITHUB, IDC_RADIO_SOURCEFORGE, IDC_RADIO_GITHUB);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDownloadSelectorDialog::OnBnClickedOk()
{
	int nDownloadSource = GetCheckedRadioButton(IDC_RADIO_BAIDUPAN,IDC_RADIO_SOURCEFORGE);
	if( nDownloadSource == IDC_RADIO_SOURCEFORGE )
		ShellExecute(NULL,_T("open"),SSCAP_DEFAULT_WEBSITE_SOURCEFORGE ,NULL,NULL,SW_SHOW );
	else if( nDownloadSource == IDC_RADIO_BAIDUPAN )
		ShellExecute(NULL,_T("open"),SSCAP_DEFAULT_WEBSITE_BAIDUPAN ,NULL,NULL,SW_SHOW );
	else if( nDownloadSource == IDC_RADIO_GITHUB )
		ShellExecute(NULL,_T("open"),SSCAP_DEFAULT_WEBSITE_GITHUB_RELEASE ,NULL,NULL,SW_SHOW );

	CDialogEx::OnOK();
}
