// TestingCurrentNodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SSCap.h"
#include "TestingCurrentNodeDlg.h"
#include "afxdialogex.h"
#include "BaseDef.h"
#include "SocketBase.h"
#include "SocksClient.h"
#include "CSSClientTesting.h"
#include "Utils.h"

// CTestingCurrentNodeDlg dialog
IMPLEMENT_DYNAMIC(CTestingCurrentNodeDlg, CDialogEx)

CTestingCurrentNodeDlg::CTestingCurrentNodeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestingCurrentNodeDlg::IDD, pParent)
{
	hTestingThread = NULL;
//	pNodeForTesting = NULL;
	bIsTestingUdp = FALSE;
}

CTestingCurrentNodeDlg::~CTestingCurrentNodeDlg()
{
	ThreadMutex.Lock();
	if( hTestingThread )
	{
		TerminateThread( hTestingThread, 0 );
		CloseHandle( hTestingThread );
		hTestingThread = NULL;
	}
	ThreadMutex.UnLock();

	pNodeForTesting.clear();
}

void CTestingCurrentNodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21_TESTLOG, m_richTestingLog);
}


BEGIN_MESSAGE_MAP(CTestingCurrentNodeDlg, CDialogEx)
	ON_WM_SIZE()
	ON_EN_SETFOCUS(IDC_RICHEDIT21_TESTLOG, &OnEnSetfocusRicheditCurrentProxyLog)
END_MESSAGE_MAP()


// CTestingCurrentNodeDlg message handlers
void CTestingCurrentNodeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect rcClient;

	if( !IsWindow( m_richTestingLog.GetSafeHwnd() ) ) return;

	GetClientRect( &rcClient );

	m_richTestingLog.SetWindowPos(NULL,0,0,rcClient.Width(),rcClient.Height() ,SWP_NOMOVE );
}

BOOL CTestingCurrentNodeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText( lm_u82u16_s(_("Testing SS server...") ) );

	m_richTestingLog.InitializeLogRichedit();
	m_richTestingLog.SetShowTimeInfo( TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTestingCurrentNodeDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}

/** @brief 得到用于显示LOG的richedit的控件指针
*/
CTestingLogRichedit *CTestingCurrentNodeDlg::GetRichEditLogCtrl()
{
	return &m_richTestingLog;
}
/** @brief 用于测试代理的线程 
* 
* @param lpParam CSocksProxyTesting类的针指
*/
DWORD WINAPI CTestingCurrentNodeDlg::Thread_TestProxy( LPVOID lpParam )
{
	CTestingCurrentNodeDlg *pDlg = (CTestingCurrentNodeDlg *)lpParam;
	CTestingLogRichedit *pRichEdit = pDlg->GetRichEditLogCtrl();
	CSSClientTesting testing( pRichEdit );

	list<CSSNodeInfo *> &listNodesInfo = pDlg->GetNodeForTesting();

	if( listNodesInfo.size() <= 0 ) return 1;

	list<CSSNodeInfo *>::iterator iter = listNodesInfo.begin();
	for( iter ; iter != listNodesInfo.end() ; iter ++ )
	{
		testing.TestSSNode( *iter , pDlg->GetIsTestUdp() );
	}

	pRichEdit->AppendTestingLogUTF8( _("Testing was finished."));

	return 1;
}

/** @brief 开始测试当前代理
*/
BOOL CTestingCurrentNodeDlg::StartTestCurrentProxy( BOOL bIsUdp /* = FALSE */)
{
	bIsTestingUdp = bIsUdp;

	ThreadMutex.Lock();
	if( hTestingThread )
	{
		TerminateThread(hTestingThread  , 0 );
		CloseHandle( hTestingThread );
		hTestingThread = NULL;
	}
	ThreadMutex.UnLock();

	m_richTestingLog.ClearText();

	ThreadMutex.Lock();
	hTestingThread = CreateThread( 
		NULL,                   // default security attributes
		0,                      // use default stack size  
		CTestingCurrentNodeDlg::Thread_TestProxy,       // thread function name
		this,			// argument to thread function 
		0,                      // use default creation flags 
		NULL );					// returns the thread identifier 
	ThreadMutex.UnLock();

	if( !hTestingThread )
	{
		return FALSE;
	}

	return TRUE;
}

void CTestingCurrentNodeDlg::OnEnSetfocusRicheditCurrentProxyLog()
{
	//m_richTestingLog.SetSel( -1, -1 );
}
