/*!
 * @date 2016/02/29 16:06
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
 * @note
*/
#include <string>
#include <json/json.h>
#include <json/value.h>

using namespace std;

class CAPPConfig
{
public:
	CAPPConfig();
	virtual ~CAPPConfig();

public:
	/** @brief 通过配置文件加载信息.
	*/
	static BOOL create(  LPCTSTR lpszDir );

	/** @brief Config信息是否有效
	*/
	static BOOL IsValidConfig();

	/** @brief 是否需要隐藏代理的IP,PORT
	*/
	static bool IsHiddenIPPort();

	static bool AlwaysEnableSysProxy();
	/** @brief 获得帮助URL
	*/
	static wstring &GetHelpUrl();
	
	/** @brief 获得联系我URL 
	*/
	static wstring &GtetContactmeUrl();
	
	/** @brief 获得反馈URL
	*/
	static wstring &GetFeedbackUrl();

	/** @brief 获得软件名称
	*/
	static wstring &GetSoftName();

	/** @brief 是否禁用了系统代理模块功能
	*/
	static bool IsDisableSysProxyFunction();
	static wstring &GetGoogleplus(){
		return strGoogleplus;
	}
	static wstring &GetTwitter(){
		return strTwitter;
	}
	static wstring &GetOtherinfo(){
		return strOtherInfo;
	}
	static wstring &GetWebsite(){
		return strWebsite;
	}
	static bool IsDisableAddNodes(){
		return disable_add_nodes;
	}
	static wstring &GetNewVersionUrl(){
		return newversion_url;
	}
	static wstring &GetNewVersionDownloadUrl_1(){
		return newverion_download_url_1;
	}
protected:
	static bool is_valid_config;				// 是否存在有效的配置信息
	static bool hidden_ip_port;				// 隐藏代理的IP,PORT
	static bool always_enable_sysproxy;		// 程序启动时总是启用系统代理
	static wstring help_url;				// 帮助URL
	static wstring contactme_url;			// 联系我URL
	static wstring feedback_url;			// 反馈URL
	static wstring softname;				// 软件名称
	static bool disable_sysproxy_function; // 禁用系统代理模块功能
	static wstring strGoogleplus;	// Google+地址
	static wstring strTwitter;		// twitter
	static wstring strOtherInfo;	// other infomation
	static wstring strWebsite;		// 网站地址
	static bool disable_add_nodes;	// 禁止添加节点
	static wstring newversion_url; // 获取新版本的URL地址. 对应的地址是一个TXT文件. 格式是版本号|更新内容
	static wstring newverion_download_url_1; // 新版本下载地址1
	//static wstring newverion_download_url_2; // 新版本下载地址2
};