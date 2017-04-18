#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

#include "CGridRowTraitText.h"

//------------------------------------------------------------------------
//! CGridRowTraitXP fixes drawing of rows when the application is using
//! classic- or XP-style.
//------------------------------------------------------------------------
class CGridRowTraitXP : public  CGridRowTraitText
{
public:
	virtual void OnCustomDraw(CGridListCtrlEx& owner, NMLVCUSTOMDRAW* pLVCD, LRESULT* pResult);

protected:
	virtual void Accept(CGridRowTraitVisitor& visitor);
};