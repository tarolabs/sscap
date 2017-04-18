#include "CGridColumnTraitEdit.h"

//------------------------------------------------------------------------
//! CGridColumnTraitMultilineEdit implements a CEdit as multiline cell-editor
//------------------------------------------------------------------------
class CGridColumnTraitMultilineEdit : public CGridColumnTraitEdit
{
public:
	CGridColumnTraitMultilineEdit();

	void SetMaxLines(UINT nMaxLines);
	UINT GetMaxLines() const;

protected:
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	CEdit* CreateEdit(CGridListCtrlEx& owner, int nRow, int nCol, DWORD dwStyle, const CRect& rect);

	UINT m_EditMaxLines;			//!< Max number of lines the CEdit will display at a time
};

//------------------------------------------------------------------------
//! CEdit for inplace edit. For internal use by CGridColumnTraitMultilineEdit
//------------------------------------------------------------------------
class CGridMultilineEditorText : public CGridEditorText
{
public:
	CGridMultilineEditorText(int nRow, int nCol);

	void SetLineHeight(int nLineHeight)	{ m_LineHeight = nLineHeight; }
	void SetMaxLines(UINT nMaxLines)	{ m_MaxLines = nMaxLines; }

protected:
	int		m_LineHeight;			//!< The height of a single line (depends on current font)
	UINT	m_MaxLines;				//!< Max number of lines the CEdit will display at a time

	afx_msg void OnEnChange();

	DECLARE_MESSAGE_MAP();

private:
	CGridMultilineEditorText();
	CGridMultilineEditorText(const CGridMultilineEditorText&);
	CGridMultilineEditorText& operator=(const CGridMultilineEditorText&);
};