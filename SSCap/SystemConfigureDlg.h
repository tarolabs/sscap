#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CSystemConfigureDlg dialog

class CSystemConfigureDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSystemConfigureDlg)

public:
	CSystemConfigureDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSystemConfigureDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CButton m_btnRunAtStartup;
	CButton m_btnStartInSystray;
	CButton m_btnShareOverLan;
	CEdit m_editSocksUser;
	CEdit m_editSocksPass;
	CButton m_btnEnableBalance;
	CButton m_btnEnableSysProxy;
	CButton m_btnAutoDisconnectConnection;
	//CComboBox m_cmbAlgorithms;
	CComboBox m_cmbSysProxyType;
	CComboBox m_cmbSysProxyPacType;
	CEdit m_editOnlinePac;
	afx_msg void OnCbnSelendokComboSysproxyMode();
	afx_msg void OnCbnSelendokComboPacSelect();
	int m_SocksPort;
	afx_msg void OnBnClickedCheckEnableSysproxy();
	CEdit m_editTestingUrl;
	CHotKeyCtrl m_hotkeyAddFromQRCode;
};
