#ifndef IPINFO_HEADER_FILE
#define IPINFO_HEADER_FILE

#pragma once

/* @brief 解析纯真IP库. 通过IP获得所在地址
  纯真IP库官方网址: http://www.cz88.net/
 
 使用方法:
 char	szAddr[100]={0};
 CQQWry NewIPInfo;

 NewIPInfo.OpenA("D:\\qqwry.dat";);
 NewIPInfo.QueryIPA("1.2.3.4",szAddr,sizeof(szAddr));
 NewIPInfo.Close();

*/
 
 
/*
QQWry.dat中都是小尾字节序
 文件夹
 记录区
 索引区
*/

#define DEFAULT_QQWRY_FILENAME _T("qqwry.dat")

#pragma pack(push,1)
#define IPINFO_INDEX_SIZE	7	//一条索引的大小
#pragma pack(pop)
 
class CQQWry
{
public:
	CQQWry(void);
	~CQQWry();
 
	//在当前目录搜索默认的几个名字,QQWry.dat,IPwry.dat,等
	BOOL OpenDefDat(void);
 
	BOOL OpenA(IN char*  pszDatPath);
	BOOL OpenW(IN WCHAR* pszDatPath);
 
	BOOL Close(void);
 
	BOOL QueryIPA(IN char* pszIP,OUT char* pszAddr,IN ULONG ulAddrBuffSize);
	BOOL QueryIPW(IN WCHAR* pszIP,OUT WCHAR* pszAddr,IN ULONG ulAddrBuffSize);
 
	BOOL QueryIPA(IN ULONG ulIP,OUT char* pszAddr,IN ULONG ulAddrBuffSize);
	BOOL QueryIPW(IN ULONG ulIP,OUT WCHAR* pszAddr,IN ULONG ulAddrBuffSize);
 
	#ifdef _UNICODE
		#define QueryIP	QueryIPW
	#else
		#define QueryIP	QueryIPA
	#endif
 
	//查找指定IP所在记录的偏移
	ULONG FindIP(IN ULONG ulIP);
 
#if 0
	//检查IP有效性
	BOOL CheckIP(IN char* pszIP);
 #endif

private:
	BOOL LoadFileData();
#if 0
	//Map数据库文件
	BOOL Map(IN ULONG ulMapSize);
 
	//UnMap数据库文件
	BOOL UnMap(void);
 #endif

	BOOL IsFileOpened()
	{
		return ( INVALID_HANDLE_VALUE != m_hDatFile );
	}
	BOOL IsDataLoaded()
	{
		return ( m_bDataLoaded );
	}
private:
	unsigned char *pFileBody;
	ULONG	m_ulFileSize;
	HANDLE	m_hDatFile;

	void*	m_pDatBase;
#if 0
	HANDLE	m_hDatMap;
 #endif

	void*	m_pIndex;				//指向索引区
	ULONG	m_ulFirstIndexOffset;	//第一条索引的偏移
	ULONG	m_ulLastIndexOffset;	//最后一条索引的偏移
	ULONG	m_ulRecordNum;			//记录条数
	BOOL	m_bDataLoaded;		// 数据已加载
};
 
#endif