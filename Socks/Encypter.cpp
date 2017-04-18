#include "stdheader.h"
#include "Encypter.h"
#include "mymutex.h"
#include "Debug.h"

using namespace debug;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define OUTPU_ENCRYPTION

// Shadowsocks 的 Keys缓存
// 组合方式是: 
// 加密方式名称:password = key
map<string,string> MapCacheKeys;
CMyMutex mutexKeyCache;

/** @brief 从缓存中查找key
* 找到就返回TRUE, 并且key保存于result中了
* 减少反复计算KEY花费的时间
*/
BOOL GetKeyFromCache( string k, string &result )
{
	CMutexParser p ( & mutexKeyCache );

	if( MapCacheKeys.empty() )
		return FALSE;

	map<string,string>::iterator iter = MapCacheKeys.find( k );
	if( iter == MapCacheKeys.end() )
		return FALSE;

	result = iter->second;

	return TRUE;
}

CCryptor::CCryptor( string method_name, string key ,int key_size, int iv_size )
{
	strMethodName = method_name ;

	strInputPass = key;

	// create iv for send.
	GenerateRandomEncryptIV( ivEncrypt, sizeof( ivEncrypt ));

	bEncryptIVSent = false;

	bDecryptIVGot = false;
	
	memset( ivDecrypt,0, AES::BLOCKSIZE);

	memset( strEncryptKey, 0 , 32 );

	nKeySize =  key_size;

	nIVSize = iv_size;

	InitializeSSEncryptKey();
}

CCryptor::~CCryptor()
{

}
/** @brief 返回strInputPass的md5
*/
void CCryptor::GetPasswordHash( char *hash /* out */ )
{
	CryptoPP::Weak::MD5 md5;
	md5.CalculateDigest( (byte*)hash, (byte*) strInputPass.c_str(), strInputPass.length() );

	return;
}

void CCryptor::GenerateRandomEncryptIV( char *iv, int length )
{
	srand( (unsigned)time(NULL) + GetTickCount() + GetCurrentThreadId() );

	for( int i = 0 ; i < length ; i ++ )
	{
		iv[i] = rand() % 255;
	}
}
/** @brief 初始化用于加密shadowsocks包的key
* 先在CacheKey中找,找到就返回, 如未找到就根据key来生成最终的key
* 规则是: 
*  1: 将key做md5得到16 bytes做为前16 bytes
*  2: 将第1步得到的16bytes + 原始key组合得到 result , 再对results 做md5 又得到16 bytes
*  3: 将第1步得到的16 bytes + 第2步得到的16 byte组保成最终的32 bytes的 key
*/
void CCryptor::InitializeSSEncryptKey()
{
	string k = strMethodName + ":" + strInputPass;

	string finalKey;

	if( GetKeyFromCache( k, finalKey ))
	{
		memcpy( strEncryptKey , finalKey.c_str(), 32 );
	}
	else 
	{
		char r[32] = {0};

		// 开始计算key
		for( int i = 0 ; i < 32 ; )
		{
			memset( r , 0, 32 );

			if( i == 0 )
			{
				CryptoPP::Weak::MD5 hash;
				hash.CalculateDigest( (byte*)r, (byte*) strInputPass.c_str(), strInputPass.length() );
			}
			else 
			{
				//char r1[32] = {0};
				// 2016.3.10 修改, 之前的采用32个字长度, 那么如果有些人的密码很久( > 16 ) 就会使得堆栈益出.
				// 这里改为动态计算长度
				int nSecondPartLen = 16 + strInputPass.length();
				char *pSecondPart = new char[ nSecondPartLen ];

				memcpy( pSecondPart, strEncryptKey, 16 );
				memcpy( pSecondPart + 16, strInputPass.c_str(), strInputPass.length() );
				CryptoPP::Weak::MD5 hash;
				hash.CalculateDigest( (byte*)r, (byte*) pSecondPart, nSecondPartLen );
			}
			memcpy( strEncryptKey + i , r , 16 );
			i += 16;
		}
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex("Encrypt Key",strEncryptKey,nKeySize );
#endif

	return;
}
/** @brief 加密
*/
BOOL CCryptor::Encrypt( char *in, int in_len, char *out , int &out_len)
{
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex("Encrypt IV",ivEncrypt,nIVSize );
#endif
	return TRUE;
}

/** @brief 解密
*/
BOOL CCryptor::Decrypt( char *in, int in_len, char *out , int &out_len)
{
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex("Decrypt IV",ivDecrypt,nIVSize );
#endif
	return TRUE;
}
/** @brief 重置加密信息, 比如重新设置KEY, IV 
*/
void CCryptor::ResetCryption()
{
	bEncryptIVSent = false;
	bDecryptIVGot = false;
}

//////////////////////////////////////////////////////////////////////////
// aes crypt

CAESCfbCryptor :: CAESCfbCryptor ( string key ,int bits ) 
	: CCryptor( bits == 128 ? AES_128_CFB : bits == 192 ? AES_192_CFB : AES_256_CFB,
	key,
	bits == 128 ? 16 : bits == 192 ? 24 : 32, 
	16 )
{

}

CAESCfbCryptor::~CAESCfbCryptor()
{

}

/** @brief 加密
*/
BOOL CAESCfbCryptor::Encrypt( char *in, int in_len, char *out , int &out_len)
{
	//memcpy(in , "asasGET / HTTP/1.1.", strlen("asasGET / HTTP/1.1."));
	//in_len = strlen( "asasGET / HTTP/1.1.");

	// http://shadowsocks.org/en/spec/protocol.html
	/** 
	The first request sent from client-side must contains the IV it generated and used for the encryption.

	+-------+----------+
	|  IV   | Payload  |
	+-------+----------+
	| Fixed | Variable |
	+-------+----------+
	第一次发包, 包头带有加密的IV
	之后就不需要了.
	*/
	if( bEncryptIVSent )
	{
		out_len = in_len;

//		CFB_Mode<AES>::Encryption cfbEncryption( (const byte*)strEncryptKey, nKeySize, (const byte*)ivEncrypt);
//		cfbEncryption.ProcessData((byte *)out, (const byte*)in, in_len );
		AesCfbEncryption.ProcessData((byte *)out, (const byte*)in, in_len );
	}
	else 
	{
		bEncryptIVSent = true;

		// FOR TEST
		//char ivEncryptTmp[16] = {0x07, 0xe6, 0x2c, 0xe1, 0xc8, 0x8b, 0x87, 0xeb, 0x80, 0x95, 0x16,0x96, 0xc8, 0x46, 0x3c, 0x2f};
		//memcpy( ivEncrypt, ivEncryptTmp, 16 );

		out_len = in_len + nIVSize;
		memcpy( out, ivEncrypt, nIVSize );

		//CFB_Mode<AES>::Encryption cfbEncryption( (const byte*)strEncryptKey, nKeySize,(const byte*) ivEncrypt);
		//cfbEncryption.ProcessData((byte *)out + 16 , (const byte*)in, in_len );

		// 之前用临时变量, 相当于每次都重置了IV, 这样的话, 服务器只能解出发过去的的第一个包,之后的解不出.
		AesCfbEncryption.SetKeyWithIV( (const byte *) strEncryptKey, nKeySize, ( const byte *) ivEncrypt );
		AesCfbEncryption.ProcessData((byte *)out + nIVSize , (const byte*)in, in_len );
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Encrypt AESCfb PlainText",in , in_len );
	PrintfHex( "Encrypt AESCfb Cipher",out, out_len );
#endif
	return __super::Encrypt(in, in_len, out, out_len );
}

/** @brief 解密
*/
BOOL CAESCfbCryptor::Decrypt( char *in, int in_len, char *out , int &out_len)
{
	// FOR TEST
	//memcpy( ivDecrypt, ivEncrypt, 16 );
	//bDecryptIVGot = true;

	if( bDecryptIVGot )
	{
		out_len = in_len;
			
		//////////////////////////////////////////////////////////////////////////
		// Encrypt
		//CFB_Mode<AES>::Decryption cfbDecryption( (const byte*)strEncryptKey, nKeySize, (const byte*)ivDecrypt);
		//cfbDecryption.ProcessData((byte *)out,(const byte*) in, in_len );
		AesCfbDecryption.ProcessData((byte *)out,(const byte*) in, in_len );
	}
	else 
	{
		bDecryptIVGot = true;

		out_len = in_len - nIVSize;
		memcpy( ivDecrypt, in, nIVSize );

		//////////////////////////////////////////////////////////////////////////
		// Encrypt
		//CFB_Mode<AES>::Decryption cfbDecryption( (const byte*)strEncryptKey, nKeySize, (const byte*)ivDecrypt);
		//cfbDecryption.ProcessData((byte *)out,(const byte*) in + 16, in_len - 16 );

		// 之前用临时变量, 相当于每次都重置了IV, 这样的话, 只能解出服务器发来的第一个包,之后的解不出.
		AesCfbDecryption.SetKeyWithIV( (const byte *)strEncryptKey, nKeySize, (const byte *) ivDecrypt );
		AesCfbDecryption.ProcessData((byte *)out,(const byte*) in + nIVSize, out_len );

	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Decrypt AESCfb Cipher",in, in_len );
	PrintfHex( "Decrypt AESCfb PlainText",out , out_len );
#endif
	return __super::Decrypt(in, in_len, out, out_len );
}

//////////////////////////////////////////////////////////////////////////
// rc4 , rc4-md5
CRC4Cryptor::CRC4Cryptor(string key ,bool bRC4Md5 /** 是否为rc4-md5 */ )
	: CCryptor( bRC4Md5? RC4_MD5 : RC4 ,key, 16, bRC4Md5 ? 16: 0 )
{
	bIsRC4Md5Cipher = bRC4Md5;
	//memset( szDecryptKey, 0 ,32 );
	memcpy( szDecryptKey , strEncryptKey, 32 );
	memcpy( szOriginalKey , strEncryptKey, 32 );
	
	// 如果是rc4-md5
	/*if( !bIsRC4Md5Cipher )
	{
		// rc4不用发送和接收iv
		bDecryptIVGot = true;
		bEncryptIVSent = true;
	}
	*/
}

CRC4Cryptor::~CRC4Cryptor()
{

}

/** @brief 加密
*/
BOOL CRC4Cryptor::Encrypt( char *in, int in_len, char *out , int &out_len)
{
	if( bEncryptIVSent )
	{
		out_len = in_len;

		//CryptoPP::Weak1::ARC4::Encryption rc4((const byte*)strEncryptKey, nKeySize);
		// Encrypt
		//rc4.ProcessString(inOut, inOutSize);
		//rc4.ProcessData( (byte*)out, (const byte*)in, in_len );
		rc4Encryption.ProcessData( (byte*)out, (const byte*)in, in_len );
	}
	else 
	{
		bEncryptIVSent = true;

		out_len = in_len + nIVSize;

		if( nIVSize > 0 )
		{
			memcpy( out, ivEncrypt, nIVSize );
			InitializeRC4Key( TRUE );
		}

		//CryptoPP::Weak1::ARC4::Encryption rc4((const byte*)strEncryptKey, nKeySize);
		// Encrypt
		//rc4.ProcessString(inOut, inOutSize);
		//rc4.ProcessData( (byte*)out + nIVSize, (const byte*) in, in_len );
		rc4Encryption.SetKey( (const byte *) strEncryptKey, nKeySize );
		rc4Encryption.ProcessData( (byte*)out + nIVSize, (const byte*) in, in_len );
	}

#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Encrypt RC4 PlainText",in , in_len );
	PrintfHex( "Encrypt RC4 Cipher",out, out_len );
#endif
	return __super::Encrypt(in, in_len, out, out_len );
}
/** @brief 解密
*/
BOOL CRC4Cryptor::Decrypt( char *in, int in_len, char *out , int &out_len)
{
	if( bDecryptIVGot )
	{
		out_len = in_len ;

		//CryptoPP::Weak1::ARC4::Decryption rc4( (const byte*)szDecryptKey, nKeySize );
		//rc4.ProcessData( (byte*)out, (const byte*) in, in_len );

		rc4Decryption.ProcessData( (byte*)out, (const byte*) in, in_len );
	}
	else 
	{
		bDecryptIVGot = true;

		out_len = in_len - nIVSize;

		if( nIVSize > 0 )
		{
			memcpy( ivDecrypt, in, nIVSize );

			InitializeRC4Key( FALSE );
		}

		//CryptoPP::Weak1::ARC4::Decryption rc4((const byte*) szDecryptKey, nKeySize );
		//rc4.ProcessData( (byte*)out,  (const byte*)in + nIVSize , in_len );
		rc4Decryption.SetKey( (const byte*) szDecryptKey, nKeySize );
		rc4Decryption.ProcessData( (byte*)out,  (const byte*)in + nIVSize , out_len );
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Decrypt RC4 Cipher",in, in_len );
	PrintfHex( "Decrypt RC4 PlainText",out , out_len );
#endif
	return __super::Decrypt(in, in_len, out, out_len );
}

void CRC4Cryptor::GenerateRC4Md5Key( char *outkey ,char *inkey, char *iv )
{
	char buffer[32] = {0};
	memcpy( buffer, inkey, 16 );
	memcpy( buffer + 16, iv, 16 );

	CryptoPP::Weak::MD5 hash;
	hash.CalculateDigest( (byte*)outkey, (byte*) buffer, 32 );
}

void CRC4Cryptor::InitializeRC4Key( BOOL bIsEncKey  /* TRUE: Enc Key, FALSE: Des Key */ )
{
	// 如果是rc4-md5
	if( bIsRC4Md5Cipher )
	{
		if( bIsEncKey )
		{
			GenerateRC4Md5Key( strEncryptKey, szOriginalKey, ivEncrypt );

#if defined OUTPU_ENCRYPTION && defined _DEBUG
			PrintfHex( "RC4 True Encrypt Key",strEncryptKey, nKeySize );
#endif
		}
		else 
		{
			GenerateRC4Md5Key( szDecryptKey,szOriginalKey, ivDecrypt );

#if defined OUTPU_ENCRYPTION && defined _DEBUG
			PrintfHex( "RC4 True Decrypt Key",szDecryptKey, nKeySize );
#endif
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// salsa20
CSalsa20Criptor::CSalsa20Criptor( string key )
	: CCryptor( SALSA20, key, 32, 8 )
{

}
/** @brief 加密
*/
BOOL CSalsa20Criptor::Encrypt( char *in, int in_len, char *out , int &out_len)
{
	if( bEncryptIVSent )
	{
		out_len = in_len;

		//Salsa20::Encryption salsa;	
		//salsa.SetKeyWithIV( (const byte*)strEncryptKey, 32,  (const byte*)ivEncrypt );
		//salsa.ProcessData( (byte *)out, (const byte*)in, in_len);

		salsaEncryption.ProcessData( (byte *)out, (const byte*)in, in_len);
	}
	else
	{
		out_len = in_len + nIVSize;

		memcpy( out, ivEncrypt, nIVSize );

		//Salsa20::Encryption salsa;	
		//salsa.SetKeyWithIV((const byte*) strEncryptKey, 32,  (const byte*)ivEncrypt );
		//salsa.ProcessData( (byte *)out + nIVSize, (const byte*)in, in_len);

		salsaEncryption.SetKeyWithIV((const byte*) strEncryptKey, nKeySize,  (const byte*)ivEncrypt );
		salsaEncryption.ProcessData( (byte *)out + nIVSize, (const byte*)in, in_len);

		bEncryptIVSent = true;
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Encrypt Salsa20 PlainText",in , in_len );
	PrintfHex( "Encrypt Salsa20 Cipher",out, out_len );
#endif
	return __super::Encrypt(in, in_len, out, out_len );
}

/** @brief 解密
*/
BOOL CSalsa20Criptor::Decrypt( char *in, int in_len, char *out , int &out_len)
{
	if( bDecryptIVGot )
	{
		out_len = in_len;

		//Salsa20::Decryption salsa;
		//salsa.SetKeyWithIV( (const byte*)strEncryptKey, 32,  (const byte*)ivDecrypt );
		//salsa.ProcessData( (byte *)out , (const byte*)in, in_len);

		salsaDecryption.ProcessData( (byte *)out , (const byte*)in , in_len);
	}
	else
	{
		out_len = in_len - nIVSize;

		memcpy( ivDecrypt, in, nIVSize );

		//Salsa20::Decryption salsa;
		//salsa.SetKeyWithIV( (const byte*)strEncryptKey, 32,  (const byte*)ivDecrypt );
		//salsa.ProcessData( (byte *)out , (const byte*)in + nIVSize , in_len);

		salsaDecryption.SetKeyWithIV( (const byte*)strEncryptKey, nKeySize,  (const byte*)ivDecrypt );
		salsaDecryption.ProcessData( (byte *)out , (const byte*)in + nIVSize , out_len);

		bDecryptIVGot = true;
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Decrypt Salsa20 Cipher",in, in_len );
	PrintfHex( "Decrypt Salsa20 PlainText",out , out_len );
#endif
	return __super::Decrypt(in, in_len, out, out_len );
}

//////////////////////////////////////////////////////////////////////////
// table
CTableCryptor::CTableCryptor( string key )
	: CCryptor( TABLE_ENC, key, 32, 0 )
{

}

/** @brief 加密
*/
BOOL CTableCryptor::Encrypt( char *in, int in_len, char *out , int &out_len)
{
	return __super::Encrypt(in, in_len, out, out_len );
}

/** @brief 解密
*/
BOOL CTableCryptor::Decrypt( char *in, int in_len, char *out , int &out_len)
{
	return __super::Decrypt(in, in_len, out, out_len );
}

//////////////////////////////////////////////////////////////////////////
// chacha20
CChaCha20Criptor::CChaCha20Criptor( string key)
	:CCryptor( CHACHA20 , key, 32, 8 )
{
	uEnCounter = 0;
	uDeCounter = 0;
}

/** @brief 加密
*/
BOOL CChaCha20Criptor::Encrypt( char *in, int in_len, char *out , int &out_len)
{
	if( bEncryptIVSent )
	{
		int padding = uEnCounter % SODIUM_BLOCK_SIZE;
		char *tempbuff =  NULL;
		char *tempout = NULL;
		int temp_in_len =0;

		temp_in_len = in_len + padding;

		tempbuff = new char [ temp_in_len ];
		tempout = new char[ temp_in_len ];

		memset( tempbuff, 0, temp_in_len );
		memset( tempout, 0, temp_in_len );

		memcpy( tempbuff + padding, in, in_len  );

		crypto_stream_chacha20_xor_ic( (unsigned char *)tempout,(unsigned char *)tempbuff, temp_in_len, (unsigned char *)ivEncrypt,uEnCounter / SODIUM_BLOCK_SIZE, (unsigned char *)strEncryptKey );

		uEnCounter += in_len;

		memcpy( out, tempout + padding , in_len );

		out_len = in_len;

		delete []tempbuff;
		delete []tempout;
	}
	else 
	{
		bEncryptIVSent = true;

		int padding = uEnCounter % SODIUM_BLOCK_SIZE;
		char *tempbuff =  NULL;
		char *tempout = NULL;
		int temp_in_len =0;

		temp_in_len = in_len + padding;

		tempbuff = new char [ temp_in_len ];
		tempout = new char[ temp_in_len ];

		memset( tempbuff, 0, temp_in_len );
		memset( tempout, 0, temp_in_len );

		memcpy( tempbuff + padding, in, in_len  );

		crypto_stream_chacha20_xor_ic( (unsigned char *)tempout,(unsigned char *)tempbuff, temp_in_len, (unsigned char *)ivEncrypt,uEnCounter / SODIUM_BLOCK_SIZE, (unsigned char *)strEncryptKey );

		uEnCounter += in_len;

		memcpy( out, ivEncrypt, nIVSize );
		memcpy( out + nIVSize , tempout + padding , in_len );

		out_len = in_len + nIVSize;

		delete []tempbuff;
		delete []tempout;
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Encrypt Chacha20 PlainText",in , in_len );
	PrintfHex( "Encrypt Chacha20 Cipher",out, out_len );
#endif
	return __super::Encrypt(in, in_len, out, out_len );
}

/** @brief 解密
*/
BOOL CChaCha20Criptor::Decrypt( char *in, int in_len, char *out , int &out_len)
{
	if( bDecryptIVGot )
	{
		int padding = uDeCounter % SODIUM_BLOCK_SIZE;
		char *tempbuff =  NULL;
		char *tempout = NULL;
		int temp_in_len =0;

		out_len = in_len ;

		temp_in_len = in_len + padding;

		tempbuff = new char [ temp_in_len ];
		tempout = new char[ temp_in_len ];

		memset( tempbuff, 0, temp_in_len );
		memset( tempout, 0, temp_in_len );

		memcpy( tempbuff + padding, in , in_len  );

		crypto_stream_chacha20_xor_ic( (unsigned char *)tempout,(unsigned char *)tempbuff, temp_in_len, (unsigned char *)ivDecrypt,uDeCounter / SODIUM_BLOCK_SIZE, (unsigned char *)strEncryptKey );

		uDeCounter += in_len;

		memcpy( out, tempout + padding , in_len );
		
		delete []tempbuff;
		delete []tempout;
	}
	else 
	{
		bDecryptIVGot = true;

		int padding = uDeCounter % SODIUM_BLOCK_SIZE;
		char *tempbuff =  NULL;
		char *tempout = NULL;
		int temp_in_len =0;
		int real_len = 0;

		memcpy( ivDecrypt, in, nIVSize );

		out_len  = real_len  = ( in_len - nIVSize );

		temp_in_len = real_len  + padding;
		
		tempbuff = new char [ temp_in_len ];
		tempout = new char[ temp_in_len ];

		memset( tempbuff, 0 ,temp_in_len );
		memset( tempout, 0, temp_in_len );

		memcpy( tempbuff + padding, in + nIVSize, real_len );

		crypto_stream_chacha20_xor_ic( (unsigned char *)tempout,(unsigned char *)tempbuff, temp_in_len, (unsigned char *)ivDecrypt,uDeCounter / SODIUM_BLOCK_SIZE, (unsigned char *)strEncryptKey );

		uDeCounter += real_len;

		memcpy( out , tempout + padding , real_len );

		delete []tempbuff;
		delete []tempout;
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Decrypt Chacha20 Cipher",in, in_len );
	PrintfHex( "Decrypt Chacha20 PlainText",out , out_len );
#endif
	return __super::Decrypt(in, in_len, out, out_len );
}
/** @brief 重置加密信息, 比如重新设置KEY, IV 
*/
void CChaCha20Criptor::ResetCryption()
{
	__super::ResetCryption();

	uEnCounter = 0;
	uDeCounter = 0;
}

//////////////////////////////////////////////////////////////////////////
// chacha20-ietf
CChaCha20IetfCriptor::CChaCha20IetfCriptor( string key)
	:CCryptor( CHACHA20_IETF_ENC , key, CHACHA20_IETF_ENC_KEY_SIZE, CHACHA20_IETF_ENC_IV_SIZE )
{
	uEnCounter = 0;
	uDeCounter = 0;
}

/** @brief 加密
*/
BOOL CChaCha20IetfCriptor::Encrypt( char *in, int in_len, char *out , int &out_len)
{
	if( bEncryptIVSent )
	{
		int padding = uEnCounter % SODIUM_BLOCK_SIZE;
		char *tempbuff =  NULL;
		char *tempout = NULL;
		int temp_in_len =0;

		temp_in_len = in_len + padding;

		tempbuff = new char [ temp_in_len ];
		tempout = new char[ temp_in_len ];

		memset( tempbuff, 0, temp_in_len );
		memset( tempout, 0, temp_in_len );

		memcpy( tempbuff + padding, in, in_len  );

		crypto_stream_chacha20_ietf_xor_ic( (unsigned char *)tempout,(unsigned char *)tempbuff, temp_in_len, (unsigned char *)ivEncrypt,uEnCounter / SODIUM_BLOCK_SIZE, (unsigned char *)strEncryptKey );

		uEnCounter += in_len;

		memcpy( out, tempout + padding , in_len );

		out_len = in_len;

		delete []tempbuff;
		delete []tempout;
	}
	else 
	{
		bEncryptIVSent = true;

		int padding = uEnCounter % SODIUM_BLOCK_SIZE;
		char *tempbuff =  NULL;
		char *tempout = NULL;
		int temp_in_len =0;

		temp_in_len = in_len + padding;

		tempbuff = new char [ temp_in_len ];
		tempout = new char[ temp_in_len ];

		memset( tempbuff, 0, temp_in_len );
		memset( tempout, 0, temp_in_len );

		memcpy( tempbuff + padding, in, in_len  );

		crypto_stream_chacha20_ietf_xor_ic( (unsigned char *)tempout,(unsigned char *)tempbuff, temp_in_len, (unsigned char *)ivEncrypt,uEnCounter / SODIUM_BLOCK_SIZE, (unsigned char *)strEncryptKey );

		uEnCounter += in_len;

		memcpy( out, ivEncrypt, nIVSize );
		memcpy( out + nIVSize , tempout + padding , in_len );

		out_len = in_len + nIVSize;

		delete []tempbuff;
		delete []tempout;
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Encrypt Chacha20 PlainText",in , in_len );
	PrintfHex( "Encrypt Chacha20 Cipher",out, out_len );
#endif
	return __super::Encrypt(in, in_len, out, out_len );
}

/** @brief 解密
*/
BOOL CChaCha20IetfCriptor::Decrypt( char *in, int in_len, char *out , int &out_len)
{
	if( bDecryptIVGot )
	{
		int padding = uDeCounter % SODIUM_BLOCK_SIZE;
		char *tempbuff =  NULL;
		char *tempout = NULL;
		int temp_in_len =0;

		out_len = in_len ;

		temp_in_len = in_len + padding;

		tempbuff = new char [ temp_in_len ];
		tempout = new char[ temp_in_len ];

		memset( tempbuff, 0, temp_in_len );
		memset( tempout, 0, temp_in_len );

		memcpy( tempbuff + padding, in , in_len  );

		crypto_stream_chacha20_ietf_xor_ic( (unsigned char *)tempout,(unsigned char *)tempbuff, temp_in_len, (unsigned char *)ivDecrypt,uDeCounter / SODIUM_BLOCK_SIZE, (unsigned char *)strEncryptKey );

		uDeCounter += in_len;

		memcpy( out, tempout + padding , in_len );

		delete []tempbuff;
		delete []tempout;
	}
	else 
	{
		bDecryptIVGot = true;

		int padding = uDeCounter % SODIUM_BLOCK_SIZE;
		char *tempbuff =  NULL;
		char *tempout = NULL;
		int temp_in_len =0;
		int real_len = 0;

		memcpy( ivDecrypt, in, nIVSize );

		out_len  = real_len  = ( in_len - nIVSize );

		temp_in_len = real_len  + padding;

		tempbuff = new char [ temp_in_len ];
		tempout = new char[ temp_in_len ];

		memset( tempbuff, 0 ,temp_in_len );
		memset( tempout, 0, temp_in_len );

		memcpy( tempbuff + padding, in + nIVSize, real_len );

		crypto_stream_chacha20_ietf_xor_ic( (unsigned char *)tempout,(unsigned char *)tempbuff, temp_in_len, (unsigned char *)ivDecrypt,uDeCounter / SODIUM_BLOCK_SIZE, (unsigned char *)strEncryptKey );

		uDeCounter += real_len;

		memcpy( out , tempout + padding , real_len );

		delete []tempbuff;
		delete []tempout;
	}
#if defined OUTPU_ENCRYPTION && defined _DEBUG
	PrintfHex( "Decrypt Chacha20 Cipher",in, in_len );
	PrintfHex( "Decrypt Chacha20 PlainText",out , out_len );
#endif
	return __super::Decrypt(in, in_len, out, out_len );
}
/** @brief 重置加密信息, 比如重新设置KEY, IV 
*/
void CChaCha20IetfCriptor::ResetCryption()
{
	__super::ResetCryption();

	uEnCounter = 0;
	uDeCounter = 0;
}