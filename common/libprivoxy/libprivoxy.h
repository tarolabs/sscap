#ifndef _LIBPRIVOXY_EXPORT_H
#define _LIBPRIVOXY_EXPORT_H

#ifdef LIBPRIVOXY_EXPORTS
#define LIBPRIVOXY_API __declspec(dllexport)
#else
#define LIBPRIVOXY_API __declspec(dllimport)
#endif

// http/https
#define HTTP	3
// SOCKS 4/4A
#define SOCKS4  4
// SOCKS 5
#define SOCKS5  5

/** @brief 启动privoxy, 可以反复调用, 如果服务本身已经启动, 再次调用只会更新privoxy服务的配置信息, 
* 例如更改forwarding socks5 ip,forwarding port, privoxy service port等信息
*
* @param listen_port: PRIVOXY监听端口, 0自动搜索本机可用端口
* @param forward_socks_5_ip: 转发到的SOCKS 5 IP
* @param forward_socks5_port: 转发到的SOCKS 5 PORT
* @param pac_file: pac 文件完整路径
*
* @return 0 启动成功, 否则启动失败
*/
extern "C" LIBPRIVOXY_API int __stdcall start_privoxy( 
	int proxy_type, /* HTTP,SOCKS4,SOCKS5 */
	const char *forward_socks_5_ip, 
	int forward_socks5_port,
	const char *username = NULL,
	const char *password = NULL,
	const char *listen_addr = "127.0.0.1",
	int listen_port = 0 ,
	const char *pac_file = "unset"
	);
/** @brief 停止privoxy
*/
extern "C" LIBPRIVOXY_API void __stdcall stop_privoxy();
/** 获得privoxy工作在哪个端口. 一般用于listen_port=0的情况下, 想知道privoxy工作在哪个端口之下
*/
extern "C" LIBPRIVOXY_API int __stdcall get_privoxy_port();

/** @brief 计算cmd的hash ,用于新加入新的cmd config时.
*/
extern "C" LIBPRIVOXY_API unsigned int __stdcall calc_cmd_hash( const char *cmd );

/** @biref 取得PAC地址
*/
extern "C" LIBPRIVOXY_API BOOL __stdcall GetPrivoxyPacUrl( char *buf, int buflen );

/** @brief 取得PRIVOXY地址
*/
extern "C" LIBPRIVOXY_API BOOL __stdcall GetPrivoxyProxyAddr( char *buf, int buflen );

extern "C" LIBPRIVOXY_API int __stdcall is_privoxy_started();
#endif