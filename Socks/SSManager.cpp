#include "stdheader.h"
#include "SocketBase.h"
#include "SocksClient.h"
#include "SSClient.h"
#include "Listener.h"
#include "SSManager.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSSManager::CSSManager()
{

}

CSSManager::~CSSManager()
{

}

/** @brief 开始服务
*/
BOOL CSSManager::StartServices( )
{
	//CSSConfigInfo *pCfg = GetConfigInfo();
	//if( !pCfg ) return FALSE;

	//SetListenPort( pCfg->localPort );
	//SetListenShareOverLan( pCfg->shareOverLan );

	BOOL bRet = Start();

	//u_short realPort = GetListenPort();

	// 配置中的端口和实际端口不同了,说明配置的端口被使用了. 并且新搜索了可用端口
	/*
	if( bRet && pCfg->localPort != realPort )
	{
		pCfg->localPort = realPort;
		SaveShadowsocksServer();
	}
	*/

	return bRet;
}

/** @brief 停止服务
*/
void CSSManager::StopServices()
{
	if( IsListenerStarted() )
		Stop();

	return ;
}