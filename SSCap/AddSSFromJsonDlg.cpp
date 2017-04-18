// AddSSFromJsonDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"

#include "SocketBase.h"
#include "SocksClient.h"
#include "SSClient.h"
#include "Listener.h"
#include "SSCapDlg.h"
#include "UIListCtrl.h"

#include "AddSSFromJsonDlg.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "BaseDef.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAddSSFromJsonDlg dialog

IMPLEMENT_DYNAMIC(CAddSSFromJsonDlg, CDialogEx)

CAddSSFromJsonDlg::CAddSSFromJsonDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddSSFromJsonDlg::IDD, pParent)
{

}

CAddSSFromJsonDlg::~CAddSSFromJsonDlg()
{
}

void CAddSSFromJsonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT_JSON, m_editJson);
}


BEGIN_MESSAGE_MAP(CAddSSFromJsonDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddSSFromJsonDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_JSON_EXAMPLE, &CAddSSFromJsonDlg::OnBnClickedButtonJsonExample)
END_MESSAGE_MAP()

int CAddSSFromJsonDlg::ParseArrayContent(Json::Value &value )
{
	if( value.type() != Json::arrayValue )
		return 0;

	int nCount = 0;
	for( int i = 0 ; i < value.size(); i ++ )
	{
		if( value[i].type() != Json::objectValue ) 
			continue;

		ParseObjectContent( value[i] );
		nCount ++;
	}

	return nCount;
}

int CAddSSFromJsonDlg::ParseObjectContent(Json::Value &value )
{
	if( value.type() != Json::objectValue )
		return 0;

	CSSNodeInfo *pNode = new CSSNodeInfo();

	pNode->charge_type = CHARGETYPE_LOCAL;

// 	string server;
	if( value.isMember( "server") )
		pNode->server = value["server"].asString();
// 	u_short server_port;
	if( value.isMember( "server_port") )
		pNode->server_port = value["server_port"].asUInt();

// 	string password;
	if( value.isMember( "password") )
		pNode->password = value["password"].asString();

// 	string method;
	if( value.isMember( "method") )
		pNode->method = value["method"].asString();

// 	string remarks;
	if( value.isMember( "remarks") )
		pNode->remarks = value["remarks"].asString();

// 	bool enable;
	if( value.isMember( "enable") )
		pNode->enable = value["enable"].asBool();

	if( pNode->server.empty() 
		|| pNode->server_port == 0 
		|| pNode->password.empty()
		|| pNode->method.empty() )
	{
		delete pNode;
		return 0;
	}

	CSSConfigInfo *pCfg = GetConfigInfo();
	pCfg->AddNode( pNode );

	CListCtrl *pList = GetSSListContainer();

	UILC_AddItem(pList,pNode );

	return 1;
}

// CAddSSFromJsonDlg message handlers
int CAddSSFromJsonDlg::ParseContent(Json::Value &value )
{
	if( value.type() == Json::arrayValue )
		return ParseArrayContent( value );
	else if( value.type() == Json::objectValue )
		return ParseObjectContent( value );

	return 0;
}

void CAddSSFromJsonDlg::OnBnClickedOk()
{
	CString strEdit;
	m_editJson.GetWindowText( strEdit );
	strEdit.Trim();
	if( strEdit.IsEmpty() )
	{
		AfxMessageBox( lm_u82u16_s( _("Input shadowsocks server with json format please.")));
		return;
	}

	char *pUtf8Str = lm_u162u8( strEdit.GetBuffer() );
	if( !pUtf8Str )
	{
		AfxMessageBox( lm_u82u16_s( _("system error.")));
		return;
	}

	string strContent = string( pUtf8Str );
	delete pUtf8Str;

	Json::Reader reader;
	Json::Value root;

	reader.parse( strContent, root );
// 	if ( !parsingSuccessful )
// 	{
// 		AfxMessageBox( lm_u82u16_s( _("Json parsed error, please input shadowsocks server with right json format.")));
// 		return;
// 	}

	int nParseRet;
	// 格式: "configs" : [{..},{...}]
	// {"method":"xx","password":"xx","server":"xx","server_port":"xx"}
	if(root.type() == Json::objectValue )
	{
		if( root.isMember( "configs" ) )
		{
			Json::Value configs = root.get( string("configs"),string("") );
			if( configs.type() != Json::arrayValue )
			{
				AfxMessageBox( lm_u82u16_s( _("Json parsed error, please input shadowsocks server with right json format.")));
				return;
			}

			nParseRet = ParseContent( configs );
		}
		else if( root.isMember( "method") 
			&& root.isMember("password") 
			&& root.isMember("server")
			&& root.isMember( "server_port") )
		{
			nParseRet = ParseContent( root );
		}
	}
	else if( root.type() == Json::arrayValue )
	{
		nParseRet = ParseContent( root );
	}

	if( nParseRet > 0 )
		SaveShadowsocksServer();

	CDialogEx::OnOK();
}


BOOL CAddSSFromJsonDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText( lm_u82u16_s( _("Add new shadowsocks server from json.") ) );
	GetDlgItem( IDC_STATIC_FROM_JSON_TIP )->SetWindowText( lm_u82u16_s( _("Input json format content below:")));
	GetDlgItem( IDOK )->SetWindowText( lm_u82u16_s( _("OK")));
	GetDlgItem( IDC_BUTTON_JSON_EXAMPLE )->SetWindowText( lm_u82u16_s( _("Example...")));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/** @brief 一个有效的json格式的例子
*
[
	{
		"server" : "192.168.1.100",
		"server_port" : 52027,
		"password" : "123456",
		"method" : "aes-256-cfb",
		"remarks" : "HK",
		"enable" : true
	},
	{
		"server" : "192.168.1.101",
		"server_port" : 52027,
		"password" : "123456",
		"method" : "aes-256-cfb",
		"remarks" : "HK1",
		"enable" : true
	}
]
*/
void CAddSSFromJsonDlg::OnBnClickedButtonJsonExample()
{
	CString strText;
	strText.Format( _T("{\"configs\" :[\r\n	{\r\n		\"server\" : \"192.168.1.100\",\r\n			\"server_port\" : 52027,\r\n			\"password\" : \"123456\",\r\n			\"method\" : \"aes-256-cfb\",\r\n			\"remarks\" : \"HK\",\r\n			\"enable\" : true\r\n	},\r\n	{\r\n		\"server\" : \"192.168.1.101\",\r\n			\"server_port\" : 52027,\r\n			\"password\" : \"123456\",\r\n			\"method\" : \"aes-256-cfb\",\r\n			\"remarks\" : \"HK1\",\r\n			\"enable\" : true\r\n		}\r\n]}"));

	::MessageBox( m_hWnd, strText , lm_u82u16_s( _("The example with right json format")), MB_OK );
}
