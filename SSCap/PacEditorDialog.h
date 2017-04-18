#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "AutoRichEditCtrl.h"

// CPacEditorDialog dialog

class CPacEditorDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CPacEditorDialog)

public:
	CPacEditorDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPacEditorDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_PAC_EDITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CStatic m_staEditortip;
	CAutoRichEditCtrl m_richEditor;
	afx_msg void OnBnClickedOk();
};
