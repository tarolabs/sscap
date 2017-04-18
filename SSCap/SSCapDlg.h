
// SSCapDlg.h : header file
//

#pragma once

#include "BaseDef.h"
#include "afxcmn.h"
#include "ToolTipEx.h"
#include "SSManager.h"
#include "afxwin.h"
#include "Label.h"
#include "BtnST.h"
#include "TrayIcon.h"
#include "SSListCtrl.h"
#include "TestingCurrentNodeDlg.h"
#include "LibQREncode/qrencode.h"

//class CSSListCtrl;
// CSSCapDlg dialog
class CCounterPageDlg;

class CSSCapDlg : public CDialogEx
{
// Construction
public:
	CSSCapDlg(CWnd* pParent = NULL);	// standard constructor
	~CSSCapDlg();

// Dialog Data
	enum { IDD = IDD_SSCAP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	static BOOL bHasShowedBalloonTip;			///< 是否已经显示过Balloon Tips, 显示过就不再显示了,一直显示很讨厌
	//CSSConfigInfo ssNodesInfo;   ///< ss节点信息
	UINT_PTR m_nMainWndTimer;
	CTrayIcon nTrayIcon;

	//////////////////////////////////////////////////////////////////////////
	// 保存启动时的IE旧的设置, 退出时恢复
	BOOL bUseAutoDetect;
	BOOL bUseAutoConfigUrl;
	TCHAR lpAutoConfigUrl[1024];
	BOOL bUseProxyServer;
	TCHAR lpProxyServer[100];
	TCHAR lpByPass[500];
	//////////////////////////////////////////////////////////////////////////

	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	CToolTipCtrlEx tooltip;
	CImageList m_ImageList;
	CSSManager ssManager;
	BOOL bMainWindowShow;						///< 是否显示主窗体, 默认是显示的. 如果通过命令行启动程序时,默认隐藏.

public:
	void SetShowMainWindow( BOOL bShow )
	{
		bMainWindowShow = bShow;
	}
	/** @brief 是否显示主窗体
	*/
	BOOL IsShowMainWindow()
	{
		return bMainWindowShow;
	}
	// 获取ssManager对象
	CSSManager *GetSSManagerObj();
	/** @brief 本地socks 5服务是否已经启动
	*/
	BOOL IsLocalSocks5ServiceStarted( );
	// 重启本地SOCKS服务
	BOOL RestartLocalSocketService( BOOL bNoticeSearchPort = FALSE /** 允许提示搜索端口,只用在启动时, 在配置界面中是不允许提示的*/);

	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonAddQRCode();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonDel();
	afx_msg void OnBnClickedButtonDelAll();
	afx_msg void OnBnClickedButtonAddBatch();
	afx_msg void OnBnClickedButtonAddLink();
	afx_msg void CopyToSSlink();
	afx_msg void CopyToPlainNodeInfo();
	afx_msg void CopyToJosn();
	afx_msg void CopyToQRCode();
	HBITMAP QRcode_encodeStrongToBitmap(const char *string, int version, QRecLevel level, QRencodeMode hint, int casesensitive);

	/** @brief 注册热键
	*/
	BOOL RegisterHotKey( CString strHotKey,WORD wNewVirtualKeyCode = 0 , WORD wNewModifiers = 0 ,BOOL bSave = FALSE);
protected:
	BOOL InitializeDialog();
	BOOL InitializeSSNodeList();
	void InitializeButtons();
	/** @brief 测试SS节点
	*
	* @param bIsUdp 是否测试UDP
	*/
	void CheckSSNode( BOOL bIsUdp );
	void PingSSNode( );

	/** @brief 测试所有SS节点
	*
	* @param bIsUdp 是否测试UDP
	*/
	void CheckAllSSNode( BOOL bIsUdp );
public:
	afx_msg void OnBnClickedButtonActive();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL DestroyWindow();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg LRESULT OnTrayNotification(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnShowMainWnd();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void EnableSelectSSNode( bool bEnable );
	/** @brief 注册开机自动运行的注册表配置
	*/
	void RegisterSysConfigAutoRun();
	void RefreshSSNodeListCountTip();
public:
	CListCtrl *GetSSListContainer()
	{
		return &m_listSSNodes;
	}

protected:
	void ChangeControlSize();
	BOOL InitWebAd();
	/** @brief 更新tray icon提示信息
	*/
	void _UpdateTrayIconMessage();
	void ShowQRCode();
	void OnDisconnectAllConnections();
	void OnClearAllTrafficData();
	/** @brief 更新线上PAC文件
	*/
	BOOL UpdateOnlinePacFile();

protected:
	CCounterPageDlg *pCounterPage;
	CWinThread * pWebLoaderTread;
	CTestingCurrentNodeDlg *pTestingCurrentProxyDlg;
	BOOL m_bRegisterHotKey;
	// 前一个使用中的item, 主要用于切换使用中的ITEM时的行首的图标显示.
	//int m_nLastActiveItem;
	CSSListCtrl m_listSSNodes;
	CButtonST m_btnAdd;
	CButtonST m_btnAddByQRCode;
	CButtonST m_btnAddByBatch;
	CButtonST m_btnAddByLink;
	CButtonST m_btnActiveNode;
	CButtonST m_btnEditNode;
	CButtonST m_btnDelNode;
	CButtonST m_btnCheck;
	CLabel m_staLocalSocksPortTip;
	CLabel m_staHotKeyTips;

	CButtonST m_btnRefresh;
	CButtonST m_btnSetting;
	CButtonST m_btnMainPage;
	CButtonST m_btnAbout;
	CButtonST m_btnExit;
public:
	afx_msg void OnBnClickedButtonExit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnNMDblclkListSsnode(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListSsnode(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListSsnode(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonCheck();
	afx_msg void OnBnClickedButtonSetting();
	afx_msg void OnBnClickedButtonMainpage();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonRefresh();
	LRESULT OnFoundNewVersion(WPARAM wParam ,LPARAM lParam);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	//CStatic m_staBottomRange;
	//CStatic m_staTopRange;
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
};

CListCtrl* GetSSListContainer();
/** @brief 取得主程序的主界面对话框的指针
*/
CSSCapDlg *GetMainDlg();