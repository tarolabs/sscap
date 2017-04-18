#pragma once

/*!
 * @file EncyptionMgr.h
 * @date 2015/10/20 20:44
 *
 * @brief 加密管理类
 *
 * 
 * @author Taro
 * Contact: sockscap64@gmail.com
 *
 *
 * @version 1.0
 *
 * @History
 * <author>      <time>      <version>         <desc>
 *
 * @TODO: long description
 *
 * @note
*/
#include <string>
#include <vector>

using namespace std;

class CCryptor;

struct __cryption_list 
{
	int idx;
	string name;
};

class CEncryptionMgr
{
public:
	CEncryptionMgr();
	virtual ~CEncryptionMgr();

public:
	static CCryptor *Create( string method, string key );
	static int GetEncyptionList( vector<string> &enc );
	/** @brief 禁止chacha20算法
	* 因为有可能加载chacha的库: sodium失败
	*/
	static void DisableChaCha20()
	{
		CEncryptionMgr::bChaCha20Disabled = true;
	}
	/** @brief 通过加密名称获取加密的IDEX
	* 
	* @return 返回加密名称对应的INDEX,未找以返回-1
	*/
	static int GetCryptionIndexByName( string name );
protected:
	static bool bChaCha20Disabled;
};