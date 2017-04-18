#pragma once


// CEditEx

class CEditEx : public CEdit
{
	DECLARE_DYNAMIC(CEditEx)

public:
	CEditEx();
	virtual ~CEditEx();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};


