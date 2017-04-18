#pragma once


/** \brief
* IE控件类，用于打开IE窗口，并导航到一个URL
* 可以选择新建窗口打开，还是利用现在有窗口打开
*
* \author : Taro
*/

#define BASICLIB_API

class BASICLIB_API CURL
{
public:
	CURL(){}
	~CURL(){}
private:
	// The default browser
	static CString m_strBrowser;
//	static HANDLE m_hIeInstance;
	static LPCTSTR GetBrowser(void);
public:
	// 获取最近一打打开的IE窗口的句柄
//	static HANDLE GetInstance();
	// 打开一个IE窗口，可以选择以新窗口打开，还是在已有IE窗口打开
	static void Open(LPCTSTR lpszURL, bool bNewWindow = true);
	// 以 CreateProcess 方式打开
	static HANDLE Open_Process(LPCTSTR lpszURL);
};
