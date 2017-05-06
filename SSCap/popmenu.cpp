#include "stdafx.h"
#include "sscap.h"
#include "msgdef.h"
#include "popmenu.h"
#include "BCMenu.h"
#include "BaseDef.h"
#include "Utils.h"
#include "SocketBase.h"
#include "SocksClient.h"
#include "SSClient.h"
#include "Listener.h"

#include "SSCapDlg.h"

#include "APPConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BCMenu *CreateDefaultMenu ()
{
	BCMenu *m = new BCMenu();
	if( !m ) return NULL;

	m->CreatePopupMenu();

	// 设置新的MENU样式
	BCMenu::SetMenuDrawMode( BCMENU_DRAWMODE_XP );

	return m;
}

void _AppendMenu ( BCMenu *m,unsigned int flag /* 菜单属性*/,unsigned int id, char *str /* 菜单文本*/, CImageList *pList, int offset )
{
	LPWSTR ts;
	ts=(LPWSTR) lm_u82u16_s ( str );
	//m->AppendODMenu ( flag, new BCMenuXPText(id, ts,icon) );
	m->AppendMenu(flag, id, (LPTSTR)(LPCTSTR)ts, pList, offset );
}
void _AppendMenuW ( BCMenu *m,unsigned int flag /* 菜单属性*/,unsigned int id,wchar_t *str /* 菜单文本*/, CImageList *pList, int offset )
{
	//m->AppendODMenu ( flag, new BCMenuXPText(id, str,icon) );
	m->AppendMenu(flag, id, (LPTSTR)(LPCTSTR)str, pList, offset  );
}
void _AppendSubMenu( BCMenu *m, unsigned int flag, BCMenu *submenu,unsigned int id, char *str /* 菜单文本*/, CImageList *pList, int offset )
{
	LPWSTR ts;
	ts=(LPWSTR) lm_u82u16_s ( str );
	flag |= MF_POPUP;
	//m->AppendODPopup(flag,submenu, new BCMenuXPText(id, ts, icon ) );
	m->AppendMenu(flag, (UINT)submenu->m_hMenu, (LPTSTR)(LPCTSTR)ts, pList, offset );
}

//////////////////////////////////////////////////////////////////////////
//
//创建工具条帮助菜单
BCMenu *ToolbarHelp_CreateMenu( )
{
	BCMenu *menu = CreateDefaultMenu();
	if( menu == NULL ) return NULL;

	_AppendMenu( menu, 0,ID_MENU_HELP_OFFICIAL_PAGE ,_("Contact me..."),NULL, 0 );
	_AppendMenu( menu, 0,ID_MENU_HELP_FEEDBACK ,_("Feedback..."),NULL, 0 );
	if( !CAPPConfig::IsValidConfig() )
	{
		CString strItem;
		strItem.Format( lm_u82u16_s( _("About %s...")), CAPPConfig::GetSoftName().c_str() );
		_AppendMenuW( menu, 0,IDM_ABOUTBOX ,strItem.GetBuffer(),NULL, 0 );
	}

	return menu;
}

BCMenu *SystemProxy_CreateMenu()
{
	BCMenu *menu = CreateDefaultMenu();
	if( menu == NULL ) return NULL;

	CSSConfigInfo *pCfg = GetConfigInfo();
	UINT nFlag = 0;

	if( !pCfg->enable )
	{
		nFlag = MF_DISABLED | MF_GRAYED;
	}
	_AppendMenu( menu, pCfg->global ? MF_CHECKED|nFlag : nFlag,ID_MENU_SYSPROXY_GLOBAL_MODE ,_("Global mode"),NULL, 0 );
	_AppendMenu( menu, pCfg->global ? nFlag : MF_CHECKED|nFlag ,ID_MENU_SYSPROXY_PAC_MODE ,_("PAC mode (Skip all chinese websites)"),NULL, 0 );
	//menu->AppendMenu( MF_SEPARATOR );
	//_AppendMenu( menu, MF_DISABLED | MF_GRAYED ,ID_MENU_SYSPROXY_UPDATE_PACFILE ,_("Update PAC file"),NULL, 0 );
	//_AppendMenu( menu, 0,ID_MENU_SYSPROXY_EDIT_PACFILE ,_("Edit local PAC file"),NULL, 0 );

	return menu;
}

BCMenu *TrayIcon_CreateProxyListMenu()
{
	BCMenu *menu = CreateDefaultMenu();
	if( !menu ) return NULL;

	UINT nFlag = 0;

	CSSConfigInfo *pCfg = GetConfigInfo();
	if( pCfg )
	{
		if( pCfg->ssNodeInfo.size() > 0 )
		{
			for( int i = 0 ; i < pCfg->ssNodeInfo.size(); i ++ )
			{
				CSSNodeInfo *pNode = pCfg->ssNodeInfo[i];

				char szText[1000] = {0};

				if( pNode->remarks.empty() )
				{
					if( CAPPConfig::IsValidConfig() && CAPPConfig::IsHiddenIPPort() )
						sprintf_s(szText, 1000, "-:-" );
					else 
						sprintf_s(szText, 1000, "%s:%d", pNode->server.c_str(), pNode->server_port );
				}
				else 
				{
					if( CAPPConfig::IsValidConfig() && CAPPConfig::IsHiddenIPPort() )
						sprintf_s(szText, 1000, "%s", pNode->remarks.c_str() );
					else 
						sprintf_s(szText, 1000, "%s (%s:%d)", pNode->remarks.c_str(), pNode->server.c_str(), pNode->server_port );
				}

				nFlag = 0;

				if( pNode->id == pCfg->idInUse )
					nFlag = MF_CHECKED;

				if( !pNode->enable )
					nFlag = MF_GRAYED|MF_DISABLED;

				_AppendMenu( menu, nFlag, ID_MENU_NODES_START + i  , szText ,NULL, 0 );
			}
		}
		else 
			_AppendMenu( menu, 0, ID_TOOLBAR_DUMMY  , _("There is No SS server to show") ,NULL, 0 );
	}
	else 
		_AppendMenu( menu, 0, ID_TOOLBAR_DUMMY  , _("There is No SS server to show") ,NULL, 0 );

	return menu;
}

// 创建系统栏弹出菜单 
BCMenu *TrayIcon_CreateMenu()
{
	BCMenu *menu = CreateDefaultMenu();
	UINT nFlag = 0;
	CSSConfigInfo *pCfg = GetConfigInfo();

	if( !menu ) return NULL;

	BCMenu *pProxyListSubMenu = TrayIcon_CreateProxyListMenu();
	if( pProxyListSubMenu )
	{
		_AppendSubMenu( menu, 0, pProxyListSubMenu, 0, _("Proxy list") ,NULL, 0);
	}

	// 只有privoxy启动了, 才可以使用系统代理的相关功能.
	if( pCfg->isPrivoxyRunned )
	{
		if( pCfg->enable )
			nFlag = MF_CHECKED;
	}
	else
	{
		nFlag = MF_DISABLED | MF_GRAYED;
	}

	_AppendMenu( menu, nFlag ,ID_MENU_ENABLE_SYSPROXY ,_("Enable system proxy") ,NULL, 0);

	BCMenu *SysProxyMenu = SystemProxy_CreateMenu();
	if( SysProxyMenu )
	{
		_AppendSubMenu( menu, 0, SysProxyMenu, 0, _("System wide proxy") ,NULL, 0);
	}

	nFlag = 0;
	_AppendMenu( menu, nFlag ,ID_MENU_SYSTEM_CONFIGURE ,_("Configuration...") ,NULL, 0);

	BCMenu *pHelpMenu = ToolbarHelp_CreateMenu(  );
	if( pHelpMenu ){
		_AppendSubMenu( menu, 0, pHelpMenu, 0, _("Help") ,NULL, 0);
	}

	if( CAPPConfig::IsValidConfig() )
	{
		menu->AppendMenu( MF_SEPARATOR );

		_AppendMenu( menu, 0,ID_MENU_UPDATE_SS_FROM_WEBSITE ,_("Refresh proxy nodes") ,NULL, 0);
	}

	menu->AppendMenu( MF_SEPARATOR );
	_AppendMenu( menu, 0,ID_MENU_EXIT ,_("Exit") ,NULL, 0);

	return menu;
}

BCMenu * ListCtrl_CreateMenu()
{
	CListCtrl *pList = GetSSListContainer();
	if( !pList ) return NULL;

	BCMenu *menu = CreateDefaultMenu();
	if( menu == NULL ) 
		return NULL;
	UINT nFlag = 0;
	if( pList->GetSelectedCount() <= 0 )
		nFlag = MF_GRAYED | MF_DISABLED;

	BOOL bIsOnlineNode = FALSE;
	int nSelectMask = pList->GetSelectionMark();
	if( nSelectMask != -1 ) 
	{
		CSSNodeInfo *pNode = ( CSSNodeInfo *) pList->GetItemData( nSelectMask );
		if( pNode && pNode->IsOnlineNode() )
			bIsOnlineNode = TRUE;
	}

	_AppendMenu( menu, nFlag,ID_MENU_CHECK_SEELECT_TCP ,_("Check selection through TCP") ,NULL, 0);
	_AppendMenu( menu, 0,ID_MENU_CHECK_ALL_NODES_TCP ,_("Check all SS nodes through TCP") ,NULL, 0);
	_AppendMenu( menu, nFlag,ID_MENU_CHECK_SEELECT_UDP ,_("Check selection through UDP") ,NULL, 0);
	_AppendMenu( menu, nFlag,ID_MENU_CHECK_PING ,_("Ping...") ,NULL, 0);

	menu->AppendMenu( MF_SEPARATOR );

	int nFlagAdd = 0;
	int nFlagEdit = 0;
	if( (CAPPConfig::IsValidConfig() && CAPPConfig::IsDisableAddNodes() ) )
		nFlagAdd = MF_GRAYED | MF_DISABLED;

	if( bIsOnlineNode )
		nFlagEdit = MF_GRAYED | MF_DISABLED;

	_AppendMenu( menu, nFlag,ID_MENU_ACTIVATE_SEELECT ,_("Activate selection") ,NULL, 0);
	_AppendMenu( menu, nFlagEdit == 0 ? nFlag: nFlagEdit,ID_MENU_EDIT_SEELECT ,_("Edit selection") ,NULL, 0);
	_AppendMenu( menu, nFlagEdit == 0 ? nFlag: nFlagEdit,ID_MENU_DELETE_SEELECT ,_("Delete selection") ,NULL, 0);
	menu->AppendMenu( MF_SEPARATOR );
	_AppendMenu( menu, 0,ID_MENU_DELETE_ALLNODES ,_("Delete all nodes") ,NULL, 0);

	menu->AppendMenu( MF_SEPARATOR );

	BOOL bEnableSelection = FALSE;
	int nSelected = pList->GetSelectionMark();

	if( nSelected != -1 )
	{
		CSSNodeInfo *pNode = (CSSNodeInfo *)pList->GetItemData( nSelected );
		if( pNode )
		{
			nFlag = 0;

			if( pNode->enable )
				bEnableSelection = FALSE;
			else 
				bEnableSelection = TRUE;
		}
		else 
		{
			nFlag = MF_GRAYED | MF_DISABLED;
		}
	}
	else
	{
		nFlag = MF_GRAYED | MF_DISABLED;
	}

	_AppendMenu( menu, nFlag,bEnableSelection?ID_MENU_ENABLE_SEELECT:ID_MENU_DISABLE_SEELECT ,bEnableSelection ?_("Enable selection") : _("Disable selection") ,NULL, 0 );

	menu->AppendMenu( MF_SEPARATOR );


	_AppendMenu( menu, nFlagAdd,ID_MENU_ADD_NEW ,_("Add new one...") ,NULL, 0);
	_AppendMenu( menu, nFlagAdd,ID_MENU_ADD_NEW_FROM_QRCODE ,_("Add new one from QR Code...") ,NULL, 0);
	_AppendMenu( menu, nFlagAdd,ID_MENU_ADD_NEW_FROM_LINK,_("Add new one from SS link...") ,NULL, 0);
	_AppendMenu( menu, nFlagAdd,ID_MENU_BATCH_ADDING,_("Add new one from Json...") ,NULL, 0);

	int nFlagHiddenIp = 0;
	if( CAPPConfig::IsValidConfig() && CAPPConfig::IsHiddenIPPort() )
		nFlagHiddenIp = nFlag = MF_GRAYED | MF_DISABLED;

	menu->AppendMenu( MF_SEPARATOR );
	_AppendMenu( menu, nFlagHiddenIp,ID_MENU_SHOW_QRCODE ,_("Show QR-Code...") ,NULL, 0);
	menu->AppendMenu( MF_SEPARATOR );

	BCMenu *pCopyMenu = CreateDefaultMenu();
	if( pCopyMenu ){
		
		if( nSelected == -1 )
			nFlag = MF_GRAYED | MF_DISABLED;
		else 
			nFlag = 0;

		_AppendMenu( pCopyMenu, nFlagHiddenIp? nFlagHiddenIp: nFlag,ID_MENU_COPY_TO_QRCODE ,_("Copy to QR Code") ,NULL, 0);
		_AppendMenu( pCopyMenu, nFlagHiddenIp? nFlagHiddenIp: nFlag,ID_MENU_COPY_TO_JSON ,_("Copy to json") ,NULL, 0);
		_AppendMenu( pCopyMenu, nFlagHiddenIp? nFlagHiddenIp: nFlag,ID_MENU_COPY_TO_SSLINK ,_("Copy to SS link") ,NULL, 0);
		_AppendMenu( pCopyMenu, nFlagHiddenIp? nFlagHiddenIp: nFlag,ID_MENU_COPY_TO_PLAIN_NODE_INFO ,_("Copy to Plain node info") ,NULL, 0);

		_AppendSubMenu( menu, 0, pCopyMenu, 0, _("Copy to") ,NULL, 0);
	}

	_AppendMenu( menu, 0,ID_MENU_DISCONNECTALLCONNECTION,_("Disconnect all connections") ,NULL, 0);
	_AppendMenu( menu, 0,ID_MENU_CLEAR_ALL_TRAFFIC_DATA,_("Clear all traffic data") ,NULL, 0);

	return menu;
}
// 弹出一个菜单
void PopupMenu( BCMenu *pMenu, int x, int y ,CWnd *pParentWnd )
{
	if( pMenu ){
		::SetForegroundWindow( AfxGetMainWnd()->GetSafeHwnd() );
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, pParentWnd?pParentWnd:AfxGetMainWnd(), 0);
	}
}
// 弹出系统栏菜单 
void TrayIcon_TrackPopupMenu( int x, int y )
{
	BCMenu *menu = TrayIcon_CreateMenu();
	if( menu ){
		PopupMenu( menu ,x,y, NULL);
		delete menu;
	}
}
void ListCtrl_TrackPopupMenu( int x , int y )
{
	BCMenu *menu = ListCtrl_CreateMenu();
	if( menu ){
		PopupMenu( menu ,x,y, NULL);
		delete menu;
	}
}