/*
 * $ Generic HTTP Client
 * ----------------------------------------------------------------------------------------------------------------
 *
 * name :          GenericHTTPClient
 *
 * version tag :     0.1.0
 *
 * description :    HTTP Client using WININET
 *
 * author :          Heo Yongsun ( gooshin@opentown.net )
 *
 * This code distributed under BSD LICENSE STYLE.
 */

#ifndef __GENERIC_HTTP_CLIENT
#define __GENERIC_HTTP_CLIENT

#include <afxwin.h>
#include <tchar.h>
#include <wininet.h>

// use stl
#include <vector>
#include <string>
using namespace std;
// PRE-DEFINED CONSTANTS
#define __DEFAULT_AGENT_NAME	"SocksCap64 (1.0/;p)"

// PRE-DEFINED BUFFER SIZE
#define	__SIZE_HTTP_ARGUMENT_NAME	256
#define __SIZE_HTTP_ARGUMENT_VALUE	1024

#define __HTTP_VERB_GET	"GET"
#define __HTTP_VERB_POST "POST"
#define __HTTP_ACCEPT_TYPE "*/*"
#define __HTTP_ACCEPT "Accept: */*\r\n"
#define __SIZE_HTTP_BUFFER	100000
#define __SIZE_HTTP_RESPONSE_BUFFER	100000
#define __SIZE_HTTP_HEAD_LINE	2048

#define __SIZE_BUFFER	1024
#define __SIZE_SMALL_BUFFER	256

class GenericHTTPClient {
public:					
	typedef struct __GENERIC_HTTP_ARGUMENT{							// ARGUMENTS STRUCTURE
		char	szName[__SIZE_HTTP_ARGUMENT_NAME];
		char szValue[__SIZE_HTTP_ARGUMENT_VALUE];
		DWORD	dwType;
		bool operator==(const __GENERIC_HTTP_ARGUMENT &argV){
			return !strcmp(szName, argV.szName) && !strcmp(szValue, argV.szValue);
		}
	} GenericHTTPArgument;

	enum RequestMethod{															// REQUEST METHOD
		RequestUnknown=0,
		RequestGetMethod=1,
		RequestPostMethod=2,
		RequestPostMethodMultiPartsFormData=3
	};

	enum TypePostArgument{													// POST TYPE 
		TypeUnknown=0,
		TypeNormal=1,
		TypeBinary=2
	};

	// CONSTRUCTOR & DESTRUCTOR
	GenericHTTPClient();
	virtual ~GenericHTTPClient();

	static GenericHTTPClient::RequestMethod GetMethod(int nMethod);
	static GenericHTTPClient::TypePostArgument GetPostArgumentType(int nType);

	// Connection handler	
	BOOL Connect(LPCSTR szAddress, LPCSTR szAgent = __DEFAULT_AGENT_NAME, unsigned short nPort = INTERNET_DEFAULT_HTTP_PORT, LPCSTR szUserAccount = NULL, LPCSTR szPassword = NULL);
	BOOL Close();
	VOID InitilizePostArguments();

	// HTTP Arguments handler	
	VOID AddPostArguments(LPCSTR szName, DWORD nValue);
	VOID AddPostArguments(LPCSTR szName, LPCSTR szValue, BOOL bBinary = FALSE);

	// HTTP Method handler 
	BOOL Request(LPCSTR szURL, int nMethod = GenericHTTPClient::RequestGetMethod, LPCSTR szAgent = __DEFAULT_AGENT_NAME);
	BOOL RequestOfURI(LPCSTR szURI, int nMethod = GenericHTTPClient::RequestGetMethod);
	BOOL Response(PBYTE pHeaderBuffer, DWORD dwHeaderBufferLength, PBYTE pBuffer, DWORD dwBufferLength, DWORD &dwResultSize);	
	LPCSTR QueryHTTPResponse();
	LPCSTR QueryHTTPResponseHeader();	

	// General Handler
	DWORD GetLastError();
	LPCSTR GetContentType(LPCSTR szName);
	VOID ParseURL(LPCSTR szURL, LPSTR szProtocol, LPSTR szAddress, DWORD &dwPort, LPSTR szURI);
	void SetTimeout( DWORD dwTimeout ){
		m_dwTimeout = dwTimeout;
	}
	DWORD GetStatusCode(){
		return m_dwStatus;
	}
	bool Is404(){
		return m_dwStatus == 404? true: false;
	}
	bool Is200(){
		return m_dwStatus == 200? true: false;
	}
	
protected:	
	DWORD   m_dwRequestFlags;
	DWORD	m_secureFlags;
	std::vector<GenericHTTPArgument> _vArguments;				// POST ARGUMENTS VECTOR

	char		_szHTTPResponseHTML[__SIZE_HTTP_BUFFER];		// RECEIVE HTTP BODY
	char		_szHTTPResponseHeader[__SIZE_HTTP_BUFFER];	// RECEIVE HTTP HEADR

	HINTERNET _hHTTPOpen;				// internet open handle
	HINTERNET _hHTTPConnection;		// internet connection hadle
	HINTERNET _hHTTPRequest;		// internet request hadle

	DWORD		_dwError;					// LAST ERROR CODE
	LPCSTR		_szHost;					 //	 HOST NAME
	DWORD		_dwPort;					//  PORT
	DWORD		m_dwTimeout;
	DWORD		m_dwStatus;	//http status code
	// HTTP Method handler
	void GetHttpCode();
	DWORD ResponseOfBytes(PBYTE pBuffer, DWORD dwSize);
	DWORD GetPostArguments(LPSTR szArguments, DWORD dwLength);
	BOOL RequestPost(LPCSTR szURI);
	BOOL RequestPostMultiPartsFormData(LPCSTR szURI);
	BOOL RequestGet(LPCSTR szURI);
	DWORD AllocMultiPartsFormData(PBYTE &pInBuffer, LPCSTR szBoundary = "--MULTI-PARTS-FORM-DATA-BOUNDARY-");
	VOID FreeMultiPartsFormData(PBYTE &pBuffer);
	DWORD GetMultiPartsFormDataLength();
	string GetQueryInfo(DWORD code);
};

#endif	// #ifndef __GENERIC_HTTP_CLIENT
