#include "stdheader.h"
#include "SocketBase.h"
#include "SocksClient.h"
#include "SSClient.h"
#include "Listener.h"
#include "Debug.h"
#include <process.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace debug;

struct ThreadData
{
	CListener *pListener;
	SOCKET s;
	u_int tid;
	HANDLE hHandle;
};
struct TerThread
{
	CSSCLient *pClient;
	HANDLE hHandle;
};

CMyMutex mutex;
map<string,TerThread *> mapClientThreads;
void PushClientThread( string tid, TerThread *pTerThread);
void RemoveClientThread( string tid );
void TerminateClientThread();

CListener::CListener()
{
	nSearchingPortAmount = DEFAULT_SEARCH_AMOUNT;
	
	hListenSocket = INVALID_SOCKET;
	//nListenPort = 0;
	//shareOverLan = false;

	uConnections = 0;

	// 2015-12-4: 不自动搜索, 如果绑定失败就提示用户,让用户自己选择. 自动搜索端口, 如果端口改变了会让用户摸不着头脑.
	bSearchingPort = false;

	mapClientThreads.clear();
	nLastError = ERR_RET_OK;
}

CListener::~CListener()
{
	CLOSESOCKET ( hListenSocket );
}


unsigned  int __stdcall Thread_Client( void * parameter )
{
	if( !parameter ) 
	{	
		return 0;
	}
	
	ThreadData *pTd = (ThreadData *)parameter;
	CListener *pListener = pTd->pListener;
	SOCKET s = pTd->s;
	u_int tid = pTd->tid;
	HANDLE hHandle = pTd->hHandle;

	CSSConfigInfo *pCfg = GetConfigInfo();

	pListener->IncrementConnection();

	PrintfA(LEVEL_INFO,"[Thread_Client] started with socket(%d).", s );

	CSSCLient ssclient( s ,pCfg->localSocksUser.c_str(),pCfg->localSocksPass.c_str() );

	char szThreadId[50] = {0};
	sprintf_s( szThreadId, "%d%d", GetTickCount(), tid );

	TerThread *pTerThread = new TerThread ;
	pTerThread->hHandle = hHandle;
	pTerThread->pClient = &ssclient;
	PushClientThread( szThreadId, pTerThread );

	ssclient.SetProxyListenPort( pCfg->localPort );
	BOOL bRet = ssclient.Start();
	pListener->DecrementConnection();

	RemoveClientThread( szThreadId );
	
	delete pTd;
	return 1;
}

unsigned  int __stdcall Thread_Accept( void * parameter )
{
	CSSConfigInfo *pCfg = GetConfigInfo();

	CListener *pListener = (CListener *)parameter;
	if( !pListener ) return 0;

	PrintfA(LEVEL_INFO,"[Thread_Accept] started." );

	while( pListener->GetSocketHandle() != INVALID_SOCKET )
	{
		struct sockaddr_in in_addr;
		int AddrLen = sizeof( in_addr );
		memset( &in_addr, 0, sizeof( in_addr ));

		//----------------------
		// Create a SOCKET for accepting incoming requests.
		SOCKET AcceptSocket = accept( pListener->GetSocketHandle(), (struct sockaddr*)&in_addr, &AddrLen );

		if ( AcceptSocket == INVALID_SOCKET )
		{
			PrintfA(LEVEL_ERROR,"[Thread_Accept] Accepted a error socket." );

			pListener->Stop();

			break;
		}
		else 
		{
			// 如果不允许其它计算机的连接
			if( !pCfg->shareOverLan )
			{
				if( in_addr.sin_addr.S_un.S_un_b.s_b1 != 127 )
				{
					PrintfA(LEVEL_ERROR,"[Thread_Accept] Connection from other computer, but your settings not allow connection from other computer. (%s)", inet_ntoa( in_addr.sin_addr ) );

					CLOSESOCKET( AcceptSocket );

					continue;
				}
			}

			ThreadData *pTd = new ThreadData;
			if( pTd )
			{
				pTd->pListener = pListener;
				pTd->s = AcceptSocket;

				u_int tid = 0;
				HANDLE hHandle = ( HANDLE ) _beginthreadex(NULL, 0, &Thread_Client, pTd, CREATE_SUSPENDED  , &tid );
				if( hHandle == NULL )
				{
					PrintfW(LEVEL_ERROR,_T("[Thread_Accept] Create Client thread failed.") );

					CLOSESOCKET( AcceptSocket );

					delete pTd;
				}
				else 
				{
					pTd->tid = tid;
					pTd->hHandle = hHandle;

					ResumeThread( hHandle );
				}
			}
		}
	}

	return 1;
}

BOOL CListener::Start()
{
	PrintfW(LEVEL_INFO,_T("CListener::Start...") );

	CSSConfigInfo *pCfg = GetConfigInfo();
	if( !pCfg ) return FALSE;

	//----------------------
	// Create a SOCKET for listening for incoming connection requests.
	hListenSocket = CreateSocket( FALSE, FALSE );

	if ( hListenSocket == INVALID_SOCKET) 
	{
		nLastError = ERR_CREATE_SOCKET;

		PrintfW(LEVEL_ERROR,_T("CListener::Start: Create socket error.") );
		return FALSE;
	}

	/** 允许重用本地地址和端口:
	* 这样的好处是，即使socket断了，调用前面的socket函数也不会占用另一个，而是始终就是一个端口
	* 这样防止socket始终连接不上，那么按照原来的做法，会不断地换端口。
	* 
	* https://msdn.microsoft.com/en-us/library/ms740476(VS.85).aspx
	*/
	int iResult = 0;
	int nREUSEADDR = 1;
	iResult = setsockopt(hListenSocket,
		SOL_SOCKET,
		SO_REUSEADDR,
		(const char*)&nREUSEADDR,
		sizeof(nREUSEADDR) );

	if( iResult == SOCKET_ERROR )
	{
		PrintfW(LEVEL_ERROR,_T("[CListener::Start] call setsockopt to set SO_REUSEADDR error, code: %d"), WSAGetLastError() );
	}

	u_short port = pCfg->localPort;
	u_short findl_port = port;
	int nSearchPortTimes = nSearchingPortAmount; // 连续搜索20个端 口

	do{
		u_long bind_addr = 0;
		//if( shareOverLan )
		bind_addr  = ADDR_ANY;
		//bind_addr  = inet_addr("127.0.0.1");

		//else 
		//	bind_addr  = inet_addr( "127.0.0.1" );

		if( !BindSocket( hListenSocket, bind_addr, port ) )
		{
			PrintfW(LEVEL_WARNING,_T("[CListener::Start] bind socket error on port: %d. code: %d"),port, WSAGetLastError() );

			// 如果搜索端口
			if( bSearchingPort )
			{
				port ++;
			}
			else
			{
				nLastError = ERR_BIND_PORT;
				CLOSESOCKET( hListenSocket );
				return FALSE;
			}
		}
		else
		{
			findl_port = port;
			break;
		}

		nSearchPortTimes --;

		if( nSearchPortTimes <= 0 )
		{
			PrintfW(LEVEL_ERROR,_T("[CListener::Start] bind socket error. code: %d"), WSAGetLastError() );

			nLastError = ERR_BIND_PORT;

			CLOSESOCKET( hListenSocket );

			return FALSE;
		}

	} while( bSearchingPort );

	PrintfW(LEVEL_INFO,_T("[CListener::Start] Bind socket on port: %d.") ,findl_port );

	//----------------------
	// Listen for incoming connection requests 
	// on the created socket
	if (listen( hListenSocket, SOMAXCONN ) == SOCKET_ERROR)
	{
		PrintfW(LEVEL_ERROR,_T("[CListener::Start] listen error.") );

		nLastError = ERR_LISTEN;

		CLOSESOCKET( hListenSocket );

		return FALSE;
	}


	unsigned threadID = 0;

	HANDLE hListenThread = ( HANDLE ) _beginthreadex(NULL, 0, &Thread_Accept, this, 0 , &threadID);
	if( hListenSocket == NULL )
	{
		PrintfW(LEVEL_ERROR,_T("[CListener::Start] Create accept thread failed.") );

		nLastError = ERR_CREATE_ACCEPT_THREAD;

		CLOSESOCKET( hListenSocket );
		return FALSE;
	}
	else 
		CloseHandle( hListenThread );

#ifdef _DEBUG
	SOCKADDR_IN sa;
	int sa_len = sizeof(sa);

	getsockname( hListenSocket, (struct sockaddr *)&sa, &sa_len  );
	PrintfA(LEVEL_INFO,("[RealStart] ns started at: %s:%d, Thread Id: %d"), inet_ntoa( sa.sin_addr) , ntohs( sa.sin_port ), threadID );
#endif

	// 配置中的端口和实际端口不同了,说明配置的端口被使用了. 并且新搜索了可用端口
	if( pCfg->localPort != findl_port )
	{
		pCfg->localPort = findl_port;
		SaveShadowsocksServer();
	}

	return TRUE;
}

void CListener::Stop()
{
	CLOSESOCKET( hListenSocket );

	Sleep( 1 );
	TerminateClientThread();
}

void CListener::IncrementConnection()
{
	InterlockedIncrement( &uConnections );
}
void CListener::DecrementConnection()
{
	InterlockedDecrement( &uConnections );
}

LPCTSTR CListener::GetLastErrorString()
{
	if( nLastError == ERR_RET_OK )
		return _T("Success");
	else if( nLastError == ERR_BIND_PORT )
		return _T("Port Bind Error");
	else if( nLastError == ERR_CREATE_SOCKET )
		return _T("Listen socket create error");
	else if( nLastError == ERR_LISTEN )
		return _T("Socket listen error");
	else if( nLastError == ERR_CREATE_ACCEPT_THREAD )
		return _T("Accept thread create error");

	return _T("No error info");
}

void PushClientThread( string tid, TerThread *pTerThread)
{
	CMutexParser p( &mutex );

	mapClientThreads.insert( make_pair( tid, pTerThread ) );

}
void RemoveClientThread( string tid )
{
	CMutexParser p( &mutex );

	map<string,TerThread*>::iterator iter = mapClientThreads.find( tid );
	if( iter != mapClientThreads.end() )
	{
		CloseHandle( iter->second->hHandle );

		delete iter->second;

		mapClientThreads.erase( iter );
	}
}
void TerminateClientThread()
{
	CMutexParser p( &mutex );

	map<string,TerThread*>::iterator iter = mapClientThreads.begin();
	for( iter; iter != mapClientThreads.end(); iter ++ )
	{
		TerminateThread( iter->second->hHandle ,0 );
		CloseHandle( iter->second->hHandle );

		delete iter->second;
	}

	mapClientThreads.clear();
}
void DisconnectAllConnection( int nNodeId )
{
	CMutexParser p( &mutex );

	map<string,TerThread*>::iterator iter = mapClientThreads.begin();
	for( iter; iter != mapClientThreads.end(); iter ++ )
	{
		CSSNodeInfo *pNodeInfo = iter->second->pClient->GetNodeInfo();
		if( pNodeInfo && pNodeInfo->id == nNodeId )
		{
			iter->second->pClient->SetUserBreaking();
		}
	}

	mapClientThreads.clear();
}