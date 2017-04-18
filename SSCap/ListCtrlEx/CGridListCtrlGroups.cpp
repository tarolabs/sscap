//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "../stdafx.h"
#include "CGridListCtrlGroups.h"

#include <shlwapi.h>	// IsCommonControlsEnabled

#pragma warning(disable:4100)	// unreferenced formal parameter

#include "CGridColumnTrait.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// WIN32 defines for group-support is only available from 2003 PSDK
#if _WIN32_WINNT >= 0x0501

// If using VS2008 then the MFC libary will complain that group mode is only available for Unicode builds
//	- By default CGridListCtrlGroups will be disabled in non-Unicode mode
//  - Define CGRIDLISTCTRLEX_GROUPMODE in stdafx.h to force group mode in non-unicode builds
#if _MSC_VER < 1500 || defined UNICODE || defined CGRIDLISTCTRLEX_GROUPMODE

#if _MSC_VER >= 1500 && defined CGRIDLISTCTRLEX_GROUPMODE
#pragma warning(disable:4996)
#endif
IMPLEMENT_DYNAMIC(CGridListCtrlGroups, CGridListCtrlEx)

BEGIN_MESSAGE_MAP(CGridListCtrlGroups, CGridListCtrlEx)
#if _WIN32_WINNT >= 0x0600
	ON_NOTIFY_REFLECT_EX(LVN_LINKCLICK, OnGroupTaskClick)	// Task-Link Click
	ON_NOTIFY_REFLECT_EX(LVN_GETEMPTYMARKUP, OnGetEmptyMarkup)	// Request text to display when empty
#endif
	ON_MESSAGE(LVM_REMOVEALLGROUPS, OnRemoveAllGroups)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


//------------------------------------------------------------------------
//! Constructor 
//------------------------------------------------------------------------
CGridListCtrlGroups::CGridListCtrlGroups()
	:m_GroupCol(-1)
	,m_GroupSort(-1)
	,m_SortSecondaryGroupView(2)
{}

//------------------------------------------------------------------------
//! Inserts a group into the list view control.
//!
//! @param nIndex The insert position of the group
//! @param nGroupId ID of the new group
//! @param strHeader The group header title
//! @param dwState Specifies the state of the group when inserted
//! @param dwAlign Indicates the alignment of the header or footer text for the group
//! @return Returns the index of the item that the group was added to, or -1 if the operation failed.
//------------------------------------------------------------------------
LRESULT CGridListCtrlGroups::InsertGroupHeader(int nIndex, int nGroupId, const CString& strHeader, DWORD dwState /* = LVGS_NORMAL */, DWORD dwAlign /*= LVGA_HEADER_LEFT*/)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupId;
	lg.state = dwState;
	lg.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
	lg.uAlign = dwAlign;

	// Header-title must be unicode (Convert if necessary)
#ifdef UNICODE
	lg.pszHeader = (LPWSTR)static_cast<LPCTSTR>(strHeader);
	lg.cchHeader = strHeader.GetLength();
#else
	CComBSTR header = static_cast<LPCTSTR>(strHeader);
	lg.pszHeader = header;
	lg.cchHeader = (int)header.Length();
#endif

#ifdef _DEBUG
	if (IsGroupStateEnabled())
	{
		// Extra check as Vista/Win7+ doesn't complain about inserting the same groupid twice
		CSimpleArray<int> groupIds;
		if (GetGroupIds(groupIds))
		{
			for(int i = 0 ; i < groupIds.GetSize(); ++i)
				VERIFY( groupIds[i] != nGroupId );
		}
	}
#endif
	return InsertGroup(nIndex, (PLVGROUP)&lg );
}

//------------------------------------------------------------------------
//! Moves a row into a group
//!
//! @param nRow The index of the row
//! @param nGroupId ID of the group
//! @return Nonzero if successful; otherwise zero
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::SetRowGroupId(int nRow, int nGroupId)
{
	//OBS! Rows not assigned to a group will not show in group-view
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_GROUPID;
	lvItem.iItem = nRow;
	lvItem.iSubItem = 0;
	lvItem.iGroupId = nGroupId;
	return SetItem( &lvItem );
}

//------------------------------------------------------------------------
//! Finds the matching group-id for the row, based on current grouping
//! If it cannot find a group-id, then it creates a new group
//!
//! @param nRow The index of the row
//! @return ID of the group
//------------------------------------------------------------------------
int CGridListCtrlGroups::FixRowGroupId(int nRow)
{
	if (!IsGroupViewEnabled())
		return -1;

	if (nRow < 0 || nRow >= GetItemCount())
		return -1;

	if (m_GroupCol < 0)
	{
		if (GetItemCount()<=1)
			EnableGroupView(FALSE);	// Rows with no groupid are not displayed in group-view
		return -1;
	}

	const CString& cellText = GetItemText(nRow, m_GroupCol);

	CSimpleArray<int> groupIds;
	if (!GetGroupIds(groupIds))
	{
		if (GetItemCount()<=1)
			EnableGroupView(FALSE);	// Rows with no groupid are not displayed in group-view
		return -1;
	}

	if (groupIds.GetSize()==0)
	{
		if (!IsGroupStateEnabled())
		{
			// WinXP Hack - Reset groups, when no items are assigned to any groups (Avoid conflicts with "invisible" group-ids)
			// Backup these before RemoveAllGroups() generates LVM_REMOVEALLGROUPS
			int groupCol = m_GroupCol;
			RemoveAllGroups();
			EnableGroupView(TRUE);
			m_GroupCol = groupCol;
		}
	}

	for(int i = 0; i < groupIds.GetSize(); ++i)
	{
		if (cellText==GetGroupHeader(groupIds[i]))
		{
			VERIFY( SetRowGroupId(nRow, groupIds[i]) );
			return groupIds[i];
		}
	}

	int nNewGroupId = groupIds.GetSize()+1;
	VERIFY( InsertGroupHeader(groupIds.GetSize(), nNewGroupId, cellText) != -1);
	VERIFY( SetRowGroupId(nRow, nNewGroupId) );
	return nNewGroupId;
}

//------------------------------------------------------------------------
//! Retrieves the group id of a row
//!
//! @param nRow The index of the row
//! @return ID of the group
//------------------------------------------------------------------------
int CGridListCtrlGroups::GetRowGroupId(int nRow)
{
	LVITEM lvi = {0};
	lvi.mask = LVIF_GROUPID;
	lvi.iItem = nRow;
	VERIFY( GetItem(&lvi) );
	return lvi.iGroupId;
}

//------------------------------------------------------------------------
//! Retrieves the group header title of a group
//!
//! @param nGroupId ID of the group
//! @return Group header title
//------------------------------------------------------------------------
CString CGridListCtrlGroups::GetGroupHeader(int nGroupId)
{
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.iGroupId = nGroupId;
	lg.mask = LVGF_HEADER | LVGF_GROUPID;
	VERIFY( GetGroupInfo(nGroupId, (PLVGROUP)&lg) != -1 );

#ifdef UNICODE
	return lg.pszHeader;
#elif  _MSC_VER >= 1300
	CString strHeader(lg.pszHeader);
	return strHeader;
#else
	USES_CONVERSION;
	return W2A(lg.pszHeader);
#endif
}

//------------------------------------------------------------------------
//! Checks if it is possible to modify the collapse state of a group.
//! This is only possible in Windows Vista.
//!
//! @return Groups can be collapsed (true / false)
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::IsGroupStateEnabled()
{
	if (!IsGroupViewEnabled())
		return FALSE;

	return CheckOSVersion(0x0600);
}

//------------------------------------------------------------------------
//! Checks whether a group has a certain state
//!
//! @param nGroupId ID of the group
//! @param dwState Specifies the state to check
//! @return The group has the state (true / false)
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::HasGroupState(int nGroupId, DWORD dwState)
{
	// Vista SDK - ListView_GetGroupState / LVM_GETGROUPSTATE
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_STATE;
	lg.stateMask = dwState;
	if ( GetGroupInfo(nGroupId, (PLVGROUP)&lg) == -1)
		return FALSE;

	return lg.state==dwState;
}

//------------------------------------------------------------------------
//! Updates the state of a group
//!
//! @param nGroupId ID of the group
//! @param dwState Specifies the new state of the group
//! @return The group state was updated (true / false)
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::SetGroupState(int nGroupId, DWORD dwState)
{
	// Vista SDK - ListView_SetGroupState / LVM_SETGROUPINFO
	if (!IsGroupStateEnabled())
		return FALSE;

	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_STATE;
	lg.state = dwState;
	lg.stateMask = dwState;

#ifdef LVGS_COLLAPSIBLE
	// Maintain LVGS_COLLAPSIBLE state
	if (HasGroupState(nGroupId, LVGS_COLLAPSIBLE))
		lg.state |= LVGS_COLLAPSIBLE;
#endif

	if (SetGroupInfo(nGroupId, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------
//! Find the group-id below the given point
//!
//! @param point Mouse position
//! @return ID of the group
//------------------------------------------------------------------------
int CGridListCtrlGroups::GroupHitTest(const CPoint& point)
{
	if (!IsGroupViewEnabled())
		return -1;

	if (HitTest(point)!=-1)
		return -1;

	if (IsGroupStateEnabled())
	{
		// Running on Vista or newer, but compiled without _WIN32_WINNT >= 0x0600
#ifndef LVM_GETGROUPRECT
#define LVM_GETGROUPRECT          (LVM_FIRST + 98)
#endif
#ifndef LVGGR_HEADER
#define LVGGR_HEADER		      (1)
#endif

		CSimpleArray<int> groupIds;
		if (!GetGroupIds(groupIds))
			return -1;

		for(int i = 0 ; i < groupIds.GetSize(); ++i)
		{
			LVGROUP lg = {0};
			lg.cbSize = sizeof(lg);
			lg.mask = LVGF_GROUPID;

			CRect rect(0,LVGGR_HEADER,0,0);
			VERIFY( SNDMSG((m_hWnd), LVM_GETGROUPRECT, (WPARAM)(groupIds[i]), (LPARAM)(RECT*)(&rect)) );

			if (rect.PtInRect(point))
				return groupIds[i];
		}
		// Don't try other ways to find the group
		return -1;
	}

	// We require that each group contains atleast one item
	if (GetItemCount()==0)
		return -1;

	// This logic doesn't support collapsible groups
	int nFirstRow = -1;
	CRect gridRect;
	GetWindowRect(&gridRect);
	for(CPoint pt = point ; pt.y < gridRect.bottom ; pt.y += 2)
	{
		nFirstRow = HitTest(pt);
		if (nFirstRow!=-1)
			break;
	}

	if (nFirstRow==-1)
		return -1;

	int nGroupId = GetRowGroupId(nFirstRow);

	// Extra validation that the above row belongs to a different group
	int nAboveRow = GetNextItem(nFirstRow,LVNI_ABOVE);
	if (nAboveRow!=-1 && nGroupId==GetRowGroupId(nAboveRow))
		return -1;

	return nGroupId;
}

//------------------------------------------------------------------------
//! Update the checkbox of the label column (first column)
//!
//! @param nGroupId ID of the group
//! @param bChecked The new check box state
//------------------------------------------------------------------------
void CGridListCtrlGroups::CheckEntireGroup(int nGroupId, bool bChecked)
{
	if (!(GetExtendedStyle() & LVS_EX_CHECKBOXES))
		return;

	for (int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		if (GetRowGroupId(nRow) == nGroupId)
		{
			SetCheck(nRow, bChecked ? TRUE : FALSE);
		}
	}
}

//------------------------------------------------------------------------
//! Removes the group and all the rows part of the group
//!
//! @param nGroupId ID of the group
//! @return Succeeded in removing the entire group
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::DeleteEntireGroup(int nGroupId)
{
	for (int nRow=0; nRow<GetItemCount(); ++nRow)
	{
		if (GetRowGroupId(nRow) == nGroupId)
		{
			DeleteItem(nRow);
			nRow--;
		}
	}
	return RemoveGroup(nGroupId)!=-1;
}

//------------------------------------------------------------------------
//! Find all registered group-ids
//!
//! @param groupIds Array of group-ids found
//! @return Succeeded in finding groups
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::GetGroupIds(CSimpleArray<int>& groupIds)
{
	if (!IsGroupViewEnabled())
		return FALSE;

	if (IsGroupStateEnabled())
	{
		// Running on Vista or newer, but compiled without _WIN32_WINNT >= 0x0600
#ifndef LVM_GETGROUPINFOBYINDEX
#define LVM_GETGROUPINFOBYINDEX   (LVM_FIRST + 153)
#endif
#ifndef LVM_GETGROUPCOUNT
#define LVM_GETGROUPCOUNT         (LVM_FIRST + 152)
#endif
		LRESULT groupCount = SNDMSG((m_hWnd), LVM_GETGROUPCOUNT, (WPARAM)0, (LPARAM)0);
		if (groupCount <= 0)
			return FALSE;
		for(int i = 0 ; i < groupCount; ++i)
		{
			LVGROUP lg = {0};
			lg.cbSize = sizeof(lg);
			lg.mask = LVGF_GROUPID;

			VERIFY( SNDMSG((m_hWnd), LVM_GETGROUPINFOBYINDEX, (WPARAM)(i), (LPARAM)(&lg)) );
			groupIds.Add(lg.iGroupId);
		}
		return TRUE;
	}
	else
	{
		// The less optimal way, but that is only on WinXP (Doesn't support negative group ids)
		for(int nRow=0 ; nRow < GetItemCount() ; ++nRow)
		{
			int nGroupId = GetRowGroupId(nRow);
			if (nGroupId>=0 && groupIds.Find(nGroupId)==-1)
				groupIds.Add(nGroupId);
		}
		return TRUE;
	}
}

//------------------------------------------------------------------------
//! Create a group for each unique values within a column
//!
//! @param nCol The index of the column
//! @return Succeeded in creating the group
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::GroupByColumn(int nCol)
{
	CWaitCursor waitCursor;

	m_GroupCol = -1;

	m_GroupSort = -1;

	SetSortArrow(-1, false);

	SetRedraw(FALSE);

	RemoveAllGroups();

	EnableGroupView( GetItemCount() > 0 );

	if (IsGroupViewEnabled())
	{
		CSimpleMap<CString,CSimpleArray<int> > groups;

		// Loop through all rows and find possible groups
		for(int nRow=0; nRow<GetItemCount(); ++nRow)
		{
			CString cellText = GetItemText(nRow, nCol);

			int nGroupIdx = groups.FindKey(cellText);
			if (nGroupIdx==-1)
			{
				CSimpleArray<int> rows;
				groups.Add(cellText, rows);
				nGroupIdx = groups.FindKey(cellText);
			}
			groups.GetValueAt(nGroupIdx).Add(nRow);
		}

		// Look through all groups and assign rows to group
		for(int nGroupIdx = 0; nGroupIdx < groups.GetSize(); ++nGroupIdx)
		{
			const CSimpleArray<int>& groupRows = groups.GetValueAt(nGroupIdx);
			DWORD dwState = LVGS_NORMAL;
#ifdef LVGS_COLLAPSIBLE
			if (IsGroupStateEnabled())
				dwState = LVGS_COLLAPSIBLE;
#endif
			VERIFY( InsertGroupHeader(nGroupIdx, nGroupIdx+1, groups.GetKeyAt(nGroupIdx), dwState) != -1);

			for(int groupRow = 0; groupRow < groupRows.GetSize(); ++groupRow)
			{
				VERIFY( SetRowGroupId(groupRows[groupRow], nGroupIdx+1) );
			}
		}

		SetRedraw(TRUE);
		Invalidate(FALSE);

		m_GroupCol = nCol;
		return TRUE;
	}

	SetRedraw(TRUE);
	Invalidate(FALSE);
	return FALSE;
}

//------------------------------------------------------------------------
//! Collapse all groups
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::CollapseAllGroups()
{
	if (!IsGroupStateEnabled())
		return FALSE;

	CSimpleArray<int> groupIds;
	if (!GetGroupIds(groupIds))
		return FALSE;

	for(int i = 0; i < groupIds.GetSize(); ++i)
	{
		if (!HasGroupState(groupIds[i],LVGS_COLLAPSED))
		{
			VERIFY( SetGroupState(groupIds[i],LVGS_COLLAPSED) );
		}
	}
	
	return TRUE;
}

//------------------------------------------------------------------------
//! Expand all groups
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::ExpandAllGroups()
{
	if (!IsGroupStateEnabled())
		return FALSE;

	CSimpleArray<int> groupIds;
	if (!GetGroupIds(groupIds))
		return FALSE;

	for(int i = 0; i < groupIds.GetSize(); ++i)
	{
		if (HasGroupState(groupIds[i],LVGS_COLLAPSED))
		{
			VERIFY( SetGroupState(groupIds[i],LVGS_NORMAL) );
		}
	}

	return TRUE;
}

//------------------------------------------------------------------------
//! Called by the framework when a drop operation is to occur, where the
//! origin is the CGridListCtrlEx itself
//!
//! @param pDataObject Points to the data object containing the data that can be dropped
//! @param dropEffect The effect that the user chose for the drop operation (DROPEFFECT_COPY, DROPEFFECT_MOVE, DROPEFFECT_LINK)
//! @param point Contains the current location of the cursor in client coordinates.
//! @return Nonzero if the drop is successful; otherwise 0
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::OnDropSelf(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	// Internal drag (Move rows to other group)
	int nRow, nCol;
	CellHitTest(point, nRow, nCol);
	if (!IsGroupViewEnabled())
		return CGridListCtrlEx::MoveSelectedRows(nRow);

	if (GetStyle() & LVS_OWNERDATA)
		return false;

	int nGroupId = nRow!=-1 ? GetRowGroupId(nRow) : GroupHitTest(point);
	if (nGroupId==-1)
		return FALSE;

	if (MoveSelectedRows(nGroupId))
	{
		if (nRow!=-1)
		{
			EnsureVisible(nRow, FALSE);
			SetFocusRow(nRow);
		}
	}
	return TRUE;
}

//------------------------------------------------------------------------
//! Moves the selected rows to the specified group
//!
//! @param nDropGroupId Moved the selected rows to this group
//! @return Was rows rearranged ? (true / false)
//------------------------------------------------------------------------
bool CGridListCtrlGroups::MoveSelectedRows(int nDropGroupId)
{
	if (GetStyle() & LVS_OWNERDATA)
		return false;

	if (nDropGroupId==-1)
		return false;

	POSITION pos = GetFirstSelectedItemPosition();
	if (pos==NULL)
		return false;

	while(pos!=NULL)
	{
		int nRow = GetNextSelectedItem(pos);
		int nGroupId = GetRowGroupId(nRow);
		if (nGroupId != nDropGroupId)
			SetRowGroupId(nRow, nDropGroupId);
	}

	return true;
}

//------------------------------------------------------------------------
//! WM_CONTEXTMENU message handler to show popup menu when mouse right
//! click is used (or SHIFT+F10 on the keyboard)
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//------------------------------------------------------------------------
void CGridListCtrlGroups::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ( IsGroupViewEnabled() )
	{
		if (pWnd!=GetHeaderCtrl())
		{
			if (point.x!=-1 && point.y!=-1)
			{
				CPoint pt = point;
				ScreenToClient(&pt);

				int nGroupId = GroupHitTest(pt);
				if (nGroupId!=-1)
				{
					OnContextMenuGroup(pWnd, point, nGroupId);
					return;
				}
			}
		}
	}
	CGridListCtrlEx::OnContextMenu(pWnd, point);
}

namespace {
	bool IsCommonControlsEnabled()
	{
		bool commoncontrols = false;
	
		// Test if application has access to common controls
		HMODULE hinstDll = ::LoadLibrary(_T("comctl32.dll"));
		if (hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion = NULL;
			(FARPROC&)pDllGetVersion = ::GetProcAddress(hinstDll, "DllGetVersion");
			if (pDllGetVersion != NULL)
			{
				DLLVERSIONINFO dvi = {0};
				dvi.cbSize = sizeof(dvi);
				HRESULT hRes = pDllGetVersion ((DLLVERSIONINFO *) &dvi);
				if (SUCCEEDED(hRes))
					commoncontrols = dvi.dwMajorVersion >= 6;
			}
			::FreeLibrary(hinstDll);
		}
		return commoncontrols;
	}
}

//------------------------------------------------------------------------
//! Override this method to change the context menu when activating context
//! menu for the column headers
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//! @param nCol The index of the column
//------------------------------------------------------------------------
void CGridListCtrlGroups::OnContextMenuHeader(CWnd* pWnd, CPoint point, int nCol)
{
	// Only Windows XP and above supports groups
	if (!IsCommonControlsEnabled())
	{
		CGridListCtrlEx::OnContextMenuHeader(pWnd, point, nCol);
		return;
	}

	// Show context-menu with the option to show hide columns
	CMenu menu;
	VERIFY( menu.CreatePopupMenu() );

	if (nCol!=-1)
	{
		// Retrieve column-title
		const CString& columnTitle = GetColumnHeading(nCol);
		menu.AppendMenu(MF_STRING, 3, CString(_T("Group by: ")) + columnTitle);
	}

	if (IsGroupViewEnabled())
	{
		menu.AppendMenu(MF_STRING, 4, _T("Disable grouping"));
	}

	CString title_editor;
	if (HasColumnEditor(nCol, title_editor))
	{
		menu.AppendMenu(MF_STRING, 1, static_cast<LPCTSTR>(title_editor));
	}

	CString title_picker;
	if (HasColumnPicker(title_picker))
	{
		menu.AppendMenu(MF_STRING, 2, static_cast<LPCTSTR>(title_picker));		
	}
	else
	{
		if (menu.GetMenuItemCount()>0)
			menu.AppendMenu(MF_SEPARATOR, 0, _T(""));

		InternalColumnPicker(menu, 6);
	}

	int nColCount = GetColumnCount();
	if (nColCount < 0)
		DebugBreak();
	CSimpleArray<CString> profiles;
	InternalColumnProfileSwitcher(menu, (UINT)nColCount + 7, profiles);

	CString title_resetdefault;
	if (HasColumnDefaultState(title_resetdefault))
	{
		if (profiles.GetSize()==0)
			menu.AppendMenu(MF_SEPARATOR, 0, _T(""));
		menu.AppendMenu(MF_STRING, 5, title_resetdefault);
	}

	// Will return zero if no selection was made (TPM_RETURNCMD)
	int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
	switch(nResult)
	{
		case 0: break;
		case 1:	OpenColumnEditor(nCol); break;
		case 2: OpenColumnPicker(); break;
		case 3: GroupByColumn(nCol); break;
		case 4:
		{
			// Very strange problem when disabling group mode, then scrollbars are not updated
			// If placed in the bottom and disables group mode, then suddenly there is a strange offset
			//	- Quick fix scroll to top, and then fix scroll bars afterwards
			int pos = GetScrollPos(SB_VERT);
			EnsureVisible(0,FALSE);
			RemoveAllGroups();
			EnableGroupView(FALSE);
			Scroll(CSize(0,pos));
		} break;
		case 5: ResetColumnDefaultState(); break;
		default:
		{
			int nShowCol = nResult-6;
			if (nShowCol < GetColumnCount())
			{
				ShowColumn(nShowCol, !IsColumnVisible(nShowCol));
			}
			else
			{
				int nProfile = nResult-GetColumnCount()-7;
				SwichColumnProfile(profiles[nProfile]);
			}
		} break;
	}
}

//------------------------------------------------------------------------
//! Override this method to change the context menu when activating context
//! menu for the group headers
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//! @param nGroupId ID of the group
//------------------------------------------------------------------------
void CGridListCtrlGroups::OnContextMenuGroup(CWnd* pWnd, CPoint point, int nGroupId)
{
	CMenu menu;
	VERIFY( menu.CreatePopupMenu() );

	const CString& groupHeader = GetGroupHeader(nGroupId);

	// Provide menu-options for collapsing groups, if the collapsible state is not available
#ifndef LVGS_COLLAPSIBLE
	if (IsGroupStateEnabled())
	{
		if (HasGroupState(nGroupId,LVGS_COLLAPSED))
		{
			CString menuText = CString(_T("Expand group: ")) + groupHeader;
			menu.AppendMenu(MF_STRING, 1, menuText);
		}
		else
		{
			CString menuText = CString(_T("Collapse group: ")) + groupHeader;
			menu.AppendMenu(MF_STRING, 2, menuText);
		}
	}
#endif

	if (GetExtendedStyle() & LVS_EX_CHECKBOXES)
	{
		CString menuText = CString(_T("Check group: ")) + groupHeader;
		menu.AppendMenu(MF_STRING, 3, menuText);
		menuText = CString(_T("Uncheck group: ")) + groupHeader;
		menu.AppendMenu(MF_STRING, 4, menuText);
	}

	if (IsGroupStateEnabled())
	{
		if (menu.GetMenuItemCount() > 0)
			menu.AppendMenu(MF_SEPARATOR, 0, _T(""));
		menu.AppendMenu(MF_STRING, 5, _T("Expand all groups"));
		menu.AppendMenu(MF_STRING, 6, _T("Collapse all groups"));
		menu.AppendMenu(MF_STRING, 7, _T("Disable grouping"));
	}

	int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
	switch(nResult)
	{
		case 1: SetGroupState(nGroupId, LVGS_NORMAL); break;
		case 2: SetGroupState(nGroupId, LVGS_COLLAPSED); break;
		case 3: CheckEntireGroup(nGroupId, true); break;
		case 4: CheckEntireGroup(nGroupId, false); break;
		case 5: ExpandAllGroups(); break;
		case 6: CollapseAllGroups(); break;
		case 7: RemoveAllGroups(); EnableGroupView(FALSE); break;
	}
}

//------------------------------------------------------------------------
//! Override this method to change the context menu when activating context
//! menu in client area with no rows
//!
//! @param pWnd Handle to the window in which the user right clicked the mouse
//! @param point Position of the cursor, in screen coordinates, at the time of the mouse click.
//------------------------------------------------------------------------
void CGridListCtrlGroups::OnContextMenuGrid(CWnd* pWnd, CPoint point)
{
	if (IsGroupStateEnabled())
	{
		CMenu menu;
		VERIFY( menu.CreatePopupMenu() );

		menu.AppendMenu(MF_STRING, 1, _T("Expand all groups"));
		menu.AppendMenu(MF_STRING, 2, _T("Collapse all groups"));
		menu.AppendMenu(MF_STRING, 3, _T("Disable grouping"));

		int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this, 0);
		switch(nResult)
		{
			case 1: ExpandAllGroups(); break;
			case 2: CollapseAllGroups(); break;
			case 3: RemoveAllGroups(); EnableGroupView(FALSE); break;
		}
	}
}

//------------------------------------------------------------------------
//! Update the description text of the group footer
//!
//! @param nGroupId ID of the group
//! @param strFooter The footer description text
//! @param dwAlign Indicates the alignment of the footer text for the group
//! @return Succeeded in updating the group footer
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::SetGroupFooter(int nGroupId, const CString& strFooter, DWORD dwAlign)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_FOOTER | LVGF_ALIGN;
	lg.uAlign = dwAlign;
#ifdef UNICODE
	lg.pszFooter = (LPWSTR)(LPCTSTR)strFooter;
	lg.cchFooter = strFooter.GetLength();
#else
	CComBSTR bstrFooter = strFooter;
	lg.pszFooter = bstrFooter;
	lg.cchFooter = bstrFooter.Length();
#endif

	if (SetGroupInfo(nGroupId, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

//------------------------------------------------------------------------
//! Update the task link of the group header
//!
//! @param nGroupId ID of the group
//! @param strTask The task description text
//! @return Succeeded in updating the group task
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::SetGroupTask(int nGroupId, const CString& strTask)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_TASK;
#ifdef UNICODE
	lg.pszTask = (LPWSTR)(LPCTSTR)strTask;
	lg.cchTask = strTask.GetLength();
#else
	CComBSTR bstrTask = strTask;
	lg.pszTask = bstrTask;
	lg.cchTask = bstrTask.Length();
#endif

	if (SetGroupInfo(nGroupId, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

//------------------------------------------------------------------------
//! Update the subtitle in the group header
//!
//! @param nGroupId ID of the group
//! @param strSubtitle The subtitle description text
//! @return Succeeded in updating the group subtitle
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::SetGroupSubtitle(int nGroupId, const CString& strSubtitle)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_SUBTITLE;
#ifdef UNICODE
	lg.pszSubtitle = (LPWSTR)(LPCTSTR)strSubtitle;
	lg.cchSubtitle = strSubtitle.GetLength();
#else
	CComBSTR bstrSubtitle = strSubtitle;
	lg.pszSubtitle = bstrSubtitle;
	lg.cchSubtitle = bstrSubtitle.Length();
#endif

	if (SetGroupInfo(nGroupId, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

//------------------------------------------------------------------------
//! Update the image icon in the group header together with top and bottom
//! description. Microsoft encourage people not to use this functionality.
//!
//! @param nGroupId ID of the group
//! @param nImage Index of the title image in the control imagelist.
//! @param strTopDesc Description text placed oppposite of the image
//! @param strBottomDesc Description text placed below the top description
//! @return Succeeded in updating the group image
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::SetGroupTitleImage(int nGroupId, int nImage, const CString& strTopDesc, const CString& strBottomDesc)
{
	if (!IsGroupStateEnabled())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	LVGROUP lg = {0};
	lg.cbSize = sizeof(lg);
	lg.mask = LVGF_TITLEIMAGE;
	lg.iTitleImage = nImage;	// Index of the title image in the control imagelist.

#ifdef UNICODE
	if (!strTopDesc.IsEmpty())
	{
		// Top description is drawn opposite the title image when there is
		// a title image, no extended image, and uAlign==LVGA_HEADER_CENTER.
		lg.mask |= LVGF_DESCRIPTIONTOP;
		lg.pszDescriptionTop = (LPWSTR)(LPCTSTR)strTopDesc;
		lg.cchDescriptionTop = strTopDesc.GetLength();
	}
	if (!strBottomDesc.IsEmpty())
	{
		// Bottom description is drawn under the top description text when there is
		// a title image, no extended image, and uAlign==LVGA_HEADER_CENTER.
		lg.mask |= LVGF_DESCRIPTIONBOTTOM;
		lg.pszDescriptionBottom = (LPWSTR)(LPCTSTR)strBottomDesc;
		lg.cchDescriptionBottom = strBottomDesc.GetLength();
	}
#else
	CComBSTR bstrTopDesc = strTopDesc;
	CComBSTR bstrBottomDesc = strBottomDesc;
	if (!strTopDesc.IsEmpty())
	{
		lg.mask |= LVGF_DESCRIPTIONTOP;
		lg.pszDescriptionTop = bstrTopDesc;
		lg.cchDescriptionTop = bstrTopDesc.Length();
	}
	if (!strBottomDesc.IsEmpty())
	{
		lg.mask |= LVGF_DESCRIPTIONBOTTOM;
		lg.pszDescriptionBottom = bstrBottomDesc;
		lg.cchDescriptionBottom = bstrBottomDesc.Length();
	}
#endif

	if (SetGroupInfo(nGroupId, (PLVGROUP)&lg)==-1)
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

//------------------------------------------------------------------------
//! LVN_GETEMPTYMARKUP message handler to show markup text when the list
//! control is empty.
//!
//! @param pNMHDR Pointer to NMLVEMPTYMARKUP structure
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::OnGetEmptyMarkup(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_EmptyMarkupText.IsEmpty())
		return FALSE;

#if _WIN32_WINNT >= 0x0600
	NMLVEMPTYMARKUP* pEmptyMarkup = reinterpret_cast<NMLVEMPTYMARKUP*>(pNMHDR);
	pEmptyMarkup->dwFlags = EMF_CENTERED;

#ifdef UNICODE
	lstrcpyn(pEmptyMarkup->szMarkup, (LPCTSTR)m_EmptyMarkupText, sizeof(pEmptyMarkup->szMarkup)/sizeof(WCHAR));
#else
#if __STDC_WANT_SECURE_LIB__
	mbstowcs_s(NULL, pEmptyMarkup->szMarkup, static_cast<LPCTSTR>(m_EmptyMarkupText), sizeof(pEmptyMarkup->szMarkup)/sizeof(WCHAR));
#else
	mbstowcs(pEmptyMarkup->szMarkup, static_cast<LPCTSTR>(m_EmptyMarkupText), sizeof(pEmptyMarkup->szMarkup)/sizeof(WCHAR));
#endif
#endif
	*pResult = TRUE;
#endif

	return TRUE;
}

//------------------------------------------------------------------------
//! LVN_LINKCLICK message handler called when a group task link is clicked
//!
//! @param pNMHDR Pointer to NMLVLINK structure
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::OnGroupTaskClick(NMHDR* pNMHDR, LRESULT* pResult)
{
#if _WIN32_WINNT >= 0x0600
	NMLVLINK* pLinkInfo = reinterpret_cast<NMLVLINK*>(pNMHDR);
	int nGroupId = pLinkInfo->iSubItem;
	(nGroupId);	// Avoid unreferenced variable warning
#endif
	return FALSE;
}

//------------------------------------------------------------------------
//! The framework calls this member function when the user double-clicks
//! the left mouse button. Used to expand and collapse groups when group
//! header is clicked.
//!
//! @param nFlags Indicates whether various virtual keys are down (MK_CONTROL, MK_SHIFT, etc.)
//! @param point Specifies the x- and y-coordinate of the cursor relative to the upper-left corner of the window.
//------------------------------------------------------------------------
void CGridListCtrlGroups::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CGridListCtrlEx::OnLButtonDblClk(nFlags, point);

	if (!IsGroupStateEnabled())
		return;

	int nGroupId = GroupHitTest(point);
	(nGroupId);	// Avoid unreferenced variable warning
}

//------------------------------------------------------------------------
//! LVM_REMOVEALLGROUPS message handler to ensure that group by column is updated
//!
//! @param wParam Not used
//! @param lParam Not used
//! @return Not used
//------------------------------------------------------------------------
LRESULT CGridListCtrlGroups::OnRemoveAllGroups(WPARAM wParam, LPARAM lParam)
{
	m_GroupCol = -1;
	m_GroupSort = -1;

	// Let CListCtrl handle the event
	return DefWindowProc(LVM_REMOVEALLGROUPS, wParam, lParam);
}

//------------------------------------------------------------------------
//! HDN_ENDDRAG message handler called after a column have been dragged,
//! but before the column order has been updated. Used to dirty cached col id
//!
//! @param pNMHDR Pointer to an NMHEADER structure with information about the column just resized
//! @param pResult If the owner is performing external (manual) drag-and-drop management, it must be set to FALSE
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::OnHeaderEndDrag(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	// Cached column index has become dirty
	NMHEADER* pNMH = reinterpret_cast<NMHEADER*>(pNMHDR);
	if (pNMH->pitem->mask & HDI_ORDER)
	{
		m_GroupCol = -1;
		m_GroupSort = -1;
	}
	return CGridListCtrlEx::OnHeaderEndDrag(id, pNMHDR, pResult);
}

//------------------------------------------------------------------------
//! Override this method to provide the group a cell belongs to.
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param nGroupId Text string to display in the cell
//! @return True if the cell belongs to a group
//------------------------------------------------------------------------
bool CGridListCtrlGroups::OnDisplayCellGroup(int nRow, int nCol, int& nGroupId)
{
	return false;
}

//------------------------------------------------------------------------
//! LVN_GETDISPINFO message handler, which is called when details are
//! needed for an item that specifies callback.
//!		- Cell-Group, when item is using I_GROUPIDCALLBACK
//!
//! @param pNMHDR Pointer to an NMLVDISPINFO structure
//! @param pResult Not used
//! @return Is final message handler (Return FALSE to continue routing the message)
//------------------------------------------------------------------------
BOOL CGridListCtrlGroups::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVDISPINFO* pNMW = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	int nRow = pNMW->item.iItem;
	int nCol = pNMW->item.iSubItem;

	if (nRow< 0 || nRow >= GetItemCount())
		return FALSE;	// requesting invalid item

	if (nCol < 0 || nCol >= GetHeaderCtrl()->GetItemCount())
		return FALSE;	// requesting invalid item

	if (pNMW->item.mask & LVIF_GROUPID)
	{
		// Request group-id of the column (Virtual-list/LVS_OWNERDATA)
		int result = -1;
		if (OnDisplayCellGroup(nRow, nCol, result))
			pNMW->item.iGroupId = result;
		else
			pNMW->item.iGroupId = I_GROUPIDNONE;
	}
	return CGridListCtrlEx::OnGetDispInfo(pNMHDR, pResult);
}

//------------------------------------------------------------------------
//! Configure whether sorting on secondary column is allowed when grouped
//! by primary column
//!
//! @param nEnable 0 = Group on sort, 1 = Allow sort, 2 = Fix sort for WinXP
//------------------------------------------------------------------------
void CGridListCtrlGroups::SetSortSecondaryGroupView(int nEnable)
{
	m_SortSecondaryGroupView = nEnable;
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
		CSimpleMap<int,CString> m_GroupNames;

		const CString& LookupGroupName(int nGroupId)
		{
			int groupIdx = m_GroupNames.FindKey(nGroupId);
			if (groupIdx==-1)
			{
				static const CString emptyStr;
				return emptyStr;
			}
			return m_GroupNames.GetValueAt(groupIdx);
		}
	};

	int CALLBACK SortFuncGroup(int leftId, int rightId, void* lParamSort)
	{
		PARAMSORT& ps = *(PARAMSORT*)lParamSort;

		const CString& leftText = ps.LookupGroupName(leftId);
		const CString& rightText = ps.LookupGroupName(rightId);

		LVITEM leftItem = {0};
		leftItem.pszText = const_cast<LPTSTR>((LPCTSTR)leftText);
		leftItem.cchTextMax = leftText.GetLength();
		LVITEM rightItem = {0};
		rightItem.pszText = const_cast<LPTSTR>((LPCTSTR)rightText);
		rightItem.cchTextMax = leftText.GetLength();
		return ps.m_pTrait->OnSortRows(leftItem, rightItem, ps.m_Ascending);
	}
}

namespace
{
	struct group_info
	{
		int m_GroupId;
		CString m_GroupHeader;
		int m_CompareMethod;
		CGridColumnTrait* m_pTrait;

		explicit group_info(int groupId)
		{
			m_GroupId = groupId;
		}

		bool operator==(const group_info& other) const
		{
			if (m_GroupId==other.m_GroupId)
				return true;
			else
				return false;
		}
	};

	int group_info_cmp(const void *a, const void *b) 
	{ 
		struct group_info *ia = (struct group_info *)a;
		struct group_info *ib = (struct group_info *)b;

		if (ia->m_CompareMethod==-1)
		{
			return ia->m_GroupId - ib->m_GroupId;
		}
		else
		{
			LVITEM leftItem = {0};
			leftItem.pszText = const_cast<LPTSTR>((LPCTSTR)ia->m_GroupHeader);
			leftItem.cchTextMax = ia->m_GroupHeader.GetLength();
			LVITEM rightItem = {0};
			rightItem.pszText = const_cast<LPTSTR>((LPCTSTR)ib->m_GroupHeader);
			rightItem.cchTextMax = ib->m_GroupHeader.GetLength();
			return ia->m_pTrait->OnSortRows(leftItem, rightItem, ia->m_CompareMethod ? true : false);
		}
	}
}

//------------------------------------------------------------------------
//! Changes the row sorting in regard to the specified column
//!
//! @param nCol The index of the column
//! @param bAscending Should the arrow be up or down 
//! @return True / false depending on whether sort is possible
//------------------------------------------------------------------------
bool CGridListCtrlGroups::SortColumn(int nCol, bool bAscending)
{
	CWaitCursor waitCursor;

	if (IsGroupViewEnabled() && (m_GroupCol==-1 || m_GroupCol==nCol || m_SortSecondaryGroupView==0))
	{
		// If already grouped, then no need to group them again
		if (m_GroupCol!=nCol)
		{
			SetRedraw(FALSE);
			
			GroupByColumn(nCol);

			SetRedraw(TRUE);
			Invalidate(FALSE);
		}

		CSimpleArray<int> groupIds;
		if (!GetGroupIds(groupIds))
			return false;

		// Cannot use GetGroupInfo during sort
		PARAMSORT paramsort(m_hWnd, nCol, bAscending, GetColumnTrait(nCol));
		for(int i=0 ; i < groupIds.GetSize() ; ++i)
		{
			int nGroupId = groupIds[i];
			paramsort.m_GroupNames.Add(nGroupId, GetGroupHeader(nGroupId));
		}

		// Avoid bug in CListCtrl::SortGroups() which differs from ListView_SortGroups
		if (!ListView_SortGroups(m_hWnd, SortFuncGroup, &paramsort))
			return false;

		m_GroupSort = bAscending ? 1 : 0;
	}
	else
	{
		if (!IsGroupViewEnabled() || IsGroupStateEnabled())
		{
			if (!CGridListCtrlEx::SortColumn(nCol, bAscending))
				return false;
		}
		else
		{
			if (m_SortSecondaryGroupView!=2)
				return false;

			if (GetItemCount()<=0)
				return true;

			// WinXP doesn't support item sorting when items are grouped
			// The workaround is to re-create the groups after having sorted the items
			SetRedraw(FALSE);
			if (!CGridListCtrlEx::SortColumn(nCol, bAscending))
			{
				SetRedraw(TRUE);
				Invalidate(FALSE);
				return false;
			}

			int pos = GetScrollPos(SB_VERT);

			// Find all groups and register what group each row belongs to
			int nItemCount = GetItemCount();
			int* rowGroupArray = new int[(unsigned int)nItemCount];
			CGridColumnTrait* pColumnTrait = m_GroupCol!=-1 ? GetColumnTrait(m_GroupCol) : NULL;
			CSimpleArray<group_info> groupNames;
			for(int nRow=0; nRow < nItemCount; ++nRow)
			{
				int nGroupId = GetRowGroupId(nRow);
				group_info groupinfo(nGroupId);
				if (nGroupId!=-1 && groupNames.Find(groupinfo)==-1)
				{
					groupinfo.m_GroupHeader = GetGroupHeader(nGroupId);
					groupinfo.m_pTrait = pColumnTrait;
					groupinfo.m_CompareMethod = m_GroupCol!=-1 ? m_GroupSort : -1;
					groupNames.Add(groupinfo);
				}
				rowGroupArray[nRow] = nGroupId;
			}

			if (groupNames.GetSize() <= 0)
			{
				SetRedraw(TRUE);
				Invalidate(FALSE);
				return true;
			}

			// Attempt to order the found groups in their current order
			qsort(groupNames.m_aT, (unsigned int)groupNames.GetSize(), sizeof(struct group_info), group_info_cmp);

			// Backup these before RemoveAllGroups() generates LVM_REMOVEALLGROUPS
			int nGroupCol = m_GroupCol;
			BOOL bGroupSort = m_GroupSort;

			RemoveAllGroups();
			EnableGroupView(FALSE);
			EnableGroupView(TRUE);

			// Restore these again
			m_GroupCol = nGroupCol;
			m_GroupSort = bGroupSort;

			// Regenerate groups again in the original order
			for(int nGroupIdx = 0; nGroupIdx < groupNames.GetSize(); ++nGroupIdx)
			{
				int nGroupId = groupNames[nGroupIdx].m_GroupId;
				const CString& strGroupName = groupNames[nGroupIdx].m_GroupHeader;
				DWORD dwState = LVGS_NORMAL;
				VERIFY( InsertGroupHeader(nGroupIdx, nGroupId, strGroupName, dwState) != -1);
			}

			// Re-add the now sorted items to their original groups
			for(int nRow2 = 0; nRow2 < nItemCount; ++nRow2)
			{
				VERIFY( SetRowGroupId(nRow2, rowGroupArray[nRow2]) );
			}

			delete [] rowGroupArray;

			Scroll(CSize(0,pos));
			SetRedraw(TRUE);
			Invalidate(FALSE);
		}
	}

	return true;
}

//------------------------------------------------------------------------
//! Notify that the window has been destroyed
//------------------------------------------------------------------------
void CGridListCtrlGroups::OnDestroy()
{
	m_GroupCol = -1;
	m_GroupSort = -1;
	CGridListCtrlEx::OnDestroy();
}

//------------------------------------------------------------------------
//! WM_PAINT message handler called when needing to redraw list control.
//! Used to display text when the list control is empty
//------------------------------------------------------------------------
void CGridListCtrlGroups::OnPaint()
{
#if _WIN32_WINNT >= 0x0600
	if (UsingVisualStyle())
	{
		// Use LVN_GETEMPTYMARKUP if available
		CListCtrl::OnPaint();	// default
		return;
	}
#endif

	CGridListCtrlEx::OnPaint();
}

// MFC headers with group-support is only availabe from VS.NET 
#if _MSC_VER < 1300

AFX_INLINE LRESULT CGridListCtrlGroups::InsertGroup(int index, PLVGROUP pgrp)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_InsertGroup(m_hWnd, index, pgrp);
}
AFX_INLINE int CGridListCtrlGroups::SetGroupInfo(int iGroupId, PLVGROUP pgrp)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_SetGroupInfo(m_hWnd, iGroupId, pgrp);
}
AFX_INLINE int CGridListCtrlGroups::GetGroupInfo(int iGroupId, PLVGROUP pgrp) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_GetGroupInfo(m_hWnd, iGroupId, pgrp);
}
AFX_INLINE LRESULT CGridListCtrlGroups::RemoveGroup(int iGroupId)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_RemoveGroup(m_hWnd, iGroupId);
}
AFX_INLINE LRESULT CGridListCtrlGroups::MoveGroup(int iGroupId, int toIndex)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_MoveGroup(m_hWnd, iGroupId, toIndex);
}
AFX_INLINE LRESULT CGridListCtrlGroups::MoveItemToGroup(int idItemFrom, int idGroupTo)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_MoveItemToGroup(m_hWnd, idItemFrom, idGroupTo);
}
AFX_INLINE void CGridListCtrlGroups::SetGroupMetrics(PLVGROUPMETRICS pGroupMetrics)
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_SetGroupMetrics(m_hWnd, pGroupMetrics);
}
AFX_INLINE void CGridListCtrlGroups::GetGroupMetrics(PLVGROUPMETRICS pGroupMetrics) const
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_GetGroupMetrics(m_hWnd, pGroupMetrics);
}
AFX_INLINE LRESULT CGridListCtrlGroups::EnableGroupView(BOOL fEnable)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_EnableGroupView(m_hWnd, fEnable);
}
AFX_INLINE BOOL CGridListCtrlGroups::SortGroups(PFNLVGROUPCOMPARE _pfnGroupCompare, LPVOID _plv)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)::SendMessage(m_hWnd, LVM_SORTGROUPS, (WPARAM)(LPARAM)_plv, (LPARAM)_pfnGroupCompare );
}
AFX_INLINE LRESULT CGridListCtrlGroups::InsertGroupSorted(PLVINSERTGROUPSORTED pStructInsert)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_InsertGroupSorted(m_hWnd, pStructInsert);
}
AFX_INLINE void CGridListCtrlGroups::RemoveAllGroups()
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_RemoveAllGroups(m_hWnd);
}
AFX_INLINE BOOL CGridListCtrlGroups::HasGroup(int iGroupId) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_HasGroup(m_hWnd, (WPARAM)iGroupId);
}
AFX_INLINE BOOL CGridListCtrlGroups::IsGroupViewEnabled() const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_IsGroupViewEnabled(m_hWnd);
}
#endif // _MSC_VER < 1300

#endif // _MSC_VER < 1500 || defined UNICODE || defined CGRIDLISTCTRLEX_GROUPMODE

#endif // _WIN32_WINNT >= 0x0501
