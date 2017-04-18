#pragma once

/*!
 * @file SocketWrapper.h
 * @date 2015/05/14 17:29
 *
 * @brief SOCKET API·â×°
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

/** @brief "recv" with handling for non-blocking sockets (blocks until data is available)
* @param[in]		hSock			Socket for calling "recv"
* @param[in]		caBuf			Buffer for calling "recv"
* @param[in]		length			buffer length of caBuf.
* @param[in]		nTimeout		timeout of recv
* @return							Amount of bytes received, SOCKET_ERROR in case of an error (use WSAGetLastError to get error info)
*/
int __stdcall Recv(SOCKET &hSock, char* caBuf, int length, int nTimeout /* seconds */ );
int __stdcall RecvFrom(SOCKET &hSock, char* caBuf, int length, struct sockaddr* AddrFrom, int *AddrLen, int nTimeout /* seconds */ );

/** @brief "Send" with handling for non-blocking sockets (blocks until data is available)
* @param[in]		hSock			Socket for calling "Send"
* @param[in]		data			Buffer for calling "Send"
* @param[in]		length			buffer length of caBuf.
* @param[in]		nTimeout		timeout of recv
* @return							Amount of bytes received, SOCKET_ERROR in case of an error (use WSAGetLastError to get error info)
*/
int __stdcall Send(SOCKET &hSock, void *data,int length, int timeout );
int __stdcall SendTo(SOCKET &hSock, void *data,int length,const struct sockaddr *To, int ToLen, int timeout );