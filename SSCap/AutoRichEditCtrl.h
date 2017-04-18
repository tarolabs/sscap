#if !defined(AFX_AUTORICHEDITCTRL_H__C26D1E0E_DD32_11D2_B39F_000092914562__INCLUDED_)
#define AFX_AUTORICHEDITCTRL_H__C26D1E0E_DD32_11D2_B39F_000092914562__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// =========================
//RICHEDIT¿‡

class CAutoRichEditCtrl : public CRichEditCtrl
{
// Construction
public:
	CAutoRichEditCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoRichEditCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL Init();
	void AppendString(CString strMsg,COLORREF clr);
	long GetSelectionFontSize();
	CString GetSelectionFontName();
	CStringArray m_saFontList;
	void GetSystemFonts(CStringArray &saFontList);
	void SetFontSize(int nPointSize);
	void SetFontName(CString sFontName);
	void SelectColor(COLORREF textClr);
	bool ParagraphIsBulleted();
	void SetParagraphBulleted();
	
	PARAFORMAT GetParagraphFormat();
	
	bool ParagraphIsRight();
	bool ParagraphIsLeft();
	bool ParagraphIsCentered();
	
	void SetParagraphRight();
	void SetParagraphLeft();
	void SetParagraphCenter();
	
	CHARFORMAT GetCharFormat(DWORD dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE);
	
	bool SelectionIsBold();
	bool SelectionIsItalic();
	bool SelectionIsUnderlined();

	void SetSelectionBold();
	void SetSelectionItalic();
	void SetSelectionUnderlined();

	virtual ~CAutoRichEditCtrl();

	// Generated message map functions
protected:
	CFont	m_font;

	//{{AFX_MSG(CAutoRichEditCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
		void SetCharStyle(int MASK, int STYLE, int nStart, int nEnd);
		static BOOL CALLBACK CBEnumFonts(LPLOGFONT lplf, LPTEXTMETRIC lptm, DWORD dwType, LPARAM lpData);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTORICHEDITCTRL_H__C26D1E0E_DD32_11D2_B39F_000092914562__INCLUDED_)
