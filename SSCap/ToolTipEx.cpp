#include "stdafx.h"
#include "ToolTipEx.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CToolTipCtrlEx::CToolTipCtrlEx()
{

}
CToolTipCtrlEx::~CToolTipCtrlEx()
{

}

BOOL CToolTipCtrlEx::Create(CWnd* pParentWnd, DWORD dwStyle)
{
	DWORD style = TTS_NOPREFIX | // prevents the system from stripping the ampersand (&) 
								// character from a string
		TTS_BALLOON  | // the ToolTip control has the appearance of
		// 0x40        // a cartoon "balloon," with rounded corners 
		// and a stem pointing to the item. 
		TTS_ALWAYSTIP;  // the ToolTip will appear when the
		// cursor is on a tool, regardless of 
		// whether the ToolTip control's owner
		// window is active or inactive

	if( dwStyle != 0 )
		style = dwStyle;

	return __super::Create( pParentWnd, style );
}

BOOL CToolTipCtrlEx::SetIconAndTitleForBalloonTip( int          tti_ICON, CString      title )
{
	return ::SendMessage( (HWND) m_hWnd, 
		(UINT)   TTM_SETTITLE, // Adds a standard icon and title string to a ToolTip    
		(WPARAM) tti_ICON, 
		(LPARAM) (LPCTSTR) title ); 
}