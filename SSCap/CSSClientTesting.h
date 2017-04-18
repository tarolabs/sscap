#pragma once

#include "SSClient.h"
#include "BaseDef.h"
#include "TestingLogRichedit.h"

class CSSClientTesting : public CSSCLient
{
public:
	CSSClientTesting( CTestingLogRichedit *pLog );
	virtual ~CSSClientTesting();

	/** @brief 测试一个SS节点
	*/
	BOOL TestSSNode( CSSNodeInfo *pNode ,BOOL bIsUdp);

protected:
	DWORD nLatency;
	DWORD nDataTransferLatency;
protected:
	BOOL TestSSNodeTcp( CSSNodeInfo *pNode 	);
	BOOL TestSSNodeUdp( CSSNodeInfo *pNode 	);
	/** @brief 连接到ss 服务器
	* 
	* @param bUdp: 是udp 连接
	* @param bNonBlocking: 非阻塞
	*/
	virtual BOOL ConnectToSSServer( BOOL bUdp = FALSE, BOOL bNonBlocking = TRUE ,SOCKADDR_IN *pSockAddrIn = NULL );
protected:
	CTestingLogRichedit *pLogCtrol;
};