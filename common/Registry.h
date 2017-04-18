#if !defined(AFX_REGISTRY_H__E0610A5D_7166_4D02_9D7E_11AF7CF8E229__INCLUDED_)
#define AFX_REGISTRY_H__E0610A5D_7166_4D02_9D7E_11AF7CF8E229__INCLUDED_

//
#include <winreg.h>
#include <list>
using namespace  std;
/////////////////////////////////////////////////////////////////////////////
// CRegistry window

class CRegistry : public CObject
{
// Construction
public:
	CRegistry(HKEY hKey=HKEY_LOCAL_MACHINE);

public:
	BOOL SaveKey(LPCTSTR lpFileName);
	BOOL RestoreKey(LPCTSTR lpFileName);
	BOOL Read(LPCTSTR lpValueName, CString* lpVal);
	BOOL Read(LPCTSTR lpValueName, DWORD* pdwVal);
	BOOL Read(LPCTSTR lpValueName, int* pnVal);
	BOOL Write(LPCTSTR lpSubKey, LPCTSTR lpVal);
	BOOL Write(LPCTSTR lpSubKey, DWORD dwVal);
	BOOL Write(LPCTSTR lpSubKey, int nVal);
	BOOL DeleteKey(HKEY hKey, LPCTSTR lpSubKey);
	BOOL DeleteValue(LPCTSTR lpValueName);
	void Close();
	BOOL EnumSubKeys( LPCTSTR lpSubKey, list<CString> &listReturn , DWORD &dwErrCode );
	BOOL Open(LPCTSTR lpSubKey);
	BOOL CreateKey(LPCTSTR lpSubKey);
	virtual ~CRegistry();

protected:
	HKEY m_hKey;
	
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_REGISTRY_H__E0610A5D_7166_4D02_9D7E_11AF7CF8E229__INCLUDED_)
