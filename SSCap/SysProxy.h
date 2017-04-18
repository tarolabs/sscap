#pragma once

/** @brief 设置系统代理 (IE代理)
*
* @param lpszProxyServer 代理服务器地址, 为NULL表示不设置
* @param lpszPacUrl pac地址,为NULL表示不设置
* @param lpszByPass 设置By pass( 跳过代理服务器的列表 ), 为NULL的时候不设置
* @param bLocalAddrNotUseProxy 对应于IE代理设置中的: 对于本地地址不使用代理服务器
*
* @note 如果指定了lpszByPass, bLocalAddrNotUseProxy将被忽略
*/
BOOL SetSystemProxy( 
	LPCTSTR lpszProxyServer, 
	LPCTSTR lpszPacUrl ,
	LPCTSTR lpszByPass = NULL,
	BOOL bLocalAddrNotUseProxy = FALSE );

/** @brief 禁用系统代理
*/
BOOL DisableSystemProxy();

/** @brief 获取IE的代理设置,( 系统代理)
* 
* @param bUseAutoDetect[out] 自动检测设置属性
* @param bUseAutoConfigUrl[out] Buffer使用自动配置脚本属性
* @param lpAutoConfigUrl[out] 自动配置脚本URL地址
* @param nAutoConfigUrlLe[out] 自动配置脚本URL Buffer长度
* @param bUseProxyServer[out] 使用代理服务器地址
* @param lpProxyServer[out] 代理服务器地址
* @param nProxyServerLen[out] 代理服务器地址BUFFER 长度
* @param lpByPass[out] By pass字符串
* @param nByPassLen[out] By pass字符串BUFFER长度

* @return 
* TRUE 成功, 返回值在以上参数中
* FALSE 失败.
*
* https://msdn.microsoft.com/en-us/library/aa385145.aspx
*/
BOOL GetSystemProxyInfo( 
	BOOL &bUseAutoDetect, // 自动检测设置属性
	BOOL &bUseAutoConfigUrl, // 使用自动配置脚本属性
	LPTSTR lpAutoConfigUrl,	// 自动配置脚本URL地址
	int nAutoConfigUrlLen,		// 自动配置脚本URL Buffer长度
	BOOL &bUseProxyServer,		// 使用代理服务器地址
	LPTSTR lpProxyServer,		// 代理服务器地址
	int nProxyServerLen,		// 代理服务器地址BUFFER 长度
	LPTSTR lpByPass,			// By pass字符串
	int nByPassLen				// By pass字符串BUFFER长度
	);

/** @brief 是否设置了系统代理
*/
BOOL IsSetSystemProxy();