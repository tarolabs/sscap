
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars


#include <afxsock.h>            // MFC socket extensions


//////////////////////////////////////////////////////////////////////////
// gettest
#include <locale.h>
#include "libintl.h"
#include <stdio.h>
#include <afxdhtml.h>
#include "Utils.h"
#include "tarobase64.h"
#include "Registry.h"
#include "mymutex.h"
#include "Ini.h"

// Doing this allows you to prepare the sources for internationalization. Later when you feel ready for the step to use the gettext library simply replace these definitions by the following:
#define _(string) gettext (string)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#pragma comment(lib,"libintl.lib")
#pragma warning(disable:4005) 
#pragma warning(disable:4800) 
// end gettext
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// version
#define SSCAP_VERSION _T("3.7")
#define SSCAP_NAME _T("SSCap")
#define SSCAP_DEFAULT_WEBSITE_SOURCEFORGE _T( "https://sourceforge.net/projects/sscap/" )
#define SSCAP_DEFAULT_WEBSITE_GITHUB _T( "https://github.com/tarolabs/sscap" )
#define SSCAP_DEFAULT_WEBSITE_BAIDUPAN _T("http://pan.baidu.com/s/1jHccwqq")

#define SSCAP_DEFAULT_WEBSITE_GITHUB_RELEASE _T( "https://github.com/tarolabs/sscap/releases" )

#define SSCAP_COUNTERPAGE_URL _T("http://www.sockscap64.com/client/client.php?version=sscap")

//#define VERSION_URL _T("https://raw.githubusercontent.com/tarolabs/sscap/master/version.txt")
#define NEWVERSION_URL _T("https://raw.githubusercontent.com/tarolabs/sscap/master/newversion.txt")

// githubµÿ÷∑.
//https://github.com/tarolabs/sscap

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


