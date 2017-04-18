#pragma once
#include "afxwin.h"


// CNewSSNodeDlg dialog

class CNewSSNodeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewSSNodeDlg)

public:
	CNewSSNodeDlg(BOOL bEdit,CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewSSNodeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_NEW_SS };

	void SetEditIndex( int i )
	{
		nEditId = i;
	}
protected:
	BOOL bIsEdit;
	int nEditId; // 正在修改的SS节点index
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckShowpassword();
	afx_msg void OnCbnSelendokComboCryptMethod();
	virtual BOOL OnInitDialog();
	bool bNodeEnabled; // 之前的SS节点的是否为ENABLE状态. (只在EDIT模式时有效)
	//int nIndexBeInsert; // 新插入节点的index
	//CSSNodeInfo *pInsertedNode;

	CString m_strServer;
	//CString m_strPort;
	int m_nPort;
	CString m_strPassword;
	CString m_strMem;
	CString strEnc;
	int m_bEnable;
	int m_nCryptionIdx; // 加密类型, 只在edit时有用.

	CComboBox m_cmbEnryption;
	CButton m_btnShowPass;
	CEdit m_editPass;
	virtual void OnOK();
	CButton m_btnEnable;
	CEdit m_editMem;
};
