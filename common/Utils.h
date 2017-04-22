#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <tchar.h>
#include <tcpmib.h>

using namespace std;
#define MY_FREE(x) { if(x){free(x); x =NULL;}}
#define MY_DELEETE(x) { if(x){delete x;x=NULL;}}
#define MY_DELEETE_ARR(x) { if(x){delete []x; x=NULL;}}

// http/https
#define HTTP	3
// SOCKS 4/4A
#define SOCKS4  4
// SOCKS 5
#define SOCKS5  5
// shadowsocks代理
#define SHADOWSOCKS 6

#define PROXY_IP_ADDRESS_MAX_LENGTH 260
#define PROXY_USERNAME_MAX_LENGTH 64
#define PROXY_PASSWORD_MAX_LENGTH 64
#define PROXY_REAL_IP_ADDRESS_MAX_LANGEHT 33

wchar_t * WINAPI lm_a2u (const char *str);
char * WINAPI lm_u2a_s (wchar_t * str);
char * WINAPI lm_u2a (wchar_t * str);
wchar_t* WINAPI lm_a2u_s (const char *str);
wchar_t* WINAPI lm_u82u16_s (const char *u8);
wchar_t* WINAPI lm_u82u16 (const char *u8);
char * WINAPI lm_u162u8_s (wchar_t* u16);
char * WINAPI lm_u162u8 (wchar_t * str);

// get time format string for specific format.
void GetTimeString( time_t t ,TCHAR *szTimeFormat ,TCHAR *szFormatOut, int len );
int WINAPI SplitStringW(const wstring& input, const wstring& delimiter, vector<wstring>& results, bool includeEmpties = true );
int WINAPI SplitStringA(const string& input, const string& delimiter, vector<string>& results, bool includeEmpties = true );
/** 修剪string的前后空格*/
std::string& WINAPI trim(string& s, const std::string& drop =" ");
/** 修剪string的前后空格*/
std::wstring& WINAPI trimW(wstring& s, const std::wstring& drop =L" ");
// 判断文件是否存在
bool IsFileExist(TCHAR *file);
typedef struct _LSA_UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;
typedef LONG NTSTATUS;
char* readallfile(TCHAR *fname,TCHAR *flags);
char* readallfile(TCHAR *fname,TCHAR *flags ,int *outlength );
/** @brief 获得两个时间相隔的秒数
* 不像difftime那样,必须指定时间的先后顺序.
* 这里参数中的时间顺序可以随便指定
*
* @param[in] t1 时间1
* @param[in] t2 时间2
*
* @return t1和t2的时间差(秒)
*/
double GetTimeInterval(time_t t1 ,time_t t2);
//unsigned char *base64_decode(const char *str, int *ret_len);
/** @brief 显示合适的字节单位的数据
* <1024的显示字节, >1024 显示Kb, >1048576显示Mb...
*
* @param[in] bytes 数据长度以字节为单位
* @param[out] szBuffer 输出正确的单位的格式
* @param[in] len szBuffer的长度.
*/
void Show_bytes_in_char( long long bytes, TCHAR *szBuffer, int len );
string   Replace(string   &str,  string string_to_replace, string new_string);
BOOL PutTextToClipboardA(LPCSTR pTxtData,HWND hWnd);

//char * base64_encode(const unsigned char *data, int len);
/** @brief 检测某个TCP端口是否占用了
*
* @returns
* -TRUE: 指定的端口可用
* -FALSE; 指定的端口不可用
*/
BOOL CheckTcpPortValid( UINT nPort );

/** @brief 搜索一个未占用的端口
*
* @param nFromPort 起始搜索端口
* @param nSearchAmount 搜索的数量 
* @returns 0 没有找到端口, 非0找到的未占用的端口
*/
UINT SearchAnUnsedPort( UINT nFromPort ,UINT nSearchAmount );
/** @brief 将一个vector<string> 组合成字符串, 以delim分隔, 类似于PHP的implode
*/
string implode(const vector<string>& vec, const char* delim);

/** @brief 解析用于测试的目标URL地址, 解析成HOST, OBJ, PORT
*
* @param szURL 要解析的URL
* @param szProtocol 解析出来的协议
* @param szAddress 解析出来的HOST
* @param dwPort 端 口
* @param szURI 解析出来的OBJ
*/
BOOL ParseDestinationUrl( LPCSTR szURL, LPSTR szProtocol,int p_len, LPSTR szAddress,int a_len, u_short &dwPort, LPSTR szURI, int u_len  );
char * strdup_printf(const char* fmt,...);

/** @brief CHotKeyCtrl的值转换RegisterHotKey
* CHotKeyCtrl::SetHotKey和GetHotKey中的wModifiers和RegisterHotKey中的fsModifiers不同：
* a. 从CHotKeyCtrl控件得到的Alt和Shift分别是HOTKEYF_ALT和HOTKEYF_SHIFT，而RegisterHotKey中的Alt和Shift则为MOD_ALT和MOD_SHIFT，所以GetHotKey之后，若要注册则需要进行转换
* b. SetHotKey时，也需要调用相关的转换函数
*/
WORD HKF2SDK(WORD mod);


/** @brief 将string转为小写 ansi*/
std::string LowerStringA(const std::string &str );