#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTraitImage.h"

//------------------------------------------------------------------------
//! CGridColumnTraitHyperLink that can launch a link using the web-browser
//------------------------------------------------------------------------
class CGridColumnTraitHyperLink : public CGridColumnTraitImage
{
public:
	CGridColumnTraitHyperLink();

	void SetLinkColor(COLORREF linkColor);
	COLORREF GetLinkColor() const;

	void SetLinkColorHot(COLORREF linkColor);
	COLORREF GetLinkColorHot() const;

	void SetShellOperation(const CString& strShellOperation);
	CString GetShellOperation() const;

	void SetShellApplication(const CString& strShellAppliction);
	CString GetShellApplication() const;

	void SetShellFilePrefix(const CString& strShellFilePrefix);
	CString GetShellFilePrefix() const;

	void SetShellFileSuffix(const CString& strShellFileSuffix);
	CString GetShellFileSuffix() const;

	void SetShellShowCommand(INT nShellShowCommand);
	INT GetShellShowCommand() const;

protected:
	virtual bool UpdateTextColor(NMLVCUSTOMDRAW* pLVCD, COLORREF& textColor);
	virtual bool UpdateTextFont(NMLVCUSTOMDRAW* pLVCD, LOGFONT& textFont);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt) { return CGridColumnTraitImage::OnEditBegin(owner, nRow, nCol, pt); }
	virtual int OnClickEditStart(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt, bool bDblClick);

	virtual void OnShellExecute(CGridListCtrlEx& owner, int nRow, int nCol, const CString& cellText);
	virtual CRect GetTextRect(CGridListCtrlEx& owner, int nRow, int nCol, const CString& cellText);

	COLORREF m_LinkColor;			//!< Standard link Color
	COLORREF m_LinkColorHot;		//!< Hot link color (mouse over)
	CString m_ShellOperation;		//!< ShellExecute operation (Ex. "open")
	CString m_ShellApplication;		//!< ShellExecute application (If blank it launches cell text with default application)
	CString m_ShellFilePrefix;		//!< ShellExecute file specifier prefix
	CString m_ShellFileSuffix;		//!< ShellExecute file specifier suffix
	INT		m_ShellShowCommand;		//!< ShellExecute show application flags (Ex. SW_SHOWNORMAL)
};