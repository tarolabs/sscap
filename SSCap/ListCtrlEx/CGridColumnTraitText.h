#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTrait.h"

//------------------------------------------------------------------------
//! CGridColumnTraitText provides customization of cell text and background
//------------------------------------------------------------------------
class CGridColumnTraitText : public CGridColumnTrait
{
public:
	CGridColumnTraitText();
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);	
	virtual int OnSortRows(LPCTSTR pszLeftValue, LPCTSTR pszRightValue, bool bAscending);
	virtual int OnSortRows(const LVITEM& leftItem, const LVITEM& rightItem, bool bAscending) { return CGridColumnTrait::OnSortRows(leftItem, rightItem, bAscending); }

	void SetSortFormatNumber(bool bValue);

protected:
	CFont*	m_pOldFont;		//!< Backup of the original font while drawing with specified font
	COLORREF m_OldTextColor;//!< Backup of the original text color while drawing with specified color
	COLORREF m_OldBackColor;//!< Backup of the original background color while drawing with specified color
	COLORREF m_TextColor;	//!< Text color to use for this column
	COLORREF m_BackColor;	//!< Background color to use for this column
	bool m_SortFormatNumber;//!< Column contains integers

	virtual bool UpdateTextFont(NMLVCUSTOMDRAW* pLVCD, LOGFONT& textFont);
	virtual bool UpdateTextColor(NMLVCUSTOMDRAW* pLVCD, COLORREF& textColor);
	virtual bool UpdateBackColor(NMLVCUSTOMDRAW* pLVCD, COLORREF& backColor);

	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual int GetCellFontHeight(CGridListCtrlEx& owner);
	virtual CRect GetCellEditRect(CGridListCtrlEx& owner, int nRow, int nCol);
};
