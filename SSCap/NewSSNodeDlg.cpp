// NewSSNodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "BaseDef.h"
#include "RunConfig.h"
#include "EncyptionMgr.h"
#include "NewSSNodeDlg.h"

#include "SocketBase.h"
#include "SocksClient.h"
#include "SSClient.h"
#include "Listener.h"
#include "SSCapDlg.h"
#include "UIListCtrl.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CNewSSNodeDlg dialog

IMPLEMENT_DYNAMIC(CNewSSNodeDlg, CDialogEx)

CNewSSNodeDlg::CNewSSNodeDlg(BOOL bEdit,CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewSSNodeDlg::IDD, pParent)
	,bIsEdit( bEdit )
	, m_strServer(_T(""))
	//, m_strPort(_T(""))
	, m_strPassword(_T(""))
	, m_strMem(_T(""))
	, nEditId( -1 )
	//, nIndexBeInsert ( -1 )
	, m_nPort(0)
	, m_bEnable( 1 )
	, m_nCryptionIdx( 0 ) 
{
	//pInsertedNode = NULL;
}

CNewSSNodeDlg::~CNewSSNodeDlg()
{
}

void CNewSSNodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SERVERIP, m_strServer);
	//DDX_Text(pDX, IDC_EDIT_SERVERPORT, m_strPort);
	DDX_Text(pDX, IDC_EDIT_SERVERPASS, m_strPassword);
	DDX_Text(pDX, IDC_EDIT_MEM, m_strMem);
	DDX_Text(pDX, IDC_EDIT_SERVERPORT, m_nPort);
	DDV_MinMaxInt( pDX, m_nPort,0,65535 );
	//DDX_Text(pDX, IDC_CHECK_ENABLE, m_bEnable);

	DDX_Control(pDX, IDC_COMBO_CRYPT_METHOD, m_cmbEnryption);
	DDX_Control(pDX, IDC_CHECK_SHOWPASSWORD, m_btnShowPass);
	DDX_Control(pDX, IDC_EDIT_SERVERPASS, m_editPass);
	DDX_Control(pDX, IDC_CHECK_ENABLE, m_btnEnable);
	DDX_Control(pDX, IDC_EDIT_MEM, m_editMem);
}


BEGIN_MESSAGE_MAP(CNewSSNodeDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_SHOWPASSWORD, &CNewSSNodeDlg::OnBnClickedCheckShowpassword)
	ON_CBN_SELENDOK(IDC_COMBO_CRYPT_METHOD, &CNewSSNodeDlg::OnCbnSelendokComboCryptMethod)
END_MESSAGE_MAP()

// CNewSSNodeDlg message handlers
void CNewSSNodeDlg::OnBnClickedCheckShowpassword()
{
	BOOL bCheck = m_btnShowPass.GetCheck();

	if( bCheck )
	{
		m_editPass.SetPasswordChar( 0 );
		//::SetWindowLong( m_editPass.GetSafeHwnd(),GWL_STYLE,GetWindowLong( m_editPass.GetSafeHwnd(),GWL_STYLE) &~ES_PASSWORD );
	}
	else 
	{
		m_editPass.SetPasswordChar( '*' );

		//::SetWindowLong( m_editPass.GetSafeHwnd(),GWL_STYLE,GetWindowLong( m_editPass.GetSafeHwnd(),GWL_STYLE) | ES_PASSWORD );
	}

	m_editPass.RedrawWindow();
}


void CNewSSNodeDlg::OnCbnSelendokComboCryptMethod()
{
	// TODO: Add your control notification handler code here
}


BOOL CNewSSNodeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_editPass.SetPasswordChar( '*' );

	SetWindowText( lm_u82u16_s( _("Add new shadowsocks server")));
	GetDlgItem( IDC_STATIC_SERVER)->SetWindowText( lm_u82u16_s( _("Server IP:")));
	GetDlgItem( IDC_STATIC_PORT)->SetWindowText( lm_u82u16_s( _("Server port:")));
	GetDlgItem( IDC_STATIC_PASSWORD)->SetWindowText( lm_u82u16_s( _("Password:")));
	GetDlgItem( IDC_STATIC_MEM)->SetWindowText( lm_u82u16_s( _("Remarks:")));
	GetDlgItem( IDC_CHECK_ENABLE)->SetWindowText( lm_u82u16_s( _("Enable this node")));

	vector<string> vecEnc;
	CEncryptionMgr::GetEncyptionList( vecEnc );
	for( int i = 0 ; i < vecEnc.size() ; i ++ )
	{
		m_cmbEnryption.AddString( lm_u82u16_s( vecEnc[i].c_str() ));
	}
	if( bIsEdit )
	{
		m_cmbEnryption.SetCurSel( m_nCryptionIdx );
	}
	else
	{
		m_cmbEnryption.SetCurSel( 0 );
	}

	m_btnEnable.SetCheck( m_bEnable );
	m_editMem.SetLimitText( 40 );

	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CNewSSNodeDlg::OnOK()
{
	if( !UpdateData( TRUE ) ) return;

	if( m_strServer.IsEmpty() )
	{
		AfxMessageBox( lm_u82u16_s( _("Input SS server IP please.") ) );
		return;
	}
	if( m_nPort == 0 )
	{
		AfxMessageBox( lm_u82u16_s( _("Port must be in 0-65535.") ) );
		return;
	}

	/*if( m_strPort.IsEmpty() )
	{
		AfxMessageBox( lm_u82u16_s( _("Input SS server port please.") ) );
		return;
	}*/

	if( m_strPassword.IsEmpty() )
	{
		AfxMessageBox( lm_u82u16_s( _("Input SS server password please.") ) );
		return;
	}

	m_cmbEnryption.GetWindowText( strEnc );

	m_bEnable = m_btnEnable.GetCheck();

	char *pServerUtf8 = lm_u162u8( m_strServer.GetBuffer( ) );
	char *pPassUtf8 = lm_u162u8( m_strPassword.GetBuffer());
	char *pMemUtf8 = lm_u162u8( m_strMem.GetBuffer());
	char *pEncUtf8 = lm_u162u8( strEnc.GetBuffer( ));

	//u_short port = _ttoi( m_strPort.GetBuffer());
	u_short port = m_nPort;

	CSSConfigInfo *pCfg = GetConfigInfo();
	CListCtrl *pList = GetSSListContainer();
	CSSNodeInfo *pNode;

	string server = string(pServerUtf8 );
	string pass = string( pPassUtf8 );
	string enc = string( pEncUtf8 );
	string mem = string( pMemUtf8 );

	if( bIsEdit )
	{
		ASSERT( nEditId != -1 );

		pNode = pCfg->EditNode( nEditId, server,port,pass,enc,mem, m_bEnable?true:false );

		UILC_EditItemById( pList,nEditId, pNode );
		pList->Invalidate( TRUE );
	}
	else 
	{
		pNode = pCfg->AddNode(CHARGETYPE_LOCAL, server, port, pass, enc, mem, m_bEnable?true:false );

		UILC_AddItem( pList, pNode );
		pList->Invalidate( TRUE );
	}

	SaveShadowsocksServer( );

	delete []pServerUtf8;
	delete []pPassUtf8;
	delete []pMemUtf8;
	delete []pEncUtf8;

	CDialogEx::OnOK();
}
