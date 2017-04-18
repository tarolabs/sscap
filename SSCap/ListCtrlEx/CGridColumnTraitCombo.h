#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridColumnTraitImage.h"

//------------------------------------------------------------------------
//! CGridColumnTraitCombo implements a CComboBox as cell-editor
//------------------------------------------------------------------------
class CGridColumnTraitCombo : public CGridColumnTraitImage
{
public:
	CGridColumnTraitCombo();

	void SetMaxItems(UINT nMaxItems);
	UINT  GetMaxItems() const;

	void SetStyle(DWORD dwStyle);
	DWORD GetStyle() const;

	void SetMaxWidth(UINT nMaxWidth);
	UINT  GetMaxWidth() const;

	void SetShowDropDown(BOOL bShowIt);
	BOOL GetShowDropDown() const;

	void LoadList(const CSimpleMap<DWORD_PTR,CString>& comboList, int nCurSel);
	void AddItem(DWORD_PTR nItemData, const CString& strItemText);
	void ClearFixedItems();

	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol);
	virtual CWnd* OnEditBegin(CGridListCtrlEx& owner, int nRow, int nCol, CPoint pt) { return CGridColumnTraitImage::OnEditBegin(owner, nRow, nCol, pt); }
	virtual void  OnEditEnd();

protected:
	virtual void Accept(CGridColumnTraitVisitor& visitor);
	virtual CComboBox* CreateComboBox(CGridListCtrlEx& owner, int nRow, int nCol, DWORD dwStyle, const CRect& rect);

	CSimpleMap<DWORD_PTR,CString> m_ComboList;	//!< Fixed list of items in the combo-box
	CComboBox* m_pComboBox;					//!< CComboBox currently open
	DWORD m_ComboBoxStyle;					//!< Style to use when creating CComboBox
	UINT m_MaxItems;						//!< Max height (in items) of the CComboBox when doing dropdown
	UINT m_MaxWidth;						//!< Max width (in pixels) of the CComboBox when doing dropdown
	BOOL m_ShowDropDown;					//!< Show drop down of the CComboBox at edit begin

private:
	// Private because they doesn't handle CSimpleMap
	CGridColumnTraitCombo(const CGridColumnTraitCombo&);
	CGridColumnTraitCombo& operator=(const CGridColumnTraitCombo& other);
};

//------------------------------------------------------------------------
//! CEdit inside CComboBox for inplace edit. For internal use by CGridColumnTraitCombo
//
// Taken from "MFC Grid control" credits Chris Maunder
// http://www.codeproject.com/KB/miscctrl/gridctrl.aspx
//------------------------------------------------------------------------
class CGridEditorComboBoxEdit : public CEdit
{
public:
	CGridEditorComboBoxEdit();

protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);

	DECLARE_MESSAGE_MAP();

private:
	CGridEditorComboBoxEdit(const CGridEditorComboBoxEdit&);
	CGridEditorComboBoxEdit& operator=(const CGridEditorComboBoxEdit&);
};

//------------------------------------------------------------------------
//! CComboBox for inplace edit. For internal use by CGridColumnTraitCombo
//
// Taken from "MFC Grid control" credits Chris Maunder
// http://www.codeproject.com/KB/miscctrl/gridctrl.aspx
//------------------------------------------------------------------------
class CGridEditorComboBox : public CComboBox
{
public:
	CGridEditorComboBox(int nRow, int nCol, UINT nMaxWidthPixels, UINT nMaxHeightItems, BOOL bShowDropDown);

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual void EndEdit(bool bSuccess);

protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
	afx_msg void OnDropDown();
	afx_msg void OnCloseUp();
	afx_msg void OnChangeSelection();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP();

	CGridEditorComboBoxEdit m_Edit;	//!< Subclassed edit control inside the CComboBox
	bool	m_Completed;			//!< Ensure the editor only reacts to a single close event
	bool	m_Modified;				//!< Register if selection was modified while the editor was open
	int		m_Row;					//!< The index of the row being edited
	int		m_Col;					//!< The index of the column being edited
	UINT	m_MaxWidthPixels;		//!< Max width (in pixels) of the CComboBox when doing dropdown
	UINT	m_MaxHeightItems;		//!< Max height (in items) of the CComboBox when doing dropdown
	BOOL	m_ShowDropDown;			//!< Show drop down of the CComboBox at edit begin

private:
	CGridEditorComboBox();
	CGridEditorComboBox(const CGridEditorComboBox&);
	CGridEditorComboBox& operator=(const CGridEditorComboBox&);
};
