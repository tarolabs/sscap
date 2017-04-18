#pragma once

#include <string>

// 线上pac文件
// base64编码
#define ONLINE_PAC_URL "https://raw.githubusercontent.com/tarolabs/sscap/master/bpac.txt"

using namespace std;
/** @brief 加载PAC文件
* 
* bForceLoad = TRUE 强制載, 即使之前加载过了.
*/
BOOL LoadPacFile( BOOL bForceLoad = FALSE );
/** @brief 获取PAC文件内容
*/
string GetPacFileContent();
/** @brief 更新线上PAC文件
*/
BOOL UpdateOnlinePacFile();
string GetUserPacFileContent();