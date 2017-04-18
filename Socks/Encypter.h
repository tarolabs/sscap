#pragma once

/*!
 * @file Encypter.h
 * @date 2015/10/19 18:18
 *
 * @brief 加密基类
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
#include <map>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "cryptopp562/cryptlib.h"
#include "cryptopp562/hex.h"
#include "cryptopp562/aes.h"
#include "cryptopp562/ccm.h"
#include "cryptopp562/modes.h"
#include "cryptopp562/arc4.h"
#include "cryptopp562/salsa.h"

#include <time.h>
#include "cryptopp562/md5.h"

using CryptoPP::Exception;
using CryptoPP::AES;
using CryptoPP::CBC_Mode;
using CryptoPP::CFB_Mode;
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;
//using CryptoPP::Weak1;
using CryptoPP::Salsa20;

#include "include/sodium.h"
using namespace std;

#define AES_256_CFB "aes-256-cfb"
#define IDX_AES_256_CFB 0

#define AES_192_CFB "aes-192-cfb"
#define IDX_AES_192_CFB 1

#define AES_128_CFB "aes-128-cfb"
#define IDX_AES_128_CFB 2

#define RC4_MD5 "rc4-md5"
#define IDX_RC4_MD5 3

#define RC4 "rc4"
#define IDX_RC4 4

#define SALSA20 "salsa20"
#define IDX_SALSA20 5

#define CHACHA20 "chacha20"
#define IDX_CHACHA20 6

#define CHACHA20_IETF_ENC "chacha20-ietf"
#define IDX_CHACHA20_IETF_ENC 7

#define TABLE_ENC "table"
#define IDX_TABLE_ENC 8

using namespace std;

class CCryptor
{
public:
	CCryptor( string method_name, string key ,int key_size /* bytes */, int iv_size /* bytes */ );
	~CCryptor();

public:
	/** @brief 加密
	*/
	virtual BOOL Encrypt( char *in, int in_len, char *out , int &out_len) = 0;
	/** @brief 解密
	*/
	virtual BOOL Decrypt( char *in, int in_len, char *out , int &out_len) = 0;
	/** @brief 重置加密信息, 比如重新设置KEY, IV 
	*/
	virtual void ResetCryption();
protected:
	/** @brief 初始化用于加密shadowsocks包的key
	* 先在CacheKey中找,找到就返回, 如未找到就根据key来生成最终的key
	* 规则是: 
	*  1: 将key做md5得到16 bytes做为前16 bytes
	*  2: 将第1步得到的16bytes + 原始key组合得到 result , 再对results 做md5 又得到16 bytes
	*  3: 将第1步得到的16 bytes + 第2步得到的16 byte组保成最终的32 bytes的 key
	*/
	virtual void InitializeSSEncryptKey();
	virtual void GenerateRandomEncryptIV( char *iv, int length );
	/** @brief 返回strInputPass的md5
	*/
	virtual void GetPasswordHash( char *hash /* out */ );
protected:
	string strMethodName; ///< 加密方式的名称, 如: rc4, aes-256-cfb
	string strInputPass;   ///< 用户指定的pass 经过调用InitializeSSKey之后生成最终的key: ( strEncryptKey )
	char strEncryptKey[32];

	bool bEncryptIVSent; ///< 本地发送出去的数据的加密IV已发
	char ivEncrypt[32];

	bool bDecryptIVGot;  ///< SS服务器发过来的数据加密的IV已收
	char ivDecrypt[16];

	int nKeySize; ///< 加密的KEY的SIZE(字节)
	int nIVSize; ///< IVSIZE(字节)
};

/** @brief 实现: aes-128-cfb,aes-192-cfb,aes-256-cfb 的加解密
*/
class CAESCfbCryptor : public CCryptor
{
public:
	CAESCfbCryptor ( string key ,int bits /** key是多少位的: 128, 192, 256 */ );
	virtual ~CAESCfbCryptor();

public:
	/** @brief 加密
	*/
	virtual BOOL Encrypt( char *in, int in_len, char *out , int &out_len);
	/** @brief 解密
	*/
	virtual BOOL Decrypt( char *in, int in_len, char *out , int &out_len);
protected:
	CFB_Mode<AES>::Encryption AesCfbEncryption;
	CFB_Mode<AES>::Decryption AesCfbDecryption;
};

/** @brief 实现rc4, 以及shadowsocks中的rc4-md5的加解密
*
* rc4-md5是ss作者自己起的名
* 旧的 RC4 加密之所以不安全是因为 Shadowsocks 在每个连接上重复使用 key，没有使用 IV。
* 但 RC4 比 AES 速度快好几倍，如果用在路由器上会带来显著性能提升。
* 所以按照正确的方式重新实现了 RC4 加密，为了区别旧的 RC4 起名为 RC4-MD5。
* https://github.com/shadowsocks/shadowsocks/issues/178
* rc4-md5的实现就是将key再hash一次
* true_key = hash(key + IV)
* 
* 
* RC4-MD5 spec:
* Generate key from password via EVP_bytes_to_key as usual.
* key is 16 bytes and IV is 16 bytes.
* Generate true RC4 key from key and IV: rc4_key = md5(key + IV). Thus we have a different RC4 key per * * connection. rc4_key is 16 bytes.
* Use RC4 key to do encryption & decryption.
*/
class CRC4Cryptor: public CCryptor
{
public:
	CRC4Cryptor( string key ,bool bRC4Md5 /** 是否为rc4-md5 */ );
	virtual ~CRC4Cryptor();

protected:
	bool bIsRC4Md5Cipher;
	char szDecryptKey[32]; // 用于rc4-md5的key, 动态接收到iv再生成的key
	char szOriginalKey[32]; // 原始key, 因为rc4-md5会用iv和原始key产生真实key, 所以这里保存原始key.
public:
	/** @brief 加密
	*/
	virtual BOOL Encrypt( char *in, int in_len, char *out , int &out_len);
	/** @brief 解密
	*/
	virtual BOOL Decrypt( char *in, int in_len, char *out , int &out_len);
protected:
	virtual void InitializeRC4Key( BOOL bIsEncKey  /* TRUE: Enc Key, FALSE: Des Key */ );
	void GenerateRC4Md5Key( char *oukey ,char *inkey, char *iv);

protected:
	CryptoPP::Weak1::ARC4::Encryption rc4Encryption;
	CryptoPP::Weak1::ARC4::Decryption rc4Decryption;
};

/** @brief salsa20
*/
class CSalsa20Criptor :public CCryptor
{
public:
	CSalsa20Criptor( string key );
	virtual ~CSalsa20Criptor(){}

public:
	/** @brief 加密
	*/
	virtual BOOL Encrypt( char *in, int in_len, char *out , int &out_len);
	/** @brief 解密
	*/
	virtual BOOL Decrypt( char *in, int in_len, char *out , int &out_len);

protected:
	Salsa20::Encryption salsaEncryption;
	Salsa20::Decryption salsaDecryption;
};

class CTableCryptor : public CCryptor
{
public:
	CTableCryptor( string key );
	virtual ~CTableCryptor(){}

public:
	/** @brief 加密
	*/
	virtual BOOL Encrypt( char *in, int in_len, char *out , int &out_len);
	/** @brief 解密
	*/
	virtual BOOL Decrypt( char *in, int in_len, char *out , int &out_len);
};


#define SODIUM_BLOCK_SIZE   64
// 使用Libsodium进行加解密
// http://doc.libsodium.org/installation/index.html
// http://doc.libsodium.org/advanced/chacha20.html
class CChaCha20Criptor : public CCryptor
{
public:
	CChaCha20Criptor( string key);
	virtual ~CChaCha20Criptor(){}

public:
	/** @brief 加密
	*/
	virtual BOOL Encrypt( char *in, int in_len, char *out , int &out_len);
	/** @brief 解密
	*/
	virtual BOOL Decrypt( char *in, int in_len, char *out , int &out_len);
	/** @brief 重置加密信息, 比如重新设置KEY, IV 
	*/
	virtual void ResetCryption();
protected:
	u_long uEnCounter;
	u_long uDeCounter;
};

#define CHACHA20_IETF_ENC_KEY_SIZE 32
#define CHACHA20_IETF_ENC_IV_SIZE 12

// 使用Libsodium进行加解密
// http://doc.libsodium.org/installation/index.html
// http://doc.libsodium.org/advanced/chacha20.html
class CChaCha20IetfCriptor : public CCryptor
{
public:
	CChaCha20IetfCriptor( string key);
	virtual ~CChaCha20IetfCriptor(){}

public:
	/** @brief 加密
	*/
	virtual BOOL Encrypt( char *in, int in_len, char *out , int &out_len);
	/** @brief 解密
	*/
	virtual BOOL Decrypt( char *in, int in_len, char *out , int &out_len);
	/** @brief 重置加密信息, 比如重新设置KEY, IV 
	*/
	virtual void ResetCryption();
protected:
	u_long uEnCounter;
	u_long uDeCounter;
};