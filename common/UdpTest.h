#pragma once

/** @brief 用于UDP 测试
*/

/** 成功返回TRUE,失败返回FALSE
* 返回TRUE的情况下，dwDataTransferLatency 包含 数据传输的延迟, 否则此值无效
*/
BOOL TestUdpConnection( 
	const char *server_ip,
	int server_port, 
	CCryptor *pCryptor, 
	LPCTSTR lpszUDPServer,
	LPCTSTR lpszUDPData,
	DWORD &dwDataTransferLatency );