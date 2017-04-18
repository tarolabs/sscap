#pragma once
#include "afxwin.h"
#include "EditEx.h"

// CAddSSFromLink dialog

class CAddSSFromLink : public CDialogEx
{
	DECLARE_DYNAMIC(CAddSSFromLink)

public:
	CAddSSFromLink(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddSSFromLink();

// Dialog Data
	enum { IDD = IDD_DIALOG_ADDSS_BY_LINK };

	int m_nAddedNodes;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEditEx m_editLinks;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
