#include "stdheader.h"
#include "BaseDef.h"
#include "mymutex.h"
#include "SSLoader.h"
#include "Utils.h"
#include "privoxy.h"
#include "tarobase64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CSSConfigInfo gConfigInfo;
static CMyMutex mutexConfig;
//static TCHAR szMainAppWorkingDirectory[1000] = {0};

void InitializeWorkingDirectory( LPCTSTR lpszWorkingDir )
{
	assert( lpszWorkingDir != NULL );
	//_tcsncpy_s( szMainAppWorkingDirectory, 1000,lpszWorkingDir, 1000-1 );

	gConfigInfo.strExeMainWorkingDirectory = wstring( lpszWorkingDir );

	return;
}
/** @brief 加载ss服务节点
*/
BOOL LoadShadowsocksServer( )
{
	//assert( szMainAppWorkingDirectory[0] != 0 );
	//if( szMainAppWorkingDirectory[0] == 0 ) return FALSE;

	if( gConfigInfo.strExeMainWorkingDirectory.empty() ) return FALSE;

	// 初始化本地PAC文件名
	//char szPacFileFullName[1024] = {0};
	//sprintf_s( szPacFileFullName, 1024, "%s%s", lm_u2a_s(szMainAppWorkingDirectory),DEFAULT_LOCAL_PAC_FILENAME);
	gConfigInfo.localPacFileFullName = gConfigInfo.strExeMainWorkingDirectory + wstring(DEFAULT_LOCAL_PAC_FILENAME );
	//gConfigInfo.localUserPacFileFullName = gConfigInfo.strExeMainWorkingDirectory  + wstring(DEFAULT_LOCAL_USERPAC_FILENAME );

	CSSLoader loader( &gConfigInfo );
	return loader.LoadSS( gConfigInfo.strExeMainWorkingDirectory.c_str() );
}

void SaveShadowsocksServer( )
{
	//assert( szMainAppWorkingDirectory[0] != 0 );
	//if( szMainAppWorkingDirectory[0] == 0 ) return;

	if( gConfigInfo.strExeMainWorkingDirectory.empty() ) return;

	CSSLoader loader( &gConfigInfo );
	return loader.SaveSS( gConfigInfo.strExeMainWorkingDirectory.c_str() );
}

CSSConfigInfo *GetConfigInfo()
{
	return &gConfigInfo;
}
/*
vector<CSSNodeInfo> *GetSSNodeInfo()
{
	return &gConfigInfo.ssNodeInfo;
}
*/
/** @brief 验证本地socks服务的帐号密码
*/
bool VerifyLocalSocksUserPass( string user, string pass )
{
	CMutexParser p( &mutexConfig );

	return ( user == gConfigInfo.localSocksUser && pass == gConfigInfo.localSocksPass ) ? TRUE: FALSE;
}

/** @brief 获得当前的ss服务器节点, 如果未启动服务器均衡的话,则是当前选中的,如果启动了服务器
* 均衡,则需要根据规则选取
*/
/*
CSSNodeInfo *GetCurrentSSServer()
{
	CMutexParser p( &mutexConfig );
	
	if( gConfigInfo.random )
	{
		// TODO
		// 启动了服务器均衡,则需要根据规则选取

	}
	else 
	{
		if( gConfigInfo.index < 0 || gConfigInfo.index >= gConfigInfo.ssNodeInfo.size() )
			return NULL;

		return &gConfigInfo.ssNodeInfo[ gConfigInfo.index ];
	}
	return NULL;
}
*/
/** @brief 编辑一个节点,参数为空则不修改
*/
CSSNodeInfo *CSSConfigInfo::EditNode( int i ,string server,u_short server_port,string password,string method,string remarks,bool enable )
{
	CMutexParser p( &mutex );

	CSSNodeInfo *pNode = GetNodeById( i );
	if( !pNode ) 
		return NULL;

	if( !server.empty() )
	{
		if( pNode->server  != server )
			pNode->iplocation = string("");

		pNode->server = server;
	}

	if( server_port != 0 )
		pNode->server_port = server_port;

	if( !password.empty() )
		pNode->password = password;

	if( !method.empty() )
		pNode->method = method;

	pNode->remarks = remarks;

	pNode->enable = enable;

	return pNode;
}

/** @brief 增加一个SS服务器节点
* 返回新插入节点的index
*/
CSSNodeInfo *CSSConfigInfo::AddNode( int charge_type,string server,u_short server_port,string password,string method,string remarks,bool enable )
{
	CMutexParser p( &mutex );

	if( server.empty()
		|| server_port == 0 
		|| password.empty()
		|| method.empty() )
		return NULL;

	int id = GetNextNodeId( TRUE );

	CSSNodeInfo *pNode = new CSSNodeInfo( id );
	pNode->server = server;
	pNode->server_port = server_port;
	pNode->password = password;
	pNode->method = method;
	pNode->remarks = remarks;
	pNode->enable = enable;
	pNode->charge_type = charge_type;

	ssNodeInfo.push_back( pNode );

	return pNode;
}
/** @brief 返回某个id指定的节点
*/
CSSNodeInfo *CSSConfigInfo::GetNodeById( int i )
{
	CMutexParser p( &mutex );

	if( ssNodeInfo.size()<= 0 )
		return NULL;
	
	for( int idx = 0 ; idx < ssNodeInfo.size(); idx ++ )
	{
		if( ssNodeInfo[idx]->id == i )
			return ssNodeInfo[idx];
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
/** @biref 按顺序取第i个节点
*/
CSSNodeInfo *CSSConfigInfo::GetNodeByIndex( int i )
{
	CMutexParser p( &mutex );

	int size = (int) ssNodeInfo.size();
	if( size <= 0 || i < 0 || i >= size ) 
		return NULL;

	return ssNodeInfo[i];
}

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
CSSNodeInfo *CSSConfigInfo::AddNodeFromLink( string link )
{
	CMutexParser p( &mutex );

	if( link.empty() ) return NULL;

	vector<string> results;
	SplitStringA(link,string("//"),results, true );

	if( results.size() != 2 ) return NULL;

	int ret_len = 0;
	string ssInfo;

	// 如果找到@字符, 可能未经BASE64 CODE的
	if( results[1].find("@") != string::npos )
	{
		ssInfo = results[1];
	}
	else
	{
		// 否则经过BASE64, 需要DECODE
		string pSSData = base64::decode( results[1].c_str() );
		ret_len = pSSData.length();
		if( !pSSData.empty() && ret_len > 0 )
		{
			ssInfo = string( pSSData );
			//free( pSSData );
		}
	}
	if( !ssInfo.empty() )
	{
		vector<string> part;
		string ip_port;
		string method_password;

		// 找到最后一个@ ( 有可能密码中含有@ )
		int ip_port_begin = ssInfo.find_last_of( "@" );
		if(  ip_port_begin == string::npos ) return NULL;

		method_password = ssInfo.substr( 0, ip_port_begin );
		ip_port = ssInfo.substr( ip_port_begin +1 );

		string part1,part2;

		part1 = method_password;
		part2 = ip_port;

		vector<string> sub1,sub2;
		// method, passowrd
		SplitStringA( part1,string(":"),sub1,true );
		// server ,port
		SplitStringA( part2,string(":"),sub2,true );

		int sub1_size = sub1.size();
		int sub2_size = sub2.size();

		if( sub1_size < 2 || sub2_size != 2 )
			return NULL;

		string ss_server,ss_port,ss_method,ss_password;
		string ss_protocol,ss_obfs; // 为了兼容其它版本的混淆协议解析.


		ss_server = sub2[0];
		ss_port = sub2[1];
		ss_method = sub1[sub1_size-2];
		ss_password = sub1[sub1_size-1];

		return AddNode(
			CHARGETYPE_LOCAL,
			ss_server, // server
			atoi(ss_port.c_str() ),// port
			ss_password, // password
			ss_method,// method
			"",
			true );
	}

	return NULL;
}
/** @brief 增加一个SS服务器节点
* 返回新插入节点的index
*/
CSSNodeInfo *CSSConfigInfo::AddNode( CSSNodeInfo *pNode )
{
	CMutexParser p( &mutex );

	assert( pNode != NULL );
	if( pNode == NULL ) return NULL;

	int id = GetNextNodeId( TRUE );
	pNode->id = id;

	ssNodeInfo.push_back( pNode );

	return pNode;
}

/** @brief 删除所有节点 
	* @param bExcludeOnline: 是否排除本地
	*/
void CSSConfigInfo::DeleteAllNodes( BOOL bExcludeLocal /* = TRUE */ )
{
	CMutexParser p( &mutex );

	if( ssNodeInfo.size() <= 0 )
		return;

	vector<CSSNodeInfo*>::iterator iter = ssNodeInfo.begin();
	for( iter; iter != ssNodeInfo.end(); /* iter ++ */ )
	{
		CSSNodeInfo *pNode = (CSSNodeInfo *)*iter;
		if( pNode && ( pNode->IsOnlineNode() || !bExcludeLocal ) )
		{
			iter = ssNodeInfo.erase( iter );

			ssDeleteNodeInfo.push_back( pNode );

			if( ssNodeInfo.size() <= 0 ) break;

			continue;
		}

		iter ++;
	}
}

/** @brief 删除一个节点
*/
BOOL CSSConfigInfo::DeleteNode( int i )
{
	CMutexParser p( &mutex );

	if( ssNodeInfo.size() <= 0 )
		return TRUE;

	vector<CSSNodeInfo*>::iterator iter = ssNodeInfo.begin();
	for( iter; iter != ssNodeInfo.end(); iter ++ )
	{
		CSSNodeInfo *pNode = (CSSNodeInfo *)*iter;
		if( pNode->id == i )
		{
			ssNodeInfo.erase( iter );

			ssDeleteNodeInfo.push_back( pNode );

			return TRUE;
		}
	}

	return FALSE;
}

/** @brief 启用/禁用一个NODE
*/
void CSSConfigInfo::EnableNode( int i , bool bEnable )
{
	CMutexParser p( &mutex );

	if( ssNodeInfo.size() <= 0 ) return ;

	CSSNodeInfo *pNode = GetNodeById( i );
	if( !pNode ) return;

	pNode->enable = bEnable;
}
/** @brief 获得系统代理全局模式的代理地址
*/
wstring CSSConfigInfo::GetSysGlobalProxyAddr()
{
	TCHAR szPacHost[200] = {0};

#ifdef USE_LIBPRIVOXY
	_stprintf_s( szPacHost,200,_T("%s:%d"),LOCAL_LOOPBACK_IP_W, GetPrivoxyListenPort() );
#else
	_stprintf_s( szPacHost,200,_T("%s:%d"),LOCAL_LOOPBACK_IP_W, localPort );
#endif
	
	return wstring( szPacHost );
}

/** @brief 获取PAC 的URL 
*/
wstring CSSConfigInfo::GetPacUrl( )
{
	CMutexParser p( &mutex );

	if( useOnlinePac )
	{
		return pacUrl;
	}

	TCHAR szPac[1024] = {0};
	_stprintf_s( szPac,1024,_T("?t=%u"),GetTickCount() );

	TCHAR szPacHost[1024] = {0};
#ifdef USE_LIBPRIVOXY
	_stprintf_s( szPacHost,1024,_T("http://%s:%d/%S%s"),LOCAL_LOOPBACK_IP_W, GetPrivoxyListenPort() ,DEFAULT_LOCAL_PAC_URL, szPac);
#else
	_stprintf_s( szPacHost,1024,_T("http://%s:%d/%S%s"),LOCAL_LOOPBACK_IP_W, localPort ,DEFAULT_LOCAL_PAC_URL,szPac);
#endif
	wstring strPac = wstring(szPacHost);
	return strPac;
}
/** @获得下一个节点的ID号
*
* @param bInrement 自增
*/
unsigned int CSSConfigInfo::GetNextNodeId( BOOL bInrement /* = TRUE */ )
{
	CMutexParser p( &mutex );

	int id = nNextNodeId;

	if( bInrement )
		InterlockedIncrement( &nNextNodeId );

	return id;
}

void CSSConfigInfo::ClearAllNodesTrafficData()
{
	CMutexParser p( &mutex );

	for( int i = 0 ; i < ssNodeInfo.size(); i ++ )
	{
		ssNodeInfo[i]->Upload_traffic = 0;
		ssNodeInfo[i]->Download_traffic = 0;
		ssNodeInfo[i]->ConnectedTimes = 0;
		ssNodeInfo[i]->FailureTimes = 0;
		ssNodeInfo[i]->Failure_rate = 0;
		ssNodeInfo[i]->Latency = 0;
		//ssNodeInfo[i]->Speed = 0;
	}
}

/** @brief 检验idInUse是否有效. 是不是给了一个无效的ID呢. 如果是则自动给0
*/
void CSSConfigInfo::VerifyInUseId()
{
	CMutexParser p( &mutex );

	for( int i = 0 ; i < ssNodeInfo.size(); i ++ )
	{
		if( ssNodeInfo[i]->id == idInUse )
			return;
	}

	// 到这里说明指定的ID是无效的.
	idInUse = 0;
	return;
}
/** @brief 设置add from qrcode 的快捷键
*/
void CSSConfigInfo::SetHotKeyForAddQRCode( WORD wModifiers,WORD wVirtualKeyCode  ,wstring hotkey )
{
	m_HotKeyForAddFromQRCode = (wModifiers << 16) + wVirtualKeyCode;
	strHotKey = hotkey;
}

/** @brief 获得add from qrcode 的快捷键
*/
BOOL CSSConfigInfo::GetHotKeyForAddQRCode( WORD &wModifiers,WORD &wVirtualKeyCode )
{
	if( m_HotKeyForAddFromQRCode  == 0 ) return FALSE;

	wModifiers = m_HotKeyForAddFromQRCode >> 16;
	wVirtualKeyCode = (WORD)m_HotKeyForAddFromQRCode;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
/** @brief 更新节点统计信息
*/
void CSSNodeInfo::UpdateLatency( unsigned int Latency /** ms */)
{
	CMutexParser p(&mutexNode);

	if( Latency <= 0 ) 
		Latency = 1;

	this->Latency = Latency;
}
// void CSSNodeInfo::UpdateSpeed(  unsigned int Speed /* bytes per sec */)
// {
// 	CMutexParser p(&mutexNode);
// 
// 	if( Speed <= 0 ) return;
// 
// 	this->Speed = Speed;
// }
void CSSNodeInfo::UpdateUpload_traffic(  unsigned int Upload_traffic/* bytes */ )
{
	CMutexParser p(&mutexNode);

	if( Upload_traffic <= 0 ) return;

	this->Upload_traffic += Upload_traffic;
}
void CSSNodeInfo::UpdateDownload_traffic(  unsigned int Download_traffic /* bytes */)
{
	CMutexParser p(&mutexNode);

	if( Download_traffic <= 0 ) return;

	this->Download_traffic += Download_traffic;
}
void CSSNodeInfo::UpdateConnectedTimes( unsigned int ConnectedTimes /* 每次增加就调用*/)
{
	CMutexParser p(&mutexNode);

	if( ConnectedTimes <= 0 ) return;

	this->ConnectedTimes += ConnectedTimes;
}
void CSSNodeInfo::UpdateFailureTimes( unsigned int FailureTimes /* 每次失败就调用*/)
{
	CMutexParser p(&mutexNode);

	if( FailureTimes <= 0 ) return;

	this->FailureTimes += FailureTimes;
}
void CSSNodeInfo::UpdateConnections( unsigned int type  /* 1: 加一个连接, 2: 减一个连接*/)
{
	CMutexParser p(&mutexNode);

	if( nConnections < 0 )
		nConnections = 0;

	if( type == 1 )
		nConnections ++;
	else if( type == 2 )
		nConnections --;
}

string CSSNodeInfo::ToSSlink()
{
	string s;
	char szText[1000] = {0};
	sprintf_s( szText, 1000,("%s:%s@%s:%d"),method.c_str(),password.c_str(),server.c_str(),server_port );

	string base64Link = base64::encode( (const unsigned char *)szText,strlen( szText ) );
	if( !base64Link.empty() )
	{
		int l = base64Link.length();
		char *finelLink = (char *)malloc( l + 100 );
		if( finelLink )
		{
			sprintf_s( finelLink,l + 100,("ss://%s"),base64Link.c_str() );
			s = string( finelLink );

			free( finelLink );
		}
		//free( base64Link );
	}

	return s;
}

string CSSNodeInfo::ToPlainNodeInfo()
{
	string s;
	char szText[1000] = {0};
	sprintf_s( szText, 1000,("IP: %s\r\n端口: %d\r\n密码: %s\r\n加密方式: %s"),server.c_str(), server_port,password.c_str(),method.c_str() );

	s = string( szText );

	return s;
}

string CSSNodeInfo::ToJson()
{
	char szText[2000] = {0};
	sprintf_s(szText, 2000, ("{\"configs\" :[\r\n{\r\n\"server\" : \"%s\",\r\n\"server_port\" : %d,\r\n\"password\" : \"%s\",\r\n\"method\" : \"%s\",\"remarks\" : \"\",\r\n\"enable\" : true\r\n}]}"),
		server.c_str(),server_port,password.c_str(),method.c_str() );

	string s = string( szText );
	return s;
}

void CSSNodeInfo::SetLastStatus( int status )
{
	lastStatus = status;
}

BOOL CSSNodeInfo::IsOnlineNode()
{
	return (charge_type == CHARGETYPE_ONLINE_ECON || charge_type == CHARGETYPE_ONLINE_PRO || charge_type == CHARGETYPE_ONLINE_VIP );
}