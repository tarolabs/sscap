#include "stdheader.h"
#include "Encypter.h"
#include "EncyptionMgr.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// libsodium 初始化失败的话, 就禁用chacha20
bool CEncryptionMgr::bChaCha20Disabled = false;

__cryption_list _crp_list_[] = 
{
	{ IDX_AES_256_CFB, AES_256_CFB	},
	{ IDX_AES_192_CFB, AES_192_CFB	},
	{ IDX_AES_128_CFB, AES_128_CFB	},
	{ IDX_RC4_MD5, RC4_MD5	},
	{ IDX_RC4, RC4	},
	{ IDX_SALSA20, SALSA20	},
	{ IDX_CHACHA20, CHACHA20	},
	{ IDX_CHACHA20_IETF_ENC, CHACHA20_IETF_ENC	},
	{ IDX_TABLE_ENC, TABLE_ENC	},
	{ -1, "" },
};

CEncryptionMgr::CEncryptionMgr()
{
	bChaCha20Disabled = false;
}

CEncryptionMgr::~CEncryptionMgr()
{

}

CCryptor *CEncryptionMgr::Create( string method, string key )
{
	// aes cfb不管128/192/256的iv都是16字节
	if( method == AES_256_CFB )
		return new CAESCfbCryptor(key, 256 );
	else if( method == AES_192_CFB )
		return new CAESCfbCryptor(key, 192 );
	else if( method == AES_128_CFB )
		return new CAESCfbCryptor(key, 128 );
	else if( method == RC4_MD5 )
		return new CRC4Cryptor( key, true );
	else if( method == RC4 )
		return new CRC4Cryptor( key, false );
	else if( method == SALSA20 )
		return new CSalsa20Criptor( key );
	else if( method == CHACHA20 && !bChaCha20Disabled )
		return new CChaCha20Criptor( key );
	else if( method == CHACHA20_IETF_ENC && !bChaCha20Disabled )
		return new CChaCha20IetfCriptor( key );

	return NULL;
}

int CEncryptionMgr::GetEncyptionList( vector<string> &enc )
{
	int idx = 0;

	while( _crp_list_[idx].idx != -1 )
	{
		if( _crp_list_[idx].idx == IDX_TABLE_ENC )
		{
			idx ++;
			continue;
		}

		// chacha20模块初始化失败,就跳过chacha20/chacha20-ietf加解密模块
		if( CEncryptionMgr::bChaCha20Disabled
			&& ( _crp_list_[idx].idx == IDX_CHACHA20 || _crp_list_[idx].idx == IDX_CHACHA20_IETF_ENC)
			)
		{
			idx ++;
			continue;
		}

		enc.push_back( _crp_list_[idx].name );

		idx ++;
	}

	return enc.size();
}

/** @brief 通过加密名称获取加密的IDEX
* 
* @return 返回加密名称对应的INDEX,未找以返回-1
*/
int CEncryptionMgr::GetCryptionIndexByName( string name )
{
	int idx = 0;

	while( _crp_list_[idx].idx != -1 )
	{
		if( _crp_list_[idx].name == name )
			return _crp_list_[idx].idx;

		idx ++;
	}

	return -1;
}