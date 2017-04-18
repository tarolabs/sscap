
// SSCap.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSSCapApp:
// See SSCap.cpp for the implementation of this class
//
#define TEMP_QRCODE_FILE _T("tempimage.png")
#include "SingleInstance.h"

class CSSCapApp : public CWinApp
{
public:
	CSSCapApp();

protected:
	CLimitSingleInstance singleInstance;

protected:
	BOOL DetectSingleInstance();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
};

extern CSSCapApp theApp;
