#include "stdafx.h"
#include "AutoRichEditCtrl.h"
#include "Utils.h"
//#include "SocksProxyTesting.h"
#include "TestingLogRichedit.h"
//#include "proxy/socks_test.h"

CTestingLogRichedit::CTestingLogRichedit()
	:CAutoRichEditCtrl()
{
	m_bShowTimeInfo = FALSE;
}

CTestingLogRichedit::~CTestingLogRichedit()
{

}
/** @brief 初始化用于显示LOG的RICHEDIT. 主要是字体及行间距的初始化
*/
BOOL CTestingLogRichedit::InitializeLogRichedit()
{
	Init();
	SetAutoURLDetect( TRUE );
	SetEventMask(ENM_MOUSEEVENTS);
	SetSel(0,0);

	return TRUE;
}
/** @brief 发送代理测试LOG给代理管理器 多字节格式
	*/
void CTestingLogRichedit::AppendTestingLogA( LPCSTR lpMsg , int msg_type )
{
	if( lpMsg ){
		wchar_t *u16 = lm_a2u( lpMsg );
		if( u16 ){
			AppendTestingLogW( CString( u16 ), msg_type );
			delete []u16;
		}
	}
}
/** @brief 插入一条代理测试的消息. version utf8
*/
void CTestingLogRichedit::AppendTestingLogUTF8( LPCSTR lpMsg , int msg_type )
{
	if( lpMsg ){
		wchar_t *u16 = lm_u82u16( lpMsg );
		if( u16 )
		{
			AppendTestingLogW( CString( u16 ), msg_type );
			delete []u16;
		}
	}
}
/** @brief 插入一条代理测试的消息.
*
* @param strMsg 要插入的消息.
* @param msg_type 消息类型
*	- TESTMSG_INFO 1 普通消息
*	- TESTMSG_AFFIRMATIVE 2 肯定的消息
*	TESTMSG_NEGATIVE 3 否定的消息
*/
void CTestingLogRichedit::AppendTestingLogW(CString strMsg, int msg_type )
{
	if( m_bShowTimeInfo )
	{
		TCHAR szTimeString[100] = {0};
		GetTimeString( 0,_T("[%M:%S]  "),szTimeString, 100 );
		AppendString( szTimeString, RGB( 0,0, 0 ) );
	}

	COLORREF clr;
	if( msg_type == TESTMSG_INFO )
		clr = RGB( 0,0, 0 );
	else if( msg_type == TESTMSG_AFFIRMATIVE )
		clr = RGB( 0, 200, 0 );
	else if( msg_type == TESTMSG_NEGATIVE )
		clr = RGB( 255, 0, 0 );
	else 
		clr = RGB( 0,0, 0 );

	AppendString( strMsg, clr );
	AppendString( _T("\r\n"), RGB( 0,0, 0 ) );
}
/** @biref 用于设置输出LOG的时候是否输出当前的时间
* 当前时间输出的格式是: 时:分:秒
*/
void CTestingLogRichedit::SetShowTimeInfo( BOOL bShow )
{
	m_bShowTimeInfo = bShow;
}

/** @brief 拷贝所有文本到剪贴板
*/
void CTestingLogRichedit::CopyTextToClipboard()
{
	SetSel(0,-1);
	Copy();
}
/** @brief 清除所有文本
*/
void CTestingLogRichedit::ClearText()
{
	SetWindowText(_T(""));
}