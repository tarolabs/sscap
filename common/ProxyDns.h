#pragma once

/*!
 * @file ProxyDns.h
 * @date 2015/06/12 18:11
 *
 * @brief 通过代理请求DNS. 主要用于测试代理的UDP
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
#include "Dns.h"
#include "ProxySocketClient.h"
#include "Utils.h"

#define _DNS_LIB_

class _DNS_LIB_ CProxyDNSClient : public CDnsClient
{
public:
	CProxyDNSClient( const char *ip, u_short port ) ;

	virtual BOOL Query(LPCTSTR lpszServer, LPCTSTR lpszHost, LPCTSTR lpszType);
protected:
	TCHAR szProxyAddress[PROXY_IP_ADDRESS_MAX_LENGTH];		///< 可能有些人会输入一个代理域名.s5.office.baidu.com 这样的
	unsigned short socks_port;
};