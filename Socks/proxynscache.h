#pragma once

/*!
 * @file proxynscache.h
 * @date 2015/05/27 8:48
 *
 * @brief 主要用于用户设置了一个域名做为代理地址的情况下, 需要帮用户解析出代理域名的真实IP, 然后CACHE起来.
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

#include <string>
#include <map>
using namespace std;

#define PROXY_NS_CACHE_VALID_TIME 172800 ///< 缓存有效时间, 默认48小时

/** @brief 代理域名的NS记录缓存
*/
struct _PROXY_NS_CACHE_
{
	string szRealIPAddress; ///< 真实IP地址.
	time_t lastResolveTime;   ///< 最后解析出IP的时间
};


/** @brief 加载代理NS记录的缓存
* 
* @param szWorkingDir 工作目录
*/
BOOL LoadProxyNSCache( TCHAR *szWorkingDir );
/** @brief 将代理的NS缓存记录保存到文件
*/
void SaveProxyNSCache();
/** @brief 查找某个域名的NS记录缓存是否存在
*
* @param domain 要查找的域名
* @return 
* - TRUE: 存在
* - FALSE: 不存在
*/
BOOL IsProxyNSRecordExist( string domain );
/** @brief 获得一个域名的IP地址.
*
* @param[in] domain 域名
* @param[out] ip 域名的IP
* @param[out] lastResolveTime 域名最后解析出IP的时间. 过期的IP调用重新解析.( 调用者负责 )
* @return 
* - TRUE: 成功,  ip中返回了获得的IP
* - FALSE: 失败.
*/
BOOL GetProxyNSIPForDomain( string domain, string &ip );
/** @brief 将一个代理的NS记录压入缓存
* 如果存在就会替换. 不存在就直接插入.
*/
void PushProxyNSCache( string domain , string ip);