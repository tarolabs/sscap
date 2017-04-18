#pragma once

/*!
 * @file SocksServer.h
 * @date 2015/10/19 15:18
 *
 * @brief 本地的socks 5 代理监听服务类
 *
 * 
 * @author Taro
 * Contact: sockscap64@gmail.com
 *
 *
 * @version 1.0
 *
 * @History
 * <author>      <time>      <version>         <desc>
 *
 * @TODO: long description
 *
 * @note
*/
#include <map>
#include <mymutex.h>
#include "SocketBase.h"

using namespace std;
class CSocketBase;

// success
#define ERR_RET_OK 0
// 端口绑定失败,(已被占用)
#define ERR_BIND_PORT 1
// 创建SOCKET失败
#define ERR_CREATE_SOCKET 2
// 监听失败
#define ERR_LISTEN 3
// 创建ACCEPT线程失败
#define ERR_CREATE_ACCEPT_THREAD 4
// 默认搜索100个端口
#define DEFAULT_SEARCH_AMOUNT 100

class CListener : public CSocketBase
{
public:
	CListener();
	virtual ~CListener();

protected:
	//u_short nListenPort;
	//bool shareOverLan;
	SOCKET hListenSocket;
	u_long uConnections;
	bool bSearchingPort; ///< 端口占用了, 自动搜索可用端 口
	int nLastError;
	int nSearchingPortAmount; ///< 端口搜索数量.

public:
	LPCTSTR GetLastErrorString();

	/* @brief 取得最后错误
	*/
	int GetLastError()
	{
		return nLastError;
	}

	void IncrementConnection();
	void DecrementConnection();
	/*
	bool IsShareOverLan()
	{
		return shareOverLan;
	}
	*/
	/** @brief 获得当前连接数
	*/
	int GetConnections()
	{
		return uConnections;
	}

	/*void SetListenPort( u_short p)
	{
		nListenPort = p;
	}*/
	/*u_short GetListenPort()
	{
		return nListenPort;
	}*/
	/*void SetListenShareOverLan( bool s )
	{
		shareOverLan = s;
	}*/

	SOCKET GetSocketHandle()
	{
		return hListenSocket;
	}
	BOOL IsListenerStarted()
	{
		return ( hListenSocket == INVALID_SOCKET ? FALSE: TRUE );
	}

public:
	BOOL Start();
	void Stop();
};
void DisconnectAllConnection( int nNodeId );