// AutoRichEditCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "RichEd.h"
#include "AutoRichEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoRichEditCtrl

CAutoRichEditCtrl::CAutoRichEditCtrl()
{
	m_font.CreatePointFont(110,_T("Arial"));
}

CAutoRichEditCtrl::~CAutoRichEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CAutoRichEditCtrl, CRichEditCtrl)
	//{{AFX_MSG_MAP(CAutoRichEditCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoRichEditCtrl message handlers
BOOL CAutoRichEditCtrl::Init()
{
	SetFont(&m_font);

	PARAFORMAT2 pf;  

	ZeroMemory(&pf,sizeof(PARAFORMAT2)); 

	pf.dwMask |= PFM_LINESPACING| PFM_OFFSET|PFM_STARTINDENT|PFM_RIGHTINDENT|PFM_SHADING ;

	pf.bLineSpacingRule =3;

	pf.dyLineSpacing = 300;
	pf.dxStartIndent = 100;
	pf.dxRightIndent = 100;
	pf.wShadingWeight = 5;
	pf.wShadingStyle = 1;

	//SetSel(0,-1);//设置处理区域 

	SetParaFormat(pf);

	SetOptions(ECOOP_OR,ECO_SAVESEL);

	return TRUE;
}

bool CAutoRichEditCtrl::SelectionIsBold()
{
	CHARFORMAT cf = GetCharFormat();	
	
	if (cf.dwEffects & CFM_BOLD)
		return true;
	else
		return false;
}

bool CAutoRichEditCtrl::SelectionIsItalic()
{
	CHARFORMAT cf = GetCharFormat();	
	
	if (cf.dwEffects & CFM_ITALIC)
		return true;
	else
		return false;
}

bool CAutoRichEditCtrl::SelectionIsUnderlined()
{
	CHARFORMAT cf = GetCharFormat();	
	
	if (cf.dwEffects & CFM_UNDERLINE)
		return true;
	else
		return false;
}

CHARFORMAT CAutoRichEditCtrl::GetCharFormat(DWORD dwMask)
{
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);

	cf.dwMask = dwMask;

	GetSelectionCharFormat(cf);

	return cf;
}

void CAutoRichEditCtrl::SetCharStyle(int MASK, int STYLE, int nStart, int nEnd)
{
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	//cf.dwMask = MASK;
	
	GetSelectionCharFormat(cf);
	
	if (cf.dwMask & MASK)	// selection is all the same
	{
		cf.dwEffects ^= STYLE; 
	}
	else
	{
		cf.dwEffects |= STYLE;
	}
	
	cf.dwMask = MASK;

	SetSelectionCharFormat(cf);

}

void CAutoRichEditCtrl::SetSelectionBold()
{
	long start=0, end=0;
	GetSel(start, end);		// Get the current selection

	SetCharStyle(CFM_BOLD, CFE_BOLD, start, end);	// Make it bold
}

void CAutoRichEditCtrl::SetSelectionItalic()
{
	long start=0, end=0;
	GetSel(start, end);

	SetCharStyle(CFM_ITALIC, CFE_ITALIC, start, end);
}

void CAutoRichEditCtrl::SetSelectionUnderlined()
{
	long start=0, end=0;
	GetSel(start, end);

	SetCharStyle(CFM_UNDERLINE, CFE_UNDERLINE, start, end);
}

void CAutoRichEditCtrl::SetParagraphCenter()
{
	PARAFORMAT paraFormat;    
	paraFormat.cbSize = sizeof(PARAFORMAT);
	paraFormat.dwMask = PFM_ALIGNMENT;    
	paraFormat.wAlignment = PFA_CENTER;
	
	SetParaFormat(paraFormat);	// Set the paragraph.
}

void CAutoRichEditCtrl::SetParagraphLeft()
{
	PARAFORMAT paraFormat;
	paraFormat.cbSize = sizeof(PARAFORMAT);
	paraFormat.dwMask = PFM_ALIGNMENT;    
	paraFormat.wAlignment = PFA_LEFT;
	
	SetParaFormat(paraFormat);
}

void CAutoRichEditCtrl::SetParagraphRight()
{
	PARAFORMAT paraFormat;
	paraFormat.cbSize = sizeof(PARAFORMAT);
	paraFormat.dwMask = PFM_ALIGNMENT;    
	paraFormat.wAlignment = PFA_RIGHT;
	
	SetParaFormat(paraFormat);
}

bool CAutoRichEditCtrl::ParagraphIsCentered()
{
	PARAFORMAT pf = GetParagraphFormat();

	if (pf.wAlignment == PFA_CENTER)
		return true;
	else
		return false;
}

bool CAutoRichEditCtrl::ParagraphIsLeft()
{
	PARAFORMAT pf = GetParagraphFormat();

	if (pf.wAlignment == PFA_LEFT)
		return true;
	else
		return false;
}

bool CAutoRichEditCtrl::ParagraphIsRight()
{
	PARAFORMAT pf = GetParagraphFormat();

	if (pf.wAlignment == PFA_RIGHT)
		return true;
	else
		return false;
}

PARAFORMAT CAutoRichEditCtrl::GetParagraphFormat()
{
	PARAFORMAT pf;
	pf.cbSize = sizeof(PARAFORMAT);

	pf.dwMask = PFM_ALIGNMENT | PFM_NUMBERING;    	

	GetParaFormat(pf);

	return pf;
}

void CAutoRichEditCtrl::SetParagraphBulleted()
{
	PARAFORMAT paraformat = GetParagraphFormat();

	if ( (paraformat.dwMask & PFM_NUMBERING) && (paraformat.wNumbering == PFN_BULLET) )
	{
		paraformat.wNumbering = 0;
		paraformat.dxOffset = 0;
		paraformat.dxStartIndent = 0;
		paraformat.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
	}
	else
	{
		paraformat.wNumbering = PFN_BULLET;
		paraformat.dwMask = PFM_NUMBERING;
		if (paraformat.dxOffset == 0)
		{
			paraformat.dxOffset = 4;
			paraformat.dwMask = PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
		}
	}
	
	SetParaFormat(paraformat);

}

bool CAutoRichEditCtrl::ParagraphIsBulleted()
{
	PARAFORMAT pf = GetParagraphFormat();

	if (pf.wNumbering == PFN_BULLET)
		return true;
	else
		return false;
}

void CAutoRichEditCtrl::SelectColor(COLORREF textClr)
{
	CHARFORMAT cf = GetCharFormat();

	if (cf.dwEffects & CFE_AUTOCOLOR) cf.dwEffects -= CFE_AUTOCOLOR;
	/*CColorDialog dlg;

	CHARFORMAT cf = GetCharFormat();

	if (cf.dwEffects & CFE_AUTOCOLOR) cf.dwEffects -= CFE_AUTOCOLOR;

	// Get a color from the common color dialog.
	if( dlg.DoModal() == IDOK )
	{	
		cf.crTextColor = dlg.GetColor();
	}
*/

	cf.crTextColor=textClr;
	cf.dwMask = CFM_COLOR;

	SetSelectionCharFormat(cf);
}

void CAutoRichEditCtrl::SetFontName(CString sFontName)
{
	CHARFORMAT cf = GetCharFormat();

	// Set the font name.
	for (int i = 0; i <= sFontName.GetLength()-1; i++)
		cf.szFaceName[i] = sFontName[i];


	cf.dwMask = CFM_FACE;

	SetSelectionCharFormat(cf);
}

void CAutoRichEditCtrl::SetFontSize(int nPointSize)
{
	CHARFORMAT cf = GetCharFormat();

	nPointSize *= 20;	// convert from to twips
	cf.yHeight = nPointSize;
	
	cf.dwMask = CFM_SIZE;

	SetSelectionCharFormat(cf);
}

void CAutoRichEditCtrl::GetSystemFonts(CStringArray &saFontList)
{
	CDC *pDC = GetDC ();

	EnumFonts (pDC->GetSafeHdc(),NULL,(FONTENUMPROC) CBEnumFonts,(LPARAM)&saFontList);//Enumerate

}

BOOL CALLBACK CAutoRichEditCtrl::CBEnumFonts(LPLOGFONT lplf, LPTEXTMETRIC lptm, DWORD dwType, LPARAM lpData)
{
	// This function was written with the help of CCustComboBox, by Girish Bharadwaj.
	// Available from Codeguru.

	if (dwType == TRUETYPE_FONTTYPE) 
	{
		((CStringArray *) lpData)->Add( lplf->lfFaceName );
	}

	return true;
}

CString CAutoRichEditCtrl::GetSelectionFontName()
{
	CHARFORMAT cf = GetCharFormat();

	CString sName = cf.szFaceName;

	return sName;
}

long CAutoRichEditCtrl::GetSelectionFontSize()
{
	CHARFORMAT cf = GetCharFormat();

	long nSize = cf.yHeight/20;

	return nSize;
}

void CAutoRichEditCtrl::AppendString(CString strMsg,COLORREF clr)
{
	try
	{
		if(::IsWindow(m_hWnd))
		{
			SetSel(-1,-1);
			SelectColor(clr);
			ReplaceSel(strMsg);
			::SendMessageA(m_hWnd,WM_VSCROLL, /*SB_LINEDOWN*/ SB_BOTTOM, (LPARAM)NULL);
		}
	}
	catch(...)
	{
	}
}
