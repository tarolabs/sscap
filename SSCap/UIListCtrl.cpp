#include "stdafx.h"
#include "BaseDef.h"
#include "UIListCtrl.h"
#include "SSListCtrl.h"
#include "ListCtrlEx/CGridListCtrlEx.h"
#include "ListCtrlEx/CGridColumnTraitText.h"
#include "Utils.h"
#include <assert.h>
#include "APPConfig.h"
#include "QQWry.h"
#include "RunConfig.h"
#include "Debug.h"
using namespace debug;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HANDLE hFeatchIPLocation = NULL;
static BOOL bStopGetIPLocation = FALSE;
CQQWry QQWry;

static unsigned __stdcall Thread_GetIPLocation( void* pArguments )
{
	while( !bStopGetIPLocation )
	{
		int i = 0 ; 
		int nGot = 0;
		CSSConfigInfo *pCfg = GetConfigInfo();

		for( i ; i < (int) pCfg->ssNodeInfo.size(); i ++)
		{
			CSSNodeInfo *pNode = pCfg->ssNodeInfo[i];
			if( pNode && pNode->iplocation.empty() )
			{
				char szIPLocation[1024] = {0};
#if 0
				if( GetIPLocation( pNode->server.c_str(), szIPLocation, 1024 ) )
				{
					pNode->iplocation = string( szIPLocation );
					nGot ++;
				}
#else
				if( QQWry.QueryIPA((char *)pNode->server.c_str(), szIPLocation, 1024 ) )
				{
					pNode->iplocation = string( szIPLocation );
					nGot ++;
				}
#endif

				Sleep( 10 );
			}
		}

		if( nGot <= 0 ) Sleep( 1000 *60 );
	}

	return 0;
}

void StartGetIPLocation( void *pvoid )
{
	//char szIPWryFile[2048] = {0};
	CString strPathFile;
	strPathFile.Format( _T("%s\\config\\%s"),CRunConfig::GetAppWorkingDirectory(),DEFAULT_QQWRY_FILENAME );
	PrintfW( LEVEL_INFO,_T("[StartGetIPLocation] %s"),strPathFile.GetBuffer() );

	if( QQWry.OpenW( strPathFile.GetBuffer() ) )
		hFeatchIPLocation = (HANDLE )_beginthreadex (NULL,0, Thread_GetIPLocation, NULL, NULL,NULL );
}
void EndGetIPLocation()
{
	bStopGetIPLocation = TRUE;
	if( hFeatchIPLocation )
	{
		TerminateThread( hFeatchIPLocation ,0);
		CloseHandle( hFeatchIPLocation );
		hFeatchIPLocation = NULL;
	}

	QQWry.Close();
}
/** @brief 删除所有ITEM,重构所有ITEM */
void UILC_RebuildListCtrl( CListCtrl *listCtrl )
{
	assert( listCtrl );
	if( listCtrl == NULL ) return;

	listCtrl->DeleteAllItems();

	CSSConfigInfo *pCfg = GetConfigInfo();
	// 加载ss nodes
	if( pCfg && pCfg->ssNodeInfo.size() > 0 )
	{
		int nCount = 0;
		int nInUseIdIndex = 0;
		for( int i = 0 ; i < (int)pCfg->ssNodeInfo.size(); i ++ )
		{
			CSSNodeInfo *pNode = pCfg->ssNodeInfo[i];

			if( pNode->id == pCfg->idInUse )
				nInUseIdIndex = i;

			UILC_AddItem( listCtrl, nCount, pNode ,false);

			nCount ++;
		}

		// 让当前使用中的代理保持可见.
		listCtrl->EnsureVisible( nInUseIdIndex , FALSE );
	}

	return;
}

/** @brief Listctrl设置Item的文本,并且带有是否改变的检测
* 意思是,如果一个Item的Text未改变,就不调用SetItemText
*
* @param listCtrl 一个Listctrl的指针
* @param item 要设置文本的Item
* @param column 要设置文本的列 
* @param pszText 要设置的文本
*/
void UILC_SetItemText_WithChangeCheck( CListCtrl *listCtrl, int item ,int column, TCHAR *pszText )
{
	if( !listCtrl || !pszText ) return;

	CSSListCtrl *pSSListCtrl = (CSSListCtrl *)listCtrl;

	TCHAR szOldText[200] = {0};
	pSSListCtrl->GetItemText( item, column, szOldText, 200 );

	// 如果新的文本和旧文本不同的话才调用SetItemText设置文本.
	if( _tcsicmp( szOldText, pszText ) != 0 )
	{
		pSSListCtrl->SetItemColumnChanged( item , column );
		pSSListCtrl->SetItemText( item, column, pszText );
	}
	else 
	{
		pSSListCtrl->SetItemColumnUnChanged( item , column );
	}

	return;
}

void UILC_SetItemText_WithChangeCheck( CListCtrl *listCtrl, int item ,int column, CString strText )
{
	UILC_SetItemText_WithChangeCheck( listCtrl, item , column, strText.GetBuffer( ));
	strText.ReleaseBuffer();
}

void UILC_InsertColumn( CListCtrl *pList )
{
	CGridListCtrlEx *pListCtrl = (CGridListCtrlEx *)pList;

	// Create Columns
	pListCtrl->InsertHiddenLabelColumn();	// Requires one never uses column 0
	//pListCtrl->InsertHiddenLabelColumn();	// Requires one never uses column 0

	int nCol = 1;

	//CGridColumnTraitText* pTrait = new CGridColumnTraitText;
	//pTrait->SetSortFormatNumber( true );
	//pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("ID")), LVCFMT_LEFT, 55, nCol -1 , pTrait);
	//nCol++;
	CGridColumnTraitText* pTrait = new CGridColumnTraitText;
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Server")), LVCFMT_LEFT, 200, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Encryption")), LVCFMT_LEFT, 90, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pTrait->SetSortFormatNumber( true );
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Connections")), LVCFMT_LEFT, 75, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pTrait->SetSortFormatNumber( true );
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Live Latency")), LVCFMT_LEFT, 65, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pTrait->SetSortFormatNumber( true );
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Live Speed")), LVCFMT_LEFT, 65, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pTrait->SetSortFormatNumber( true );
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Upload traffic")), LVCFMT_LEFT, 75, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pTrait->SetSortFormatNumber( true );
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Download traffic")), LVCFMT_LEFT, 75, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pTrait->SetSortFormatNumber( true );
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Failure rate")), LVCFMT_LEFT, 80, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("Last status")), LVCFMT_LEFT, 80, nCol -1 , pTrait);
	nCol++;
	pTrait = new CGridColumnTraitText;
	pListCtrl->InsertColumnTrait( nCol , lm_u82u16_s(_("IP Location")), LVCFMT_LEFT, 160, nCol -1 , pTrait);
	nCol++;

	

	return;
}
void UILC_AddItem( CListCtrl *pList ,CSSNodeInfo *pNode ,bool bEnsureInUseVisible )
{
	ASSERT( pList != NULL );
	int nCount = pList->GetItemCount();

	return UILC_AddItem( pList, nCount,pNode ,bEnsureInUseVisible);
}


void UILC_SetItemsTextFrom4Col(CListCtrl *pList, int nItem, CSSNodeInfo *pNode )
{
	assert( pList != NULL );

	int nCol = 3;
	TCHAR tcharText[500] = {0};
	CString strText;

	//connections
	strText.Format(_T("%d"), pNode->nConnections >= 0 ? pNode->nConnections: 0  );
	UILC_SetItemText_WithChangeCheck( pList, nItem, nCol, strText );
	nCol ++;

	//Latency
	strText.Format(_T("%d ms"),pNode->Latency >= 0 ? pNode->Latency : 0  );
	UILC_SetItemText_WithChangeCheck( pList, nItem, nCol, strText );
	nCol ++;

	//Speed
	strText.Format(_T("%d KB/s"),pNode->GetKBSpeed() );
	UILC_SetItemText_WithChangeCheck( pList, nItem, nCol, strText );
	nCol ++;

	//Upload_traffic
	//strText.Format(_T("%d"),pNode->Upload_traffic );
	Show_bytes_in_char( pNode->Upload_traffic, tcharText, 500 );
	UILC_SetItemText_WithChangeCheck( pList, nItem, nCol, tcharText );
	nCol ++;

	//Download_traffic
	//strText.Format(_T("%d"),pNode->Download_traffic );
	Show_bytes_in_char( pNode->Download_traffic, tcharText, 500 );
	UILC_SetItemText_WithChangeCheck( pList, nItem, nCol, tcharText );
	nCol ++;

	//failure rate
	int failure_rate = 0;

	if( pNode->ConnectedTimes > 0 )
		failure_rate  = (double)(( (double)pNode->FailureTimes/(double)pNode->ConnectedTimes )) * 100;

	strText.Format(_T("%d%%"),failure_rate >= 0 ? failure_rate : 0  );
	UILC_SetItemText_WithChangeCheck( pList, nItem, nCol, strText );
	nCol ++;

	if( pNode->lastStatus == 0 ) strText = _T("不可用");
	else if( pNode->lastStatus == 1 ) strText = _T("可用");
	else strText = _T("未知");
	UILC_SetItemText_WithChangeCheck( pList, nItem, nCol, strText );
	nCol ++;

	strText = _T("-");
	if( !pNode->iplocation.empty() )
		strText.Format( _T("%s") , lm_a2u_s(pNode->iplocation.c_str()));
	UILC_SetItemText_WithChangeCheck( pList, nItem, nCol, strText );
	nCol ++;


}

void UILC_AddItem( CListCtrl *pList , int nItem , CSSNodeInfo *pNode ,bool bEnsureInUseVisible )
{
	ASSERT( pList != NULL );
	ASSERT( pNode != NULL );

	int nCount = nItem;
	CGridListCtrlEx *pListCtrl = (CGridListCtrlEx *)pList;

	//TCHAR szBuffer[100];
	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM |LVIF_IMAGE; 
	lvi.iItem = nCount; 
	lvi.iSubItem = 0; 
	lvi.pszText = _T(""); 
	lvi.lParam = (LPARAM) pNode;
	lvi.iImage = 0;

	pListCtrl->InsertItem( &lvi );

	char szText[1000] = {0};
	TCHAR tcharText[500] = {0};

	CString strText;

	int nCol = 1;
	//BOOL bHiddenIp = CAPPConfig::IsValidConfig() && CAPPConfig::IsHiddenIPPort() ;
	//strText.Format(_T("%d"),pNode->id );

	//pListCtrl->SetItemText( nCount, nCol, strText );
	//wz1983324nCol ++;

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

	// server
	pListCtrl->SetItemText( nCount, nCol, lm_u82u16_s( szText ) );
	nCol ++;

	// method
	if( CAPPConfig::IsValidConfig() && CAPPConfig::IsHiddenIPPort() )
		pListCtrl->SetItemText( nCount, nCol, _T("") );
	else 
		pListCtrl->SetItemText( nCount, nCol, lm_u82u16_s( pNode->method.c_str() ) );
	nCol ++;

	UILC_SetItemsTextFrom4Col(pList, nCount, pNode );

	if( bEnsureInUseVisible )
		pListCtrl->EnsureVisible( nCount, FALSE );

	return;
}

void UILC_EditItemById( CListCtrl *pList , int nId, CSSNodeInfo *pNode )
{
	int nCount = pList->GetItemCount();
	if( nCount <= 0 ) return;

	for( int i = 0 ; i < nCount; i ++ )
	{
		CSSNodeInfo *pNode = (CSSNodeInfo  *)pList->GetItemData( i );
		if( pNode == NULL || pNode->id != nId )
			continue;

		UILC_EditItem( pList, i , pNode );
		break;
	}

	return;
}

void UILC_EditItem( CListCtrl *pList , int nItem, CSSNodeInfo *pNode )
{
	assert( pList!= NULL );

	char szText[1000] = {0};
	CString strText;

	CGridListCtrlEx *pListCtrl = (CGridListCtrlEx *)pList;
	int nCount = nItem;

	int nCol = 1;

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

	// server
	pListCtrl->SetItemText( nCount, nCol, lm_u82u16_s( szText ) );
	nCol ++;

	// method
	if( CAPPConfig::IsValidConfig() && CAPPConfig::IsHiddenIPPort() )
		pListCtrl->SetItemText( nCount, nCol, _T("") );
	else 
		pListCtrl->SetItemText( nCount, nCol, lm_u82u16_s( pNode->method.c_str() ) );
	nCol ++;

	UILC_SetItemsTextFrom4Col( pList, nCount, pNode );

	return;
}
void UILC_DeleteItem( CListCtrl *pList , int nItem )
{
	CGridListCtrlEx *pListCtrl = (CGridListCtrlEx *)pList;

	int nCount = pListCtrl->GetItemCount();

	if( nItem < 0 || nItem >= nCount ) 
		return;

	pListCtrl->DeleteItem( nItem );

	return;
}
void UILC_RefreshItems( CListCtrl *pList )
{
	pList->Invalidate( TRUE );
	
	CSSConfigInfo *pCfg = GetConfigInfo();
	CGridListCtrlEx *pListCtrl = (CGridListCtrlEx *)pList;
	int nCount = pListCtrl->GetItemCount();
	//CSSConfigInfo ssConfigInfo = *pCfg;

	int i = 0;
	for( i  ; i < nCount; i ++ )
	{

		CSSNodeInfo *pNode = (CSSNodeInfo  *)pListCtrl->GetItemData( i );
		if( !pNode ) continue;

		UILC_SetItemsTextFrom4Col( pList, i , pNode );
	}

	// 因为除了用户在界面上删除节点外, 程序自身不会删除节点,所以这里不做多余的节点删除操作. 

	return;
}