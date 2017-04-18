#pragma once

/*!
 * @file ProxySocketClient.h
 * @date 2015/06/12 15:54
 *
 * @brief 通过代理的SOCKET. 主要配合dns类使用,通过代理请求DNS操作.以此来判断一个代理的UDP是否能正常工作.
 * 使用这个类时,必须是已经自己建立了tcp controller了.这个类不会建立TCP连接.只会封装UDP包,直接发给代理.
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

#define _DNS_LIB_

#include "Utils.h"
#include "SocketEx.h"

/** @brief 通过代理请求DNS, 使用前需要自己建立tcp controller连接,从而得到bound port
*/
class _DNS_LIB_ CProxySocketClient : public CSocketEx
{
public:
	/** @brief 通过代理请求DNS, 使用前需要自己建立tcp controller连接,从而得到bound port
	* 
	* @param addr 代理服务器的IP地址
	* @param bound_port 自己向代理服务器请求tcp controller后得到的udp bound port
	*/
	CProxySocketClient( LPCTSTR proxyaddr, u_short proxy_port , LPCTSTR destaddr, u_short dest_port ) ;

	virtual BOOL ConnectTo(LPCTSTR lpszHost , int nPort);

	/** @brief 连接到代理服务器
	*/
	virtual BOOL ConnectTo();
	/** @brief 构造UDP转发的基础包
	*/
	virtual int _make_udpsend_packet_base(  char *real_buffer , int buf_len );
	/** @brief 构造UDP数据的完整包,包括包头以及实际的数据包
	* 
	* @param Buffer 实际要发送的数据包
	* @param Length 实际要发送的数据包长度
	* @param real_udp_buffer_len 封装好的UDP包的总长
	* @return 封装好的UDP数据包
	*/
	virtual char *_MakeUDPProxyData( void *Buffer, int Length ,int &real_udp_buffer_len );
	virtual BOOL WriteBytes ( void * Buffer, int Length );
	virtual int ReadBytes (void * Buffer , int Length);
protected:
	// 代理地址
	TCHAR szPorxyAddress[PROXY_IP_ADDRESS_MAX_LENGTH];
	// 代理BOUND PORT
	u_short m_Proxyport;
	// DNS IP
	TCHAR szDestAddress[PROXY_IP_ADDRESS_MAX_LENGTH];
	// DNS PORT
	u_short m_Destport;

protected:
	SOCKADDR_IN m_addrDestination; // 目的地址( DNS的地址:53)
};