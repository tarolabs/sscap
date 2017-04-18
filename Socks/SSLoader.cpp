#include "stdheader.h"
#include "SSLoader.h"
#include "Utils.h"
#include "Debug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SS_CONFIG_FILE _T("gui-config.json")

using namespace debug;

BOOL CSSLoader::LoadSS( LPCTSTR lpszWorkingDir )
{
	assert( pSSNodeObj!= NULL );

	PrintfW(LEVEL_INFO,_T("[LoadSS] is calling..."));

	assert( lpszWorkingDir!= NULL );

	TCHAR szProxyFile[MAX_PATH];

	_stprintf_s( szProxyFile, MAX_PATH, _T("%s\\config\\%s"),lpszWorkingDir, SS_CONFIG_FILE );

	PrintfW(LEVEL_INFO,_T("[LoadSS] from file: %s"), szProxyFile );

	if( !IsFileExist( szProxyFile ) ) return FALSE;

	char *filebody = readallfile( szProxyFile,_T("rb") );
	if( !filebody ) return FALSE;

	BOOL bRet = Parse( string( filebody ) );
	free( filebody );

	return bRet;
}

void CSSLoader::SaveSS( LPCTSTR lpszWorkingDir )
{
	assert( pSSNodeObj!= NULL );

	Json::Value root;

	GenerateJsonFromSSNodes( root );

	Json::StyledWriter writer;
	std::string o = writer.write(root);

	TCHAR szProxyFile[MAX_PATH];
	_stprintf_s( szProxyFile, MAX_PATH, _T("%s\\config\\%s"),lpszWorkingDir, SS_CONFIG_FILE );
	PrintfW(LEVEL_INFO,_T("[LoadSS] Save file: %s"), szProxyFile );

	FILE *fp = NULL;
	fp = _tfsopen( szProxyFile, _T("wb"), _SH_DENYNO );
	if( fp )
	{
		fwrite( o.c_str(),1,o.length(), fp );

		fclose( fp );
	}

	return;
}
/** 解析JSON树*/
BOOL CSSLoader::ParseValueTree(Json::Value &value)
{
	assert( pSSNodeObj!= NULL );

	PrintfW(LEVEL_INFO,_T("[ParseValueTree] is calling..."));

	// runAtStartup
	if( value.isMember( "runAtStartup" ) )
	{
		pSSNodeObj->runAtStartup = value["runAtStartup"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] runAtStartup: %d"),pSSNodeObj->runAtStartup);
	}
	else 
	{
		pSSNodeObj->runAtStartup = false;
	}

	// startInSystray
	if( value.isMember( "startInSystray" ) )
	{
		pSSNodeObj->startInSystray = value["startInSystray"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] startInSystray: %d"),pSSNodeObj->startInSystray);
	}
	else 
	{
		pSSNodeObj->startInSystray = false;
	}

	// index
	// 不再使用其它版本的gui的index值.
	/*
	if( value.isMember( "index" ) )
	{
		pSSNodeObj->idInUse = value["index"].asInt();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] idInUse: %d"),pSSNodeObj->idInUse);
	}
	*/

	if( value.isMember( "idInUse" ) )
	{
		pSSNodeObj->idInUse = value["idInUse"].asInt();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] idInUse: %d"),pSSNodeObj->idInUse);
	}
	else 
		pSSNodeObj->idInUse = 0;

// 	bool random;  ///< 随机挑选节点( 服务器均衡 )
	if( value.isMember( "random") )
	{
		pSSNodeObj->random = value["random"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] random: %s"),pSSNodeObj->random?_T("yes"):_T("no") );
	}
	else 
		pSSNodeObj->random = false;

// 	bool global; ///< 系统代理Global模式
	if( value.isMember("global"))
	{
		pSSNodeObj->global = value["global"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] global: %s"),pSSNodeObj->global?_T("yes"):_T("no") );
	}
	else 
		pSSNodeObj->global = false;

// 	bool enable; ///< 启用系统代理
	if( value.isMember("enable"))
	{
		pSSNodeObj->enable = value["enable"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] enable: %s"),pSSNodeObj->enable?_T("yes"):_T("no") );
	}
	else 
		pSSNodeObj->enable = false;

	//auto_disconnect_connection
	if( value.isMember("auto_disconnect_connection"))
	{
		pSSNodeObj->auto_disconnect_connection = value["auto_disconnect_connection"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] auto_disconnect_connection: %s"),pSSNodeObj->auto_disconnect_connection?_T("yes"):_T("no") );
	}
	else 
		pSSNodeObj->auto_disconnect_connection = false;

// 	bool shareOverLan; ///< 允许其它人连接
	if( value.isMember("shareOverLan"))
	{
		pSSNodeObj->shareOverLan = value["shareOverLan"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] shareOverLan: %s"),pSSNodeObj->shareOverLan?_T("yes"):_T("no") );
	}
	else pSSNodeObj->shareOverLan = false;

// 	u_short localPort; ///< 本地socks5服务端口
	if( value.isMember("localPort"))
	{
		pSSNodeObj->localPort= value["localPort"].asInt();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] localPort: %d"),pSSNodeObj->localPort);
	}
	else 
		pSSNodeObj->localPort = DEFAULT_SOCKS_PORT;

// 	string localSocksUser; ///< 本地socks5服务帐号
	if( value.isMember("localSocksUser"))
	{
		pSSNodeObj->localSocksUser = value["localSocksUser"].asString();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] localSocksUser: %S"),pSSNodeObj->localSocksUser.c_str());
	}

// 	string localSocksPass; ///< 本地socks5服务密码
	if( value.isMember("localSocksPass"))
	{
		pSSNodeObj->localSocksPass = value["localSocksPass"].asString();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] localSocksPass: %S"),pSSNodeObj->localSocksPass.c_str());
	}

// 	string pacUrl; ///< 在线pac的url.
	if( value.isMember("pacUrl"))
	{
		pSSNodeObj->pacUrl = lm_u82u16_s( (const char *)value["pacUrl"].asString().c_str() );
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] pacUrl: %s"),pSSNodeObj->pacUrl.c_str());
	}

// 	bool useOnlinePac; ///< 使用在线pac
	if( value.isMember("useOnlinePac"))
	{
		pSSNodeObj->useOnlinePac = value["useOnlinePac"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] useOnlinePac: %s"),pSSNodeObj->useOnlinePac?_T("yes"):_T("no") );
	}
	else 
		pSSNodeObj->useOnlinePac = false;

// 	int reconnectTimes; ///< 重连次数
	if( value.isMember("reconnectTimes"))
	{
		int reconnectTimes = value["reconnectTimes"].asInt();
		if( reconnectTimes <0 || reconnectTimes > DEFAULT_RECONNECT_TIMES )
			reconnectTimes = DEFAULT_RECONNECT_TIMES;

		pSSNodeObj->reconnectTimes = reconnectTimes;

		PrintfW(LEVEL_INFO,_T("[ParseValueTree] reconnectTimes: %d"),pSSNodeObj->reconnectTimes);
	}
	else 
		pSSNodeObj->reconnectTimes = DEFAULT_RECONNECT_TIMES;

// 	int randomAlgorithm; ///< radmon = true ( 服务器均衡 ) 的情况下的算法.
	
	/*
	if( value.isMember("randomAlgorithm"))
	{
		pSSNodeObj->randomAlgorithm = value["randomAlgorithm"].asInt();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] randomAlgorithm: %d"),pSSNodeObj->randomAlgorithm);
	}
	*/

// 	bool autoban; ///< 自动禁止出错的服务器节点.
	if( value.isMember("autoban"))
	{
		pSSNodeObj->autoban = value["autoban"].asBool();
		PrintfW(LEVEL_INFO,_T("[ParseValueTree] autoban: %s"),pSSNodeObj->autoban?_T("yes"):_T("no") );
	}
	else 
		pSSNodeObj->autoban = false;

	// 	string testing_url;
	if( value.isMember("testing_url"))
	{
		pSSNodeObj->testing_url = value["testing_url"].asString();
		PrintfA(LEVEL_INFO,("[ParseValueTree] testing_url: %s"),pSSNodeObj->testing_url.c_str());
	}

	// 	m_HotKeyForAddFromQRCode;
	if( value.isMember("m_HotKeyForAddFromQRCode"))
	{
		pSSNodeObj->m_HotKeyForAddFromQRCode = value["m_HotKeyForAddFromQRCode"].asUInt();
		PrintfA(LEVEL_INFO,("[ParseValueTree] Hot key for add from qrcode: %u"),pSSNodeObj->m_HotKeyForAddFromQRCode );
	}
	else 
		pSSNodeObj->m_HotKeyForAddFromQRCode  = 0;

	// 	strHotKey
	if( value.isMember("strHotKey"))
	{
		pSSNodeObj->strHotKey = lm_u82u16_s(value["strHotKey"].asString().c_str());
		PrintfA(LEVEL_INFO,("[ParseValueTree] Hot key for add from qrcode: %s"),pSSNodeObj->strHotKey.c_str() );
	}

	if( pSSNodeObj->bLoadSaveNodes )
	{
		Json::Value nodes = value.get( string("configs"), string("") );
		if( nodes.type() == Json::arrayValue )
		{
			// parse ss nodes
			int size = nodes.size();
			for( int index = 0 ; index < size ; index ++ )
			{
				if( nodes[index].type() != Json::objectValue ) continue;

				int charge_type = CHARGETYPE_LOCAL;
				if( nodes[index].isMember("charge_type") )
					charge_type = nodes[index]["charge_type"].asInt();

				// 线上类型的代理是不会被加载的.
				if( charge_type != CHARGETYPE_LOCAL ) 
					continue;

				int nId = pSSNodeObj->GetNextNodeId( TRUE );

				CSSNodeInfo *pNode = new CSSNodeInfo( nId  );

	// 			string server;
				if( nodes[index].isMember("server") )
					pNode->server = nodes[index]["server"].asString();

	// 			u_short server_port;
				if( nodes[index].isMember("server_port") )
					pNode->server_port = nodes[index]["server_port"].asInt();

	// 			string password;
				if( nodes[index].isMember("password") )
					pNode->password = nodes[index]["password"].asString();

	// 			string method;
				if( nodes[index].isMember("method") )
					pNode->method = nodes[index]["method"].asString();

	// 			string remarks;
				if( nodes[index].isMember("remarks") )
					pNode->remarks = nodes[index]["remarks"].asString();

	// 			bool enable;
				if( nodes[index].isMember("enable") )
					pNode->enable = nodes[index]["enable"].asBool();
				else 
					pNode->enable = true;

				// ss节点的动态计算信息
	// 			unsigned int Latency;
				// 2016.4.21: 不加载节点延迟
				//if( nodes[index].isMember("Latency") )
				//	pNode->Latency = nodes[index]["Latency"].asUInt();
				//else 
				pNode->Latency = 0;
	// 			unsigned int Speed;
				// 2016.4.21: 不加载节点速度
				//if( nodes[index].isMember("Speed") )
				//	pNode->Speed = nodes[index]["Speed"].asUInt();
				//else 
				//pNode->Speed  = 0;
	// 			unsigned int Upload_traffic;
				if( nodes[index].isMember("Upload_traffic") )
					pNode->Upload_traffic = nodes[index]["Upload_traffic"].asDouble();
				else 
					pNode->Upload_traffic  = 0;
	// 			unsigned int Download_traffic;
				if( nodes[index].isMember("Download_traffic") )
					pNode->Download_traffic = nodes[index]["Download_traffic"].asDouble();
				else 
					pNode->Download_traffic  = 0;

	// 			unsigned int ConnectedTimes;
				if( nodes[index].isMember("ConnectedTimes") )
					pNode->ConnectedTimes = nodes[index]["ConnectedTimes"].asUInt();
				else 
					pNode->ConnectedTimes  = 0;
	// 			unsigned int FailureTimes;
				if( nodes[index].isMember("FailureTimes") )
					pNode->FailureTimes = nodes[index]["FailureTimes"].asUInt();
				else 
					pNode->FailureTimes  = 0;
	// 			unsigned int Failure_rate;
				if( pNode->ConnectedTimes <= 0 )
					pNode->Failure_rate = 0;
				else
				{
					pNode->Failure_rate = ( pNode->FailureTimes / pNode->ConnectedTimes ) * 100;
				}

				PrintfA(LEVEL_INFO,("[ParseValueTree] SS: Server: %s, Port: %d, Password: %s, Method: %s, Remarks: %s, Enable: %s"),
					pNode->server.c_str(),pNode->server_port,pNode->password.c_str(),pNode->method.c_str(),pNode->remarks.c_str(),pNode->enable?("yes"):("no")
					);

				if( !pNode->server.empty()
					&& pNode->server_port != 0 
					&& !pNode->password.empty()
					&& !pNode->method.empty()
					)
					pSSNodeObj->ssNodeInfo.push_back( pNode );
			}
		}
	}

	// 验正一下idInUse是否有效.
	pSSNodeObj->VerifyInUseId();

	PrintfW(LEVEL_INFO,_T("[ParseValueTree] Parsed [%d] SS Nodes."), pSSNodeObj->ssNodeInfo.size() );

	return TRUE;
}

BOOL CSSLoader::GenerateJsonFromSSNodes( Json::Value &value )
{
	assert( pSSNodeObj != NULL );

	// runAtStartup
	value["runAtStartup"] = pSSNodeObj->runAtStartup;
	value["startInSystray"] = pSSNodeObj->startInSystray;

// 	int index;  ///< 当前使用中的节点
	value["index"] = 0; // 只是为了兼容其它版本的GUI程序.

	value["idInUse"] = pSSNodeObj->idInUse;

// 	bool random;  ///< 随机挑选节点( 服务器均衡 )
	value["random"] = pSSNodeObj->random;
// 	bool global; ///< 系统代理Global模式
	value["global"] = pSSNodeObj->global;
// 	bool enable; ///< 启用系统代理
	value["enable"] = pSSNodeObj->enable;
	// auto_disconnect_connection
	value["auto_disconnect_connection"] = pSSNodeObj->auto_disconnect_connection;
// 	bool shareOverLan; ///< 允许其它人连接
	value["shareOverLan"] = pSSNodeObj->shareOverLan;
// 	u_short localPort; ///< 本地socks5服务端口
	value["localPort"] = pSSNodeObj->localPort;
// 	string localSocksUser; ///< 本地socks5服务帐号
	value["localSocksUser"] = pSSNodeObj->localSocksUser;
// 	string localSocksPass; ///< 本地socks5服务密码
	value["localSocksPass"] = pSSNodeObj->localSocksPass;
// 	string pacUrl; ///< 在线pac的url.
	value["pacUrl"] = lm_u162u8_s((wchar_t *)pSSNodeObj->pacUrl.c_str());
// 	bool useOnlinePac; ///< 使用在线pac
	value["useOnlinePac"] = pSSNodeObj->useOnlinePac;
// 	int reconnectTimes; ///< 重连次数
	value["reconnectTimes"] = pSSNodeObj->reconnectTimes;
// 	int randomAlgorithm; ///< radmon = true ( 服务器均衡 ) 的情况下的算法.
	//value["randomAlgorithm"] = pSSNodeObj->randomAlgorithm;
// 	bool autoban; ///< 自动禁止出错的服务器节点.
	value["autoban"] = pSSNodeObj->autoban;
	// testing_url
	value["testing_url"] = pSSNodeObj->testing_url;
	// m_HotKeyForAddFromQRCode
	value["m_HotKeyForAddFromQRCode"] = pSSNodeObj->m_HotKeyForAddFromQRCode;
	// strHotKey
	value["strHotKey"] = lm_u162u8_s((wchar_t*)pSSNodeObj->strHotKey.c_str());

	if( pSSNodeObj->bLoadSaveNodes )
	{
		Json::Value nodes;
		int nAdded = 0;
		for(int i = 0 ; i < pSSNodeObj->ssNodeInfo.size(); i ++ )
		{
			CSSNodeInfo *pNode = pSSNodeObj->ssNodeInfo[i];

			// 线上节点是肯定不会被保存的
			if( pNode->IsOnlineNode() )
				continue;

			// charge_type
			nodes[i]["charge_type"] = pNode->charge_type;
	// 		string server;
			nodes[i]["server"] = pNode->server;
	// 		u_short server_port;
			nodes[i]["server_port"] = pNode->server_port;
	// 		string password;
			nodes[i]["password"] = pNode->password;
	// 		string method;
			nodes[i]["method"] = pNode->method;
	// 		string remarks;
			nodes[i]["remarks"] = pNode->remarks;
	// 		bool enable;
			nodes[i]["enable"] = pNode->enable;
	// 
	// 		// ss节点的动态计算信息
	// 		unsigned int Latency;
			nodes[i]["Latency"] = pNode->Latency;
	// 		unsigned int Speed;
			//nodes[i]["Speed"] = pNode->Speed;
	// 		unsigned int Upload_traffic;
			nodes[i]["Upload_traffic"] = pNode->Upload_traffic;
	// 		unsigned int Download_traffic;
			nodes[i]["Download_traffic"] = pNode->Download_traffic;
	// 		unsigned int ConnectedTimes;
			nodes[i]["ConnectedTimes"] = pNode->ConnectedTimes;
	// 		unsigned int FailureTimes;
			nodes[i]["FailureTimes"] = pNode->FailureTimes;
	// 		unsigned int Failure_rate;

			nAdded ++;
		}

		if( nAdded > 0 )
			value["configs"] = nodes;
	}

	return TRUE;
}