#include "stdafx.h"
#include "SocketBase.h"
#include "SocksClient.h"
#include "CSSClientTesting.h"
#include "Debug.h"
#include "EncyptionMgr.h"
#include "APPConfig.h"
#include "UdpTest.h"

using namespace  debug;

#define TESTING_TCP_DST_PORT 80
#define TESTING_TCP_DST_DOMAIN "global.bing.com"
#define DEFAULT_PROXY_TEST_UDP_DNS _T("114.114.114.114")
#define DEFAULT_PROXY_TEST_UDP_DOMAIN _T("baidu.com")

#define CHECK_PROXY_HEADER "GET %s HTTP/1.1\r\n"\
	"Host: %s\r\n"\
	"Connection: keep-alive\r\n"\
	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.130 Safari/537.36\r\n"\
	"Accept-Encoding: gzip, deflate, sdch\r\n"\
	"Accept-Language: en-US,en;q=0.8\r\n\r\n"

CSSClientTesting::CSSClientTesting( CTestingLogRichedit *pLog )
	: CSSCLient( INVALID_SOCKET )
{
	assert( pLog!= NULL );

	pLogCtrol = pLog;

	nDataTransferLatency = 0;
	nLatency = 0;
}
CSSClientTesting::~CSSClientTesting()
{

}

/** @brief 测试一个SS节点
*
* @returns
* 0: 成功, 没有任何错误
* 1: 没有指定SS节点信息,或者指定的SS节点包含 无效的信息
*/
BOOL CSSClientTesting::TestSSNode( CSSNodeInfo *pNode ,BOOL bIsUdp )
{
	assert( pNode != NULL );

	PrintfW( LEVEL_INFO, _T("CSSClientTesting::TestSSNode...") );

	pLogCtrol->AppendTestingLogUTF8( _("Testing started."),TESTMSG_INFO );

	if( !pNode 
		|| pNode->server.empty()
		|| pNode->server_port == 0 
		|| pNode->password.empty()
		|| pNode->method.empty()
		)
	{
		PrintfW( LEVEL_ERROR, _T("CSSClientTesting::TestSSNode: Shadowsocks server node invalid.") );
		pLogCtrol->AppendTestingLogUTF8( _("Shadowsocks server node invalid."),TESTMSG_NEGATIVE );

		return FALSE;
	}

	pNodeInfo = pNode;

	pCryptor = CEncryptionMgr::Create( pNode->method,pNode->password );
	if( !pCryptor )
	{
		PrintfW( LEVEL_ERROR, _T("CSSClientTesting::TestSSNode: Create Encryption error.") );
		pLogCtrol->AppendTestingLogUTF8( _("Create Encryption error."),TESTMSG_NEGATIVE );

		return FALSE;
	}

	char szMsg[1024] = {0};
	BOOL bHiddenIp = CAPPConfig::IsValidConfig() && CAPPConfig::IsHiddenIPPort();
	//////////////////////////////////////////////////////////////////////////
	// Print Server info
	sprintf_s(szMsg, 1024, _("Server IP: %s"), bHiddenIp?"-":pNode->server.c_str() );
	pLogCtrol->AppendTestingLogUTF8( szMsg,TESTMSG_INFO );

	sprintf_s(szMsg, 1024, _("Server Port: %d"), bHiddenIp?0:pNode->server_port	);
	pLogCtrol->AppendTestingLogUTF8( szMsg,TESTMSG_INFO );

	sprintf_s(szMsg, 1024, _("Server password: %s"), bHiddenIp?"-":pNode->password.c_str() );
	pLogCtrol->AppendTestingLogUTF8( szMsg,TESTMSG_INFO );

	sprintf_s(szMsg, 1024, _("Encryption: %s"),	bHiddenIp?"-":pNode->method.c_str() );
	pLogCtrol->AppendTestingLogUTF8( szMsg,TESTMSG_INFO );

	pNodeInfo->UpdateConnections( 1 );
	BOOL bRet = FALSE;
	if( bIsUdp )
		bRet = TestSSNodeUdp( pNode );
	else 
		bRet = TestSSNodeTcp( pNode );

	//nSentSeconds = nSentSeconds > MIN_CHECK_SPEED_TIME ? nSentSeconds : MIN_CHECK_SPEED_TIME;
	//int speed = (int)(nSentKB / nSentSeconds);

	//pNodeInfo->UpdateSpeed( speed );
	pNodeInfo->UpdateConnections( 2 );
	pNodeInfo->SetLastStatus( bRet );

	if( bRet )
	{
#if 0
		// 不测速度.数据量太少, 无意义.
		sprintf_s( szMsg, 1024, _("Speed : %d KB/s (Approximate)"),pNodeInfo->GetKBSpeed() );
		pLogCtrol->AppendTestingLogUTF8( szMsg ,TESTMSG_AFFIRMATIVE );
#endif
		if( bIsUdp )
		{
			sprintf_s( szMsg, 1024, _("Latency : %d ms"),nDataTransferLatency );
		}
		else 
		{
			sprintf_s( szMsg, 1024, _("Latency : %d ms"),nLatency );
		}

		pLogCtrol->AppendTestingLogUTF8( szMsg ,TESTMSG_AFFIRMATIVE );
	}

	pLogCtrol->AppendTestingLogUTF8( _("Testing is done!"),TESTMSG_INFO );
	pLogCtrol->AppendTestingLogW( _T("//////////////////////////////////////////////"),TESTMSG_INFO );

	return bRet;
}

BOOL CSSClientTesting::TestSSNodeTcp( CSSNodeInfo *pNode 	)
{
	PrintfW( LEVEL_INFO, _T("CSSClientTesting::TestSSNodeTcp...") );

	if( !ConnectToSSServer(FALSE, TRUE, NULL ) )
	{
		return FALSE;
	}

	CSSConfigInfo *pCfg = GetConfigInfo();

	char szProtocol[100] = {0};
	char szHost[1024] = {0};
	char szObj[1024] = {0};
	u_short uPort = 0;
	char szMsg[2048] = {0};

	sprintf_s( szMsg, 2048, _("Testing url: %s"), pCfg->testing_url.c_str() );
	pLogCtrol->AppendTestingLogUTF8( szMsg,TESTMSG_INFO );

	ParseDestinationUrl( pCfg->testing_url.c_str(), szProtocol, 100, szHost, 1024,uPort, szObj, 1024 );

	uDestinationPort = uPort;
	sprintf_s(uDestinationAddr, SOCKS_MAX_DOMAIN_LEN + 1,"%s", szHost );
	ATyp = SOCKS_ATYPE_DOMAIN;

	pLogCtrol->AppendTestingLogUTF8( _("Sending protocol packet..."),TESTMSG_INFO );
	if( !SendSSProtocol() )
	{
		pLogCtrol->AppendTestingLogUTF8( _("Protocol packet Sent failed."),TESTMSG_NEGATIVE );
		return FALSE;
	}
	pLogCtrol->AppendTestingLogUTF8( _("Protocol packet sent."),TESTMSG_AFFIRMATIVE );

	char *pCheckProxyHeader = strdup_printf( CHECK_PROXY_HEADER,szObj, szHost );
	if( !pCheckProxyHeader )
	{
		pLogCtrol->AppendTestingLogUTF8( _("Generate request header error."),TESTMSG_NEGATIVE );
		return FALSE;
	}

	int len = strlen( pCheckProxyHeader );
	sprintf_s( szMsg, 2048, _("Sending request packet...(length: %d bytes)"), len );

	pLogCtrol->AppendTestingLogUTF8( szMsg,TESTMSG_INFO );
	int ret = SSSend( hRemoteSocket, pCheckProxyHeader, len , 5 );

	if( ret <= 0 )
	{
		pLogCtrol->AppendTestingLogUTF8( _("Request packet sent failed."),TESTMSG_NEGATIVE );
		return FALSE;
	}
	pLogCtrol->AppendTestingLogUTF8( _("Request packet sent."),TESTMSG_AFFIRMATIVE );

	pLogCtrol->AppendTestingLogUTF8( _("Waiting response..."),TESTMSG_INFO );
	ret = SSRecv( hRemoteSocket,szMsg, 2047, 5 );
	if( ret <= 0 )
	{
		pLogCtrol->AppendTestingLogUTF8( _("Response received failed! connection reset by peer or timeout, maybe your password or encryption is wrong."),TESTMSG_NEGATIVE );
		return FALSE;
	}
	if( strnicmp( szMsg, "http/", 5 ) == 0 )
	{
		sprintf_s( szMsg, 2048, _("Got correct response (length: %d bytes)."), ret );

		pLogCtrol->AppendTestingLogUTF8( szMsg ,TESTMSG_AFFIRMATIVE );
	}
	else 
	{
		sprintf_s( szMsg, 2048, _("Got %d bytes unrecognized packet, maybe encryption is wrong."), ret );

		pLogCtrol->AppendTestingLogUTF8( szMsg ,TESTMSG_NEGATIVE );

		return FALSE;
	}

	return TRUE;
}

BOOL CSSClientTesting::TestSSNodeUdp( CSSNodeInfo *pNode 	)
{
	PrintfW( LEVEL_INFO, _T("CSSClientTesting::TestSSNodeUdp...") );

	pLogCtrol->AppendTestingLogUTF8( _("Testing UDP relay..."),TESTMSG_INFO );

	SOCKADDR_IN ServerAddr;

	if( !TranslateSSServerAddress( ServerAddr, (char *)pNodeInfo->server.c_str(), pNodeInfo->server_port ) )
	{
		PrintfW( LEVEL_ERROR, _T("CSSClientTesting::TestSSNodeUdp: TranslateSSServerAddress error.") );

		pLogCtrol->AppendTestingLogUTF8( _("Can not resolve SS server address."),TESTMSG_NEGATIVE );

		return FALSE;
	}

	BOOL bRet = TestUdpConnection( 
		inet_ntoa( ServerAddr.sin_addr ), 
		ntohs( ServerAddr.sin_port ),
		pCryptor,
		DEFAULT_PROXY_TEST_UDP_DNS,
		DEFAULT_PROXY_TEST_UDP_DOMAIN,
		nDataTransferLatency
		);

	if( bRet )
	{
		pLogCtrol->AppendTestingLogUTF8( _("Testing UDP relay...Passed!"),TESTMSG_AFFIRMATIVE);
	}
	else 
	{
		pLogCtrol->AppendTestingLogUTF8( _("Testing UDP relay...Unpassed!"),TESTMSG_NEGATIVE );
	}

	return bRet;
}

/** @brief 连接到ss 服务器
*/
BOOL CSSClientTesting::ConnectToSSServer( BOOL bUdp,BOOL bNonBlocking,SOCKADDR_IN *pSockAddrIn )
{
	PrintfW( LEVEL_INFO, _T("CSSClientTesting::ConnectToSSServer...") );

	pLogCtrol->AppendTestingLogUTF8( _("Connecting to SS node..."),TESTMSG_INFO );

	CSSConfigInfo *pCfg = GetConfigInfo();

	int times = 1;
	SOCKET s = INVALID_SOCKET;

	do{
		PrintfW( LEVEL_INFO, _T("CSSClientTesting::ConnectToSSServer: the %d time connect to server.") ,times );

		// create tcp no-blocking socket.
		s = CreateSocket( bUdp, bNonBlocking );

		if( s == INVALID_SOCKET )
		{
			PrintfW( LEVEL_ERROR, _T("CSSClientTesting::ConnectToSSServer: Create remote socket error.") );

			pLogCtrol->AppendTestingLogUTF8( _("Connecting to SS node error."),TESTMSG_NEGATIVE );

			return FALSE;
		}
		PrintfW( LEVEL_INFO, _T("CSSClientTesting::ConnectToSSServer: Remote socket created. Socket(%d)..."), s );

		if( !BindSocket( s ))
		{
			CLOSESOCKET( s );

			pLogCtrol->AppendTestingLogUTF8( _("Connecting to SS node error."),TESTMSG_NEGATIVE );

			return FALSE;
		}
		SOCKADDR_IN ServerAddr;
		pNodeInfo->UpdateConnectedTimes( 1 );

		if( !TranslateSSServerAddress( ServerAddr, (char *)pNodeInfo->server.c_str(), pNodeInfo->server_port ) )
		{
			PrintfW( LEVEL_ERROR, _T("CSSClientTesting::ConnectToSSServer: TranslateSSServerAddress error.") );
			
			pLogCtrol->AppendTestingLogUTF8( _("Can not resolve SS server address."),TESTMSG_NEGATIVE );

			CLOSESOCKET( s );

			pNodeInfo->UpdateFailureTimes( 1 );

			return FALSE;
		}

		if( pSockAddrIn )
		{
			memcpy( pSockAddrIn, &ServerAddr, sizeof( ServerAddr ) );
		}

		//connect SOCKS server
		int iRet =  0;
		DWORD start = GetTickCount();
		DWORD end = 0;
		iRet = connect( s, (const struct sockaddr*)&ServerAddr, sizeof(ServerAddr) );

		if (iRet == SOCKET_ERROR) 
		{
			// 异步SOCKET,等待10秒完成链接.
			if( WSAGetLastError() == WSAEWOULDBLOCK )
			{
				if( WaitCompletion( s, 10 ) == SOCKET_ERROR )
				{
					PrintfW( LEVEL_ERROR, _T("CSSClientTesting::ConnectToSSServer: Connect to Shadowsocks server error.") );
					pLogCtrol->AppendTestingLogUTF8( _("Connecting to SS node error."),TESTMSG_NEGATIVE );

					CLOSESOCKET( s );

					pNodeInfo->UpdateFailureTimes( 1 );

					return FALSE;
				}
				else 
				{
					end = GetTickCount();
					nLatency = end - start;

					pNodeInfo->UpdateLatency( nLatency );

					break;
				}
			}
			else
			{
				PrintfW( LEVEL_ERROR, _T("CSSClientTesting::ConnectToSSServer: Connect to Shadowsocks server error code: %d."),WSAGetLastError() );
				pLogCtrol->AppendTestingLogUTF8( _("Connecting to SS node error."),TESTMSG_NEGATIVE );

				CLOSESOCKET( s );

				pNodeInfo->UpdateFailureTimes( 1 );

				return FALSE;
			}
		}
		else
		{
			end = GetTickCount();
			nLatency = end - start;
			pNodeInfo->UpdateLatency( nLatency );

			break;
		}

		times ++;
	} while( times <= pCfg->reconnectTimes );

	if( bNonBlocking )
	{
		// 连接完成后, 将SOCKET设置为阻塞的. 否则之后的交互过程,会因为非阻塞而无法立即完成.
		SetNoBlocking( s, FALSE );
	}

	// set no delay
	SetNodelay( s );

	hRemoteSocket = s;

	pLogCtrol->AppendTestingLogUTF8( _("Connected to SS node."),TESTMSG_AFFIRMATIVE );

	return TRUE;
}