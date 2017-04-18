#include "stdafx.h"
#include "sscap.h"
#include "AboutDlg.h"
#include "Utils.h"
#include "APPConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char* __buildtime = __TIME__ " "__DATE__ ;

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VALUE_VERSION, m_staVersion);
	DDX_Control(pDX, IDC_STATIC_VALUE_BUILDTIME, m_staBuildtime);
	DDX_Control(pDX, IDC_STATIC_VALUE_SOCKSCAP64, m_staWebsite1);
	DDX_Control(pDX, IDC_STATIC_VALUE_GOOGLE, m_staGoogleplus);
	DDX_Control(pDX, IDC_STATIC_VALUE_TWITTER, m_staTwitter);
	DDX_Control(pDX, IDC_STATIC_OTHERINFO, m_staOtherInfo);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString strTitle;
	strTitle.Format( lm_u82u16_s( _("About %s")), CAPPConfig::GetSoftName().c_str() );
	SetWindowText( strTitle );

	CString strText;
	GetDlgItem( IDC_STATIC_TIP_VERSION )->SetWindowText( lm_u82u16_s( _("Version: ")) );
	GetDlgItem( IDC_STATIC_TIP_BUILDTIME )->SetWindowText( lm_u82u16_s( _("Build Time: ")) );
	GetDlgItem( IDC_STATIC_TIP_WEBSITE )->SetWindowText( lm_u82u16_s( _("Website: ")) );

	strText.Format( _T("%s"), lm_a2u_s( __buildtime ) );
	m_staBuildtime.SetWindowText( strText );

	m_staVersion.SetWindowText( SSCAP_VERSION );

	if( CAPPConfig::GetWebsite().empty() )
		m_staWebsite1.ShowWindow( SW_HIDE );
	else 
	{
		m_staWebsite1.ShowWindow( SW_SHOW );
		m_staWebsite1.SetWindowText( CAPPConfig::GetWebsite().c_str() );
		m_staWebsite1.SetURL( CAPPConfig::GetWebsite().c_str() );
	}

	if( CAPPConfig::GetGoogleplus().empty() )
		m_staGoogleplus.ShowWindow( SW_HIDE );
	else 
	{
		m_staGoogleplus.ShowWindow( SW_SHOW );
		m_staGoogleplus.SetWindowText( _T("Google+") );
		m_staGoogleplus.SetURL( CAPPConfig::GetGoogleplus().c_str() );
	}
	
	if( CAPPConfig::GetTwitter().empty() )
		m_staTwitter.ShowWindow( SW_HIDE );
	else 
	{
		m_staTwitter.ShowWindow( SW_SHOW );
		m_staTwitter.SetWindowText( _T("Twitter") );
		m_staTwitter.SetURL( CAPPConfig::GetTwitter().c_str() );
	}

	if( CAPPConfig::GetOtherinfo().empty() )
		m_staOtherInfo.ShowWindow( SW_HIDE );
	else 
	{
		m_staOtherInfo.ShowWindow( SW_SHOW );
		m_staOtherInfo.SetWindowText( CAPPConfig::GetOtherinfo().c_str() );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
