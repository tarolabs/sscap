#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

class CGridColumnTrait;
class CGridColumnTraitImage;
class CGridColumnTraitCombo;
class CGridColumnTraitDateTime;
class CGridColumnTraitEdit;
class CGridColumnTraitText;

//------------------------------------------------------------------------
//! CGridColumnTraitVisitor enables the use of the visitor-pattern to
//! add extra behavior to the CGridColumnTrait classes
//------------------------------------------------------------------------
class CGridColumnTraitVisitor
{
public:
	void Visit(CGridColumnTrait&)			{}
	void Visit(CGridColumnTraitImage&)	{}
	void Visit(CGridColumnTraitCombo&)		{}
	void Visit(CGridColumnTraitDateTime&)	{}
	void Visit(CGridColumnTraitEdit&)		{}
	void Visit(CGridColumnTraitText&)		{}
};