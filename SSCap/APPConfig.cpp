#include "stdafx.h"
#include "SSCap.h"
#include "APPConfig.h"
#include <assert.h>
#include "Utils.h"

bool CAPPConfig::is_valid_config = false;				// 是否存在有效的信息
bool CAPPConfig::hidden_ip_port = false;				// 隐藏代理的IP,PORT
bool CAPPConfig::always_enable_sysproxy = false;		// 程序启动时总是启用系统代理
wstring CAPPConfig::help_url = SSCAP_DEFAULT_WEBSITE_GITHUB;		// 帮助URL
wstring CAPPConfig::contactme_url = SSCAP_DEFAULT_WEBSITE_GITHUB;	// 联系我URL
wstring CAPPConfig::feedback_url = SSCAP_DEFAULT_WEBSITE_GITHUB;	// 反馈URL
wstring CAPPConfig::softname = SSCAP_NAME;				// 软件名称
bool CAPPConfig::disable_sysproxy_function = false;		// 禁用系统代理模块功能
wstring CAPPConfig::strGoogleplus = _T("https://plus.google.com/+Sockscap64bit");	// Google+地址
wstring CAPPConfig::strTwitter = _T("https://twitter.com/sockscap64");		// twitter
wstring CAPPConfig::strOtherInfo = _T("tarolabs1@gmail.com");	// other infomation
wstring CAPPConfig::strWebsite = SSCAP_DEFAULT_WEBSITE_GITHUB;		// 网站地址
bool CAPPConfig::disable_add_nodes = false; // 禁止添加节点
// // 获取新版本的URL地址. 对应的地址是一个TXT文件. 格式是版本号|更新内容
wstring CAPPConfig::newversion_url = NEWVERSION_URL;
wstring CAPPConfig::newverion_download_url_1 = SSCAP_DEFAULT_WEBSITE_GITHUB_RELEASE;

CAPPConfig::CAPPConfig()
{

}

CAPPConfig::~CAPPConfig()
{

}

/** @brief 通过配置文件加载信息.
*/
BOOL CAPPConfig::create(  LPCTSTR lpszDir )
{
	return TRUE;
}

/** @brief Config信息是否有效
*/
BOOL CAPPConfig::IsValidConfig()
{
	return is_valid_config;
}

/** @brief 是否需要隐藏代理的IP,PORT
*/
bool CAPPConfig::IsHiddenIPPort()
{
	return hidden_ip_port;
}

/** @brief 获得帮助URL
*/
wstring &CAPPConfig::GetHelpUrl()
{
	return help_url;
}
	
/** @brief 获得联系我URL 
*/
wstring &CAPPConfig::GtetContactmeUrl()
{
	return contactme_url;
}
	
/** @brief 获得反馈URL
*/
wstring &CAPPConfig::GetFeedbackUrl()
{
	return feedback_url;
}

/** @brief 获得软件名称
*/
wstring &CAPPConfig::GetSoftName()
{
	return softname;
}
bool CAPPConfig::AlwaysEnableSysProxy()
{
	return always_enable_sysproxy;
}

/** @brief 是否禁用了系统代理模块功能
	*/
bool CAPPConfig::IsDisableSysProxyFunction()
{
	return disable_sysproxy_function;
}