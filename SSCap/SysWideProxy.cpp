#include "stdafx.h"
#include "SSCap.h"
#include "privoxy.h"
#include "utils.h"
#include "SysProxy.h"
#include "SysWideProxy.h"
#include "BaseDef.h"
#include "privoxy.h"
#include "RunConfig.h"
#include "APPConfig.h"
#include "Debug.h"
using namespace debug;

BOOL CheckCurrentProxy( HWND hParentHwnd ,BOOL bPopupMsg )
{
	CSSConfigInfo *pCfg = GetConfigInfo();
	if( pCfg->GetNodeSize() <= 0 )
	{
		if( bPopupMsg )
		{
			wchar_t *body = lm_u82u16(_("You have not add any proxy yet,so you can't enable system wide proxy."));
			if( body )
			{
				::MessageBox( hParentHwnd, body,CAPPConfig::GetSoftName().c_str(), MB_OK);
				delete []body;
			}
		}
		return FALSE;
	}

	if( pCfg->GetActiveNodeInfo() == NULL )
	{
		if( bPopupMsg )
		{
			wchar_t *body = lm_u82u16(_("Please activate a proxy first before enable system wide proxy."));
			if( body ){
				::MessageBox( hParentHwnd, body,CAPPConfig::GetSoftName().c_str(), MB_OK);
				delete []body;
			}
		}
		return FALSE;
	}

	return TRUE;
}

BOOL EnableSysWideProxy( HWND hParentHwnd ,BOOL bEnable ,BOOL bGlobalMode, BOOL bPopupMsg )
{
	CSSConfigInfo *pCfg = GetConfigInfo();

	PrintfW( LEVEL_INFO, _T("[EnableSysWideProxy]: is calling. bEnable: %d, bGlobalMode: %d"),bEnable, bGlobalMode );

	if( !bEnable )
	{
		if( DisableSystemProxy() )
		{
			pCfg->enable = false;

			SaveShadowsocksServer();
			return TRUE;
		}
	}
	else 
	{
		if( !CheckCurrentProxy( hParentHwnd,bPopupMsg ) ) return FALSE;

		if( !pCfg->isPrivoxyRunned )
		{
			pCfg->isPrivoxyRunned = RunPrivoxy( );
		}

		if( !pCfg->isPrivoxyRunned ) 
		{
			if( bPopupMsg )
			{
				wchar_t *body = lm_u82u16(_("Operations failed because the Privoxy is not started."));
				if( body ){
					::MessageBox( hParentHwnd, body,CAPPConfig::GetSoftName().c_str(), MB_OK);
					delete []body;
				}
			}

			return FALSE;
		}

		BOOL bSetProxyRet;
		// global ?
		if( bGlobalMode )
		{
			bSetProxyRet = SetSystemProxy( pCfg->GetSysGlobalProxyAddr().c_str(), NULL );
		}
		else 
		{
			bSetProxyRet = SetSystemProxy( NULL, pCfg->GetPacUrl().c_str() );
		}

		if( bSetProxyRet  )
		{
			pCfg->global = bGlobalMode?true:false;
			pCfg->enable = true;
			SaveShadowsocksServer();

			return TRUE;
		}
	}

	return FALSE;
}

BOOL ChangeSysProxyMode( HWND hParentHwnd, BOOL bGlobalMode )
{
	if( !CheckCurrentProxy(hParentHwnd ,TRUE ) ) return FALSE;

	CSSConfigInfo *pCfg = GetConfigInfo();

	if( !pCfg->isPrivoxyRunned )
	{
		pCfg->isPrivoxyRunned = RunPrivoxy( );
	}

	if( !pCfg->isPrivoxyRunned ) 
	{
		wchar_t *body = lm_u82u16(_("Operations failed because the Privoxy is not started."));
		if( body ){
			::MessageBox( hParentHwnd, body,CAPPConfig::GetSoftName().c_str(), MB_OK);
			delete []body;
		}

		return FALSE;
	}

	BOOL bSetProxyRet;

	// global ?
	if( bGlobalMode )
	{
		bSetProxyRet = SetSystemProxy( pCfg->GetSysGlobalProxyAddr().c_str(), NULL );
	}
	else 
	{
		bSetProxyRet = SetSystemProxy( NULL, pCfg->GetPacUrl().c_str() );
	}

	if( bSetProxyRet )
	{
		pCfg->global = bGlobalMode?true:false;
		pCfg->enable = true;

		SaveShadowsocksServer();
	}

	return TRUE;
}
