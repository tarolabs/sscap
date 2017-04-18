// PacEditorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"
#include "PacEditorDialog.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "BaseDef.h"
#include "pac.h"
#include "SysProxy.h"
#include "privoxy.h"
#include "SysWideProxy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CPacEditorDialog dialog

IMPLEMENT_DYNAMIC(CPacEditorDialog, CDialogEx)

CPacEditorDialog::CPacEditorDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPacEditorDialog::IDD, pParent)
{

}

CPacEditorDialog::~CPacEditorDialog()
{
}

void CPacEditorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_EDITOR_TIP, m_staEditortip);
	DDX_Control(pDX, IDC_RICHEDIT_PAC_EDITOR, m_richEditor);
}


BEGIN_MESSAGE_MAP(CPacEditorDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CPacEditorDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// CPacEditorDialog message handlers


BOOL CPacEditorDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_staEditortip.SetWindowText( lm_u82u16_s( _("Specify domains or IPs open through proxy. per domain per line.")));
	GetDlgItem( IDOK ) ->SetWindowText( lm_u82u16_s( _("Save") ) );

	m_richEditor.Init();
	m_richEditor.SetEventMask(ENM_MOUSEEVENTS);
	m_richEditor.SetSel(0,0);

	string UserPacFile = GetUserPacFileContent();
	m_richEditor.SetWindowText( lm_u82u16_s( UserPacFile.c_str() ) );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CPacEditorDialog::OnBnClickedOk()
{
#if 0
	CString strPacBody;
	m_richEditor.GetWindowText( strPacBody );
	strPacBody.Trim();

	// 如为空, 则给一个空格写入文件, 否则找度为0无法写入.
	if( strPacBody.IsEmpty() )
	{
		strPacBody = _T(" ");
	}

	char *pU8Body = lm_u162u8( strPacBody.GetBuffer() );
	strPacBody.ReleaseBuffer();

	if( pU8Body )
	{
		FILE *f=NULL;
		CSSConfigInfo *pCfg = GetConfigInfo();
		int length = strlen( pU8Body );

		f = _tfsopen( pCfg->localUserPacFileFullName.c_str(), _T("wb"), _SH_DENYNO );
		if( f )
		{
			fwrite( pU8Body, 1, length, f );

			fflush( f );
			fclose( f );
		}

		delete pU8Body;
	}

	LoadPacFile( TRUE );

	CSSConfigInfo *pCfg = GetConfigInfo();
	EnableSysWideProxy( m_hWnd, pCfg->enable,pCfg->global, TRUE );
	
	// 只有privoxy启动了, 才可以使用系统代理的相关功能.
	// 如果配置启用系统代理.
#if 0
	if( pCfg->isPrivoxyRunned && pCfg->enable )
	{
		// global ?
		if( pCfg->global )
		{
			TCHAR szProxyServer[100] = {0};
#ifdef USE_LIBPRIVOXY
			_stprintf_s( szProxyServer, 100, _T("%s:%d"),LOCAL_LOOPBACK_IP_W, GetPrivoxyListenPort() );
#else
			_stprintf_s( szProxyServer, 100, _T("%s:%d"),LOCAL_LOOPBACK_IP_W,pCfg->localPort);
#endif
			SetSystemProxy( szProxyServer, NULL );
		}
		else 
		{
			SetSystemProxy( NULL, pCfg->GetPacUrl().c_str() );
		}
	}
#endif
#endif 

	CDialogEx::OnOK();
}
