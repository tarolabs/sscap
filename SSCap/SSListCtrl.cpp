#include "stdafx.h"
#include "SSListCtrl.h"
#include "BaseDef.h"
#include "Utils.h"

#include "ListCtrlEx/CGridColumnTraitText.h"
#include "ListCtrlEx/CGridRowTraitText.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_UPLOAD_TRAFFIC 6
#define COLUMN_DOWNLOAD_TRAFFIC 7

CSSListCtrl::CSSListCtrl()
{

}
CSSListCtrl::~CSSListCtrl()
{

}

//------------------------------------------------------------------------
//! Override this method to provide text string and image index when drawing cells
//!	- Called when using LPSTR_TEXTCALLBACK with CListCtrl::SetItemText()
//!	- Called when using I_IMAGECALLBACK with SetCellImage()
//!
//! @param lvi The item that requires cell text and image index
//------------------------------------------------------------------------
void CSSListCtrl::OnDisplayCellItem(LVITEM& lvi)
{
	if(lvi.mask & LVIF_TEXT)
	{
		// Request text
		CString result;
		if (OnDisplayCellText(lvi.iItem, lvi.iSubItem, result))
		{
			if( result != CString(lvi.pszText) )
			{
				SetItemColumnChanged( lvi.iItem, lvi.iSubItem );
			}
			else 
			{
				SetItemColumnUnChanged( lvi.iItem, lvi.iSubItem );
			}

#if __STDC_WANT_SECURE_LIB__
			_tcscpy_s(lvi.pszText, lvi.cchTextMax, static_cast<LPCTSTR>(result) );
#else
			_tcsncpy(lvi.pszText, static_cast<LPCTSTR>(result), pNMW->item.cchTextMax);
#endif
		}
	}

	if (lvi.mask & LVIF_IMAGE)
	{
		// Request-Image
		int result = -1;
		if (OnDisplayCellImage(lvi.iItem, lvi.iSubItem, result))
			lvi.iImage = result;
		else
			lvi.iImage = I_IMAGECALLBACK;
	}

	if (lvi.mask & LVIF_STATE)
	{
		// Request-selection/Focus state (Virtual-list/LVS_OWNERDATA)
		// Use LVM_SETITEMSTATE to set selection/focus state in LVS_OWNERDATA
	}

	// OBS! Append LVIF_DI_SETITEM to the mask if the item-text/image from now on should be cached in the list (SetItem)
	//	- Besides this bonus option, then don't touch the mask
}
//------------------------------------------------------------------------
//! Override this method to provide text string when drawing cells
//! Only called when using LPSTR_TEXTCALLBACK with CListCtrl::SetItemText()
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param strText Text string to display in the cell
//! @return True if there is text string to display
//------------------------------------------------------------------------
bool CSSListCtrl::OnDisplayCellText(int nRow, int nCol, CString& strText)
{
	// 返回FALSE. 为了让那些格式变化时产生颜色显示
	return false;

	CSSConfigInfo *pCfg = GetConfigInfo();
	CSSNodeInfo *pNode = (CSSNodeInfo *)GetItemData( nRow );
	if( pNode == NULL ) return false;

	//CString strText;
	char szText[1000] = {0};

	switch( nCol )
	{
		// blank column
	case 0:{}break;
		// id
	case 1:
		{
			strText.Format(_T("%d"), nRow );
		}
		break;
		// server
	case 2:
		{
			if( pNode->remarks.empty() )
				sprintf_s(szText, 1000, "%s:%d", pNode->server.c_str(), pNode->server_port );
			else 
				sprintf_s(szText, 1000, "%s (%s:%d)", pNode->remarks.c_str(), pNode->server.c_str(), pNode->server_port );

			strText.Format( _T("%s"), lm_u82u16_s(szText) );
		}
		break;
		// method
	case 3:
		{
			strText.Format( _T("%s"), lm_u82u16_s( pNode->method.c_str() ) );
		}
		break;
		// connections
	case 4:
		{
			strText.Format(_T("%d"), pNode->nConnections >= 0 ? pNode->nConnections: 0  );
		}
		break;
		// latency
	case 5:
		{
			strText.Format(_T("%d ms"),pNode->Latency >= 0 ? pNode->Latency : 0  );
		}
		break;
		// speed
	case 6:
		{
			strText.Format(_T("%d KB/s"),pNode->GetKBSpeed() );
		}
		break;
		// upload traffic
	case 7:
		{
			TCHAR tcharText[500] = {0};
			Show_bytes_in_char( pNode->Upload_traffic, tcharText, 500 );
			strText = CString( tcharText );
		}
		break;
		// download traffic
	case 8:
		{
			TCHAR tcharText[500] = {0};
			Show_bytes_in_char( pNode->Download_traffic, tcharText, 500 );
			strText = CString( tcharText );
		}
		break;
		// failure rate
	case 9:
		{
			//failure rate
			int failure_rate = 0;

			if( pNode->ConnectedTimes > 0 )
				//failure_rate  = (double)(( (double)pNode->FailureTimes/(double)pNode->ConnectedTimes )) * 100;

			strText.Format(_T("%d%%"),failure_rate >= 0 ? failure_rate : 0  );
		}
		break;
	}

	return true;
}

//------------------------------------------------------------------------
//! Override this method to provide icon index when drawing cells
//! Only called when using I_IMAGECALLBACK with SetCellImage()
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param nImageId The icon index in the list control image list
//! @return True if there is an icon image to display
//------------------------------------------------------------------------
bool CSSListCtrl::OnDisplayCellImage(int nRow, int nCol, int& nImageId)
{
	CSSConfigInfo *pCfg = GetConfigInfo();
	CSSNodeInfo *pNode = (CSSNodeInfo *)GetItemData( nRow );
	if( pNode == NULL ) return false;

	if( pCfg->idInUse == pNode->id && nCol == 1 )
	{
		if( pCfg->idInUse == pNode->id )
		{
			nImageId = 1;
		}
	}
	return true;
}
/*
BOOL CSSListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	__super::OnGetDispInfo( pNMHDR, pResult );

	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	int nRow = pDispInfo->item.iItem;
	int nCol = pDispInfo->item.iSubItem;
	*pResult = 0;

	CSSConfigInfo *pCfg = GetConfigInfo();
	CSSNodeInfo *pNode = (CSSNodeInfo *)GetItemData( nRow );
	if( pNode == NULL ) return FALSE;

	if( pDispInfo->item.mask & LVIF_IMAGE )
	{
		if( pCfg->idInUse == pNode->id )
		{
			if( pCfg->idInUse == pNode->id )
			{
				pDispInfo->item.iImage = 1;
				pDispInfo->item.iSubItem = 1;
			}
			else 
			{
				pDispInfo->item.iImage = 0;
				pDispInfo->item.iSubItem = 1;
			}
		}
		
	}
	
	if (pDispInfo->item.mask & LVIF_TEXT) 
	{

		CString strText;
		char szText[1000] = {0};

		switch( nCol )
		{
			// blank column
		case 0:{}break;
			// id
		case 1:
			{
				strText.Format(_T("%d"), nRow );
				::lstrcpy (pDispInfo->item.pszText, strText );
			}
			break;
			// server
		case 2:
			{
				if( pNode->remarks.empty() )
					sprintf_s(szText, 1000, "%s:%d", pNode->server.c_str(), pNode->server_port );
				else 
					sprintf_s(szText, 1000, "%s (%s:%d)", pNode->remarks.c_str(), pNode->server.c_str(), pNode->server_port );

				::lstrcpy (pDispInfo->item.pszText, lm_u82u16_s(szText) );
			}
			break;
			// method
		case 3:
			{
				::lstrcpy (pDispInfo->item.pszText, lm_u82u16_s( pNode->method.c_str() ) );
			}
			break;
			// connections
		case 4:
			{
				strText.Format(_T("%d"), pNode->nConnections >= 0 ? pNode->nConnections: 0  );
				::lstrcpy (pDispInfo->item.pszText, strText );
			}
			break;
			// latency
		case 5:
			{
				strText.Format(_T("%d ms"),pNode->Latency >= 0 ? pNode->Latency : 0  );
				::lstrcpy (pDispInfo->item.pszText, strText );
			}
			break;
			// speed
		case 6:
			{
				strText.Format(_T("%d KB/s"),pNode->Speed >= 0 ? pNode->Speed : 0 );
				::lstrcpy (pDispInfo->item.pszText, strText );
			}
			break;
			// upload traffic
		case 7:
			{
				TCHAR tcharText[500] = {0};
				Show_bytes_in_char( pNode->Upload_traffic, tcharText, 500 );
				::lstrcpy (pDispInfo->item.pszText, tcharText );
			}
			break;
			// download traffic
		case 8:
			{
				TCHAR tcharText[500] = {0};
				Show_bytes_in_char( pNode->Download_traffic, tcharText, 500 );
				::lstrcpy (pDispInfo->item.pszText, tcharText );
			}
			break;
			// failure rate
		case 9:
			{
				//failure rate
				int failure_rate = 0;

				if( pNode->ConnectedTimes > 0 )
					failure_rate  = ( pNode->FailureTimes/pNode->ConnectedTimes ) * 100;

				strText.Format(_T("%d%%"),failure_rate >= 0 ? failure_rate : 0  );

				::lstrcpy (pDispInfo->item.pszText, strText );
			}
			break;
		}
	}

	return FALSE;
}
*/

//------------------------------------------------------------------------
//! Override this method to change the color used for drawing a row
//!
//! @param nRow The index of the row
//! @param textColor The text color used when drawing the row
//! @param backColor The background color when drawing the row
//! @return Color is overrided
//------------------------------------------------------------------------
bool CSSListCtrl::OnDisplayRowColor(int nRow, COLORREF& textColor, COLORREF& backColor)
{
	CSSNodeInfo *pNode = (CSSNodeInfo  *)GetItemData( nRow );
	if( pNode )
	{
		// 被禁止
		if( !pNode->enable )
		{
			backColor = RGB( 236,239,246 );
			textColor = RGB( 163,163,163);
		}
		else if( nRow %2 == 0 )
		{
			backColor = RGB(229,232,239);

			return true;
		}
	}
	else if( nRow %2 == 0 )
	{
		backColor = RGB(229,232,239);

		return true;
	}

	return false;
}

//------------------------------------------------------------------------
//! Override this method to change the colors used for drawing a cell
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param textColor The text color used when drawing the cell
//! @param backColor The background color when drawing the cell
//! @return Color is overrided
//------------------------------------------------------------------------
bool CSSListCtrl::OnDisplayCellColor(int nRow, int nCol, COLORREF& textColor, COLORREF& backColor)
{
#define ITEM_COLUMN_CHANGED RGB(255,255, 0)

	CSSNodeInfo *pNode = (CSSNodeInfo  *)GetItemData( nRow );

	

	char szbuf[100] = {0};
	sprintf_s( szbuf, 100,"%d%d", nRow, nCol );
	DWORD dwNum = atol( szbuf );

	mutexItemChange.Lock();
	ITER_SET_COLUMN_COLOR iter =  setColumnChanged.find(dwNum);
	if( iter != setColumnChanged.end() )
	{
		backColor = ITEM_COLUMN_CHANGED;
	}
	else 
	{
		// 被禁止
		if( !pNode->enable )
		{
			backColor = RGB( 236,239,246 );
			textColor = RGB( 163,163,163);
		}
		// column failure rate
		else if( pNode && nCol == 8 )
		{
			//failure rate
			int failure_rate = 0;

			if( pNode->ConnectedTimes > 0 )
				failure_rate  = (double )(( double (pNode->FailureTimes)/ ( double  )pNode->ConnectedTimes ) ) * 100;

			if( failure_rate == 0 )
			{
				if( nRow %2 == 0 )
					backColor = RGB(229,232,239);
			}
			else
			{
				double g_rate = ( ( double ) failure_rate / (double ) 100 ) * 255;

				int r = 255;
				int g = 255 - g_rate;
				int b = 255 - g_rate;

				backColor = RGB(r,g,b );
			}
		}
		// last status
		else if( pNode && nCol == 9 )
		{
			if( pNode->lastStatus == 0 )
				backColor = RGB( 255,0,0 );
		}
		else if( nRow %2 == 0 )
		{
			backColor = RGB(229,232,239);

			return true;
		}
	}

	mutexItemChange.UnLock();

	return false;
}

//------------------------------------------------------------------------
//! Configures the initial style of the list control when the it is created
//------------------------------------------------------------------------
void CSSListCtrl::OnCreateStyle()
{
	// Will be called twice when placed inside a CView

	// Not using VERIFY / ASSERT as MessageBox cannot be opened during subclassing/creating
	if (!(GetStyle() & LVS_REPORT))
		DebugBreak();	// CListCtrl must be created with style LVS_REPORT
	if (GetStyle() & LVS_OWNERDRAWFIXED)
		DebugBreak();	// CListCtrl must be created without style LVS_OWNERDRAWFIXED

	ModifyStyle(0, LVS_SHOWSELALWAYS);

	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_GRIDLINES);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_SUBITEMIMAGES);
#if (_WIN32_WINNT >= 0x501)
	if (CheckOSVersion(0x501))
		SetExtendedStyle(GetExtendedStyle() | LVS_EX_DOUBLEBUFFER);
#endif

	// Enable Vista-look if possible
	EnableVisualStyles(true);

	// Win8 has a drawing bug when using grid-lines together with visual styles, so we disable them
	if (CheckOSVersion(0x602) && UsingVisualStyle())
		SetExtendedStyle(GetExtendedStyle() & ~LVS_EX_GRIDLINES);

	// Enable the standard tooltip
	EnableToolTips(TRUE);

	// Disable the builtin tooltip (if available)
	CToolTipCtrl* pToolTipCtrl = (CToolTipCtrl*)CWnd::FromHandle((HWND)::SendMessage(m_hWnd, LVM_GETTOOLTIPS, 0, 0L));
	if (pToolTipCtrl!=NULL && pToolTipCtrl->m_hWnd!=NULL)
		pToolTipCtrl->Activate(FALSE);
}

void CSSListCtrl::SetItemColumnChanged( int row, int col )
{
	char szbuf[100] = {0};
	sprintf_s( szbuf, 100,"%d%d", row, col );

	DWORD dwNum = atol( szbuf );

	mutexItemChange.Lock();
	ITER_SET_COLUMN_COLOR iter =  setColumnChanged.find(dwNum);
	if( iter == setColumnChanged.end() )
	{
		setColumnChanged.insert( dwNum );
	}
	mutexItemChange.UnLock();
}

void CSSListCtrl::SetItemColumnUnChanged( int row, int col )
{
	char szbuf[100] = {0};
	sprintf_s( szbuf, 100,"%d%d", row, col );

	DWORD dwNum = atol( szbuf );

	mutexItemChange.Lock();
	ITER_SET_COLUMN_COLOR iter =  setColumnChanged.find(dwNum);
	if( iter != setColumnChanged.end() )
	{
		setColumnChanged.erase( iter );
	}
	mutexItemChange.UnLock();
}
namespace {
	struct PARAMSORT
	{
		PARAMSORT(HWND hWnd, int nCol, bool bAscending, CGridColumnTrait* pTrait)
			:m_hWnd(hWnd)
			,m_pTrait(pTrait)
			,m_ColumnIndex(nCol)
			,m_Ascending(bAscending)
		{}

		HWND m_hWnd;
		int  m_ColumnIndex;
		bool m_Ascending;
		CGridColumnTrait* m_pTrait;
	};
	// Comparison extracts values from the List-Control
	int CALLBACK SSListCtrlSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		TCHAR leftText[256] = _T(""), rightText[256] = _T("");

		LVITEM leftItem = {0};
		leftItem.iItem = (int)lParam1;
		leftItem.iSubItem = ps.m_ColumnIndex;
		leftItem.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
		leftItem.pszText = leftText;
		leftItem.cchTextMax = sizeof(leftText)/sizeof(TCHAR);
		ListView_GetItem(ps.m_hWnd, &leftItem);

		CSSNodeInfo *pLeftNode = (CSSNodeInfo  *)leftItem.lParam;
	
		LVITEM rightItem = {0};
		rightItem.iItem = (int)lParam2;
		rightItem.iSubItem = ps.m_ColumnIndex;
		rightItem.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
		rightItem.pszText = rightText;
		rightItem.cchTextMax = sizeof(rightText)/sizeof(TCHAR);
		ListView_GetItem(ps.m_hWnd, &rightItem);

		CSSNodeInfo *pRightNode = (CSSNodeInfo  *)rightItem.lParam;
	
		if( ( ps.m_ColumnIndex == COLUMN_DOWNLOAD_TRAFFIC || ps.m_ColumnIndex == COLUMN_UPLOAD_TRAFFIC )
			&& pLeftNode && pRightNode )
		{
			int nLeftValue = 0;
			int nRightValue = 0;

			// download traffic
			if( ps.m_ColumnIndex == COLUMN_DOWNLOAD_TRAFFIC  )
			{
				nLeftValue = pLeftNode->Download_traffic;
				nRightValue = pRightNode->Download_traffic;
			}
			else if( ps.m_ColumnIndex == COLUMN_UPLOAD_TRAFFIC )
			{
				nLeftValue = pLeftNode->Upload_traffic;
				nRightValue = pRightNode->Upload_traffic;
			}

			if (ps.m_Ascending)
				return nLeftValue - nRightValue;
			else
				return nRightValue - nLeftValue;
		}

		return ps.m_pTrait->OnSortRows(leftItem, rightItem, ps.m_Ascending);
	}
}
//------------------------------------------------------------------------
//! Changes the row sorting in regard to the specified column
//!
//! @param nCol The index of the column
//! @param bAscending Perform sorting in ascending or descending order
//! @return True / false depending on whether sort is possible
//------------------------------------------------------------------------
bool CSSListCtrl::SortColumn(int nCol, bool bAscending)
{
	// virtual lists cannot be sorted with this method
	if (GetStyle() & LVS_OWNERDATA)
		return false;

	if (GetItemCount()<=0)
		return true;

	CWaitCursor waitCursor;

	// Uses SortItemsEx because it requires no knowledge of datamodel
	//	- CListCtrl::SortItems() is faster with direct access to the datamodel
	PARAMSORT paramsort(m_hWnd, nCol, bAscending, GetColumnTrait(nCol));
	ListView_SortItemsEx(m_hWnd, SSListCtrlSortFunc, &paramsort);
	return true;
}