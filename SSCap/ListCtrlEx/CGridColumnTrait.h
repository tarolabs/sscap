#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

class CGridColumnTraitVisitor;
class CGridListCtrlEx;

#pragma warning(push)
#pragma warning(disable:4100)	// unreferenced formal parameter

//------------------------------------------------------------------------
//! CGridColumnTrait specifies the methods needed for custom cell handling
//------------------------------------------------------------------------
class CGridColumnTrait
{
public:
	//! Destructor
	virtual ~CGridColumnTrait() {}

	//! Override OnInsertColumn() to provide your own special styling of the column,
	//! after column has been added.
	//!
	//! @param owner The list control adding column
	//! @param nCol The index of the column just added
	virtual void OnInsertColumn(CGridListCtrlEx& owner, int nCol) {}

	//! Override OnCustomDraw() to provide your own special cell-drawing.
	//!   - Must handle selection drawing
	//!   - Must handle focus drawing
	//!	  - Must handle selection drawing when the CListCtrl doesn't have focus
	//!   - Must query owner if special foreground/background-color or font should be used
	//!
	//! @param owner The list control drawing
	//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
	//! @param pResult Modification to the drawing stage (CDRF_NEWFONT, etc.)
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult) {}

	//! Override OnClickEditStart() to control whether cell edit should be started
	//! when clicked with the mouse. OnEditBegin() will be called when return value >= 1.
	//! Do NOT start the editor within this method, as it will cause havoc in the mouse click handler.
	//!
	//! @param owner The list control being clicked
	//! @param nRow The index of the row
	//! @param nCol The index of the column
	//! @param pt The position clicked, in client coordinates.
	//! @param bDblClick The position was double clicked
	//! @return How should the cell editor be started (0 = No editor, 1 = Start Editor, 2 = Start Editor and block click-event)
	virtual int OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt, bool bDblClick) { return 0; }

	//! Override OnEditBegin() to provide your own special cell-edit control.
	//!   - The edit control must inherit from CWnd
	//!   - The edit control must delete itself when it looses focus
	//!   - The edit control must send a LVN_ENDLABELEDIT message when edit is complete
	//!
	//! @param owner The list control starting edit
	//! @param nRow The index of the row for the cell to edit
	//! @param nCol The index of the column for the cell to edit
	//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol) { return NULL; }

	//! Override OnEditBegin() to provide your own special cell-edit control.
	//!   - The edit control must inherit from CWnd
	//!   - The edit control must delete itself when it looses focus
	//!   - The edit control must send a LVN_ENDLABELEDIT message when edit is complete
	//!
	//! @param owner The list control starting edit
	//! @param nRow The index of the row for the cell to edit
	//! @param nCol The index of the column for the cell to edit
	//! @param pt The position clicked, in client coordinates.
	//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt) { return OnEditBegin(owner, nRow, nCol); }

	//! Override OnEditEnd() in case one need to change state after a cell-edit.
	virtual void OnEditEnd() {}

	//! Override OnSortRows() to provide your own special row sorting
	//!
	//! @param leftItem Left cell item
	//! @param rightItem Right cell item
	//! @param bAscending Perform sorting in ascending or descending order
	//! @return Is left value less than right value (-1) or equal (0) or larger (1)
	virtual int OnSortRows(const LVITEM& leftItem, const LVITEM& rightItem, bool bAscending) { return OnSortRows(leftItem.pszText, rightItem.pszText, bAscending); }

	//! Override OnSortRows() to provide your own special row sorting
	//!
	//! @param pszLeftValue Left cell value
	//! @param pszRightValue Right cell value
	//! @param bAscending Perform sorting in ascending or descending order
	//! @return Is left value less than right value (-1) or equal (0) or larger (1)
	virtual int OnSortRows(LPCTSTR pszLeftValue, LPCTSTR pszRightValue, bool bAscending) { return 0; }

	//! Override Accept() and update CGridColumnTraitVisitor for new column-trait classes.
	//!   - Will enable the use of the visitor-pattern ex. for serialization of column-traits
	virtual void Accept(CGridColumnTraitVisitor& visitor) {}

	//! Override IsCellReadOnly() to provide custom control whether a cell can be edited
	//!
	//! @param owner The list control starting edit
	//! @param nRow The index of the row for the cell
	//! @param nCol The index of the column for the cell
	//! @param pt The position clicked, in client coordinates.
	//! @return Is cell read only ? (true / false)
	virtual bool IsCellReadOnly(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt) const { return !m_ColumnState.m_Editable; }

	// Maintaining column visible state, etc.
	struct ColumnState
	{
		ColumnState()
			:m_Visible(true)
			,m_OrgWidth(0)
			,m_OrgPosition(-1)
			,m_AlwaysHidden(false)
			,m_AlwaysVisible(false)
			,m_Sortable(true)
			,m_Editable(true)
			,m_Resizable(true)
			,m_MetaFlags(0)
			,m_MinWidth(-1)
			,m_MaxWidth(-1)
		{}
		bool m_Visible;		//!< Column is visible or not
		int  m_OrgWidth;	//!< Width it had before being hidden
		int  m_OrgPosition;	//!< Position it had before being hidden
		bool m_AlwaysHidden;//!< Column can never be visible
		bool m_AlwaysVisible;//!< Column can never be hidden
		bool m_Sortable;	//!< Rows can be sorted according to column
		bool m_Editable;	//!< Cells in the column can be edited
		bool m_Resizable;	//!< Column width is resizable
		int  m_MinWidth;	//!< Column width has a min size
		int  m_MaxWidth;	//!< Column width has a max size

		//! Meta-Flags (and data) can be used to store extra properties for a column
		//! when deriving from CGridListCtrlEx.
		DWORD m_MetaFlags;
	};
	inline ColumnState& GetColumnState() { return m_ColumnState; }

	inline BOOL HasMetaFlag(DWORD flag) { return (m_ColumnState.m_MetaFlags & flag) ? TRUE : FALSE; }
	void SetMetaFlag(DWORD flag, bool enable)
	{
		if (enable)
			m_ColumnState.m_MetaFlags |= flag;
		else
			m_ColumnState.m_MetaFlags &= ~flag;
	}

protected:
	ColumnState m_ColumnState;
};

#pragma warning(pop)