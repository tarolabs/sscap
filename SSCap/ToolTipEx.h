#pragma once

class CToolTipCtrlEx : public CToolTipCtrl
{
public:
	CToolTipCtrlEx();
	~CToolTipCtrlEx();

	virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle = 0);

	BOOL SetIconAndTitleForBalloonTip( 	int          tti_ICON, 
		// TTI_NONE    = 0 - no icon 
		// TTI_INFO    = 1 - information icon 
		// TTI_WARNING = 2 - warning icon 
		// TTI_ERROR   = 3 - error icon 
		CString      title ); 
};