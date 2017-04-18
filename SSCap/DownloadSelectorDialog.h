#pragma once
#include "afxcmn.h"
#include <string>
#include "afxwin.h"
using namespace std;
// CDownloadSelectorDialog dialog

class CDownloadSelectorDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CDownloadSelectorDialog)

public:
	CDownloadSelectorDialog(string strVersion, string strChange, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDownloadSelectorDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_DOWNLOAD_SELECTOR };

protected:
	int nDownloadSource;
	CRichEditCtrl m_richChangelog;
	string m_strVersion;
	string m_strChangelog;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CButton m_btnMirror1;
};
