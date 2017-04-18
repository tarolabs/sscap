#pragma once
#include <string>
using namespace std;

/** @brief 调用zbar执行二维码扫描
*
* filename 要扫描的文件
* QRResult 扫描的结果
*/
BOOL ZBarParseQRCode( string filename, string &QRResult );