#pragma once

#include "Dns.h"
#include "ProxyDns.h"
#include "Socks/Encypter.h"

class CSSDnsClient: public CProxyDNSClient
{
public:
	CSSDnsClient( const char *ip, u_short port , CCryptor *pCryptor );

	virtual BOOL Query(LPCTSTR lpszServer, LPCTSTR lpszHost, LPCTSTR lpszType);
	virtual void GetRecords( CDnsRRPtrList & List , int Total );

protected:
	CCryptor *m_pCryptor;
	CDnsBuffer RspBuffer;
};