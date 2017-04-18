#pragma once

#include "ProxySocketClient.h"
#include "Socks/Encypter.h"
class CSSProxySocketClient : public CProxySocketClient
{
public:
	CSSProxySocketClient( LPCTSTR proxyaddr, u_short proxy_port , LPCTSTR destaddr, u_short dest_port ,  CCryptor *pCryptor) ;

	/** @brief 构造UDP转发的基础包
	* ss协议和 SOCKS协议相比较, 不需要前边的3个字节的节就可以了
	*/
	virtual int _make_udpsend_packet_base(  char *real_buffer , int buf_len );
	/** @brief 构造UDP数据的完整包,包括包头以及实际的数据包
	* 
	* @param Buffer 实际要发送的数据包
	* @param Length 实际要发送的数据包长度
	* @param real_udp_buffer_len 封装好的UDP包的总长
	* @return 封装好的UDP数据包
	*/
	//virtual char *_MakeUDPProxyData( void *Buffer, int Length ,int &real_udp_buffer_len );
	virtual BOOL WriteBytes ( void * Buffer, int Length );
	virtual int ReadBytes (void * Buffer , int Length);
protected:
	CCryptor *m_pCryptor;
};