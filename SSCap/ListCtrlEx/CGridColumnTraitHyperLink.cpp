#include "../stdafx.h"
#include "CGridColumnTraitHyperLink.h"

#include "CGridListCtrlEx.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//------------------------------------------------------------------------
//! CGridColumnTraitHyperLink - Constructor
//------------------------------------------------------------------------
CGridColumnTraitHyperLink::CGridColumnTraitHyperLink()
{
	m_ShellFilePrefix = _T("http://google.com/?q=");
	m_ShellShowCommand = SW_SHOWNORMAL;
	m_LinkColor = RGB(22, 74, 170);
	m_LinkColorHot = RGB(0, 85, 255);
	SetSingleClickEdit(true);
}

//------------------------------------------------------------------------
//! Set the standard link color
//!
//! @param linkColor Link RGB Colors
//------------------------------------------------------------------------
void CGridColumnTraitHyperLink::SetLinkColor(COLORREF linkColor)
{
	m_LinkColor = linkColor;
}

//------------------------------------------------------------------------
//! Get the standard link color
//!
//! @return Link RGB Colors
//------------------------------------------------------------------------
COLORREF CGridColumnTraitHyperLink::GetLinkColor() const
{
	return m_LinkColor;
}

//------------------------------------------------------------------------
//! Set the link color when mouse is over the cell
//!
//! @param linkColor Link RGB Colors
//------------------------------------------------------------------------
void CGridColumnTraitHyperLink::SetLinkColorHot(COLORREF linkColor)
{
	m_LinkColorHot = linkColor;
}

//------------------------------------------------------------------------
//! Get the link color when mouse is over the cell
//!
//! @return Link RGB Colors
//------------------------------------------------------------------------
COLORREF CGridColumnTraitHyperLink::GetLinkColorHot() const
{
	return m_LinkColorHot;
}

//------------------------------------------------------------------------
//! Set the ShellExecute file operation to perform on the file-specifier
//!
//! @param strShellOperation Link Action Name
//------------------------------------------------------------------------
void CGridColumnTraitHyperLink::SetShellOperation(const CString& strShellOperation)
{
	m_ShellOperation = strShellOperation;
}

//------------------------------------------------------------------------
//! Get the ShellExecute file operation to perform on the file-specifier
//!
//! @return Link Action Name
//------------------------------------------------------------------------
CString CGridColumnTraitHyperLink::GetShellOperation() const
{
	return m_ShellOperation;
}

//------------------------------------------------------------------------
//! Set the ShellExecute application to use to launch the file specifier
//!
//! @param strShellAppliction Application path (If blank then it just uses the default handler for the file type)
//------------------------------------------------------------------------
void CGridColumnTraitHyperLink::SetShellApplication(const CString& strShellAppliction)
{
	m_ShellApplication = strShellAppliction;
}

//------------------------------------------------------------------------
//! Get the ShellExecute application to use to launch the file specifier
//!
//! @return Application path
//------------------------------------------------------------------------
CString CGridColumnTraitHyperLink::GetShellApplication() const
{
	return m_ShellApplication;
}

//------------------------------------------------------------------------
//! Set the file specifier prefix for the ShellExecute operation
//! (Ex. protocol details like 'mailto:' or 'http://')
//!
//! @param strShellFilePrefix Prefix to insert infront of the file-sepecifier
//------------------------------------------------------------------------
void CGridColumnTraitHyperLink::SetShellFilePrefix(const CString& strShellFilePrefix)
{
	m_ShellFilePrefix = strShellFilePrefix;
}

//------------------------------------------------------------------------
//! Get the file specifier prefix for the ShellExecute operation
//!
//! @return Prefix to insert infront of the filename specifier
//------------------------------------------------------------------------
CString CGridColumnTraitHyperLink::GetShellFilePrefix() const
{
	return m_ShellFilePrefix;
}

//------------------------------------------------------------------------
//! Set the file specifier suffix for the ShellExecute operation
//!
//! @param strShellFileSuffix File operation to perform on the file-specifier
//------------------------------------------------------------------------
void CGridColumnTraitHyperLink::SetShellFileSuffix(const CString& strShellFileSuffix)
{
	m_ShellFileSuffix = strShellFileSuffix;
}

//------------------------------------------------------------------------
//! Get the file specifier suffix for the ShellExecute operation
//!
//! @return Suffix to append after the filename specifier
//------------------------------------------------------------------------
CString CGridColumnTraitHyperLink::GetShellFileSuffix() const
{
	return m_ShellFileSuffix;
}

//------------------------------------------------------------------------
//! Set the show window flags for the ShellExecute operation
//!
//! @param nShellShowCommand Show Command Flags
//------------------------------------------------------------------------
void CGridColumnTraitHyperLink::SetShellShowCommand(INT nShellShowCommand)
{
	m_ShellShowCommand = nShellShowCommand;
}

//------------------------------------------------------------------------
//! Get the show window flags for the ShellExecute operation
//!
//! @return Show Command Flags
//------------------------------------------------------------------------
INT CGridColumnTraitHyperLink::GetShellShowCommand() const
{
	return m_ShellShowCommand;
}

//------------------------------------------------------------------------
//! Changes the text color if one is specified
//!
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param textColor Current text color
//! @return New text color was specified (true / false)
//------------------------------------------------------------------------
bool CGridColumnTraitHyperLink::UpdateTextColor(NMLVCUSTOMDRAW* pLVCD, COLORREF& textColor)
{
	if (pLVCD->nmcd.uItemState & CDIS_HOT)
		textColor = m_LinkColorHot;
	else
		textColor = m_LinkColor;
	return true;
}

//------------------------------------------------------------------------
//! Specifies af the font color if one is specified
//!
//! @param pLVCD Pointer to NMLVCUSTOMDRAW structure
//! @param textFont New font specification
//! @return New font was specified (true / false)
//------------------------------------------------------------------------
bool CGridColumnTraitHyperLink::UpdateTextFont(NMLVCUSTOMDRAW* pLVCD, LOGFONT& textFont)
{
	CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
	CFont* pCurrentFont = pDC->GetCurrentFont();
	pCurrentFont->GetLogFont(&textFont);
	textFont.lfUnderline = 1;
	return true;
}

//------------------------------------------------------------------------
//! Returns dimensions of the cell text clicked
//!
//! @param owner The list control being clicked
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param cellText The contents of the cell clicked
//! @return The dimensions of the cell text
//------------------------------------------------------------------------
CRect CGridColumnTraitHyperLink::GetTextRect(CGridListCtrlEx& owner, int nRow, int nCol, const CString& cellText)
{
	CRect rect;
	ASSERT(nRow != -1);
	CDC* pDC = owner.GetDC();
	CFont* pOldFont = pDC->SelectObject(owner.GetCellFont());
	CSize size = pDC->GetTextExtent(cellText);
	pDC->SelectObject(pOldFont);
	owner.ReleaseDC(pDC);

	owner.GetCellRect(nRow, nCol, LVIR_LABEL, rect);

	HDITEM hditem = {0};
	hditem.mask = HDI_FORMAT;
	owner.GetHeaderCtrl()->GetItem(nCol, &hditem);

	// First item (Label) doesn't have a margin (Subitems does)
	if (nCol!=0 && !(hditem.fmt & HDF_CENTER))
	{
		if (hditem.fmt & HDF_RIGHT)
			rect.OffsetRect(-7, 0);
		else
			rect.OffsetRect(4, 0);
	}

	if (hditem.fmt & HDF_CENTER)
		rect.DeflateRect((rect.Width()-size.cx)/2, 0);
	else if (hditem.fmt & HDF_RIGHT)
		rect.left = rect.right - size.cx;
	else
		rect.right = rect.left + size.cx;
	return rect;
}

//------------------------------------------------------------------------
//! Overrides OnEditBegin() to launch ShellExecute when starting editor
//!
//! @param owner The list control starting edit
//! @param nRow The index of the row for the cell to edit
//! @param nCol The index of the column for the cell to edit
//! @return Pointer to the cell editor to use (NULL if cell edit is not possible)
//------------------------------------------------------------------------
CWnd* CGridColumnTraitHyperLink::OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol)
{
	CString cellText = owner.GetItemText(nRow, nCol);
	OnShellExecute(owner, nRow, nCol, cellText);
	LV_DISPINFO dispinfo = {0};
	SendEndLabelEdit(owner, nRow, nCol, dispinfo);
	return NULL;
}

//------------------------------------------------------------------------
//! Checks if the mouse click should start the cell editor (OnEditBegin)
//!	Validates that the click was on the text-link within the label-part
//!
//! @param owner The list control being clicked
//! @param nRow The index of the row
//! @param nCol The index of the column
//! @param pt The position clicked, in client coordinates.
//! @param bDblClick Whether the position was double clicked
//! @return How should the cell editor be started (0 = No editor, 1 = Start Editor, 2 = Start Editor and block click-event)
//------------------------------------------------------------------------
int CGridColumnTraitHyperLink::OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt, bool bDblClick)
{
	int startEdit = CGridColumnTraitImage::OnClickEditStart(owner, nRow, nCol, pt, bDblClick);
	if (startEdit)
	{
		// Check if mouse click was inside the label-part of the cell
		CRect labelRect;
		if (owner.GetCellRect(nRow, nCol, LVIR_LABEL, labelRect) && labelRect.PtInRect(pt))
		{
			// Check if mouse click was inside the text-link of the cell
			CString cellText = owner.GetItemText(nRow, nCol);
			if (GetTextRect(owner,nRow,nCol,cellText).PtInRect(pt))
				return startEdit;
			else
				return 0;
		}
	}
	return startEdit;
}

//------------------------------------------------------------------------
//! Performs the ShellExecute operation on the given file specifier
//!
//! @param owner The list control starting edit
//! @param nRow The index of the row for the cell to edit
//! @param nCol The index of the column for the cell to edit
//! @param cellText The contents of the cell clicked
//------------------------------------------------------------------------
void CGridColumnTraitHyperLink::OnShellExecute(CGridListCtrlEx& owner, int nRow, int nCol, const CString& cellText)
{
	(nRow);	// Avoid unreferenced variable warning
	(nCol);	// Avoid unreferenced variable warning
	if (m_ShellApplication.IsEmpty())
		ShellExecute(owner.m_hWnd, m_ShellOperation.IsEmpty() ? (LPCTSTR)NULL : static_cast<LPCTSTR>(m_ShellOperation), m_ShellFilePrefix + cellText + m_ShellFileSuffix, NULL, NULL, m_ShellShowCommand);
	else
		ShellExecute(owner.m_hWnd, m_ShellOperation.IsEmpty() ? (LPCTSTR)NULL : static_cast<LPCTSTR>(m_ShellOperation), m_ShellApplication, m_ShellFilePrefix + cellText + m_ShellFileSuffix, NULL, m_ShellShowCommand);
}