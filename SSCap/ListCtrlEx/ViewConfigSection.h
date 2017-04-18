#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all (New BSD License)
//------------------------------------------------------------------------

//------------------------------------------------------------------------
//! Abstract interface for persisting view configuration
//------------------------------------------------------------------------
class CViewConfigSection
{
protected:
	CString m_ViewName;		//!< Configuration name used when persisting the state (Translates into a section name)

	//! Pure virtual interface for reading setting from persisting layer
	virtual CString ReadSetting(const CString& strSection, const CString& strSetting, const CString& strDefval) const = 0;
	//! Pure virtual interface for writing setting to persisting layer
	virtual void WriteSetting(const CString& strSection, const CString& strSetting, const CString& strValue) = 0;
	//! Pure virtual interface for removing setting section from persisting layer
	virtual void RemoveSection(const CString& strSection) = 0;

	// Converters
	virtual CString ConvertBoolSetting(bool bValue) const;
	virtual CString ConvertIntSetting(int nValue) const;
	virtual CString ConvertFloatSetting(double nValue, int nDecimals = 6) const;
	virtual CString ConvertArraySetting(const CSimpleArray<CString>& values, const CString& strDelimiter = _T(", ")) const;
	virtual CString ConvertArraySetting(const CSimpleArray<int>& values, const CString& strDelimiter = _T(", ")) const;
	virtual CString ConvertLogFontSetting(const LOGFONT& font) const;
	virtual CString ConvertRectSetting(const RECT& rect) const;
	virtual CString ConvertColorSetting(COLORREF color) const;
	virtual void	SplitArraySetting(const CString& strArray, CSimpleArray<CString>& values, const CString& strDelimiter = _T(", ")) const;

	virtual const CString& GetSectionName() const;

public:
	explicit CViewConfigSection(const CString& strViewName);
	virtual ~CViewConfigSection();

	// Getters
	virtual CString GetSetting(const CString& strName, const CString& strDefval = _T("")) const;
	virtual bool GetBoolSetting(const CString& strName, bool bDefval = false) const;
	virtual int GetIntSetting(const CString& strName, int nDefval = 0) const;
	virtual double GetFloatSetting(const CString& strName, double nDefval = 0.0) const;
	virtual LOGFONT GetLogFontSetting(const CString& strName) const;
	virtual CRect GetRectSetting(const CString& strName, const CRect& rectDefval = CRect(0,0,0,0)) const;
	virtual COLORREF GetColorSetting(const CString& strName, const COLORREF colorDefval = RGB(0,0,0)) const;
	virtual void GetArraySetting(const CString& strName, CSimpleArray<CString>& values, const CString& strDelimiter = _T(", ")) const;
	virtual void GetArraySetting(const CString& strName, CSimpleArray<int>& values, const CString& strDelimiter = _T(", ")) const;

	// Setters
	virtual void SetSetting(const CString& strName, const CString& strValue);
	virtual void SetBoolSetting(const CString& strName, bool bValue);
	virtual void SetIntSetting(const CString& strName, int nValue);
	virtual void SetFloatSetting(const CString& strName, double nValue, int nDecimals = 6);
	virtual void SetArraySetting(const CString& strName, const CSimpleArray<CString>& values, const CString& strDelimiter = _T(", "));
	virtual void SetArraySetting(const CString& strName, const CSimpleArray<int>& values, const CString& strDelimiter = _T(", "));
	virtual void SetLogFontSetting(const CString& strName, const LOGFONT& font);
	virtual void SetRectSetting(const CString& strName, const RECT& rect);
	virtual void SetColorSetting(const CString& strName, COLORREF color);

	virtual void RemoveCurrentConfig();
};

//------------------------------------------------------------------------
//! Abstract interface for persisting view configuration, that can use
//! an in-memory default-configuration.
//!
//! It will use the values in the default-config if nothing else can be found
//------------------------------------------------------------------------
class CViewConfigSectionDefault : public CViewConfigSection
{
protected:
	//! Inner class that stores the default configuration in memory
	class CViewConfigSectionLocal : public CViewConfigSection
	{
	protected:
		CSimpleMap<CString,CString> m_LocalSettings;	//!< Default configuration

		// Persistence of settings
		virtual CString ReadSetting(const CString& strSection, const CString& strName, const CString& strDefval) const;
		virtual void WriteSetting(const CString& strSection, const CString& strName, const CString& strValue);
		virtual void RemoveSection(const CString& strSection);

	public:
		explicit CViewConfigSectionLocal(const CString& strViewName);
		CViewConfigSectionLocal(const CViewConfigSectionLocal& other);
		CViewConfigSectionLocal& operator=(const CViewConfigSectionLocal& other);

		bool HasSettings() const;
		void CopySettings(CViewConfigSection& destination) const;
	};
	CViewConfigSectionLocal m_DefaultConfig;	//!< Default configuration stored in memory

public:
	explicit CViewConfigSectionDefault(const CString& strViewName);

	virtual CViewConfigSection& GetDefaultConfig();
	virtual bool HasDefaultConfig() const;
	virtual void ResetConfigDefault();

	virtual CString GetSetting(const CString& strName, const CString& strDefval = _T("")) const;
};

//------------------------------------------------------------------------
//! Abstract interface for persisting view configuration, that can switch
//! between different view configuration profiles.
//------------------------------------------------------------------------
class CViewConfigSectionProfiles : public CViewConfigSectionDefault
{
protected:
	mutable CString m_CurrentSection;	//!< Section name combined from the viewname and the current profile name
	virtual const CString& GetSectionName() const;

	virtual void SplitSectionName(const CString& strSection, CString& strViewName, CString& strProfile);
	virtual CString JoinSectionName(const CString& strViewName, const CString& strProfile) const;

public:
	explicit CViewConfigSectionProfiles(const CString& strViewName);

	virtual void RemoveCurrentConfig();

	virtual void GetProfiles(CSimpleArray<CString>& profiles) const;
	virtual CString GetActiveProfile();
	virtual void SetActiveProfile(const CString& strProfile);
	virtual void AddProfile(const CString& strProfile);
	virtual void DeleteProfile(const CString& strProfile);
};

//------------------------------------------------------------------------
//! Can persist the column configuration using CWinApp::WriteProfile()
//------------------------------------------------------------------------
class CViewConfigSectionWinApp : public CViewConfigSectionProfiles
{
protected:
	virtual CString ReadSetting(const CString& strSection, const CString& strSetting, const CString& strDefval) const;
	virtual void WriteSetting(const CString& strSection, const CString& strSetting, const CString& strValue);
	virtual void RemoveSection(const CString& strSection);

public:
	CViewConfigSectionWinApp(const CString& strViewName);
};

