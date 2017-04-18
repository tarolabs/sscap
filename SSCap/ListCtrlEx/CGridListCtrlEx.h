#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
// Version: 2.1
//
// Change History:
//	2.2 - CGridColumnTraitCombo now has the option to display dropdown on edit begin (SetShowDropDown)
//		  All column trait editors now has the option to start editor on first mouse click (SetSingleClickEdit)
//		  CGridColumnTraitHyperLink will sent LVN_ENDLABELEDIT notification to parent on mouse click (Simulates button click)
//		  Checkbox state icon used by subitems has been improved on Windows 7/8 by disabling image scaling
//		  Fixed several small bugs
//	2.1 - Fixed bug introduced in 2.0, where grouping of items no longer worked on WinXP.
//		  Improved performance when sorting item groups, especially on Vista/Win7+.
//		  Added new column trait CGridColumnTraitHyperLink, that can display cell text as weblinks.
//		  Added new column trait CGridColumnTraitMultilineEdit, that can edit cell text that contains newlines
//		  Fixed several small bugs
//	2.0 - Added copy/paste support for CDateTimeCtrl editor (DTS_APPCANPARSE)
//		  When having grouped by column, then column header click now sorts instead of grouping by column.
//		  Changed the row sorting to be case insensitive by default
//		  Fixed bug where cell editor discarded changes performed using the mouse (copy/paste using context menu)
//		  Fixed several compiler warnings, and small bugs
// 	1.9 - Added new CGridColumnTrait::OnSortRows() to take LVITEM as parameter (2011-05-30)
//		  Renamed CGridColumnConfig to CViewConfigSection (Now general purpose settings manager)
//		  Removed CGridColumnManager and moved LoadState/SaveState into CGridListCtrlEx (Breaking change)
//		  Fixed breaking change in 1.8 where OnEditBegin overrides stopped working
//	1.8 - Added checkbox support for all column editor types (2010-10-01)
//		  Added checkbox toggle for all selected rows
//		  Added min and max width for columns
//	1.7 - Added CGridColumnTraitImage, that provides checkbox editing (2009-12-12)
//		  Renamed OnTraitCustomDraw() to OnCustomDrawCell()
//		  Renamed OnTraitEditBegin() to OnEditBegin()
//		  Renamed OnTraitEditComplete() to OnEditComplete()
//	1.6 - Added OLE drag and drop support (2009-08-10)
//		  Added support for LVS_EX_CHECKBOXES with LVS_OWNERDATA
//		  Added better support for keyboard search with LVS_OWNERDATA
//	1.5 - Added column manager CGridColumnManager (2009-03-29)
//		  Added support for groups on VC6 with platform SDK
//		  Added sample projects for the different versions of Visual Studio
//		  Improved documentation through Doxygen comments
//	1.4 - Added clipboard support (2008-11-07)
//		  Renamed the "Callback"-functions to "OnDisplay"
//	1.3 - Added support for compiling on VC6 (2008-10-09)
//	1.2 - Added row traits and CGridRowTraitXP (2008-09-24)
//	1.1 - Added support for groups with CGridListCtrlGroups (2008-09-18)
//		  Added support for CDateTimeCtrl as cell editor
//	1.0 - Initial Release (2008-09-04)
//------------------------------------------------------------------------

class COleDataSource;
class CViewConfigSection;
class CViewConfigSectionProfiles;
class CGridColumnTrait;
class CGridRowTrait;
template<class T> class COleDropTargetWnd;

//------------------------------------------------------------------------
//! \mainpage Introduction
//! CGridListCtrlEx extends the CListCtrl from MFC. CGridListCtrlGroups
//! extends CGridListCtrlEx even further to support categorization of rows in groups.
//!
//! CGridListCtrlEx makes use of different helper classes that makes it easy
//! to customize and extend it even further.
//! 
//! - CGridColumnTrait provides special behavior to the cells in a column
//!		- CGridColumnTraitEdit Implements cell editing using CEdit
//!		- CGridColumnTraitCombo Implements cell editing using CComboBox
//!		- CGridColumnTraitDateTime Implements cell editing using CDateTimeCtrl
//!		- CGridColumnTraitHyperLink Implements cell behavior as hyperlinks
//!		- CGridColumnTraitImage Implements cell editing using cell-image (can mimic checkbox)
//!		- CGridColumnTraitMultilineEdit Implements cell editing using multiline CEdit 
//! - CGridRowTrait provides an interface to perform custom drawing at row level
//!		- CGridRowTraitText implements alternate row coloring
//!			- CGridRowTraitXP removes the white background for cell images on WinXP
//! - CViewConfigSection provides column state persistence
//!		- CViewConfigSectionProfiles provides the ability switch between different column setups
//------------------------------------------------------------------------

//------------------------------------------------------------------------
//! CGridListCtrlEx extends the CListCtrl with several features.
//! - Cell editing and customization through use of CGridColumnTrait
//! - Sortable
//! - Simple column picker
//! - Cell navigation
//! - Keyboard search navigation
//! - Cell tooltip
//! - Clipboard (copy only)
//! - OLE Drag and drop
//! - Column state persistence (width, order, etc.)
//------------------------------------------------------------------------
class CGridListCtrlEx : public CListCtrl
{
	DECLARE_DYNAMIC(CGridListCtrlEx)
public:
	CGridListCtrlEx();
	~CGridListCtrlEx();

	virtual void DeleteObjects();
	// CListCtrl
	LRESULT EnableVisualStyles(bool bValue);
	inline bool UsingVisualStyle() const { return m_UsingVisualStyle; }
	virtual CFont* GetCellFont();
	virtual void SetCellMargin(double margin);
	void SetEmptyMarkupText(const CString& strText);
	void SetTooltipMaxWidth(int width) { m_TooltipMaxWidth = width; }
	int GetTooltipMaxWidth() const { return m_TooltipMaxWidth; }
	static bool CheckOSVersion(WORD requestOS);

	// Row
	int GetFocusRow() const;
	void SetFocusRow(int nRow);
	bool IsRowSelected(int nRow) const;
	BOOL SelectRow(int nRow, bool bSelect);
	virtual CGridRowTrait* GetRowTrait(int nRow);
	virtual void SetDefaultRowTrait(CGridRowTrait* pRowTrait);

	// Column
	const CHeaderCtrl* GetHeaderCtrl() const;
	CHeaderCtrl* GetHeaderCtrl() { return CListCtrl::GetHeaderCtrl(); }
	int GetColumnCount() const;
	int GetColumnData(int nCol) const;
	int GetColumnOrder(int nCol) const;
	CString GetColumnHeading(int nCol) const;
	virtual BOOL EnsureColumnVisible(int nCol, bool bPartialOK);
	virtual BOOL SetColumnWidthAuto(int nCol = -1, bool bIncludeHeader = false);
	virtual void SetSortArrow(int nCol, bool bAscending);
	virtual BOOL ShowColumn(int nCol, bool bShow);
	virtual bool IsColumnVisible(int nCol);
	virtual bool IsColumnResizable(int nCol);
	virtual bool IsColumnAlwaysVisible(int nCol);
	virtual bool IsColumnAlwaysHidden(int nCol);
	virtual int GetFirstVisibleColumn();
	virtual int InsertHiddenLabelColumn();
	virtual int InsertColumnTrait(int nCol, const CString& strColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1, CGridColumnTrait* pTrait = NULL);
	virtual CGridColumnTrait* GetColumnTrait(int nCol);
	virtual int GetColumnTraitSize() const;

	// Cell / Subitem 
	UINT CellHitTest(const CPoint& pt, int& nRow, int& nCol) const;
	BOOL GetCellRect(int nRow, int nCol, int nCode, CRect& rect);
	inline int GetFocusCell() const { return m_FocusCell; }
	virtual void SetFocusCell(int nCol, bool bRedraw = false);
	virtual CWnd* EditCell(int nRow, int nCol);
	virtual CWnd* EditCell(int nRow, int nCol, CPoint pt);
	bool IsCellEditorOpen() const;
	bool IsCellCallback(int nRow, int nCol) const;
	int GetCellImage(int nRow, int nCol) const;
	BOOL SetCellImage(int nRow, int nCol, int nImageId);
	virtual CGridColumnTrait* GetCellColumnTrait(int nRow, int nCol);

	// Column Editor
	virtual void SetupColumnConfig(CViewConfigSectionProfiles* pColumnConfig, bool bConfigOwner = true);
	virtual void LoadState(CViewConfigSection& config);
	virtual void SaveState(CViewConfigSection& config);
	virtual void LoadColumnState(int nConfigCol, int nOwnerCol, CViewConfigSection& config);
	virtual void SaveColumnState(int nConfigCol, int nOwnerCol, CViewConfigSection& config);
	virtual bool HasColumnEditor(int nCol, CString& strTitle);
	virtual void OpenColumnEditor(int nCol);
	virtual bool HasColumnPicker(CString& strTitle);
	virtual void OpenColumnPicker();
	virtual bool HasColumnDefaultState(CString& strTitle);
	virtual void ResetColumnDefaultState();
	virtual CString HasColumnProfiles(CSimpleArray<CString>& profiles, CString& strTitle);
	virtual void SwichColumnProfile(const CString& strProfile);
	virtual void OnSaveStateColumnPick();
	virtual void OnSaveStateColumnResize();
	virtual void OnSaveStateKillFocus();

	// DataModel callbacks
	virtual void OnDisplayCellItem(LVITEM& lvi);
	virtual bool OnDisplayCellText(int nRow, int nCol, CString& strResult);
	virtual bool OnDisplayCellImage(int nRow, int nCol, int& nImageId);
	virtual bool OnDisplayCellTooltip(const CPoint& point) const;
	virtual bool OnDisplayCellTooltip(int nRow, int nCol, CString& strResult);
	virtual bool OnDisplayCellColor(NMLVCUSTOMDRAW* pLVCD);
	virtual bool OnDisplayCellColor(int nRow, int nCol, COLORREF& textColor, COLORREF& backColor);
	virtual bool OnDisplayCellFont(NMLVCUSTOMDRAW* pLVCD, LOGFONT& font);
	virtual bool OnDisplayCellFont(int nRow, int nCol, LOGFONT& font);
	virtual bool OnDisplayRowColor(int nRow, COLORREF& textColor, COLORREF& backColor);
	virtual bool OnDisplayRowFont(int nRow, LOGFONT& font);
	virtual void OnDisplayDragOverRow(int nRow);
	virtual bool OnDisplayToClipboard(CString& strResult, bool includeHeader = true);
	virtual bool OnDisplayToClipboard(int nRow, CString& strResult);
	virtual bool OnDisplayToClipboard(int nRow, int nCol, CString& strResult);
	virtual bool OnDisplayToDragDrop(CString& strResult);
	virtual bool OnOwnerDataDisplayCheckbox(int nRow);
	virtual void OnOwnerDataToggleCheckBox(int nRow, bool bChecked);
	virtual int  OnKeyboardSearch(int nCol, int nStartRow, const CString& strSearch);

protected:
	// Maintaining column traits (and visible state)
	CSimpleArray<CGridColumnTrait*> m_ColumnTraits;	//!< Column traits registered (One for each column)
	virtual void InsertColumnTrait(int nCol, CGridColumnTrait* pTrait);
	virtual void DeleteColumnTrait(int nCol);
	CViewConfigSectionProfiles* m_pColumnConfig;	//!< Column state persistence
	bool m_bConfigOwner; //!< Column state persistence object is freed by destructor
	int InternalColumnPicker(CMenu& menu, UINT offset);
	int InternalColumnProfileSwitcher(CMenu& menu, UINT offset, CSimpleArray<CString>& profiles);

	// Maintaining row traits
	CGridRowTrait* m_pDefaultRowTrait;	//!< Default row trait used for special row drawing

	// Maintaining cell/subitem focus
	int m_FocusCell;			//!< Column currently having focus (-1 means entire row)
	virtual void MoveFocusCell(bool bMoveRight);

	// Maintaining Keyboard search
	CString m_LastSearchString;	//!< Last search criteria for keyboard search
	CTime	m_LastSearchTime;	//!< Time of last search attempt for keyboard search
	int		m_LastSearchCell;	//!< Last column used in keyboard search
	int		m_LastSearchRow;	//!< Last row matched in keyboard search
	int		m_RepeatSearchCount;//!< How many times the same search have been repeated (same key pressed)

	// Maintaining row sorting
	int m_SortCol;				//!< Rows are sorted according to this column
	bool m_Ascending;			//!< Rows are sorted ascending / descending
	virtual bool SortColumn(int nCol, bool bAscending);

	// Maintaining cell editing
	CWnd* m_pEditor;			//!< Cell value editor currently in use

	bool m_UsingVisualStyle;	//!< Vista Style has been enabled (alpha blend)

	int m_TooltipMaxWidth;		//!< Whether tooltips should be split in multiple lines

	// Maintaining margin
	CFont m_GridFont;			//!< Original font of the the list control
	CFont m_CellFont;			//!< Current font to draw rows
	double m_Margin;			//!< Current margin between original font and cell font

	CString m_EmptyMarkupText;	//!< Text to display when list control is empty
	bool m_InvalidateMarkupText;//!< Ensure that the empty markup text is cleared properly

	// Maintaining drag drop
	COleDropTargetWnd<CGridListCtrlEx>* m_pOleDropTarget;	//!< Maintains OLE drag drop target
	friend class COleDropTargetWnd<CGridListCtrlEx>;
	virtual BOOL RegisterDropTarget();
	virtual DROPEFFECT DoDragDrop(COleDataSource& oleDataSource);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual BOOL OnDropSelf(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual BOOL OnDropExternal(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual bool MoveSelectedRows(int nDropRow);

	// CustomDraw handlers
	virtual void OnCustomDrawRow(int nRow, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);
	virtual void OnCustomDrawCell(int nRow, int nCol, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);

	// Cell editing handlers
	virtual int OnClickEditStart(int nRow, int nCol, CPoint pt, bool bDblClick);
	virtual CWnd* OnEditBegin(int nRow, int nCol);
	virtual CWnd* OnEditBegin(int nRow, int nCol, CPoint pt);
	virtual bool OnEditComplete(int nRow, int nCol, CWnd* pEditor, LV_DISPINFO* pLVDI);

	// Context Menu Handlers
	virtual void OnContextMenuGrid(CWnd* pWnd, CPoint point);
	virtual void OnContextMenuHeader(CWnd* pWnd, CPoint point, int nCol);
	virtual void OnContextMenuKeyboard(CWnd* pWnd, CPoint point);
	virtual void OnContextMenuCell(CWnd* pWnd, CPoint point, int nRow, int nCol);

	virtual void OnCreateStyle();

	virtual void OnCopyToClipboard();

	//{{AFX_VIRTUAL(CGridListCtrlEx)
	virtual void PreSubclassWindow();
#if defined(_WIN64)
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
#else
	virtual int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
#endif
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGridListCtrlEx)
	virtual afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual afx_msg LRESULT OnDeleteColumn(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnInsertColumn(WPARAM wParam, LPARAM lParam);
	virtual afx_msg BOOL OnItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnItemDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	virtual afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg LRESULT OnSetColumnWidth(WPARAM wParam, LPARAM lParam);
	virtual afx_msg BOOL OnHeaderDividerDblClick(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderBeginResize(UINT, NMHDR* pNmhdr, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderBeginDrag(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderEndResize(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderItemChanging(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderEndDrag(UINT, NMHDR* pNmhdr, LRESULT* pResult);
	virtual afx_msg BOOL OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnToolNeedText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnBeginLabelEdit(NMHDR* pNMHDR,LRESULT* pResult);
	virtual afx_msg BOOL OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnOwnerDataFindItem(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual afx_msg void OnContextMenu(CWnd*, CPoint point);
	virtual afx_msg void OnDestroy();
	virtual afx_msg void OnPaint();
	virtual afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual afx_msg LRESULT OnCopy(WPARAM wParam, LPARAM lParam);
	virtual afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	virtual afx_msg BOOL OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	virtual afx_msg BOOL OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP();

private:
	CGridListCtrlEx(const CGridListCtrlEx&);
	CGridListCtrlEx& operator=(const CGridListCtrlEx&);
};

//{{AFX_INSERT_LOCATION}}
