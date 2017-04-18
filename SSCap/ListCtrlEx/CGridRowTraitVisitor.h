#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

class CGridRowTrait;
class CGridRowTraitText;
class CGridRowTraitXP;

//------------------------------------------------------------------------
//! CGridRowTraitVisitor enables the use of the visitor-pattern to
//! add extra behavior to the CGridRowTrait classes
//------------------------------------------------------------------------
class CGridRowTraitVisitor
{
public:
	void Visit(CGridRowTrait&)				{}
	void Visit(CGridRowTraitText&)			{}
	void Visit(CGridRowTraitXP&)			{}
};