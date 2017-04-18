////////////////////////////////////////////////////////////////
// CTrayIcon
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.

#ifndef _TRAYICON_H
#define _TRAYICON_H

//#include "Subclass.h"

////////////////
// CTrayIcon manages an icon in the Windows 95 system tray. 
// The sample program TRAYTEST shows how to use it.
// 
class CTrayIcon : public CCmdTarget {
public:
	CTrayIcon(UINT uID);
	~CTrayIcon();

	// Call this to receive tray notifications
	void SetNotificationWnd(CWnd* pNotifyWnd, UINT uCbMsg);

	// SetIcon functions. To remove icon, call SetIcon(0)
	//
	BOOL SetIcon(UINT uID); // main variant you want to use
	BOOL SetIcon(HICON hicon, LPCTSTR lpTip);
	BOOL SetIcon(LPCTSTR lpResName, LPCTSTR lpTip)
		{ return SetIcon(lpResName ? 
			AfxGetApp()->LoadIcon(lpResName) : NULL, lpTip); }
	BOOL SetStandardIcon(LPCTSTR lpszIconName, LPCTSTR lpTip)
		{ return SetIcon(::LoadIcon(NULL, lpszIconName), lpTip); }

	BOOL SetVersion(UINT uVersion);

	// Show balloon tip
	/* NIIF_NONE (0x00000000)0x00000000. No icon.
	NIIF_INFO (0x00000001) An information icon.
	NIIF_WARNING (0x00000002)   A warning icon.
	NIIF_ERROR (0x00000003)  An error icon.\
	NIIF_USER (0x00000004) Windows XP: Use the icon identified in hIcon as the notification balloon's title icon.
							Windows Vista and later: Use the icon identified in hBalloonIcon as the notification balloon's title icon.
	NIIF_NOSOUND (0x00000010) Windows XP and later. Do not play the associated sound. Applies only to notifications.
	NIIF_LARGE_ICON (0x00000020) Windows Vista and later. The large version of the icon should be used as the notification icon. This corresponds to the icon with dimensions SM_CXICON x SM_CYICON. If this flag is not set, the icon with dimensions XM_CXSMICON x SM_CYSMICON is used.
	NIIF_RESPECT_QUIET_TIME (0x00000080) Windows 7 and later. Do not display the balloon notification if the current user is in "quiet time",
	NIIF_ICON_MASK (0x0000000F) Windows XP and later. Reserved.
	*/
	BOOL ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle,
		UINT uTimeout, DWORD dwInfoFlags=NIIF_INFO);

	// Show balloon tip: use resource ID instead of LPCSTR.
	BOOL ShowBalloonTip(UINT uID, LPCTSTR szTitle,
		UINT uTimeout, DWORD dwInfoFlags=NIIF_INFO);

// Following is obsolete. CTrayIcon does default handling auotmatically.
//	virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);
	virtual LRESULT OnTrayNotify(WPARAM uID, LPARAM lEvent);
	virtual LRESULT OnTaskBarCreate(WPARAM wp, LPARAM lp);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
protected:
	NOTIFYICONDATA m_nid;		  // struct for Shell_NotifyIcon args

	// private class used to hook tray notification and taskbarcreated
	/*
	class CTrayHook : public CSubclassWnd {
	private:
		CTrayIcon* m_pTrayIcon;
		virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);
		friend CTrayIcon;
	};
	friend CTrayHook;

	CTrayHook m_notifyHook; // trap tray notifications
	CTrayHook m_parentHook; // trap taskbarcreated message
	*/

	DECLARE_DYNAMIC(CTrayIcon)
};

#endif
