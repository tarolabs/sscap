
// SSCap.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "include/sodium.h"

#include "RunConfig.h"

#include "Debug.h"
#include "Encypter.h"
#include "EncyptionMgr.h"

#include <direct.h>

#include "SocketBase.h"
#include "SocksClient.h"
#include "SSClient.h"
#include "Listener.h"

#include "SSCap.h"
#include "SSCapDlg.h"
#include "SysProxy.h"
#include "privoxy.h"
#include "Utils.h"

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "cryptopp562/cryptlib.h"
#include "cryptopp562/md5.h"

#include "APPConfig.h"

using namespace debug;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSSCapApp

BEGIN_MESSAGE_MAP(CSSCapApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CSSCapApp construction
CSSCapApp::CSSCapApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CSSCapApp object

CSSCapApp theApp;


// CSSCapApp initialization

BOOL CSSCapApp::InitInstance()
{
	CRunConfig::InitializeAppPath();
	InitializeWorkingDirectory( CRunConfig::GetAppWorkingDirectory() );

	PrintfW(LEVEL_INFO, _T("SSCap Started at: %s"), CRunConfig::GetAppWorkingDirectory() );

	if( !DetectSingleInstance() )
	{
		CString strMsgBody;
		strMsgBody.Format( lm_u82u16_s( _("%s is already running. If you want to start multiple instance, copy %s to another directory and run it again.")), CAPPConfig::GetSoftName().c_str(),CAPPConfig::GetSoftName().c_str() );
		::MessageBox(NULL,strMsgBody,CAPPConfig::GetSoftName().c_str(),MB_OK|MB_ICONERROR );
		return FALSE;
	}

	if (sodium_init() == -1) {
		PrintfW(LEVEL_WARNING, _T("Sodium_init failed.") );
		CEncryptionMgr::DisableChaCha20();
	}

	AfxInitRichEdit2();

	_mkdir( "config" );
	_mkdir( "log" );
	_mkdir( "temp" );

	// ≥ı ºªØ∂‡”Ô—‘
	Init_Locale( CRunConfig::GetAppWorkingDirectory() );

	// set output log.
	TCHAR szLogFile[MAX_PATH] = {0};
	_stprintf_s( szLogFile, MAX_PATH,_T("%s\\log\\sscap"),CRunConfig::GetAppWorkingDirectory() );
	CDebug::SetOutputType( OUTPUT_TYPE_FILE );
	CDebug::SetDebugLevel( LEVEL_KERNEL );
	CDebug::SetLogFile( szLogFile );

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	//MagickCore::MagickCoreGenesis( NULL, MagickFalse );

	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CSSCapDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	return FALSE;
}

BOOL CSSCapApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}

BOOL CSSCapApp::DetectSingleInstance()
{
	char *pFileName = lm_u2a( (wchar_t *)CRunConfig::GetApplicationFullPathName() );
	if( !pFileName )
		return TRUE;

	unsigned char r[32] = {0};
	CryptoPP::Weak::MD5 hash;
	hash.CalculateDigest( (byte*)r, (byte*) pFileName, strlen( pFileName ) );

	char md5[33]  = {0};

	_snprintf (md5,32,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7],
		r[8],r[9],r[10],r[11],r[12],r[13],r[14],r[15]);

	wchar_t *pWmd5 = lm_a2u( md5 );

	if( !pWmd5 )
	{
		delete pFileName;
		return TRUE;
	}

	wchar_t szMutexName[MAX_PATH] ={0};
	_stprintf_s( szMutexName, MAX_PATH, _T("Global\\{_SSCAP-%s}"), pWmd5 );
	
	singleInstance.Create( szMutexName );

	delete pFileName;
	delete pWmd5;

	return !singleInstance.IsAnotherInstanceRunning();
}