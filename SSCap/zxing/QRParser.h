#pragma once
#include <string>
using namespace std;

/** @brief 解析QR文件
*
* @param filename 要解析的QR文件
* @param QRResult 解析出来的结果
*/
bool ZXingParseQRInfo( string filename,string& QRResult);
