#pragma once

#define SSCAP_PRIVOXY_LOCAL_IP "127.0.0.1"

/** @brief 运行privoxy进程
*/
BOOL RunPrivoxy( );
/** @biref 获取privoxy进程工作端口
*/
unsigned short GetPrivoxyListenPort();
BOOL IsPrivoxyStarted();