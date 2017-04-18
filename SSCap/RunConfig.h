#pragma once

class CRunConfig
{
public:
	CRunConfig(){}
	virtual ~CRunConfig(){}

public:
	static BOOL InitializeAppPath();
	static LPCTSTR GetAppWorkingDirectory();
	static LPCTSTR GetApplicationFullPathName();

protected:
	static TCHAR szApplicationFileName[1024 ];	///< 当前EXE文件的完整路径文件名.
	static TCHAR szProgramWorkingDirectory[ 1024 ];			///< SocksCap64的主工作目录
};

void Init_Locale( const TCHAR *szWorkingDir );