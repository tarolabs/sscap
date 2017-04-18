#pragma once
#include "afxwin.h"
#include "JsonParser.h"
#include "EditEx.h"

// CAddSSFromJsonDlg dialog

class CAddSSFromJsonDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddSSFromJsonDlg)

public:
	CAddSSFromJsonDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddSSFromJsonDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_ADD_SS_FROM_JSON };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEditEx m_editJson;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();

protected:
	int ParseArrayContent(Json::Value &value );
	int ParseObjectContent(Json::Value &value );
	int ParseContent(Json::Value &value );
public:
	afx_msg void OnBnClickedButtonJsonExample();
};
