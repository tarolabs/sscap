#pragma once

/*!
 * @file SSClient.h
 * @date 2015/10/19 10:31
 *
 * @brief shadowsocks的相关操作类
 *
 * Shadowsocks protocol
 * https://shadowsocks.org/en/spec/protocol.html
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
/*
 *
 * SOCKS UDP Request
 * Socks Client发给SSCAP的UDP结构是这样的.
 * +----+------+------+----------+----------+----------+
 * |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
 * +----+------+------+----------+----------+----------+
 * | 2  |  1   |  1   | Variable |    2     | Variable |
 * +----+------+------+----------+----------+----------+
 *
 * SOCKS5 UDP Response
 * SSCap发给Socks Client的UDP结构是这样的.
 * +----+------+------+----------+----------+----------+
 * |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
 * +----+------+------+----------+----------+----------+
 * | 2  |  1   |  1   | Variable |    2     | Variable |
 * +----+------+------+----------+----------+----------+
 *
 * shadowsocks UDP Request (before encrypted)
 * SSCap发给Shadowsocks 的UDP结构是这样的.
 * +------+----------+----------+----------+
 * | ATYP | DST.ADDR | DST.PORT |   DATA   |
 * +------+----------+----------+----------+
 * |  1   | Variable |    2     | Variable |
 * +------+----------+----------+----------+
 *
 * shadowsocks UDP Response (before encrypted)
 * Shadowsocks 发回给SSCap的UDP结构是这样的.
 * +------+----------+----------+----------+
 * | ATYP | DST.ADDR | DST.PORT |   DATA   |
 * +------+----------+----------+----------+
 * |  1   | Variable |    2     | Variable |
 * +------+----------+----------+----------+
 *
 * shadowsocks UDP Request and Response (after encrypted)
 * SSCap和Shadowsocks服务器交互时加密后的数据是这样的
 * 第一个数据包会包含IV,其它的包不含 
 * +-------+--------------+
 * |   IV  |    PAYLOAD   |
 * +-------+--------------+
 * | Fixed |   Variable   |
 * +-------+--------------+
 *
 * SSCap与Shadowsocks服务器交互的TCP数据结构
 * 只有第一个包会有这个头. 之后的包不会有这个头.
 * +------+----------+----------+----------+
 * | ATYP | DST.ADDR | DST.PORT |   DATA   |
 * +------+----------+----------+----------+
 * |  1   | Variable |    2     | Variable |
 * +------+----------+----------+----------+
 *
 * tcp包加密后的结构是
 * 第一个数据包会包含IV,其它的包不含 
 * +-------+--------------+
 * |   IV  |    PAYLOAD   |
 * +-------+--------------+
 * | Fixed |   Variable   |
 * +-------+--------------+
 */
class CSocksClient;
#include "BaseDef.h"
#include "Encypter.h"
#define MIN_CHECK_SPEED_TIME 0.005

class CSSCLient: public CSocksClient
{
public:
	CSSCLient( SOCKET s ,const char *username =NULL  /* proxy user */, const char *password = NULL/* proxy pass */);
	virtual ~CSSCLient();

	/** @brief 处理socks 5请求
	*/
	virtual BOOL Start();

	/** @brief 取得当前节点信息
	*/
	CSSNodeInfo *GetNodeInfo()
	{
		return pNodeInfo;
	}

	/** @brief 用户希望主动中止当前代理转发连接
	*/
	void SetUserBreaking();
protected:
	CSSNodeInfo *pNodeInfo;
	CCryptor *pCryptor;

	//double nSentKB;// 已发送KB
	//double nRecvKB; // 已接收KB
	//double nSentSeconds; // 发送用时(秒)
	//double nRecvSeconds; // 接收用时(秒)
	//unsigned long nUpdateTimes;

	BOOL bIsCipher;	// 是密文否, 与privoxy交互的数据是明文
	//string strPacBodyCache;	// 本地PAC文件内容缓存.

	BOOL bUserBreaking;		// 用户主动中止当前连接
protected:
	/** @brief 处理socks udp 连接请求
	*/
	virtual BOOL ProcessSocksUdpConnection();

	/** @brief 执行完ProcessSocksTcpConnection之后的操作
	*/
	virtual BOOL AfterProcessSocksTcpConnection();
	/** @brief 执行完ProcessSocks5UdpConnection之后的操作
	*/
	virtual BOOL AfterProcessSocksUdpConnection();
	/** @brief 处理其它请求
	* 
	* @returns
	*	0: 失败, 调用者可以直接返回了.
	*	1: 成功, 但是调用者可以直接返回不需要接着处理MainLoop了
	*	2: 成功, 调用者需要接着去处理下边的MainLoop
	*/
	virtual int ProcessOtherRequest(int nUnrecognizedByte);
	/** @brief 执行数据传递的loop
	*/
	virtual BOOL MainLoop();
	BOOL UDPMainLoop();
	BOOL TCPMainLoop();
protected:
	/** @brief 执行shadowsocks的发送操作
	* 分为发给远程SS服务器和发给SOCKS CLIENT
	* SS CLIENT -> SS SERVER :　需要执行加密过程
	* SS SERVER -> SS CLIENT :  需要执行解密过程
	*/
	int SSSend( SOCKET s, char *buffer, int length  ,int timeout );
	int SSSendTo( SOCKET s, char *buffer, int length  ,int timeout );

	/** @brief 执行shadowsocks的接收操作
	* 分为接收远程SS服务器和接收SOCKS CLIENT
	* SS CLIENT -> SS SERVER :　需要执行加密过程
	* SS SERVER -> SS CLIENT :  需要执行解密过程
	*/
	int SSRecv( SOCKET s,  char *buffer, int length  ,int timeout );
	int SSRecvFrom( SOCKET s,  char *buffer, int length  ,int timeout );

	BOOL IsRemoteSocket( SOCKET s )
	{
		return s == hRemoteSocket ? TRUE: FALSE;
	}
	BOOL IsLocalSocket( SOCKET s )
	{
		return s == hSocksSocket ? TRUE: FALSE;
	}
	/** @brief 是不是UDP CLIENT SOCKET , 等待SOCKS CLIENT发送数据的UDP SOCKET
	*/
	BOOL IsUdpClientSocket( SOCKET s )
	{
		return s == udpSocket ? TRUE : FALSE;
	}
	/** @brief 连接到ss 服务器
	* 
	* @param bUdp: 是udp 连接
	* @param bNonBlocking: 非阻塞
	*/
	virtual BOOL ConnectToSSServer( BOOL bUdp = FALSE, BOOL bNonBlocking = TRUE ,SOCKADDR_IN *pSockAddrIn = NULL );
	/** @brief  发送SS协议
	* https://shadowsocks.org/en/spec/protocol.html
	* +--------------+---------------------+------------------+----------+
	* | Address Type | Destination Address | Destination Port |   Data   |
	* +--------------+---------------------+------------------+----------+
	* |      1       |       Variable      |         2        | Variable |
	* +--------------+---------------------+------------------+----------+
	*/
	BOOL SendSSProtocol();
	/** @brief 解析代理的地址. 有可能是一个域名, 则解析成IP
	* 如果解析成功, 代理的代址信息通过sockAddr返回
	*/
	BOOL TranslateSSServerAddress( SOCKADDR_IN &sockAddr , char *szAddress, u_short port );

	/** @brief 在SS服务器发过来的UDP包前增加3个字节
	* 收到的是:
	* +------+----------+----------+----------+
	* | ATYP | DST.ADDR | DST.PORT |   DATA   |
	* +------+----------+----------+----------+
	* |  1   | Variable |    2     | Variable |
	* +------+----------+----------+----------+
	* 调整为:
	* +----+------+------+----------+----------+----------+
	* |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
	* +----+------+------+----------+----------+----------+
	* | 2  |  1   |  1   | Variable |    2     | Variable |
	* +----+------+------+----------+----------+----------+
	*
	* @param buffer: 需要修改的BUFFER
	* @param DataLen: buffer中的数据长度
	* @param BufLen: Buffer总长度
	*/
	BOOL Add3BytesForUdpHeader( char *buffer, int DataLen, int BufLen );
	/** @brief 在Socks Client发过来的UDP包前减少头3个字节
	* 收到的是:
	* +----+------+------+----------+----------+----------+
	* |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
	* +----+------+------+----------+----------+----------+
	* | 2  |  1   |  1   | Variable |    2     | Variable |
	* +----+------+------+----------+----------+----------+
	* 调整为:
	* +------+----------+----------+----------+
	* | ATYP | DST.ADDR | DST.PORT |   DATA   |
	* +------+----------+----------+----------+
	* |  1   | Variable |    2     | Variable |
	* +------+----------+----------+----------+
	*
	* @param buffer: 需要修改的BUFFER
	* @param DataLen: buffer中的数据长度
	* @param BufLen: Buffer总长度
	*/
	BOOL Remove3BytesForUdpHeader( char *bufer, int DataLen );
	/** @brief 返回PAC文件内容
	*/
	BOOL RespPacFileBody();
	BOOL ConnectToPrivoxy();
};