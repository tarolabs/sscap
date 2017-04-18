//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "../stdafx.h"
#pragma warning(disable:4100)	// unreferenced formal parameter

#include "CGridColumnTraitDateTime.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//------------------------------------------------------------------------
//! CGridColumnTraitDateTime - Constructor
//------------------------------------------------------------------------
CGridColumnTraitDateTime::CGridColumnTraitDateTime()
	:m_ParseDateTimeFlags(0)
	,m_ParseDateTimeLCID(LOCALE_USER_DEFAULT)
	,m_DateTimeCtrlStyle(DTS_APPCANPARSE)
{}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridColumnTraitDateTime::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Set the DateTime format used to display the date in the editor
//!
//! @param strFormat Date Format string
//------------------------------------------------------------------------
void CGridColumnTraitDateTime::SetFormat(const CString& strFormat)
{
	m_Format = strFormat;
}

//------------------------------------------------------------------------
//! Get the DateTime format used to display the date in the editor
//!
//! @return Date Format string
//------------------------------------------------------------------------
CString CGridColumnTraitDateTime::GetFormat() const
{
	return m_Format;
}

//------------------------------------------------------------------------
//! Set the flags for converting the datetime in text format to actual date.
//!
//! @param dwFlags Flags for locale settings and parsing
//! @param lcid Locale ID to use for the conversion
//------------------------------------------------------------------------
void CGridColumnTraitDateTime::SetParseDateTime(DWORD dwFlags, LCID lcid)
{
	m_ParseDateTimeFlags = dwFlags;
	m_ParseDateTimeLCID = lcid;
}

//------------------------------------------------------------------------
//! Set style used when creating CDataTimeCtrl for cell value editing
//!
//! @param dwStyle Style flags
//------------------------------------------------------------------------
void CGridColumnTraitDateTime::SetStyle(DWORD dwStyle)
{
	m_DateTimeCtrlStyle = dwStyle;
}

//------------------------------------------------------------------------
//! Get style used when creating CDataTimeCtrl for cell value editing
//!
//! @return Style Flags
//------------------------------------------------------------------------
DWORD CGridColumnTraitDateTime::GetStyle() const
{
	return m_DateTimeCtrlStyle;
}

//------------------------------------------------------------------------
//! Parse the input string into a datetime value
//!
//! @param lpszDate The input string
//! @param dateTime The datetime value
//! @return Could the input string be converted into a valid datetime value ?
//------------------------------------------------------------------------
BOOL CGridColumnTraitDateTime::ParseDateTime(LPCTSTR lpszDate, COleDateTime& dateTime)
{
	if(dateTime.ParseDateTime(lpszDate, m_ParseDateTimeFlags, m_ParseDateTimeLCID)==FALSE)
	{
		dateTime.SetDateTime(1970, 1, 1, 0, 0, 0);
		return FALSE;
	}
	else
		return TRUE;
}

//------------------------------------------------------------------------
//! Create a CDateTimeCtrl as cell value editor
//!
//! @param owner The list control starting a cell edit
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param dwStyle The windows style to use when creating the CEdit
//! @param rect The rectangle where the inplace cell value editor should be placed
//! @return Pointer to the cell editor to use
//------------------------------------------------------------------------
CDateTimeCtrl* CGridColumnTraitDateTime::CreateDateTimeCtrl(CGridListCtrlEx& owner, int nRow, int nCol, DWORD dwStyle, const CRect& rect)
{
	// Create control to edit the cell
	CDateTimeCtrl* pDateTimeCtrl = new CGridEditorDateTimeCtrl(nRow, nCol, this);
	VERIFY( pDateTimeCtrl->Create(WS_CHILD | dwStyle, rect, &owner, 0) );
	if (!owner.UsingVisualStyle())
		pDateTimeCtrl->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE, SWP_FRAMECHANGED);	// Remove sunken edge
	return pDateTimeCtrl;
}

//------------------------------------------------------------------------
//! Overrides OnEditBegin() to provide a CDateTimeCtrl cell value editor
//!
//! @param owner The list control starting edit
//! @param nRow The index of the row for the cell to edit
//! @param nCol The index of the column for the cell to edit
//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
//------------------------------------------------------------------------
CWnd* CGridColumnTraitDateTime::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Convert cell-text to date/time format
	CString cellText = owner.GetItemText(nRow, nCol);
	COleDateTime dateTime;
	ParseDateTime(cellText, dateTime);

	// Get position of the cell to edit
	CRect rectCell = GetCellEditRect(owner, nRow, nCol);

	// Get the text-style of the cell to edit
	DWORD dwStyle = m_DateTimeCtrlStyle;
	HDITEM hd = {0};
	hd.mask = HDI_FORMAT;
	VERIFY( owner.GetHeaderCtrl()->GetItem(nCol, &hd) );
	if (hd.fmt & HDF_RIGHT)
		dwStyle |= DTS_RIGHTALIGN;

	// Create control to edit the cell
	CDateTimeCtrl* pDateTimeCtrl = CreateDateTimeCtrl(owner, nRow, nCol, dwStyle, rectCell);
	VERIFY(pDateTimeCtrl!=NULL);
	if (pDateTimeCtrl==NULL)
		return NULL;

	// Configure font
	pDateTimeCtrl->SetFont(owner.GetCellFont());

	// Configure datetime format
	if (!m_Format.IsEmpty())
		pDateTimeCtrl->SetFormat(m_Format);

	pDateTimeCtrl->SetTime(dateTime);

	// Check with the original string
	CString timeText;
	pDateTimeCtrl->GetWindowText(timeText);
	if (cellText!=timeText)
	{
		dateTime.SetDateTime(1970, 1, 1, 0, 0, 0);
		pDateTimeCtrl->SetTime(dateTime);
	}

	return pDateTimeCtrl;
}

//------------------------------------------------------------------------
//! Compares two cell values according to specified sort order
//!
//! @param pszLeftValue Left cell value
//! @param pszRightValue Right cell value
//! @param bAscending Perform sorting in ascending or descending order
//! @return Is left value less than right value (-1) or equal (0) or larger (1)
//------------------------------------------------------------------------
int CGridColumnTraitDateTime::OnSortRows(LPCTSTR pszLeftValue, LPCTSTR pszRightValue, bool bAscending)
{
	COleDateTime leftDateTime, rightDateTime;
	ParseDateTime(pszLeftValue, leftDateTime);
	ParseDateTime(pszRightValue, rightDateTime);

	if (bAscending)
		return (int)(leftDateTime - rightDateTime);
	else
		return (int)(rightDateTime - leftDateTime);
}

//------------------------------------------------------------------------
// CGridEditorDateTimeCtrl (For internal use)
//------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGridEditorDateTimeCtrl, CDateTimeCtrl)
	//{{AFX_MSG_MAP(CGridEditorDateTimeCtrl)
	ON_WM_KILLFOCUS()
	ON_NOTIFY_REFLECT(DTN_DATETIMECHANGE, OnDateTimeChange)
	ON_NOTIFY_REFLECT(DTN_USERSTRINGW, OnUserString)
	ON_NOTIFY_REFLECT(DTN_USERSTRINGA, OnUserString)
	ON_NOTIFY_REFLECT(DTN_CLOSEUP, OnCloseUp)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------------
//! CGridEditorDateTimeCtrl - Constructor
//!
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pColumnTrait The parent column trait, used for datetime validation
//------------------------------------------------------------------------
CGridEditorDateTimeCtrl::CGridEditorDateTimeCtrl(int nRow, int nCol, CGridColumnTraitDateTime* pColumnTrait)
	:m_Row(nRow)
	,m_Col(nCol)
	,m_Completed(false)
	,m_Modified(false)
	,m_pColumnTrait(pColumnTrait)
{}

//------------------------------------------------------------------------
//! The cell value editor was closed and the entered should be saved.
//!
//! @param bSuccess Should the entered cell value be saved
//------------------------------------------------------------------------
void CGridEditorDateTimeCtrl::EndEdit(bool bSuccess)
{
	// Avoid two messages if key-press is followed by kill-focus
	if (m_Completed)
		return;

	m_Completed = true;

	// Format time back to string
	CString str;
	GetWindowText(str);

	// Send Notification to parent of ListView ctrl
	LV_DISPINFO dispinfo = {0};
	if (bSuccess && m_Modified)
	{
		dispinfo.item.mask = LVIF_TEXT;
		dispinfo.item.pszText = str.GetBuffer(0);
		dispinfo.item.cchTextMax = str.GetLength();
	}
	ShowWindow(SW_HIDE);
	CGridColumnTraitImage::SendEndLabelEdit(*GetParent(), m_Row, m_Col, dispinfo);
	PostMessage(WM_CLOSE);
}

//------------------------------------------------------------------------
//! WM_KILLFOCUS message handler called when CDateTimeCtrl is loosing focus
//! to other control. Used register that cell value editor should close.
//!
//! @param pNewWnd Pointer to the window that receives the input focus (may be NULL or may be temporary).
//------------------------------------------------------------------------
void CGridEditorDateTimeCtrl::OnKillFocus(CWnd *pNewWnd)
{
	CDateTimeCtrl::OnKillFocus(pNewWnd);
	if (GetMonthCalCtrl()==NULL)
	{
		// Special case when a dynamic CEdit is created (DTS_APPCANPARSE)
		if (pNewWnd == NULL || pNewWnd->GetParent()!=this)
			EndEdit(true);
	}
}

//------------------------------------------------------------------------
//! DTN_CLOSEUP notification message handler called when CMonthCalCtrl
//! window has closed. Fallback solution for closing inplace CDateTimeCtrl,
//! in case focus is not given back to the CDateTimeCtrl.
//!
//! @param pNMHDR Pointer to NMHDR structure
//! @param pResult Is not used
//------------------------------------------------------------------------
void CGridEditorDateTimeCtrl::OnCloseUp(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (GetFocus()!=this)
		EndEdit(true);	// Force close if focus has been stolen
}

//------------------------------------------------------------------------
//! Called by the default OnNcDestroy (WM_NCDESTROY) message handler, 
//! when CDateTimeCtrl window has been be destroyed. 
//! Used to delete the inplace CDateTimeCtrl editor object as well.
//! This is necessary when the CDateTimeCtrl is created dynamically.
//------------------------------------------------------------------------
void CGridEditorDateTimeCtrl::PostNcDestroy()
{
	CDateTimeCtrl::PostNcDestroy();
	delete this;
}

//------------------------------------------------------------------------
//! WM_CHAR message handler to monitor date modifications
//!
//! @param nChar Specifies the virtual key code of the given key.
//! @param nRepCnt Repeat count (the number of times the keystroke is repeated as a result of the user holding down the key).
//! @param nFlags Specifies the scan code, key-transition code, previous key state, and context code
//------------------------------------------------------------------------
void CGridEditorDateTimeCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_Modified = true;
	CDateTimeCtrl::OnChar(nChar, nRepCnt, nFlags);
}

//------------------------------------------------------------------------
//! DTN_DATETIMECHANGE notification handler to monitor date modifications
//!
//! @param pNMHDR Pointer to NMDATETIMECHANGE structure
//! @param pResult Must be set to zero
//------------------------------------------------------------------------
void CGridEditorDateTimeCtrl::OnDateTimeChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_Modified = true;
	*pResult = 0;
}

//------------------------------------------------------------------------
//! DTN_USERSTRING notification handler to convert clipboard to datetime
//!
//! @param pNMHDR Pointer to NMDATETIMESTRING structure
//! @param pResult Must be set to zero
//------------------------------------------------------------------------
void CGridEditorDateTimeCtrl::OnUserString(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMDATETIMESTRINGW* pDateInfoW = reinterpret_cast<NMDATETIMESTRINGW*>(pNMHDR);
	NMDATETIMESTRINGA* pDateInfoA = reinterpret_cast<NMDATETIMESTRINGA*>(pNMHDR);

	CString userstr;
	if (pNMHDR->code == DTN_USERSTRINGA)
		userstr = pDateInfoA->pszUserString;
	else
		userstr = pDateInfoW->pszUserString;

	if (m_pColumnTrait)
	{
		COleDateTime dt;
		if (m_pColumnTrait->ParseDateTime(userstr, dt))
		{
			if (pNMHDR->code == DTN_USERSTRINGA)
			{
				pDateInfoA->dwFlags = GDT_VALID;
				dt.GetAsSystemTime(pDateInfoA->st);
			}
			else
			{
				pDateInfoW->dwFlags = GDT_VALID;
				dt.GetAsSystemTime(pDateInfoW->st);
			}
		}
		else
		{
			if (pNMHDR->code == DTN_USERSTRINGA)
				pDateInfoA->dwFlags = GDT_NONE;
			else
				pDateInfoW->dwFlags = GDT_NONE;
		}
	}

    *pResult = 0;
}

//------------------------------------------------------------------------
//! Hook to proces windows messages before they are dispatched.
//! Catch keyboard events that can should cause the cell value editor to close
//!
//! @param pMsg Points to a MSG structure that contains the message to process
//! @return Nonzero if the message was translated and should not be dispatched; 0 if the message was not translated and should be dispatched.
//------------------------------------------------------------------------
BOOL CGridEditorDateTimeCtrl::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		{
			switch(pMsg->wParam)
			{
				case VK_RETURN: EndEdit(true); return TRUE;
				case VK_TAB: EndEdit(true); return FALSE;
				case VK_ESCAPE: EndEdit(false);return TRUE;
			}
			break;
		};
		case WM_MOUSEWHEEL: EndEdit(true); return FALSE;	// Don't steal event
	}
	return CDateTimeCtrl::PreTranslateMessage(pMsg);
}
