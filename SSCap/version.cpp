#include "stdafx.h"
#include "version.h"
#include <process.h>
#include "GenericHTTPClient.h"
#include "Utils.h"
#include "msgdef.h"
#include "APPConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HANDLE hGetNewVerision = NULL;
static BOOL bGetNewVerionThreadEnd = FALSE;
static string g_version_no;
static string g_versionChange;

// 版本比较,返回TRUE需要更新
BOOL VersionCompare( string localversion, string remoteversion )
{
	vector<string> l;
	vector<string> r;

	if( localversion == remoteversion ) return FALSE;

	SplitStringA( localversion, ".", l, true );
	SplitStringA( remoteversion, ".", r, true );

	if( l.size() != 2 ) return FALSE;
	if( r.size() <= 0 ) return FALSE;

	if( r.size() == 1 ) {
		r.push_back("0");
	}
	int fl = 0;
	int fr = 0;

	int l_n1 = 0 , r_n1 = 0 ;
	int l_n2 = 0 , r_n2 = 0 ;

	// 10位
	l_n1 = atoi( l[0].c_str() );
	r_n1 = atoi( r[0].c_str() );

	// 个位
	l_n2 = atoi( l[1].c_str() );
	r_n2 = atoi( r[1].c_str() );

	// local version
	fl = l_n1 * 10 + l_n2;
	// remote version;
	fr = r_n1 * 10 + r_n2;

	if( fr > fl ) return TRUE;

	return FALSE;
}
static unsigned __stdcall Thread_GetNewVersion( void* pArguments )
{
	char szNewVerionUrl[1024] = {0};
	GenericHTTPClient client;
	CWnd *pParentDlg = (CWnd *)pArguments;
	char *szMainUrl = lm_u2a( (wchar_t*) CAPPConfig::GetNewVersionUrl().c_str() );
	if( !szMainUrl  ) goto End;

	sprintf_s( szNewVerionUrl, 1023, "%s",szMainUrl );

	delete []szMainUrl;
	szMainUrl = NULL;

	if(client.Request(szNewVerionUrl,GenericHTTPClient::RequestGetMethod)){
		if( !client.Is200() ){
			goto End;
		}
		LPCSTR szResult=client.QueryHTTPResponse();

		string strContent = string(szResult);

		if( strContent.empty() )
			goto End;

		vector<string> results;

		SplitStringA( strContent,string("|"),results,false );

		if( results.size() < 2 )
			goto End;

		string strVer = results[0];
		string strBody = results[1];

		// 可能带有\r\n, 如果有就去掉
		//Replace(strContent,"\r","");
		//Replace(strContent,"\n","");

		trim( strVer );

		g_version_no = strVer;
		g_versionChange = strBody;

		char *local_version_no = lm_u2a( SSCAP_VERSION );
		if( !local_version_no )goto End;

		if( VersionCompare( local_version_no, g_version_no ) ) 
		{
			if( pParentDlg )
				::PostMessage(pParentDlg->GetSafeHwnd(),WM_MSG_FOUND_NEWVERSION,0,0);
		}

		delete []local_version_no;
	}else{
#ifdef    _DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			client.GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPTSTR>(&lpMsgBuffer),
			0,
			NULL);
		if( lpMsgBuffer )
		{
			OutputDebugString(reinterpret_cast<LPTSTR>(lpMsgBuffer));
			LocalFree(lpMsgBuffer);
		}
#endif
	}
End:
	bGetNewVerionThreadEnd = TRUE;
	return 0;
}
void StartGetNewVersionThread( void *pWnd )
{
	hGetNewVerision = (HANDLE )_beginthreadex (NULL,0, Thread_GetNewVersion, pWnd, NULL,NULL );
}
void EndGetNewVersionThread()
{
	if( hGetNewVerision )
	{
		if( !bGetNewVerionThreadEnd )  TerminateThread( hGetNewVerision ,0);
		CloseHandle( hGetNewVerision );
		hGetNewVerision = NULL;
	}
}
string NewVersion_GetVersionNo()
{
	return g_version_no;
}
string NewVersion_GetVersionChange()
{
	return g_versionChange;
}