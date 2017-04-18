#include "stdheader.h"
#include "pac.h"
#include "BaseDef.h"
#include "Utils.h"
#include "Mymutex.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static string strPacBodyCache;
static string UserPacBodyCache;
static CMyMutex mutextLoader;

/** @brief 加载基本PAC文件, 由我们指定
*/
static string LoadBasicPacFile( BOOL bForceLoad )
{
	CSSConfigInfo *pCfg = GetConfigInfo();

	int length = 0;
	char *filebody = NULL;
	string BasicPacFile;

	filebody = readallfile( (TCHAR *)pCfg->localPacFileFullName.c_str(),_T("rb") , &length );
	if( !filebody ) return FALSE;	

	BasicPacFile = string( filebody );
	free( filebody );

	return BasicPacFile;
}
#if 0
/** @brief 加载用户PAC文件, 用户需要指定新的PAC规则在这里指定
*/
static string LoadUserPacFile( BOOL bForceLoad )
{
	CSSConfigInfo *pCfg = GetConfigInfo();

	int length = 0;
	char *filebody = NULL;
	string UserPacFile;

	filebody = readallfile( (TCHAR *)pCfg->localUserPacFileFullName.c_str(),_T("rb") , &length );
	if( !filebody ) 
		return string(" ");

	UserPacFile = string( filebody );

	trim( UserPacFile );

	mutextLoader.Lock();
	UserPacBodyCache = UserPacFile;
	mutextLoader.UnLock();

	if( UserPacFile.empty() )
	{
		return string(" ");
	}

	vector<string> results;

	Replace( UserPacFile, string("\r\n"), string("\n") );
	SplitStringA( UserPacFile, string("\n"), results , false );
	if( results.empty( ) )
	{
		return string(" ");
	}

	//string strResult = implode( results, ",\r\n" );
	string strResult;
	int count = (int ) results.size();
	for( int i = 0 ; i < count; i ++ )
	{
		strResult += string(",\"") + results[i] + string( "\":1\r\n" );
	}

	return strResult;
}
#endif
/** @brief 加载PAC文件
* 
* bForceLoad = TRUE 强制載, 即使之前加载过了.
*/
BOOL LoadPacFile( BOOL bForceLoad )
{
	if( !bForceLoad && !strPacBodyCache.empty() )
		return TRUE;

	string BasicPacFile;
	string UserPacFile;
	CSSConfigInfo *pCfg = GetConfigInfo();

	BasicPacFile = LoadBasicPacFile( bForceLoad );
	//UserPacFile = LoadUserPacFile( bForceLoad );

	char szLocalProxy[100] = {0};
	sprintf_s( szLocalProxy, 50, "PROXY 127.0.0.1:%d;",pCfg->localPort );

	string proxy = string( szLocalProxy );
	Replace( BasicPacFile,string("__PROXY__"),proxy  );
	Replace( BasicPacFile,string("__USERRULE__"),UserPacFile  );

	mutextLoader.Lock();
	strPacBodyCache = BasicPacFile;
	mutextLoader.UnLock();

	return TRUE;
}

/** @brief 获取PAC文件内容
*/
string GetPacFileContent()
{
	if( !LoadPacFile( FALSE ) )
		return string("");

	return strPacBodyCache;
}
string GetUserPacFileContent()
{
	return UserPacBodyCache;
}

