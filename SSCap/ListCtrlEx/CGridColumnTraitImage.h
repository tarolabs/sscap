#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTraitText.h"

//------------------------------------------------------------------------
//! CGridColumnTraitImage implements an image switcher (can mimic a checkbox)
//!
//! By adding checkbox state-images to the official imagelist using
//! AppendStateImages(), then one can use this column trait as checkbox
//! editor. To get/set the checkbox value of a cell use the methods
//! GetCellImage()/SetCellImage() on CGridListCtrlEx
//------------------------------------------------------------------------
class CGridColumnTraitImage : public CGridColumnTraitText
{
public:
	CGridColumnTraitImage();
	CGridColumnTraitImage(int nImageIndex, int nImageCount);

	void AddImageIndex(int nImageIdx);
	void AddImageIndex(int nImageIdx, const CString& strImageText, bool bEditable = true);

	void SetImageText(int nImageIdx, const CString& strImageText, bool bEditable = true);

	void SetSortImageIndex(bool bValue);
	bool GetSortImageIndex() const;

	void SetToggleSelection(bool bValue);
	bool GetToggleSelection() const;

	void SetSingleClickEdit(bool bValue);
	bool GetSingleClickEdit() const;

	void SetIconClickBeginEdit(bool bValue);
	bool GetIconClickBeginEdit() const;

	static int AppendStateImages(CGridListCtrlEx& owner, CImageList& imagelist);
	static LRESULT SendEndLabelEdit(CWnd& wndListCtrl, int nRow, int nCol, LV_DISPINFO& dispInfo);

protected:
	virtual int OnSortRows(LPCTSTR pszLeftValue, LPCTSTR pszRightValue, bool bAscending) { return CGridColumnTraitText::OnSortRows(pszLeftValue, pszRightValue, bAscending); }
	virtual int OnSortRows(const LVITEM& leftItem, const LVITEM& rightItem, bool bAscending);
	virtual bool IsCellReadOnly(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt) const;
	virtual int OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt, bool bDblClick);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol) { return CGridColumnTraitText::OnEditBegin(owner, nRow, nCol); }
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt);
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual int FlipImageIndex(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual CWnd* OnEditBeginImage(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual CWnd* OnEditBeginCheckbox(CGridListCtrlEx& owner, int nRow, int nCol);

	//! @cond INTERNAL
	struct ImageCell
	{
		CString m_CellText;
		bool m_Editable;

		ImageCell()
			: m_Editable(true) {}
		ImageCell(const CString& cellText, bool editable)
			: m_CellText(cellText), m_Editable(editable) {}
	};
	//! @endcond INTERNAL

	CSimpleMap<int,ImageCell> m_ImageIndexes;	//!< Fixed list of image items to switch between

	bool m_SortImageIndex;	//!< Should image be used as primary sort index ?
	bool m_ToggleSelection;	//!< Should the image of all selected rows be flipped, when clicked ?
	bool m_SingleClickEdit;	//!< Should it start editor on first click, instead of first waiting for cell to have focus first
	bool m_IconClickBeginEdit; //!< Should it start editor when clicking the icon area ?
};