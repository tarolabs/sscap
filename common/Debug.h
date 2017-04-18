#pragma once
#include <stdio.h>
#include <tchar.h>
#include "mymutex.h"
typedef void (CALLBACK FAR * DEBUG_CALLBACK)(TCHAR *msg,int curlevel);
#define _DEBUG_OUPUT //make debug output valid.

namespace debug
{
	/** 输出到文件*/
#define OUTPUT_TYPE_FILE	0
	/** 输出到回调, 调用者自己处理输出*/
#define OUTPUT_TYPE_CALLBACK	1
	/** 输出到DEBUG窗口*/
#define OUTPUT_TYPE_DEBUG	2

#define LEVEL_KERNEL		0
#define LEVEL_DEBUG			1
#define LEVEL_INFO			2
#define LEVEL_WARNING		3
#define LEVEL_ERROR			4
#define LEVEL_HEX
#define LEVEL_NONE			10

	class CDebug
	{
	public:
		CDebug(void);
		~CDebug(void);


	public:
		static void __stdcall  SetOutputType(int type)
		{
			m_nOutputType = type;
		}
		static int __stdcall  GetOutputType()
		{
			return m_nOutputType;
		}
		static void __stdcall SetDebugCallback( DEBUG_CALLBACK pCB ){
			pDebugCallback = pCB;
		}
		static void __stdcall SetLogFile( TCHAR *logfile ){
			_tcscpy_s( szLogFile,MAX_PATH, logfile );
		}
		static void __stdcall SetDebugLevel( int level ){
			m_nDebugLevel = level;
		}
		static void _stdcall SetHostProcessName( TCHAR *szName )
		{
			if( szName )
				_tcscpy_s( szHostProcessName,MAX_PATH, szName );
		}

		static const TCHAR *LevelString(int nLevel);
		/** 打印到调试窗口*/
		static void PrintfToDebugWindow(TCHAR *msg,int curlevel);
		/** 打印到回调*/
		static void PrintfToCallback(TCHAR *msg,int curlevel);
		/** 打印到文件*/
		static void PrintfToFile(TCHAR *msg,int curlevel);
		//static void PrintfHexToFile( char *msg );
		static TCHAR szLogFile[MAX_PATH];
		static TCHAR szHostProcessName[MAX_PATH];
		static int				m_nOutputType;
		static DEBUG_CALLBACK   pDebugCallback;
		static CMyMutex			m_nMutex;
		static int				m_nDebugLevel;
	};
}
#if defined _DEBUG && defined _DEBUG_OUPUT
/** 打印输出 unicode */
void _cdecl PrintfW( int level,const wchar_t *fmt, ...);
void _cdecl PrintfInfoW(const wchar_t *fmt, ...);
void _cdecl PrintfWarningW(const wchar_t *fmt, ...);
void _cdecl PrintfErrorW(const wchar_t *fmt, ...);
/** 打印输出 Ansi */
void __cdecl PrintfA(int level,const char *fmt, ...);
void __cdecl PrintfInfoA( const char *fmt, ... );
void __cdecl PrintfWarningA( const char *fmt, ... );
void __cdecl PrintfErrorA( const char *fmt, ... );
void PrintfHex( char *desc, void *addr, int len) ;
void PrintfHex1( char *desc, void *addr, int len) ;
#endif

#if !defined _DEBUG_OUPUT || !defined _DEBUG
#define PrintfW
#define PrintfInfoW
#define PrintfWarningW
#define PrintfErrorW

#define PrintfA
#define PrintfInfoA
#define PrintfWarningA
#define PrintfErrorA

#define PrintfHex
#define PrintfHex1
#endif