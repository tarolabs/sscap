#pragma once
#include "afxcmn.h"
#include "TestingLogRichedit.h"
#include "mymutex.h"
#include "BaseDef.h"
#include <list>

using namespace std;

class CTestingCurrentNodeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTestingCurrentNodeDlg)

public:
	/** @brief 开始测试当前代理
	*/
	BOOL StartTestCurrentProxy( BOOL bIsUdp = FALSE );

public:
	CTestingCurrentNodeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTestingCurrentNodeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TESTING_CURRENT_SERVER };
	void PushSSNodeInfo( CSSNodeInfo *pNode )
	{
		if( !pNode ) return;

		pNodeForTesting.push_back( pNode );
	}
	void ClearSSTestingNodes()
	{
		pNodeForTesting.clear();
	}
	list<CSSNodeInfo *> &GetNodeForTesting()
	{
		return pNodeForTesting;
	}
	BOOL GetIsTestUdp()
	{
		return bIsTestingUdp;
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	list<CSSNodeInfo *> pNodeForTesting;

	CTestingLogRichedit m_richTestingLog;
	HANDLE hTestingThread; ///< 测试线程句柄
	CMyMutex ThreadMutex; ///< 用于处理测试线程的句柄.
	BOOL bIsTestingUdp;
	/** @brief 用于测试代理的线程 
	* 
	* @param lpParam CSocksProxyTesting类的针指
	*/
	static DWORD WINAPI Thread_TestProxy( LPVOID lpParam );
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	/** @brief 得到用于显示LOG的richedit的控件指针
	*/
	CTestingLogRichedit *GetRichEditLogCtrl();
	afx_msg void OnEnSetfocusRicheditCurrentProxyLog();
};
