// SocketEx.h: interface for the CSocketEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETEX_H__78840EDA_D918_4017_A6AB_D3ADF120E9D8__INCLUDED_)
#define AFX_SOCKETEX_H__78840EDA_D918_4017_A6AB_D3ADF120E9D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <atlbase.h>
//#include <afxsock.h>

#define _DNS_LIB_
#include <WinSock2.h>

class _DNS_LIB_ CSocketEx  
{
public:
	SOCKET	m_hSocket;
	int		m_nLastError;
	SOCKADDR_IN m_nSockAddr;

public:
	CSocketEx();
	virtual ~CSocketEx()
	{
		Close();
	}

	void operator = (CSocketEx & Socket)
	{
		Detach();
		Attach(Socket.m_hSocket);
	}

	BOOL Accept(CSocketEx & Socket);

	BOOL Attach(SOCKET hSocket);

	BOOL Connect(LPCTSTR lpszHost, int nPort);

	BOOL Create(int nPort, LPCTSTR lpszHost,int socket_type = SOCK_DGRAM /* UDP: SOCK_STREAM,TCP: SOCK_STREAM */ );

	void SetTimeout( int Timeout = 10 * 1000 /* in milisecods */);

	SOCKET Detach()
	{
		SOCKET hSocket = m_hSocket;
		m_hSocket = INVALID_SOCKET;
		return hSocket;
	}

	BOOL Disconnect();

	BOOL GetPeerName(string & sPeerAddress, int & nPeerPort);

	wstring GetLastError();

	BOOL Listen()
	{
		// Socket should be valid.
		if(m_hSocket == INVALID_SOCKET)
		{
			return false;
		}

		return (listen(m_hSocket,SOMAXCONN) != SOCKET_ERROR);
	}

	BOOL Close()
	{
		SOCKET hSocket = m_hSocket;
		m_hSocket = INVALID_SOCKET;
		if( hSocket != INVALID_SOCKET)
		{
			return (closesocket(hSocket) != SOCKET_ERROR);
		}

		return true;
	}

	int Receive(LPVOID lpBuffer, int nSize);

	int Send(LPVOID lpBuffer, int nSize);

	void SetProxySettings(int nVersion, LPCTSTR lpszProxyHost, int nProxyPort, LPCTSTR lpszUsername, LPCTSTR lpszPassword)
	{
	}

	BOOL Shutdown(int nMethod)
	{
		if(m_hSocket != INVALID_SOCKET)
		{
			return(shutdown(m_hSocket,nMethod)!= SOCKET_ERROR);
		}
		return true;
	}
	BOOL TranslateAddress(LPCTSTR lpszHost, int nPort, SOCKADDR_IN &sockAddr);

	BOOL GetSocketName(string &sAddress, int &nPort);

	BOOL ReadLine(string &String);

	BOOL TranslateRemoteAddress(LPCTSTR lpszHost, int nPort, SOCKADDR_IN &sockAddr);

	BOOL Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress);

};

#endif // !defined(AFX_SOCKETEX_H__78840EDA_D918_4017_A6AB_D3ADF120E9D8__INCLUDED_)