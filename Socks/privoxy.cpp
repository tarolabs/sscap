#include "stdheader.h"
#include "privoxy.h"
#include <TlHelp32.h>
#include "Utils.h"
#include <assert.h>
#include "BaseDef.h"
#include "libprivoxy/libprivoxy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL gPrivoxyStarted = FALSE;
static u_short gPrivoxyListenPort = 0;

BOOL IsPrivoxyStarted()
{
	return gPrivoxyStarted;
}

BOOL RunPrivoxy( )
{
	CSSConfigInfo *pCfg = GetConfigInfo();

	// 如果PRIVOXY模块已经启动的话, 则只会修改配置文件,并马上会生效的.
	int ret = start_privoxy( 
		SOCKS5, 
		LOCAL_LOOPBACK_IP,
		pCfg->localPort ,
		pCfg->localSocksUser.c_str(),
		pCfg->localSocksPass.c_str() ,
		"0.0.0.0",
		0,
		"unset" );
	if( ret == 0 )
		gPrivoxyStarted = TRUE;

	gPrivoxyListenPort = get_privoxy_port();
	return gPrivoxyStarted;
}

/** @biref 获取privoxy进程工作端口
*/
unsigned short GetPrivoxyListenPort()
{
	return gPrivoxyListenPort;
}