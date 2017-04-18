#include "../stdafx.h"
#include "CGridColumnTraitMultilineEdit.h"

#include "CGridColumnTraitVisitor.h"
#include "CGridListCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//------------------------------------------------------------------------
//! CGridColumnTraitMultilineEdit - Constructor
//------------------------------------------------------------------------
CGridColumnTraitMultilineEdit::CGridColumnTraitMultilineEdit()
	:m_EditMaxLines(4)
{
	m_EditStyle |= ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL;
}

//------------------------------------------------------------------------
//! Accept Visitor Pattern
//------------------------------------------------------------------------
void CGridColumnTraitMultilineEdit::Accept(CGridColumnTraitVisitor& visitor)
{
	visitor.Visit(*this);
}

//------------------------------------------------------------------------
//! Set max number of lines that can the CEdit will display at a time
//!	For multiline editing then add these styles ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL
//!
//! @param nMaxLines The text limit, in lines.
//------------------------------------------------------------------------
void CGridColumnTraitMultilineEdit::SetMaxLines(UINT nMaxLines)
{
	m_EditMaxLines = nMaxLines;
}

//------------------------------------------------------------------------
//! Get max number of lines that can the CEdit will display at a time
//!
//! @return Max number of display lines for the multiline CEdit
//------------------------------------------------------------------------
UINT CGridColumnTraitMultilineEdit::GetMaxLines() const
{
	return m_EditMaxLines;
}


namespace
{
	int CharacterCount(const CString& csHaystack, LPCTSTR sNeedle)
	{
		if (csHaystack.IsEmpty())
			return 0;

		int nFind = -1;
		int nCount = 0;
		do
		{
			nCount++;
			nFind = csHaystack.Find( sNeedle, nFind + 1 );
		} while (nFind != -1);
		
		return nCount-1;
	}
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
CEdit* CGridColumnTraitMultilineEdit::CreateEdit(CGridListCtrlEx& owner, int nRow, int nCol, DWORD dwStyle, const CRect& rect)
{
	CGridMultilineEditorText* pEdit = new CGridMultilineEditorText(nRow, nCol);

	CRect limitRect(rect);
	if (m_EditMaxLines > 1 && GetStyle() & ES_MULTILINE)
	{
		// Calculate the number of lines in the cell text, expand the CEdit to match this
		CString cellText = owner.GetItemText(nRow, nCol);
		int nLineHeight = GetCellFontHeight(owner);
		int nLineCount = CharacterCount(cellText, _T("\n"));
		if (nLineCount > 0)
		{
			if ((UINT)nLineCount > m_EditMaxLines-1)
				nLineCount = m_EditMaxLines-1;
			limitRect.bottom += nLineHeight*nLineCount;
		}
		pEdit->SetMaxLines(m_EditMaxLines);
		pEdit->SetLineHeight(nLineHeight);
	}

	VERIFY( pEdit->Create( WS_CHILD | dwStyle, limitRect, &owner, 0) );
	return pEdit;
}

//------------------------------------------------------------------------
// CGridMultilineEditorText (For internal use)
//------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CGridMultilineEditorText, CGridEditorText)
	//{{AFX_MSG_MAP(CGridEditorText)
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------------------------------
//! CGridMultilineEditorText - Constructor
//------------------------------------------------------------------------
CGridMultilineEditorText::CGridMultilineEditorText(int nRow, int nCol)
	:CGridEditorText(nRow, nCol)
	,m_LineHeight(0)
	,m_MaxLines(0)
{
}

//------------------------------------------------------------------------
//! EN_CHANGE notification handler to monitor text modifications
//------------------------------------------------------------------------
void CGridMultilineEditorText::OnEnChange()
{
	if (!m_InitialModify && (GetStyle() & ES_MULTILINE))
		m_InitialModify = false;// ES_MULTILINE causes EN_CHANGE not to fire at initial SetWindowText

	// If multiline support, then resize the edit according to contents
	if ((m_MaxLines > 1) && (GetStyle() & ES_MULTILINE) && (m_LineHeight > 0))
	{
		// Get number of text lines
		CString cellText;
		GetWindowText(cellText);
		int nLineCount = CharacterCount(cellText, _T("\n"));
		if (nLineCount > 0)
		{
			if ((UINT)nLineCount > m_MaxLines-1)
				nLineCount = m_MaxLines-1;
		}

		// Check if the current rect matches the number of lines
		CRect rect;
		GetWindowRect(&rect);
		if (rect.Height() / m_LineHeight != nLineCount + 1)
		{
			rect.bottom += (nLineCount + 1 - rect.Height() / m_LineHeight) * m_LineHeight;
			GetParent()->ScreenToClient(&rect);
			MoveWindow(rect.left, rect.top, rect.Width(), rect.Height(), TRUE);
		}
	}
	CGridEditorText::OnEnChange();
}