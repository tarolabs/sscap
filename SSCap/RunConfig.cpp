#include "stdafx.h"
#include "RunConfig.h"
#include "Utils.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TCHAR CRunConfig::szApplicationFileName[ 1024 ] ={0};	///< 当前EXE文件的完整路径文件名.
TCHAR CRunConfig::szProgramWorkingDirectory[ 1024 ] ={0};			///< SocksCap64的主工作目录

BOOL CRunConfig::InitializeAppPath()
{
	GetModuleFileName(NULL,szProgramWorkingDirectory, 500 );  

	_tcscpy_s(szApplicationFileName ,1024 ,szProgramWorkingDirectory );

	TCHAR *p = _tcsrchr( szProgramWorkingDirectory , L'\\' );

	if( p )	*p=0x00;

	return TRUE;
}

LPCTSTR CRunConfig::GetAppWorkingDirectory()
{
	return szProgramWorkingDirectory;
}

LPCTSTR CRunConfig::GetApplicationFullPathName()
{
	return szApplicationFileName;
}

void Init_Locale( const TCHAR *szWorkingDir )
{
	if( !szWorkingDir ) return;

	char szLocale[MAX_PATH] = {0};
	sprintf_s(szLocale,MAX_PATH,"%s\\lang",lm_u2a_s( (TCHAR *)szWorkingDir ) );

	//setlocale (LC_ALL, "");
	//char *szlocale = setlocale (LC_ALL, "en_US.UTF-8");
	//char *szlocale =setlocale (LC_ALL, "en-US");
	textdomain ( "sscap" );
	bindtextdomain ("sscap", szLocale );
	bind_textdomain_codeset("sscap","utf-8");

	SetThreadLocale (LOCALE_USER_DEFAULT);
}