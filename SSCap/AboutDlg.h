#pragma once
#include "afxwin.h"
#include "HyperLink.h"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CStatic m_staVersion;
	CStatic m_staBuildtime;
	CHyperLink m_staWebsite1;
	CHyperLink m_staGoogleplus;
	CHyperLink m_staTwitter;
	CStatic m_staOtherInfo;
};