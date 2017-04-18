#pragma once
#include "afxwin.h"
// CShowQRCodeDialog dialog
class CShowQRCodeDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CShowQRCodeDialog)

public:
	CShowQRCodeDialog(HBITMAP hBitmap, CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowQRCodeDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_SHOW_QRCODE };

protected:
	HBITMAP m_hQRCodeBithmap;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	//CPictureEx m_staQRCode;
	virtual BOOL OnInitDialog();
	CStatic m_staQRCode1;
};
