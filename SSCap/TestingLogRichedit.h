#pragma once

/*!
 * @file TestingLogRichedit.h
 * @date 2015/06/18 14:32
 *
 * @brief 用于显示代理测试LOG的Richedit.
 *
 * 
 * @author Taro
 * Contact: sockscap64@gmail.com
 *
 *
 * @version 1.0
 *
 * @History
 *
 * @note
*/
#include "AutoRichEditCtrl.h"

/** 测试代理时的一些过程提示信息*/
#define TESTMSG_INFO 1				///< 普通的信息
#define TESTMSG_AFFIRMATIVE 2		///< 肯定的文本. (成功)
#define TESTMSG_NEGATIVE 3			///< 否定的文本. (失败)

class CAutoRichEditCtrl;
class CTestingLogRichedit : public CAutoRichEditCtrl
{
public:
	CTestingLogRichedit();
	~CTestingLogRichedit();

public:
	/** @brief 初始化用于显示LOG的RICHEDIT. 主要是字体及行间距的初始化
	*/
	BOOL InitializeLogRichedit();
	/** @brief 发送代理测试LOG给代理管理器 多字节格式
	*/
	void AppendTestingLogA( LPCSTR lpMsg , int msg_type = TESTMSG_INFO);
	/** @brief 插入一条代理测试的消息. version utf8
	*/
	void AppendTestingLogUTF8( LPCSTR lpMsg , int msg_type = TESTMSG_INFO);
	/** @brief 插入一条代理测试的消息.
	*
	* @param strMsg 要插入的消息.
	* @param msg_type 消息类型
	*	- TESTMSG_INFO 1 普通消息
	*	- TESTMSG_AFFIRMATIVE 2 肯定的消息
	*	TESTMSG_NEGATIVE 3 否定的消息
	*/
	void AppendTestingLogW(CString strMsg, int msg_type = TESTMSG_INFO );
	/** @biref 用于设置输出LOG的时候是否输出当前的时间
	* 当前时间输出的格式是: 时:分:秒
	*/
	void SetShowTimeInfo( BOOL bShow );
	/** @brief 拷贝所有文本到剪贴板
	*/
	void CopyTextToClipboard();
	/** @brief 清除所有文本
	*/
	void ClearText();
protected:
	BOOL m_bShowTimeInfo;
};