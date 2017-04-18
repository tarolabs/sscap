//#include "stdheader.h"
#include <afxwin.h>
#include "IeControl.h"

CString CURL::m_strBrowser;
//HANDLE CURL::m_hIeInstance = FALSE;

//HANDLE CURL::GetInstance()
//{
//	return m_hIeInstance;
//}
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// 以 CreateProcess 方式打开
HANDLE CURL::Open_Process(LPCTSTR lpszURL)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	LPCTSTR str=GetBrowser();

	TCHAR buffer[260]={0};
	_stprintf_s(buffer,260,_T("%s %s") ,str,lpszURL);
	// Start the child process. 
	if( !CreateProcess( NULL, // No module name (use command line). 
		buffer, // Command line. 
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		0,                // No creation flags. 
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi )             // Pointer to PROCESS_INFORMATION structure.
		) 
	{
		//ErrorExit( "CreateProcess failed." );
		return NULL;
	}

	return pi.hProcess;
}

void CURL::Open(LPCTSTR lpszURL, bool bNewWindow )
{
	//change by barry,because the internet explorer can't open a file such as rar and etc.
	CString tmpUrl(lpszURL);

	tmpUrl.MakeLower();
	tmpUrl.Trim();
	//处理本地文件时（包括mailto）不需要强制新窗口，即可在新窗口打开
	//window默认对http和www起头的字符串处理为网页
	if (tmpUrl.Find(_T("http"),0)!=0&&tmpUrl.Find(_T("www"),0)!=0&&tmpUrl.Find(_T("ftp"),0)!=0)
		bNewWindow=false;

	if (bNewWindow)
	{
		LPCTSTR str=GetBrowser();
		if (str)
			::ShellExecute(NULL, NULL, str, lpszURL, NULL, SW_SHOWNORMAL);
		else
			::ShellExecute(NULL, NULL, lpszURL, NULL, NULL, SW_SHOWNORMAL);
	}
	else
		::ShellExecute(NULL, NULL, lpszURL, NULL, NULL, SW_SHOWNORMAL);
}

LPCTSTR CURL::GetBrowser(void)
{
	CFileFind fd;
	// Do we have the default browser yet?
	if (m_strBrowser.IsEmpty())
	{
		// Get the default browser from HKCR\http\shell\open\command
		HKEY hKey = NULL;
		// Open the registry
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			// Data size
			DWORD cbData = 0;
			// Get the default value
			if (::RegQueryValueEx(hKey, NULL, NULL, NULL, NULL, &cbData) == ERROR_SUCCESS && cbData > 0)
			{
				// Allocate a suitable buffer
				TCHAR* psz = new TCHAR [cbData];
				// Success?
				if (psz != NULL)
				{
					if (::RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)psz, &cbData) ==ERROR_SUCCESS)
					{
						// Success!
						m_strBrowser = psz;
					}
					delete [] psz;
				}
			}
			::RegCloseKey(hKey);
		}

		//加强判断如果没有该文件则清空
		if (!m_strBrowser.IsEmpty())
			if(!fd.FindFile(m_strBrowser,0)) m_strBrowser.Empty();

		if (m_strBrowser.IsEmpty())
		{
			//使用旧的方法
			// Get the default browser from HKCR\http\shell\open\command
			HKEY hKey = NULL;
			// Open the registry
			if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("http\\shell\\open\\command"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				// Data size
				DWORD cbData = 0;
				// Get the default value
				if (::RegQueryValueEx(hKey, NULL, NULL, NULL, NULL, &cbData) == ERROR_SUCCESS && cbData > 0)
				{
					// Allocate a suitable buffer
					TCHAR* psz = new TCHAR [cbData];
					// Success?
					if (psz != NULL)
					{
						if (::RegQueryValueEx(hKey, NULL, NULL,
							NULL, (LPBYTE)psz, &cbData) ==
							ERROR_SUCCESS)
						{
							// Success!
							m_strBrowser = psz;
						}
						delete [] psz;
					}
				}
				::RegCloseKey(hKey);
			}
			// Do we have the browser?
			if (m_strBrowser.GetLength() > 0)
			{
				// Strip the full path from the string
				int nStart = m_strBrowser.Find('"');
				int nEnd = m_strBrowser.ReverseFind('"');
				// Do we have either quote?
				// If so, then the path contains spaces
				if (nStart >= 0 && nEnd >= 0)
				{
					// Are they the same?
					if (nStart != nEnd)
					{			
						// Get the full path
						m_strBrowser = m_strBrowser.Mid(nStart + 1, nEnd - nStart - 1);
					}
				}
				else
				{
					// We may have a pathname with spaces but
					// no quotes (Netscape), e.g.:
					//   C:\PROGRAM FILES\NETSCAPE\COMMUNICATOR\PROGRAM\NETSCAPE.EXE -h "%1"
					// Look for the last backslash
					int nIndex = m_strBrowser.ReverseFind('\\');
					// Success?
					if (nIndex > 0)
					{
						// Look for the next space after the final
						// backslash
						int nSpace = m_strBrowser.Find(' ', nIndex);
						// Do we have a space?
						if (nSpace > 0)
							m_strBrowser = m_strBrowser.Left(nSpace);				
					}
				}
			}
			//加强判断如果没有该文件则清空
			if (!m_strBrowser.IsEmpty())
				if(!fd.FindFile(m_strBrowser,0)) m_strBrowser.Empty();
		}
	}
	// Done
	return m_strBrowser;
}