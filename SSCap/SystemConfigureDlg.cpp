// SystemConfigureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"
#include "SystemConfigureDlg.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "BaseDef.h"
#include "Registry.h"
#include "RunConfig.h"
#include "SysProxy.h"

#include "SocketBase.h"
#include "SocksClient.h"
#include "SSClient.h"
#include "Listener.h"

#include "SSManager.h"
#include "SSCapDlg.h"

#include "privoxy.h"
#include "msgdef.h"

#include "SysWideProxy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSystemConfigureDlg dialog

IMPLEMENT_DYNAMIC(CSystemConfigureDlg, CDialogEx)

CSystemConfigureDlg::CSystemConfigureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSystemConfigureDlg::IDD, pParent)
	, m_SocksPort( DEFAULT_SOCKS_PORT )
{

}

CSystemConfigureDlg::~CSystemConfigureDlg()
{
}

void CSystemConfigureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_AUTOSTART, m_btnRunAtStartup);
	DDX_Control(pDX, IDC_CHECK_SHAREOVERLAN, m_btnShareOverLan);
	DDX_Control(pDX, IDC_EDIT_LOCAL_SOCKS5_USER, m_editSocksUser);
	DDX_Control(pDX, IDC_EDIT_LOCAL_SOCKS5_PASS, m_editSocksPass);
	DDX_Control(pDX, IDC_CHECK_ENABLE_BALANCE, m_btnEnableBalance);
	//DDX_Control(pDX, IDC_COMBO_BALANCE_ALOGRITHM, m_cmbAlgorithms);
	DDX_Control(pDX, IDC_COMBO_SYSPROXY_MODE, m_cmbSysProxyType);
	DDX_Control(pDX, IDC_COMBO_PAC_SELECT, m_cmbSysProxyPacType);
	DDX_Control(pDX, IDC_EDIT_ONLINE_PAC_url, m_editOnlinePac);
	DDX_Text(pDX, IDC_EDIT_LOCAL_SOCKS5_PORT, m_SocksPort);
	DDV_MinMaxInt(pDX, m_SocksPort, 1, 65535 );
	DDX_Control(pDX, IDC_CHECK_START_IN_SYSTRAY, m_btnStartInSystray);
	DDX_Control(pDX, IDC_CHECK_ENABLE_SYSPROXY, m_btnEnableSysProxy);
	DDX_Control(pDX, IDC_CHECK_AUTO_DISCONNECT_CONNECTION, m_btnAutoDisconnectConnection);
	DDX_Control(pDX, IDC_EDIT_TESTING_URL, m_editTestingUrl);
	DDX_Control(pDX, IDC_HOTKEY_HOTKEY_FOR_ADDFROMQRCODE, m_hotkeyAddFromQRCode);
}


BEGIN_MESSAGE_MAP(CSystemConfigureDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSystemConfigureDlg::OnBnClickedOk)
	ON_CBN_SELENDOK(IDC_COMBO_SYSPROXY_MODE, &CSystemConfigureDlg::OnCbnSelendokComboSysproxyMode)
	ON_CBN_SELENDOK(IDC_COMBO_PAC_SELECT, &CSystemConfigureDlg::OnCbnSelendokComboPacSelect)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_SYSPROXY, &CSystemConfigureDlg::OnBnClickedCheckEnableSysproxy)
END_MESSAGE_MAP()


// CSystemConfigureDlg message handlers
BOOL CSystemConfigureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CSSConfigInfo *pCfg = GetConfigInfo();

	SetWindowText( lm_u82u16_s( _("Configuration")));
	GetDlgItem( IDC_STATIC_GROUP_BASIC )->SetWindowText( lm_u82u16_s( _("Basic settings")));
	GetDlgItem( IDC_CHECK_AUTOSTART )->SetWindowText( lm_u82u16_s( _("Run at system startup")));
	GetDlgItem( IDC_CHECK_SHAREOVERLAN )->SetWindowText( lm_u82u16_s( _("Allow connections from other computer")));
	GetDlgItem( IDC_CHECK_AUTO_DISCONNECT_CONNECTION )->SetWindowText( lm_u82u16_s( _("Auto-Disconnect connections when you changed node.")));
	GetDlgItem( IDC_STATIC_GROUP_LOCAL_SOCKS5 )->SetWindowText( lm_u82u16_s( _("Local socks 5 settings ( empty means anonymous )")));
	GetDlgItem( IDC_STATIC_LOCAL_SOCKS5_USER )->SetWindowText( lm_u82u16_s( _("Username:")));
	GetDlgItem( IDC_STATIC_LOCAL_SOCKS5_PASS )->SetWindowText( lm_u82u16_s( _("Password:")));
	GetDlgItem( IDC_STATIC_LOCAL_SOCKS5_PORT )->SetWindowText( lm_u82u16_s( _("Socks 5 Port:")));
	GetDlgItem( IDC_STATIC_TEST_URL_TIP )->SetWindowText( lm_u82u16_s( _("Testing url for node availability: ( not support https )")));
	GetDlgItem( IDC_STATIC_HOTKEY_FORADDFROMQRCODE )->SetWindowText( lm_u82u16_s( _("Hot key for add from QR code:")));

	m_btnStartInSystray.SetWindowText( lm_u82u16_s( _("Start minimized in the system tray")) );
	m_btnEnableSysProxy.SetWindowText( lm_u82u16_s( _("Enable system wide proxy")));

	//GetDlgItem( IDC_STATIC_GROUP_BALANCE )->SetWindowText( lm_u82u16_s( _("Server balance settings")));

	GetDlgItem( IDC_CHECK_ENABLE_BALANCE )->SetWindowText( lm_u82u16_s( _("Enable Server balance")));
	//GetDlgItem( IDC_STATIC_BALANCE_ALGORITHMS )->SetWindowText( lm_u82u16_s( _("Algorithms:")));

	GetDlgItem( IDC_STATIC_GROUP_SYSPROXY )->SetWindowText( lm_u82u16_s( _("System proxy(Don't change it if you don't know what's system proxy)")));
	GetDlgItem( IDC_STATIC_SYSPROXY_MODE )->SetWindowText( lm_u82u16_s( _("Type:")));
	GetDlgItem( IDC_STATIC_PAC_SELECT )->SetWindowText( lm_u82u16_s( _("PAC Type:")));
	GetDlgItem( IDC_STATIC_ONLINE_PAC_URL )->SetWindowText( lm_u82u16_s( _("Online PAC Url:")));
	GetDlgItem( IDOK )->SetWindowText( lm_u82u16_s( _("OK")));
	GetDlgItem( IDCANCEL )->SetWindowText( lm_u82u16_s( _("Cancel")));
	
	m_cmbSysProxyType.AddString( lm_u82u16_s( _("Global mode")));
	m_cmbSysProxyType.AddString( lm_u82u16_s( _("PAC mode")));
	m_cmbSysProxyType.SetCurSel( 0 );
	//m_cmbSysProxyType.EnableWindow( pCfg->isPrivoxyRunned );

	m_cmbSysProxyPacType.AddString( lm_u82u16_s( _("Local PAC script")));
	m_cmbSysProxyPacType.AddString( lm_u82u16_s( _("Online PAC script")));
	m_cmbSysProxyPacType.SetCurSel( 0 );
	//m_cmbSysProxyPacType.EnableWindow( pCfg->isPrivoxyRunned );
	
	/*
	m_cmbAlgorithms.AddString( lm_u82u16_s( _("up to down")));
	m_cmbAlgorithms.AddString( lm_u82u16_s( _("random")));
	m_cmbAlgorithms.AddString( lm_u82u16_s( _("low latancy first")));
	m_cmbAlgorithms.AddString( lm_u82u16_s( _("less errors first")));
	m_cmbSysProxyPacType.SetCurSel( 1 ); // random is default
	*/

	WORD wVirtualKeyCode = 0 ;
	WORD wModifiers = 0 ;

	pCfg->GetHotKeyForAddQRCode( wModifiers, wVirtualKeyCode );
	// ctrl + alt 组合的快捷键
	m_hotkeyAddFromQRCode.SetRules(HKCOMB_NONE,HOTKEYF_CONTROL|HOTKEYF_ALT );

	if( wModifiers != 0 && wVirtualKeyCode != 0 )
	{
		m_hotkeyAddFromQRCode.SetHotKey( wVirtualKeyCode, wModifiers );
	}

	m_btnRunAtStartup.SetCheck( pCfg->runAtStartup ? 1: 0);
	m_btnStartInSystray.SetCheck( pCfg->startInSystray ? 1: 0);
	m_btnAutoDisconnectConnection.SetCheck( pCfg->auto_disconnect_connection ? 1: 0);

	m_btnShareOverLan.SetCheck( pCfg->shareOverLan ? 1: 0);
	m_btnEnableBalance.SetCheck( pCfg->random ? 1 : 0 );

	CString strUser, strPass;
	strUser = lm_u82u16_s( pCfg->localSocksUser.c_str() );
	strPass = lm_u82u16_s( pCfg->localSocksPass.c_str() );

	m_editSocksUser.SetWindowText( strUser );
	m_editSocksPass.SetWindowText( strPass );

	CString strTestingUrl;
	strTestingUrl = lm_u82u16_s( pCfg->testing_url.c_str() );
	m_editTestingUrl.SetWindowText( strTestingUrl );

	m_btnEnableSysProxy.SetCheck( pCfg->enable? 1: 0 );
	m_cmbSysProxyType.SetCurSel( pCfg->global ? 0 : 1 );
	m_cmbSysProxyPacType.SetCurSel( pCfg->useOnlinePac ? 1: 0 );

	CString strPac;
	strPac = CString( pCfg->pacUrl.c_str() );

	m_editOnlinePac.SetWindowText( strPac );

	m_SocksPort = pCfg->localPort;

	if( !pCfg->isPrivoxyRunned 
		|| !pCfg->enable )
	{
		m_cmbSysProxyType.EnableWindow( FALSE );
		m_cmbSysProxyPacType.EnableWindow( FALSE );
		m_editOnlinePac.EnableWindow( FALSE );
	}

	if( pCfg->enable )
	{
		m_cmbSysProxyType.EnableWindow( TRUE );

		if( pCfg->global )
		{
			m_cmbSysProxyPacType.EnableWindow( FALSE );
			m_editOnlinePac.EnableWindow( FALSE );
		}
		else 
		{
			m_cmbSysProxyPacType.EnableWindow( TRUE );

			if( pCfg->useOnlinePac )
			{
				m_editOnlinePac.EnableWindow( TRUE );
			}
			else 
				m_editOnlinePac.EnableWindow( FALSE );
		}
	}
	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSystemConfigureDlg::OnBnClickedOk()
{
	if( !UpdateData( TRUE ) )
		return;

	int nRunAtStartup = m_btnRunAtStartup.GetCheck();
	int nShareOverLan = m_btnShareOverLan.GetCheck();
	int nAutoDisconnect = m_btnAutoDisconnectConnection.GetCheck();
	int nBalance = m_btnEnableBalance.GetCheck();
	int nStartInSystray = m_btnStartInSystray.GetCheck();
	int nEnableSysProxy = m_btnEnableSysProxy.GetCheck();

	CString strSocks5User, strSocks5Pass,strTestingUrl;
	m_editSocksUser.GetWindowText( strSocks5User );
	m_editSocksPass.GetWindowText( strSocks5Pass );
	m_editTestingUrl.GetWindowText( strTestingUrl );

	WORD wVirtualKeyCode = 0 ;
	WORD wModifiers = 0 ;
	CString strHotKey;

	m_hotkeyAddFromQRCode.GetHotKey( wVirtualKeyCode, wModifiers );
	strHotKey = m_hotkeyAddFromQRCode.GetHotKeyName();
	
	int nSysProxyType = m_cmbSysProxyType.GetCurSel();
	int nPacType = m_cmbSysProxyPacType.GetCurSel();

	CString strOnlinePacUrl;
	// PAC模式, 并且为 ONLINE PAC
	if( nSysProxyType == 1 && nPacType == 1 )
	{
		m_editOnlinePac.GetWindowText( strOnlinePacUrl );
		strOnlinePacUrl.Trim();

		if( strOnlinePacUrl.IsEmpty() )
		{
			AfxMessageBox( lm_u82u16_s( _("Specify PAC url please.")));
			return;
		}

		strOnlinePacUrl.MakeLower();

		if( strOnlinePacUrl.Left( 7 ) != _T("http://") 
			&& strOnlinePacUrl.Left( 8 ) != _T("https://")  )
		{
			AfxMessageBox( lm_u82u16_s( _("Input a valid PAC url please.")));
			return;
		}
	}

	CSSConfigInfo *pCfg = GetConfigInfo();
	CSSCapDlg *pMainDlg = GetMainDlg();

	WORD wOldVirtualKeyCode = 0 ;
	WORD wOldModifiers = 0 ;
	pCfg->GetHotKeyForAddQRCode( wOldModifiers, wOldVirtualKeyCode );

	if( pMainDlg )
		pMainDlg->RegisterHotKey( strHotKey,wVirtualKeyCode, wModifiers, TRUE );

	pCfg->runAtStartup = nRunAtStartup == 1 ? true: false;
	pCfg->startInSystray = nStartInSystray == 1 ? true: false;
	pCfg->shareOverLan = nShareOverLan == 1 ? true: false;
	pCfg->auto_disconnect_connection = nAutoDisconnect == 1 ? true: false;
	pCfg->random = nBalance == 1 ? true: false;
	pCfg->localSocksUser = string( lm_u162u8_s( strSocks5User.GetBuffer()) );
	pCfg->localSocksPass = string( lm_u162u8_s( strSocks5Pass.GetBuffer()) );
	pCfg->global = nSysProxyType == 0 ? true: false;
	pCfg->useOnlinePac = nPacType == 1 ? true: false;
	pCfg->pacUrl = wstring( strOnlinePacUrl.GetBuffer( ));
	pCfg->enable = nEnableSysProxy == 1 ? true: false;
	pCfg->testing_url = string( lm_u162u8_s( strTestingUrl.GetBuffer()));
	//pCfg->localPort = m_SocksPort;

	//SaveShadowsocksServer();

	CRegistry registry;
	if( nRunAtStartup == 1 )
	{
		//写注册表
		if( registry.Open( _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) ){
			TCHAR szOutBuffer[MAX_PATH];
			_stprintf_s(szOutBuffer,MAX_PATH,_T("\"%s\""),CRunConfig::GetApplicationFullPathName() );
			registry.Write( SSCAP_NAME, szOutBuffer );
			registry.Close();
		}
	}
	else
	{
		if( registry.Open( _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")) ){
			registry.DeleteValue( SSCAP_NAME );
			registry.Close();
		}
	}

	// 本地SOCKS 5端口改变了的情况下需要重启PRIVOXY
	BOOL bPortChanged = FALSE;

	// 修改了端口
	// 如果之前绑定端口失败了,那么这个时候就算用户没有修改新的端口也需要再次偿试绑定一次.
	// 因为有可能之前占用的端口现在已经被释放了.
	if( pMainDlg 
		&& 
		( ( m_SocksPort != pCfg->localPort )
			|| !pMainDlg->IsLocalSocks5ServiceStarted()
		)
		)
	{
		// 修改了端口
		if( pCfg->localPort != m_SocksPort )
		{
			pCfg->localPort = m_SocksPort;
			bPortChanged = TRUE;
			//SaveShadowsocksServer();
		}

		if( !pMainDlg->RestartLocalSocketService( FALSE ) )
			return;
	}

	// 必须要重启privoxy
	BOOL bRunPriv = TRUE;

	// 本地SOCKS 5端口改变, 需要重启privoxy
	if( bPortChanged )
	{
		pCfg->isPrivoxyRunned = RunPrivoxy( );
	}

	EnableSysWideProxy( m_hWnd, pCfg->enable,pCfg->global, TRUE );

	SaveShadowsocksServer();
	CDialogEx::OnOK();
}


void CSystemConfigureDlg::OnCbnSelendokComboSysproxyMode()
{
	int nSel = m_cmbSysProxyType.GetCurSel();
	if( nSel == 0 )
	{
		m_cmbSysProxyPacType.EnableWindow( FALSE );
		m_editOnlinePac.EnableWindow( FALSE );
	}
	else
	{
		m_cmbSysProxyPacType.EnableWindow( TRUE );

		int nPacSel = m_cmbSysProxyPacType.GetCurSel();
		if( nPacSel )
		{
			m_editOnlinePac.EnableWindow( TRUE );
		}
		else 
			m_editOnlinePac.EnableWindow( FALSE );
	}
}


void CSystemConfigureDlg::OnCbnSelendokComboPacSelect()
{
	int nSel = m_cmbSysProxyPacType.GetCurSel();
	if( nSel == 0 )
	{
		m_editOnlinePac.EnableWindow( FALSE );
	}
	else 
	{
		m_editOnlinePac.EnableWindow( TRUE ); 
	}
}


void CSystemConfigureDlg::OnBnClickedCheckEnableSysproxy()
{
	int nCheck = m_btnEnableSysProxy.GetCheck();
	int nSysProxyType = m_cmbSysProxyType.GetCurSel();
	int nPacType = m_cmbSysProxyPacType.GetCurSel();

	if( nCheck )
	{
		m_cmbSysProxyType.EnableWindow( TRUE );

		if( nSysProxyType == 0 )
		{
			m_cmbSysProxyPacType.EnableWindow( FALSE );
			m_editOnlinePac.EnableWindow( FALSE );
		}
		else 
		{
			m_cmbSysProxyPacType.EnableWindow( TRUE );

			if( nPacType == 1 )
			{
				m_editOnlinePac.EnableWindow( TRUE );
			}
			else 
				m_editOnlinePac.EnableWindow( FALSE );
		}
	}
	else 
	{
		m_cmbSysProxyType.EnableWindow( FALSE );
		m_cmbSysProxyPacType.EnableWindow( FALSE );
		m_editOnlinePac.EnableWindow( FALSE );
	}
}
