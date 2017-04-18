#pragma once

/*!
 * @file SocketBase.h
 * @date 2015/10/19 10:52
 *
 * @brief socket的基类, 实现socket的相关基础操作
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

#define CLOSESOCKET(x) { if( x != INVALID_SOCKET ){ closesocket( x ) ; x = INVALID_SOCKET ;} }

class CSocketBase
{
public:
	CSocketBase();
	virtual ~CSocketBase();

protected:
	/** @brief 禁用TCP NAGLE 算法
	*  当我们通过 TCP socket 分多次发送较少的数据时，比如小于 1460 或者 100 以内，对端可能会很长时间收不到数据，导致本端应用程序认为超时报错。这时可能是受到了 TCP NAGLE 算法的影响。
	*/
	void SetNodelay( SOCKET s );
	/** @brief 从socket中接收一个字节
	*/
	virtual  BOOL _GetOneByteFromSocket( SOCKET s, int &n ,int timeout );
	
	virtual void _SetSocketTimeout( SOCKET s, int nTimeout = 5 /** 默认5秒*/, int t = 0 /** 0: 设置收和发超时, 1: 只设置收, 2: 只设置发*/ );

	virtual int _Recv( SOCKET s,  char *buffer, int length  ,int timeout);
	virtual int _RecvFrom( SOCKET s,  char *buffer, int length  , struct sockaddr* AddrFrom, int *AddrLen,int timeout);

	virtual int _Send( SOCKET s, char *buffer, int length  ,int timeout);
	virtual int _SendTo( SOCKET s, char *buffer, int length  ,const struct sockaddr *To, int ToLen,int timeout);

	/** @brief 创建套接字
	*/
	SOCKET CreateSocket( BOOL bIsUdp = FALSE /* FALSE: TCP, TRUE: UDP */, BOOL bNonBlocking = TRUE );
	void SetNoBlocking( SOCKET s , BOOL bIsNonBlocking = TRUE /** TRUE: NON- BLOCKING, FALSE: BLOCKING */ );

	/** @brief 绑定套接字
	*/
	BOOL BindSocket( SOCKET s, u_long addr = ADDR_ANY , u_short port = 0 );
	/*  @brief Waits until socket states completion of operation
	* @param[in]		hSock			Socket for calling "select"
	* @param[in] nTimeout timeout in secods.
	* @return							0 on success, SOCKET_ERROR on error (use WSAGetLastError to get error info)
	*/
	int WaitCompletion(SOCKET hSock , int nTimeout /* seconds */);
};