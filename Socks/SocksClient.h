#pragma once

/*!
 * \file SocksClient.h
 * @All Right Reserved (C), 2014-2015, sockscap64.com
 * Socks5代理的基础类
 * 执行Socks5代理的一些基本交互
 *
 * \author Taro
 * \date 十月 2015
 * \version 1.0
 * \remark
 *
 * \History
 * <author>      <time>      <version>         <desc>
 * 
 * 
 */


#define SOCKS_V5 0x05
#define SOCKS_V4 0x04

#define SOCKS_AUTH_ANONYMOUS 0x00	// X'00' NO AUTHENTICATION REQUIRED
#define SOCKS_AUTH_GSSAPI 0x01	// X'01' GSSAPI
#define SOCKS_AUTH_USERPASS 0x02	// X'02' USERNAME/PASSWORD
// X'03' to X'7F' IANA ASSIGNED
// X'80' to X'FE' RESERVED FOR PRIVATE METHODS

// X'FF' NO ACCEPTABLE METHODS
#define SOCKS_AUTH_NO_METHODS 0xFF

//  The VER field contains the current version of the subnegotiation, which is X'01'
// http://tools.ietf.org/html/rfc1929
#define SOCKS_AUTH_VERSION 0x01

#define SOCKS_MAX_USER_LEN 255
#define SOCKS_MAX_PASS_LEN 255
#define SOCKS_MAX_DOMAIN_LEN 255

/**
o  CONNECT X'01'
o  BIND X'02'
o  UDP ASSOCIATE X'03'
*/
#define SOCKS_CMD_CONNECT 0x01
#define SOCKS_CMD_BIND 0x02
#define SOCKS_CMD_UDP_ASSOCIATE 0x03

#define SOCKS_ATYPE_IPV4 0x01
#define SOCKS_ATYPE_DOMAIN 0x03
#define SOCKS_ATYPE_IPV6 0x04

//o  X'00' succeeded
#define SOCKS_ERR_SECCEEDED 0x00
//o  X'01' general SOCKS server failure
#define SOCKS_ERR_GENERAL 0x01
//o  X'02' connection not allowed by ruleset
#define SOCKS_ERR_CONNECTION_NOT_ALLOWED 0x02
//o  X'03' Network unreachable
#define SOCKS_ERR_NETWORK_UNREACHABLE 0x03
//o  X'04' Host unreachable
#define SOCKS_ERR_HOST_UNREACHABLE 0x04
//o  X'05' Connection refused
#define SOCKS_ERR_CONNECTION_REFUSED 0x05
//o  X'06' TTL expired
#define SOCKS_ERR_TTL_EXPRED 0x06
//o  X'07' Command not supported
#define SOCKS_ERR_COMMAND_NOT_SUPPORTED 0x07
//o  X'08' Address type not supported
#define SOCKS_ERR_ADDRESS_NO_SUPPRTED 0x08
//o  X'09' to X'FF' unassigned

// 保留字段
#define SOCKS_RESERVED 0x00

// 默认超时:5 秒
#define DEFAULT_TIMEOUT 10

#define MAX_UDP_PACKET_SIZE 65507
#define BUF_SIZE MAX_UDP_PACKET_SIZE

// BIND命令时的超时时间
#define BIND_TIMEOUT    120     /* Seconds */
// UDP等待时间
#define UDP_TIMEOUT     60      /* Seconds */

// socks v4 connect
#define SOCKS4_CMD_CONNECT 0x01
#define SOCKS4_CMD_BIND 0x02

// 	90: request granted
#define SOCKS4_ERR_GRANTED 90
//	91: request rejected or failed
#define SOCKS4_ERR_REJECTED 91
// 	92: request rejected becasue SOCKS server cannot connect to identd on the client
#define SOCKS4_ERR_CANNOT_CONNECT 92
// 	93: request rejected because the client program and identd report different user-ids
#define SOCKS4_ERR_UNAUTH 93
#define TCP_BUFSIZE 1024

class CSocketBase;
class CSocksClient : public CSocketBase
{
public:
	CSocksClient( SOCKET s ,const char *username =NULL  /* proxy user */, const char *password = NULL/* proxy pass */ );
	
	virtual ~CSocksClient();

protected:
	SOCKET hSocksSocket; ///< socks5的连接套接字
	SOCKET hRemoteSocket; ///< 连接远程的SOCKET. UDP ASSOCIATE时是UDP的

	//BOOL bNeedAuthByUserPass; ///< 需要通过user, pass验证
	UINT Ver;
	UINT Cmd;
	UINT Rsv;
	UINT ATyp;

	char szSocksV4UserId[256];
	char uDestinationAddr[ SOCKS_MAX_DOMAIN_LEN + 1 ];		 ///< 目标地址
	u_short uDestinationPort; ///< 目的端口
	//char szDestinationDomainName[SOCKS_MAX_DOMAIN_LEN + 1]; ///< 目标域名

	//char SrcAddr[16];
	//u_short SrcPort;
	struct sockaddr SocksSrcAddr;

	SOCKET udpSocket;			// UDP 接收SOCKS CLIENT发过来的包的SOCKET
	//char udpSrcAddr[16];  // UDP 接收数据时, 发送方的源地址.
	//UINT udpSrcPort;		// UDP 接收数据时, 发送方的源端口.
	struct sockaddr udpSrcAddr;

	//char udpSrcBindAddr[16]; // udp, 建立TCP连接时, 发送过来的源绑定地址.  以后接收数据时会判断一下是否和源绑定地址一至. 避免恶意攻击.
	//UINT udpSrcBindPort;	// udp, 建立TCP连接时, 发送过来的源绑定端口.   以后接收数据时会判断一下是否和源绑定地址一至. 避免恶意攻击.
	struct sockaddr udpSrcBindAddr;

	struct sockaddr_in udpDstAddr;	// udp的目的地址.

	int nUnrecognizedByte;	//遇到未能识别的请求,因为已经读取了一个节字,所以这里保存起来备用.
	BOOL bFoundUnrecognizedByte; // 遇到未能识别的请求.
	BOOL bCouldProcessOtherRequest; // 可以处理其它请求,如HTTP代理请求, PAC文件请求.
	u_short nProxyListenPort;	//本地代理监听的端口. 会有用到.
	
	char szProxyUser[256]; ///< 代理用户名
	char szProxyPass[256];	///< 代理密码

public:
	void SetProxyListenPort( u_short port );
	/** @brief 处理socks 5请求
	*/
	virtual BOOL Start();
	/** @brief 设置可以处理其它请求, HTTP,HTTPS代理,PAC文件
	*/
	void SetCouldProcessOtherRequest( BOOL bCould )
	{
		bCouldProcessOtherRequest = bCould;
	}
	BOOL IsGotUnrecognizedByte()
	{
		return bFoundUnrecognizedByte;
	}
	int GetUnrecognizedByte()
	{
		return nUnrecognizedByte;
	}
protected:
	/** @brief Socks5 auth medthod 
	*/
	virtual BOOL ProcessSocksAuthMethod();
	/** @brief 处理Socks v4请求
	*/
	virtual BOOL ProcessSocksV4();
	/** @biref socks5 auth 
	*/
	virtual BOOL ProcessSocksAuth();
	/** @brief 处理socks连接请求
	*/
	virtual BOOL ProcessSocksConnection();
	/** @brief 处理socks5 tcp 连接请求
	*/
	virtual BOOL ProcessSocksTcpConnection();
	/** @brief 处理socks5 udp 连接请求
	*/
	virtual BOOL ProcessSocksUdpConnection();
	/** @brief 处理socks5 bind请求
	*/
	virtual BOOL ProcessSocksV5Bind();
	/**
	The client connects to the SOCKS server and sends a BIND request when
	it wants to prepare for an inbound connection from an application server.
	This should only happen after a primary connection to the application
	server has been established with a CONNECT.  Typically, this is part of
	the sequence of actions:

	-bind(): obtain a socket
	-getsockname(): get the IP address and port number of the socket
	-listen(): ready to accept call from the application server
	-use the primary connection to inform the application server of
	the IP address and the port number that it should connect to.
	-accept(): accept a connection from the application server
	*/
	virtual BOOL ProcessSocksV4Bind();
protected:
	/** @brief 验证帐号密码
	*/
	virtual BOOL VerifySocksUsernamePassword( char *username, char * password );
	/** @brief 执行完ProcessSocks5TcpConnection之后的操作
	*/
	virtual BOOL AfterProcessSocksTcpConnection() = 0 ;
	/** @brief 执行完ProcessSocks5UdpConnection之后的操作
	*/
	virtual BOOL AfterProcessSocksUdpConnection() = 0 ;
	/** @brief 处理其它请求
	* 
	* @returns
	*	0: 失败, 调用者可以直接返回了.
	*	1: 成功, 但是调用者可以直接返回不需要接着处理MainLoop了
	*	2: 成功, 调用者需要接着去处理下边的MainLoop
	*/
	virtual int ProcessOtherRequest( int nUnrecognizedByte ) = 0;
	/** @brief 执行数据传递的loop
	*/
	virtual BOOL MainLoop() = 0;
protected:
	BOOL DirectConnectTo();
	void CloseAllSocket();
	/** @brief method是否有效. 我们只支持匿名及USER/PASS两种认证方式
	*/
	BOOL _IsValidProxyMethod( int method );
	/** @brief 检测UDP发过来的包与原先建立的TCP连接中指定的UDP地址,端口是否一至.
	* 如果建立TCP连接中的指定的UDP地址和端口是0,则这里直接通过
	*
	* @return TRUE 检查通过, FALSE 不通过, 收到的包忽略掉
	*/
	BOOL _CheckUdpSrcBindAddr(struct sockaddr *addr );
	/** @brief 是否收到有效的命令
	*/
	BOOL _IsReceivedValidCommand( UINT cmd );
	/** 是否收到一个有效的SOCKS版本号
	*/
	BOOL _IsReceivedValidSocksVersion( UINT &r_version );

	/** @brief 从local socket( app连入的socket ) 中接收一个字节
	*/
	virtual BOOL _GetOneByteFromLocalSocket( int &n );
	/** @brief 从local socket( 连接到目的地的socket ) 中接收一个字节
	*/
	virtual BOOL _GetOneByteFromRemoteSocket( int &n );
	
	virtual int _LocalRecv( char *buffer, int length );
	virtual int _RemoteRecv( char *buffer, int length );
	virtual int _LocalSend( char *buffer, int length );
	virtual int _RemoteSend( char *buffer, int length );
	/* @brief 获取连接客户端 信息
	*/
	void _GetClientInfo( );
	/** @brief 返回socks的请求代码
	* 如果nErrCode = 0, 那么addr, port应该保含对应的地址和端口,nErrCode != 0 时addr, port无所谓.
	*/
	BOOL _MakeSocksRspV5( UINT nErrCode , u_long addr, u_short port /** 网络节字 */ );
	BOOL _MakeSocksRspV4( UINT nErrCode , u_long addr, u_short port /** 网络节字 */ );
};