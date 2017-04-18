#pragma once

#include "ListCtrlEx/CGridListCtrlEx.h"
#include <set>
#include "mymutex.h"

using namespace std;

struct stColor
{
	int nRow;
	int nCol;
	COLORREF rgb;
};
typedef set<DWORD> SET_COLUMN_COLOR;
typedef set<DWORD>::iterator ITER_SET_COLUMN_COLOR;

class CSSListCtrl : public CGridListCtrlEx
{
public:
	CSSListCtrl();
	~CSSListCtrl();

	//------------------------------------------------------------------------
	//! Override this method to change the color used for drawing a row
	//!
	//! @param nRow The index of the row
	//! @param textColor The text color used when drawing the row
	//! @param backColor The background color when drawing the row
	//! @return Color is overrided
	//------------------------------------------------------------------------
	bool CSSListCtrl::OnDisplayRowColor(int nRow, COLORREF& textColor, COLORREF& backColor);
	virtual bool OnDisplayCellColor(int nRow, int nCol, COLORREF& textColor, COLORREF& backColor);
	virtual bool OnDisplayCellText(int nRow, int nCol, CString& strResult);
	virtual bool OnDisplayCellImage(int nRow, int nCol, int& nImageId);
	// ÖØÔØ, ½ûÖ¹ITEMÍÏ×§
	virtual void OnCreateStyle();
	// DataModel callbacks
	virtual void OnDisplayCellItem(LVITEM& lvi);
	virtual bool SortColumn(int nCol, bool bAscending);

	void SetItemColumnChanged( int row, int col );
	void SetItemColumnUnChanged( int row, int col );
protected:
	CMyMutex mutexItemChange;
	SET_COLUMN_COLOR setColumnChanged;
	//virtual afx_msg BOOL OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
};