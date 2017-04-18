#pragma once

#include <string>
#include <vector>
#include "mymutex.h"
#include "SpeedAnalytics.h"

using namespace std;

#define DEFAULT_SOCKS_PORT 1080
#define DEFAULT_RECONNECT_TIMES 3
// 默认的本地PAC地址.
//#define DEFAULT_LOCAL_PAC_HSOT "http://127.0.0.1:%d"
#ifdef USE_LIBPRIVOXY
#define DEFAULT_LOCAL_PAC_URL "echo-pac"
#else 
#define DEFAULT_LOCAL_PAC_URL "sscap_pac"
#endif
#define DEFAULT_LOCAL_PAC_FILENAME _T("/config/pac.txt")
//#define DEFAULT_LOCAL_USERPAC_FILENAME _T("/config/userpac.txt")
#define DEFAULT_TESTING_URL "http://global.bing.com"
#define LOCAL_LOOPBACK_IP_W _T("127.0.0.1")
#define LOCAL_LOOPBACK_IP "127.0.0.1"

#define CHARGETYPE_LOCAL			0	// local
#define CHARGETYPE_ONLINE_ECON		1	// online Economic 
#define CHARGETYPE_ONLINE_PRO		2	// online pro
#define CHARGETYPE_ONLINE_VIP		3

class CSSNodeInfo : public CSpeedAnalytics
{
public:
	CSSNodeInfo()
	{
		id = 0;
		server_port = 0;
		enable = true;
		Latency = 0;
		//Speed = 0;
		Upload_traffic = 0;
		Download_traffic = 0;
		ConnectedTimes = 0;
		FailureTimes = 0;
		Failure_rate = 0;
		nConnections = 0;
		lastStatus = -1 ;
		charge_type = CHARGETYPE_LOCAL;
	}
	CSSNodeInfo( int nId )
	{
		id = nId;
		server_port = 0;
		enable = true;
		Latency = 0;
		//Speed = 0;
		Upload_traffic = 0;
		Download_traffic = 0;
		ConnectedTimes = 0;
		FailureTimes = 0;
		Failure_rate = 0;
		nConnections = 0;
		lastStatus = -1;
		charge_type = CHARGETYPE_LOCAL;
	}

	CSSNodeInfo( const CSSNodeInfo &node )
	{
		id = node.id;
		server = node.server;
		server_port = node.server_port;
		password = node.password;
		method = node.method;
		remarks = node.remarks;
		enable = node.enable;

		Latency = node.Latency;
		//Speed = node.Speed;
		Upload_traffic = node.Upload_traffic;
		Download_traffic = node.Download_traffic;
		ConnectedTimes = node.ConnectedTimes;
		FailureTimes = node.FailureTimes;
		Failure_rate = node.Failure_rate;

		nConnections = node.nConnections;
		lastStatus = node.lastStatus;

		charge_type = node.charge_type;
	}

	void operator = ( CSSNodeInfo &node )
	{
		id = node.id;
		server = node.server;
		server_port = node.server_port;
		password = node.password;
		method = node.method;
		remarks = node.remarks;
		enable = node.enable;

		Latency = node.Latency;
		//Speed = node.Speed;
		Upload_traffic = node.Upload_traffic;
		Download_traffic = node.Download_traffic;
		ConnectedTimes = node.ConnectedTimes;
		FailureTimes = node.FailureTimes;
		Failure_rate = node.Failure_rate;

		nConnections = node.nConnections;
		lastStatus = node.lastStatus;

		charge_type = node.charge_type;
	}
public:
	//////////////////////////////////////////////////////////////////////////
	/** @brief 更新节点统计信息
	*/
	void UpdateLatency( unsigned int Latency /** ms */);
	//void UpdateSpeed( unsigned int Speed /* bytes per sec */ );
	void UpdateUpload_traffic(unsigned int Upload_traffic /* bytes */);
	void UpdateDownload_traffic(unsigned int Download_traffic /* bytes */);
	void UpdateConnectedTimes( unsigned int ConnectedTimes /* 每次增加就调用*/);
	void UpdateFailureTimes( unsigned int FailureTimes /* 每次失败就调用*/);
	void UpdateConnections( unsigned int type /* 1: 加一个连接, 2: 减一个连接*/);
	void SetLastStatus( int status );

	BOOL IsOnlineNode();
	//////////////////////////////////////////////////////////////////////////
	string ToSSlink();
	string ToPlainNodeInfo();
	string ToJson();
public:
	unsigned int id;	// 节点ID. id不重复.添加新的节点, ID从最后一个ID顺序排下去.
	string server;
	u_short server_port;
	string password;
	string method;
	string remarks;
	bool enable;
	string iplocation;		// IP所在位置.

public:
	// ss节点的动态计算信息
	unsigned int Latency;  // ms
	//unsigned int Speed;		// kb/s
	double Upload_traffic;	// bytes
	double Download_traffic;	// bytes
	unsigned int ConnectedTimes;	
	unsigned int FailureTimes;
	unsigned int Failure_rate; // %

	// 连接数
	unsigned int nConnections;

	int lastStatus; /// 最后测试的状态, 0: 不可用, 1: 可用, -1: 未知

	int charge_type; // 付费类型

	CMyMutex mutexNode;
};

class CSSConfigInfo 
{
public:
	CSSConfigInfo()
	{
		runAtStartup = false;
		startInSystray = false;
		idInUse = 0;
		random = false;
		global = false;
		enable = false;
		shareOverLan = false;
		auto_disconnect_connection = false;
		localPort = DEFAULT_SOCKS_PORT;
		useOnlinePac = false;
		reconnectTimes = DEFAULT_RECONNECT_TIMES;
		//randomAlgorithm = 2; // 随机
		autoban = false;
		nNextNodeId = 0;
		localPacFileFullName = wstring(_T(".")) + wstring( DEFAULT_LOCAL_PAC_FILENAME );
		//localUserPacFileFullName = wstring(_T(".")) + wstring( DEFAULT_LOCAL_USERPAC_FILENAME );
		isPrivoxyRunned = false;
		testing_url = string( DEFAULT_TESTING_URL );
		m_HotKeyForAddFromQRCode = 0;
		bLoadSaveNodes = true;
	}
	~CSSConfigInfo()
	{
		int i = 0;
		for( i = 0 ; i < (int)ssNodeInfo.size(); i ++ )
		{
			CSSNodeInfo *pNode = ssNodeInfo[i];

			if( pNode )
				delete pNode;
		}
		ssNodeInfo.clear();

		for( i = 0 ; i <(int) ssDeleteNodeInfo.size(); i ++ )
		{
			CSSNodeInfo *pNode = ssDeleteNodeInfo[i];

			if( pNode )
				delete pNode;
		}
		ssDeleteNodeInfo.clear();
	}
public:
	int GetActiveProxyId(){
		return idInUse;
	}
	CSSNodeInfo *GetActiveNodeInfo()
	{
		CMutexParser p( &mutex );

		if( ssNodeInfo.size() <= 0 ) 
			return NULL;

		for( int i = 0 ; i < (int)ssNodeInfo.size(); i ++ )
		{
			if( ssNodeInfo[i]->id == idInUse )
				return ssNodeInfo[i];
		}

		return NULL;
	}
	void ActiveSSNode( int idx )
	{
		CMutexParser p( &mutex );

		if( ssNodeInfo.size() <= 0 ) 
			return;

		for( int i = 0 ; i < (int)ssNodeInfo.size(); i ++ )
		{
			// 找到这个ID,才设置它为使用.
			if( ssNodeInfo[i]->id == idx )
			{
				idInUse = idx;
				break;
			}
		}
	}
	/** @biref 按顺序取第i个节点
	*/
	CSSNodeInfo *GetNodeByIndex( int i );

	/** @brief 返回某个id指定的节点
	*/
	CSSNodeInfo *GetNodeById( int i );

	/** @brief 编辑一个节点,参数为空则不修改
	*/
	CSSNodeInfo *EditNode( int i ,string server,u_short server_port,string password,string method,string remarks,bool enable );
	/** @brief 增加一个SS服务器节点
	* 返回新插入节点的index
	*/
	CSSNodeInfo *AddNode( CSSNodeInfo *pNode );
	/** @brief 增加一个SS服务器节点
	* 返回新插入节点的index
	*/
	CSSNodeInfo *AddNode(int charge_type, string server,u_short server_port,string password,string method,string remarks,bool enable );
	/** @brief 通过ss link增加一个SS服务器节点
	* 返回新插入节点的index
	* 返回-1插入未成功
	*
	* 一个SS LINK格式是:
	* ss://YWVzLTI1Ni1jZmI6YXNkMTQ3QGxhbGFhaS1tb3V5dWVtb3VyaS5teWFsYXVkYS5jbjoxMDcwMA==
	* 由BASE64编码, 解码后是:
	* ss://aes-256-cfb:asd147@lalaai-mouyuemouri.myalauda.cn:10700
	* 格式为: ss://加密方式:密码@服务器:端口
	*/
	CSSNodeInfo *AddNodeFromLink( string link );
	/** @brief 删除一个节点
	*/
	BOOL DeleteNode( int i );
	/** @brief 删除所有节点 
	* @param bExcludeOnline: 是否排除本地
	*/
	void DeleteAllNodes( BOOL bExcludeLocal = TRUE );
		
	/** @brief 启用/禁用一个NODE
	*/
	void EnableNode( int i , bool bEnable );
	/** @brief 获取PAC 的URL 
	*/
	wstring GetPacUrl( );
	/** @brief 获得系统代理全局模式的代理地址
	*/
	wstring GetSysGlobalProxyAddr();
	int GetNodeSize()
	{
		return (int)ssNodeInfo.size();
	}
	void ClearAllNodesTrafficData();
	/** @brief 设置add from qrcode 的快捷键
	*/
	void SetHotKeyForAddQRCode( WORD wModifiers,WORD wVirtualKeyCode ,wstring hotkey );

	/** @brief 获得add from qrcode 的快捷键
	*/
	BOOL GetHotKeyForAddQRCode( WORD &wModifiers,WORD &wVirtualKeyCode );
public:
	CMyMutex mutex;
	vector<CSSNodeInfo *> ssNodeInfo;			// ss节点信息
	vector<CSSNodeInfo *> ssDeleteNodeInfo;		// 被用户删除的节点信息, 在程序退出时再释放

	bool runAtStartup;	///< 开机自启
	bool startInSystray; ///< 启动后缩小到系统托盘.
	int idInUse;  ///< 当前使用中的节点,CSSNodeInfo的id值.
	bool random;  ///< 随机挑选节点( 服务器均衡 )
	bool global; ///< 系统代理Global模式 true: 全局模式, false: pac模式
	bool enable; ///< 启用系统代理
	bool shareOverLan; ///< 允许其它人连接
	bool auto_disconnect_connection; ///< 切换节点时自动断开连接
	u_short localPort; ///< 本地socks5服务端口
	string localSocksUser; ///< 本地socks5服务帐号
	string localSocksPass; ///< 本地socks5服务密码
	wstring pacUrl; ///< 在线pac的url.
	bool useOnlinePac; ///< 使用在线pac
	int reconnectTimes; ///< 重连次数
	//int randomAlgorithm; ///< radmon = true ( 服务器均衡 ) 的情况下的算法.
	bool autoban; ///< 自动禁止出错的服务器节点.
	wstring localPacFileFullName;  // 本地pac文件全名
	//wstring localUserPacFileFullName;  // 本地用户pac文件全名
	bool isPrivoxyRunned;	// privoxy是否已经启动,若未启动则不能使用系统代理相关的功能.
	string testing_url;	// 测试URL 
	unsigned int m_HotKeyForAddFromQRCode;	///< add from qrcode的快捷方式
	wstring strHotKey;

	bool bLoadSaveNodes;	// 是否加载或者保存节点信息

	wstring strExeMainWorkingDirectory; /// EXE程序的工作目录
public:
	/** @brief 检验idInUse是否有效. 是不是给了一个无效的ID呢. 如果是则自动给0
	*/
	void VerifyInUseId();
	/** @获得下一个节点的ID号
	*
	* @param bInrement 自增
	*/
	unsigned int GetNextNodeId( BOOL bInrement = TRUE );

	unsigned int nNextNodeId; /// 节点ID计数器. 每新增一个就加1, 节点的ID就是从这个值取到的. 某节点被删除了,它的ID不会被再次使用.
};

void InitializeWorkingDirectory( LPCTSTR lpszWorkingDir );
/** @brief 加载ss服务节点
*/
BOOL LoadShadowsocksServer( );
void SaveShadowsocksServer( );

CSSConfigInfo *GetConfigInfo();
//vector<CSSNodeInfo> *GetSSNodeInfo();
/** @brief 验证本地socks服务的帐号密码
*/
bool VerifyLocalSocksUserPass( string user, string pass );
/** @brief 获得当前的ss服务器节点, 如果未启动服务器均衡的话,则是当前选中的,如果启动了服务器
* 均衡,则需要根据规则选取
*/
//CSSNodeInfo *GetCurrentSSServer();