//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "../stdafx.h"
#pragma warning(disable:4100)	// unreferenced formal parameter

#include "CGridColumnTraitEdit.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//------------------------------------------------------------------------
//! CGridColumnTraitEdit - Constructor
//------------------------------------------------------------------------
CGridColumnTraitEdit::CGridColumnTraitEdit()
	:m_EditStyle(ES_AUTOHSCROLL | ES_NOHIDESEL | WS_BORDER)
	,m_EditLimitText(UINT_MAX)
{
}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridColumnTraitEdit::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Set style used when creating CEdit for cell value editing
//!
//! @param dwStyle Style flags
//------------------------------------------------------------------------
void CGridColumnTraitEdit::SetStyle(DWORD dwStyle)
{
	m_EditStyle = dwStyle;
}

//------------------------------------------------------------------------
//! Get style used when creating CEdit for cell value editing
//!
//! @return Style flags
//------------------------------------------------------------------------
DWORD CGridColumnTraitEdit::GetStyle() const
{
	return m_EditStyle;
}

//------------------------------------------------------------------------
//! Set max number of characters the CEdit will accept
//!
//! @param nMaxChars The text limit, in characters.
//------------------------------------------------------------------------
void CGridColumnTraitEdit::SetLimitText(UINT nMaxChars)
{
	m_EditLimitText = nMaxChars;
}

//------------------------------------------------------------------------
//! Get max number of characters the CEdit will accept
//!
//! @return The text limit, in characters
//------------------------------------------------------------------------
UINT CGridColumnTraitEdit::GetLimitText() const
{
	return m_EditLimitText;
}


//------------------------------------------------------------------------
//! Create a CEdit as cell value editor
//!
//! @param owner The list control starting a cell edit
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param dwStyle The windows style to use when creating the CEdit
//! @param rect The rectangle where the inplace cell value editor should be placed
//! @return Pointer to the cell editor to use
//------------------------------------------------------------------------
CEdit* CGridColumnTraitEdit::CreateEdit(CGridListCtrlEx& owner, int nRow, int nCol, DWORD dwStyle, const CRect& rect)
{
	CGridEditorText* pEdit = new CGridEditorText(nRow, nCol);
	VERIFY( pEdit->Create( WS_CHILD | dwStyle, rect, &owner, 0) );
	return pEdit;
}

//------------------------------------------------------------------------
//! Overrides OnEditBegin() to provide a CEdit cell value editor
//!
//! @param owner The list control starting edit
//! @param nRow The index of the row for the cell to edit
//! @param nCol The index of the column for the cell to edit
//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
//------------------------------------------------------------------------
CWnd* CGridColumnTraitEdit::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	// Get position of the cell to edit
	CRect rectCell = GetCellEditRect(owner, nRow, nCol);

	// Get the text-style of the cell to edit
	DWORD dwStyle = m_EditStyle;
	HDITEM hditem = {0};
	hditem.mask = HDI_FORMAT;
	VERIFY( owner.GetHeaderCtrl()->GetItem(nCol, &hditem) );
	if (hditem.fmt & HDF_CENTER)
		dwStyle |= ES_CENTER;
	else if (hditem.fmt & HDF_RIGHT)
		dwStyle |= ES_RIGHT;
	else
		dwStyle |= ES_LEFT;

	// Create edit control to edit the cell
	CEdit* pEdit = CreateEdit(owner, nRow, nCol, dwStyle, rectCell);
	VERIFY(pEdit!=NULL);
	if (pEdit==NULL)
		return NULL;

	// Configure font
	pEdit->SetFont(owner.GetCellFont());

	// First column (Label) doesn't have a margin when imagelist is assigned
	if (nCol==0 && owner.GetImageList(LVSIL_SMALL)!=NULL)
		pEdit->SetMargins(0, 0);
	else
	// First column (Label) doesn't have a margin when checkboxes are enabled
	if (nCol==0 && owner.GetExtendedStyle() & LVS_EX_CHECKBOXES)
		pEdit->SetMargins(1, 0);
	else
	// Label column doesn't have margin when not first in column order
	if (nCol==0 && owner.GetFirstVisibleColumn()!=nCol)
		pEdit->SetMargins(1, 0);
	else
	if (dwStyle & ES_CENTER)
		pEdit->SetMargins(0, 0);
	else
	if (dwStyle & ES_RIGHT)
		pEdit->SetMargins(0, 7);
	else
		pEdit->SetMargins(4, 0);

	if (m_EditLimitText!=UINT_MAX)
		pEdit->SetLimitText(m_EditLimitText);

	CString cellText = owner.GetItemText(nRow, nCol);
	pEdit->SetWindowText(cellText);
	pEdit->SetSel(0, -1, 0);
	return pEdit;
}

//------------------------------------------------------------------------
// CGridEditorText (For internal use)
//------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGridEditorText, CEdit)
	//{{AFX_MSG_MAP(CGridEditorText)
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------------
//! CGridEditorText - Constructor
//------------------------------------------------------------------------
CGridEditorText::CGridEditorText(int nRow, int nCol)
	:m_Row(nRow)
	,m_Col(nCol)
	,m_Completed(false)
	,m_Modified(false)
	,m_InitialModify(true)
{}

//------------------------------------------------------------------------
//! The cell value editor was closed and the entered should be saved.
//!
//! @param bSuccess Should the entered cell value be saved
//------------------------------------------------------------------------
void CGridEditorText::EndEdit(bool bSuccess)
{
	// Avoid two messages if key-press is followed by kill-focus
	if (m_Completed)
		return;

	m_Completed = true;

	// Send Notification to parent of ListView ctrl
	CString str;
	GetWindowText(str);

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
//! WM_KILLFOCUS message handler called when CEdit is loosing focus
//! to other control. Used register that cell value editor should close.
//!
//! @param pNewWnd Pointer to the window that receives the input focus (may be NULL or may be temporary).
//------------------------------------------------------------------------
void CGridEditorText::OnKillFocus(CWnd *pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	EndEdit(true);
}

//------------------------------------------------------------------------
//! Called by the default OnNcDestroy (WM_NCDESTROY) message handler, 
//! when CEdit window has been be destroyed. 
//! Used to delete the inplace CEdit editor object as well.
//! This is necessary when the CEdit is created dynamically.
//------------------------------------------------------------------------
void CGridEditorText::PostNcDestroy()
{
	CEdit::PostNcDestroy();
	delete this;
}

//------------------------------------------------------------------------
//! EN_CHANGE notification handler to monitor text modifications
//------------------------------------------------------------------------
void CGridEditorText::OnEnChange()
{
	if (!m_InitialModify)
		m_Modified = true;
	else
		m_InitialModify = false;
}

//------------------------------------------------------------------------
//! Hook to proces windows messages before they are dispatched.
//! Catch keyboard events that can should cause the cell value editor to close
//!
//! @param pMsg Points to a MSG structure that contains the message to process
//! @return Nonzero if the message was translated and should not be dispatched; 0 if the message was not translated and should be dispatched.
//------------------------------------------------------------------------
BOOL CGridEditorText::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
		case WM_KEYDOWN:
		{
			switch(pMsg->wParam)
			{
				case VK_RETURN:
				{
					if (GetStyle() & ES_WANTRETURN)
						break;

					EndEdit(true);
					return TRUE;
				}
				case VK_TAB: EndEdit(true); return FALSE;
				case VK_ESCAPE: EndEdit(false);return TRUE;
			}
			break;
		};
		case WM_MOUSEWHEEL: EndEdit(true); return FALSE;	// Don't steal event
	}
	return CEdit::PreTranslateMessage(pMsg);
}
