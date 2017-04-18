#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridListCtrlEx.h"

//------------------------------------------------------------------------
//! CGridListCtrlGroups extends the CGridListCtrlEx with grouping.
//! This can be used to put rows into category groups.
//!
//! Placed in its own file as all features requires _WIN32_WINNT >= 0x0501
//------------------------------------------------------------------------
class CGridListCtrlGroups : public CGridListCtrlEx
{
	DECLARE_DYNAMIC(CGridListCtrlGroups)
public:
// WIN32 defines for group-support is only available from 2003 PSDK
#if _WIN32_WINNT >= 0x0501

// VS2008 Marks group-mode functionality as deprecated if not Unicode build (define CGRIDLISTCTRLEX_GROUPMODE in stdafx.h to avoid this check)
#if _MSC_VER < 1500 || defined UNICODE || defined CGRIDLISTCTRLEX_GROUPMODE
	CGridListCtrlGroups();

	virtual LRESULT InsertGroupHeader(int nIndex, int nGroupId, const CString& strHeader, DWORD dwState = 0, DWORD dwAlign = 0);

	virtual CString GetGroupHeader(int nGroupId);
	virtual int GetRowGroupId(int nRow);
	virtual BOOL SetRowGroupId(int nRow, int nGroupId);
	virtual int GroupHitTest(const CPoint& point);
	virtual int FixRowGroupId(int nRow);

	virtual BOOL GetGroupIds(CSimpleArray<int>& groupIds);

	virtual BOOL GroupByColumn(int nCol);
	virtual BOOL DeleteEntireGroup(int nGroupId);
	virtual BOOL IsGroupStateEnabled();

	virtual void CheckEntireGroup(int nGroupId, bool bChecked);

	virtual void SetSortSecondaryGroupView(int nEnable);
	virtual bool SortColumn(int nCol, bool bAscending);

	virtual BOOL HasGroupState(int nGroupId, DWORD dwState);
	virtual BOOL SetGroupState(int nGroupId, DWORD dwState);

	virtual BOOL CollapseAllGroups();
	virtual BOOL ExpandAllGroups();

	virtual BOOL SetGroupFooter(int nGroupId, const CString& strFooter, DWORD dwAlign = 0);
	virtual BOOL SetGroupTask(int nGroupId, const CString& strTask);
	virtual BOOL SetGroupSubtitle(int nGroupId, const CString& strSubtitle);
	virtual BOOL SetGroupTitleImage(int nGroupId, int nImage, const CString& strTopDesc, const CString& strBottomDesc);

	// DataModel callbacks
	virtual bool OnDisplayCellGroup(int nRow, int nCol, int& nGroupId);

protected:
	// Context Menu Handlers
	virtual void OnContextMenuGrid(CWnd* pWnd, CPoint point);
	virtual void OnContextMenuHeader(CWnd* pWnd, CPoint point, int nCol);
	virtual void OnContextMenuGroup(CWnd* pWnd, CPoint point, int nGroupId);

	// Drag Drop Interface
	virtual BOOL OnDropSelf(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual bool MoveSelectedRows(int nGroupId);

	//{{AFX_MSG(CGridListCtrlGroups)
	virtual afx_msg void OnContextMenu(CWnd*, CPoint point);
	virtual afx_msg BOOL OnGroupTaskClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual afx_msg BOOL OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnGetEmptyMarkup(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg void OnDestroy();
	virtual afx_msg void OnPaint();
	virtual afx_msg LRESULT OnRemoveAllGroups(WPARAM wParam, LPARAM lParam);
	virtual afx_msg BOOL OnHeaderEndDrag(UINT, NMHDR* pNmhdr, LRESULT* pResult);
	//}}AFX_MSG

	int m_GroupCol;		//!< Rows are grouped according to this column
	int m_GroupSort;	//!< Groups are sorted (-1 = Unsorted, 1 = Ascending, 0 = Descending)
	int m_SortSecondaryGroupView;	//!< When grouped by primary column, then allow sort of secondary column (0 = Always group, 1 = Allow sort, 2 = Fix sort for xp)

	DECLARE_MESSAGE_MAP();

public:
// MFC headers with group-support is only availabe from VS.NET 
#if _MSC_VER < 1300
	LRESULT InsertGroup(int index, PLVGROUP pgrp);
	int SetGroupInfo(int iGroupId, PLVGROUP pgrp);
	int GetGroupInfo(int iGroupId, PLVGROUP pgrp) const;	
	LRESULT RemoveGroup(int iGroupId);	
	LRESULT MoveGroup(int iGroupId, int toIndex);
	LRESULT MoveItemToGroup(int idItemFrom, int idGroupTo);
	void SetGroupMetrics(PLVGROUPMETRICS pGroupMetrics);
	void GetGroupMetrics(PLVGROUPMETRICS pGroupMetrics) const;
	LRESULT EnableGroupView(BOOL fEnable);
	BOOL SortGroups(PFNLVGROUPCOMPARE _pfnGroupCompare, LPVOID _plv);
	LRESULT InsertGroupSorted(PLVINSERTGROUPSORTED pStructInsert);
	void RemoveAllGroups();
	BOOL HasGroup(int iGroupId) const;
	BOOL IsGroupViewEnabled() const;
#endif	// _MSC_VER < 1300
#endif	// _MSC_VER < 1500 || defined UNICODE || defined CGRIDLISTCTRLEX_GROUPMODE
#endif	// _WIN32_WINNT >= 0x0501

private:
	CGridListCtrlGroups(const CGridListCtrlGroups&);
	CGridListCtrlGroups& operator=(const CGridListCtrlGroups&);
};
