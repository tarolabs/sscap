// GenericHTTPClient.cpp: implementation of the GenericHTTPClient class.
//
//////////////////////////////////////////////////////////////////////

#include "GenericHTTPClient.h"
#include <winreg.h>
#pragma comment(lib,"wininet.lib")
#pragma warning( disable:4996 )	//This function or variable may be unsafe. Consider using strcat_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

GenericHTTPClient::GenericHTTPClient(){
	_hHTTPOpen=_hHTTPConnection=_hHTTPRequest=NULL;
	m_dwTimeout = 10 * 1000;
	m_dwStatus = 0;
	m_dwRequestFlags = INTERNET_FLAG_RELOAD|INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_UI|INTERNET_FLAG_DONT_CACHE;
	m_secureFlags = INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID|SECURITY_FLAG_IGNORE_UNKNOWN_CA|SECURITY_FLAG_IGNORE_REVOCATION | INTERNET_FLAG_NO_AUTO_REDIRECT;
}

GenericHTTPClient::~GenericHTTPClient(){
	_hHTTPOpen=_hHTTPConnection=_hHTTPRequest=NULL;
}

GenericHTTPClient::RequestMethod GenericHTTPClient::GetMethod(int nMethod){	
	return nMethod<=0 ? GenericHTTPClient::RequestGetMethod : static_cast<GenericHTTPClient::RequestMethod>(nMethod);
}

GenericHTTPClient::TypePostArgument GenericHTTPClient::GetPostArgumentType(int nType){
	return nType<=0 ? GenericHTTPClient::TypeNormal : static_cast<GenericHTTPClient::TypePostArgument>(nType);
}

BOOL GenericHTTPClient::Connect(LPCSTR szAddress, LPCSTR szAgent, unsigned short nPort, LPCSTR szUserAccount, LPCSTR szPassword){

	_hHTTPOpen=::InternetOpenA(szAgent,												// agent name
												INTERNET_OPEN_TYPE_PRECONFIG,	// proxy option
												"",														// proxy
												"",												// proxy bypass
												INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_UI | INTERNET_FLAG_DONT_CACHE);					// flags

	if(!_hHTTPOpen){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}	

	if( m_dwTimeout ) {
		// Sets or retrieves an unsigned long integer value that contains the time-out value, in milliseconds, to use for Internet connection requests. Setting this option to infinite (0xFFFFFFFF) will disable this timer.
		InternetSetOption(_hHTTPOpen,INTERNET_OPTION_CONNECT_TIMEOUT,&m_dwTimeout,sizeof(DWORD) );
		InternetSetOption(_hHTTPOpen,INTERNET_OPTION_SEND_TIMEOUT ,&m_dwTimeout,sizeof(DWORD) );
		InternetSetOption(_hHTTPOpen,INTERNET_OPTION_RECEIVE_TIMEOUT ,&m_dwTimeout,sizeof(DWORD) );
	}

	_hHTTPConnection=::InternetConnectA(	_hHTTPOpen,	// internet opened handle
															szAddress, // server name
															nPort, // ports
															szUserAccount, // user name
															szPassword, // password 
															INTERNET_SERVICE_HTTP, // service type
															INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE,	// service option																														
															0);	// context call-back option

	if(!_hHTTPConnection){		
		_dwError=::GetLastError();
		::CloseHandle(_hHTTPOpen);
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	return TRUE;															
}

BOOL GenericHTTPClient::Close(){

	if(_hHTTPRequest)
		::InternetCloseHandle(_hHTTPRequest);

	if(_hHTTPConnection)
		::InternetCloseHandle(_hHTTPConnection);

	if(_hHTTPOpen)
		::InternetCloseHandle(_hHTTPOpen);

	return TRUE;
}

BOOL GenericHTTPClient::Request(LPCSTR szURL, int nMethod, LPCSTR szAgent){

	BOOL bReturn=TRUE;
	DWORD dwPort=0;
	char szProtocol[__SIZE_BUFFER]="";
	char szAddress[__SIZE_BUFFER]="";	
	char szURI[__SIZE_BUFFER]="";
	DWORD dwSize=0;

	ParseURL(szURL, szProtocol, szAddress, dwPort, szURI);

	if( dwPort == 443 )
		m_dwRequestFlags |= m_secureFlags;

	if(Connect(szAddress, szAgent, dwPort)){
		if(!RequestOfURI(szURI, nMethod)){
			bReturn=FALSE;
		}else{
			if(!Response((PBYTE)_szHTTPResponseHeader, __SIZE_HTTP_BUFFER, (PBYTE)_szHTTPResponseHTML, __SIZE_HTTP_BUFFER, dwSize)){
				bReturn=FALSE;		
			}
		}
		Close();
	}else{
		bReturn=FALSE;
	}

	return bReturn;
}

BOOL GenericHTTPClient::RequestOfURI(LPCSTR szURI, int nMethod){

	BOOL bReturn=TRUE;

	try{
		switch(nMethod){
		case	GenericHTTPClient::RequestGetMethod:
		default:
			bReturn=RequestGet(szURI);
			break;
		case	GenericHTTPClient::RequestPostMethod:		
			bReturn=RequestPost(szURI);
			break;
		case	GenericHTTPClient::RequestPostMethodMultiPartsFormData:
			bReturn=RequestPostMultiPartsFormData(szURI);
			break;
		}
	}catch(CException *e){
		e;
#ifdef	_DEBUG
		TRACE("\nEXCEPTION\n");
		TCHAR szERROR[1024];
		e->GetErrorMessage(szERROR, 1024);
		TRACE(szERROR);
#endif
	}
	

	return bReturn;
}

BOOL GenericHTTPClient::RequestGet(LPCSTR szURI){

	CONST char *szAcceptType=__HTTP_ACCEPT_TYPE;

	_hHTTPRequest=::HttpOpenRequestA(	_hHTTPConnection,
															__HTTP_VERB_GET, // HTTP Verb
															szURI, // Object Name
															HTTP_VERSIONA, // Version
															"", // Reference
															&szAcceptType, // Accept Type
															m_dwRequestFlags,
															0); // context call-back point

	if(!_hHTTPRequest){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	// REPLACE HEADER
	if(!::HttpAddRequestHeadersA( _hHTTPRequest, __HTTP_ACCEPT, strlen(__HTTP_ACCEPT), HTTP_ADDREQ_FLAG_REPLACE)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	// SEND REQUEST
	if(!::HttpSendRequest( _hHTTPRequest,	// handle by returned HttpOpenRequest
									NULL, // additional HTTP header
									0, // additional HTTP header length
									NULL, // additional data in HTTP Post or HTTP Put
									0) // additional data length
									){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}
	GetHttpCode();

	return TRUE;
}
void GenericHTTPClient::GetHttpCode()
{
	string status = GetQueryInfo(HTTP_QUERY_STATUS_CODE);
	if( !status.empty() )
	{
		m_dwStatus = atoi( (const char *)status.c_str() );
	}
	else 
		m_dwStatus = 0;
}
string GenericHTTPClient::GetQueryInfo(DWORD code)
{
	char Buffer[4097]={0};
	DWORD BufferLength = 4096;
	string strResult;

	if(HttpQueryInfoA(_hHTTPRequest,code,Buffer,&BufferLength,NULL))
	{
		Buffer[BufferLength] = 0;
		TRACE(Buffer);
		TRACE("\n");

		strResult = Buffer;
	}

	return strResult;
}
BOOL GenericHTTPClient::RequestPost(LPCSTR szURI){

	CONST char *szAcceptType=__HTTP_ACCEPT_TYPE;
	char			szPostArguments[__SIZE_BUFFER]={0};
	LPCSTR szContentType="Content-Type: application/x-www-form-urlencoded\r\n";

	DWORD dwPostArgumentsLegnth=GetPostArguments(szPostArguments, __SIZE_BUFFER);
	
	_hHTTPRequest=::HttpOpenRequestA(	_hHTTPConnection,
															__HTTP_VERB_POST, // HTTP Verb
															szURI, // Object Name
															NULL, // Version
															"", // Reference
															NULL, // Accept Type
															m_dwRequestFlags,
															0); // context call-back point

	if(!_hHTTPRequest){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE("\n%d : %s\n", _dwError, reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}
	/** 设置接收，发送超时*/
	if( m_dwTimeout )
	{
		InternetSetOption(_hHTTPRequest,INTERNET_OPTION_SEND_TIMEOUT ,&m_dwTimeout,sizeof(DWORD));
		InternetSetOption(_hHTTPRequest,INTERNET_OPTION_RECEIVE_TIMEOUT ,&m_dwTimeout,sizeof(DWORD));
	}
	// REPLACE HEADER
	if(!::HttpAddRequestHeadersA( _hHTTPRequest, szContentType, strlen(szContentType), HTTP_ADDREQ_FLAG_ADD_IF_NEW)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE("\n%d : %s\n", _dwError, reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	// SEND REQUEST
	if(!::HttpSendRequestA( _hHTTPRequest,	// handle by returned HttpOpenRequest
									NULL, // additional HTTP header
									0, // additional HTTP header length
									(LPVOID)szPostArguments, // additional data in HTTP Post or HTTP Put
									strlen(szPostArguments)) // additional data length
									){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE("\n%d : %s\n", _dwError, reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	GetHttpCode();

	return TRUE;
}

BOOL GenericHTTPClient::RequestPostMultiPartsFormData(LPCSTR szURI){

	CONST char *szAcceptType=__HTTP_ACCEPT_TYPE;	
	LPCSTR szContentType="Content-Type: multipart/form-data; boundary=--MULTI-PARTS-FORM-DATA-BOUNDARY\r\n";		
	
	// ALLOCATE POST MULTI-PARTS FORM DATA ARGUMENTS
	PBYTE pPostBuffer=NULL;
	DWORD dwPostBufferLength=AllocMultiPartsFormData(pPostBuffer, "--MULTI-PARTS-FORM-DATA-BOUNDARY");

	m_dwRequestFlags |= INTERNET_FLAG_FORMS_SUBMIT;

	_hHTTPRequest=::HttpOpenRequestA(	_hHTTPConnection,
															__HTTP_VERB_POST, // HTTP Verb
															szURI, // Object Name
															HTTP_VERSIONA, // Version
															"", // Reference
															&szAcceptType, // Accept Type
															m_dwRequestFlags,	// flags
															0); // context call-back point
	if(!_hHTTPRequest){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	// REPLACE HEADER
	if(!::HttpAddRequestHeadersA( _hHTTPRequest, __HTTP_ACCEPT, strlen(__HTTP_ACCEPT), HTTP_ADDREQ_FLAG_REPLACE)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	if(!::HttpAddRequestHeadersA( _hHTTPRequest, szContentType, strlen(szContentType), HTTP_ADDREQ_FLAG_ADD_IF_NEW)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	char	szContentLength[__SIZE_BUFFER]="";
	::ZeroMemory(szContentLength, __SIZE_BUFFER);

	sprintf( szContentLength, "Content-Length: %d\r\n", dwPostBufferLength);

	if(!::HttpAddRequestHeadersA( _hHTTPRequest, szContentLength, strlen(szContentLength), HTTP_ADDREQ_FLAG_ADD_IF_NEW)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

#ifdef	_DEBUG
	char	szHTTPRequest[__SIZE_HTTP_BUFFER]="";
	DWORD	dwHTTPRequestLength=__SIZE_HTTP_BUFFER;

	::ZeroMemory(szHTTPRequest, __SIZE_HTTP_BUFFER);
	if(!::HttpQueryInfo(_hHTTPRequest, HTTP_QUERY_RAW_HEADERS_CRLF, szHTTPRequest, &dwHTTPRequestLength, NULL)){
		_dwError=::GetLastError();
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE("\n%d : %s\n", ::GetLastError(), reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
		//return FALSE;
	}

	TRACE(szHTTPRequest);
#endif

	// SEND REQUEST WITH HttpSendRequestEx and InternetWriteFile
	INTERNET_BUFFERS InternetBufferIn={0};
	InternetBufferIn.dwStructSize=sizeof(INTERNET_BUFFERS);
	InternetBufferIn.Next=NULL;	
	
	if(!::HttpSendRequestEx(_hHTTPRequest, &InternetBufferIn, NULL, HSR_INITIATE, 0)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		TRACE(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	DWORD dwOutPostBufferLength=0;
	if(!::InternetWriteFile(_hHTTPRequest, pPostBuffer, dwPostBufferLength, &dwOutPostBufferLength)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	if(!::HttpEndRequest(_hHTTPRequest, NULL, HSR_INITIATE, 0)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}
	GetHttpCode();

	// FREE POST MULTI-PARTS FORM DATA ARGUMENTS
	FreeMultiPartsFormData(pPostBuffer);

	return TRUE;
}

DWORD GenericHTTPClient::ResponseOfBytes(PBYTE pBuffer, DWORD dwSize){

	static DWORD dwBytes=0;

	if(!_hHTTPRequest){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return 0;
	}

	::ZeroMemory(pBuffer, dwSize);
	if(!::InternetReadFile(	_hHTTPRequest,
									pBuffer,
									dwSize,
									&dwBytes)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return 0;
	}

	return dwBytes;
}

BOOL GenericHTTPClient::Response(PBYTE pHeaderBuffer, DWORD dwHeaderBufferLength, PBYTE pBuffer, DWORD dwBufferLength, DWORD &dwResultSize){

	BYTE pResponseBuffer[__SIZE_BUFFER]="";	
	DWORD dwNumOfBytesToRead=0;

	if(!_hHTTPRequest){
		_dwError=::GetLastError();
#ifdef	_DEBUG		
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	::ZeroMemory(pBuffer, dwBufferLength);
	dwResultSize=0;
	while((dwNumOfBytesToRead=ResponseOfBytes(pResponseBuffer, __SIZE_BUFFER))!=NULL && dwResultSize<dwBufferLength){
		::CopyMemory( (pBuffer+dwResultSize), pResponseBuffer, dwNumOfBytesToRead);		
		dwResultSize+=dwNumOfBytesToRead;
	}

	::ZeroMemory(pHeaderBuffer, dwHeaderBufferLength);
	if(!::HttpQueryInfo(_hHTTPRequest, HTTP_QUERY_RAW_HEADERS_CRLF, pHeaderBuffer, &dwHeaderBufferLength, NULL)){
		_dwError=::GetLastError();
#ifdef	_DEBUG
		LPVOID     lpMsgBuffer;
		DWORD dwRet=FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
														  NULL,
														  ::GetLastError(),
														  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
														  reinterpret_cast<LPSTR>(&lpMsgBuffer),
														  0,
														  NULL);
		OutputDebugStringA(reinterpret_cast<LPSTR>(lpMsgBuffer));
		LocalFree(lpMsgBuffer);		
#endif
		return FALSE;
	}

	return (dwResultSize? TRUE: FALSE);
}

VOID GenericHTTPClient::AddPostArguments(LPCSTR szName, LPCSTR szValue, BOOL bBinary){

	GenericHTTPArgument	arg;
	::ZeroMemory(&arg, sizeof(arg));

	strcpy(arg.szName, szName);
	strcpy(arg.szValue, szValue);

	if(!bBinary)
		arg.dwType = GenericHTTPClient::TypeNormal;
	else
		arg.dwType = GenericHTTPClient::TypeBinary;	

	_vArguments.push_back(arg);

	return;
}

VOID GenericHTTPClient::AddPostArguments(LPCSTR szName, DWORD nValue){

	GenericHTTPArgument	arg;
	::FillMemory(&arg, sizeof(arg), 0x00);

	strcpy(arg.szName, szName);
	sprintf(arg.szValue, "%d", nValue);
	arg.dwType = GenericHTTPClient::TypeNormal;

	_vArguments.push_back(arg);

	return;
}

DWORD GenericHTTPClient::GetPostArguments(LPSTR szArguments, DWORD dwLength){

	std::vector<GenericHTTPArgument>::iterator itArg;

	::ZeroMemory(szArguments, dwLength);
	for(itArg=_vArguments.begin(); itArg<_vArguments.end(); ){
		strcat(szArguments, itArg->szName);
		strcat(szArguments, "=");
		strcat(szArguments, itArg->szValue);

		if((++itArg)<_vArguments.end()){
			strcat(szArguments, "&");
		}
	}

	//*(szArguments+dwLength)='\0';

	return strlen(szArguments);
}


VOID GenericHTTPClient::InitilizePostArguments(){
	_vArguments.clear();
	return;
}

VOID GenericHTTPClient::FreeMultiPartsFormData(PBYTE &pBuffer){

	if(pBuffer){
		::LocalFree(pBuffer);
		pBuffer=NULL;
	}

	return;
}

DWORD GenericHTTPClient::AllocMultiPartsFormData(PBYTE &pInBuffer, LPCSTR szBoundary){

	if(pInBuffer){
		::LocalFree(pInBuffer);
		pInBuffer=NULL;
	}

	pInBuffer=(PBYTE)::LocalAlloc(LPTR, GetMultiPartsFormDataLength());	
	std::vector<GenericHTTPArgument>::iterator itArgv;

	DWORD dwPosition=0;
	DWORD dwBufferSize=0;

	for(itArgv=_vArguments.begin(); itArgv<_vArguments.end(); ++itArgv){

		PBYTE pBuffer=NULL;

		// SET MULTI_PRATS FORM DATA BUFFER
		switch(itArgv->dwType){
		case	GenericHTTPClient::TypeNormal:
			pBuffer=(PBYTE)::LocalAlloc(LPTR, __SIZE_HTTP_HEAD_LINE*4);

			sprintf(	(char*)pBuffer,							
							"--%s\r\n"
							"Content-Disposition: form-data; name=\"%s\"\r\n"
							"\r\n"
							"%s\r\n",
							szBoundary,
							itArgv->szName,
							itArgv->szValue);

			dwBufferSize=strlen((char*)pBuffer);

			break;
		case	GenericHTTPClient::TypeBinary:
			DWORD	dwNumOfBytesToRead=0;
			DWORD	dwTotalBytes=0;

			HANDLE hFile=::CreateFileA(itArgv->szValue, 
									GENERIC_READ, // desired access
									FILE_SHARE_READ, // share mode
									NULL, // security attribute
									OPEN_EXISTING, // create disposition
									FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, // flag and attributes
									NULL); // template file handle

			DWORD	dwSize=::GetFileSize(hFile, NULL);

			pBuffer=(PBYTE)::LocalAlloc(LPTR, __SIZE_HTTP_HEAD_LINE*3+dwSize+1);
			BYTE	pBytes[__SIZE_BUFFER+1]="";
			
			sprintf( (char*)pBuffer,
							"--%s\r\n"
							"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
							"Content-Type: %s\r\n"
							"\r\n",
							szBoundary,
							itArgv->szName, itArgv->szValue,
							GetContentType(itArgv->szValue)
							);

			DWORD	dwBufPosition=strlen((char*)pBuffer);	
	
			while(::ReadFile(hFile, pBytes, __SIZE_BUFFER, &dwNumOfBytesToRead, NULL) && dwNumOfBytesToRead>0 && dwTotalBytes<=dwSize){
				::CopyMemory((pBuffer+dwBufPosition+dwTotalBytes), pBytes, dwNumOfBytesToRead);
				::ZeroMemory(pBytes, __SIZE_BUFFER+1);
				dwTotalBytes+=dwNumOfBytesToRead;				
			}

			::CopyMemory((pBuffer+dwBufPosition+dwTotalBytes), "\r\n", strlen("\r\n"));
			::CloseHandle(hFile);

			dwBufferSize=dwBufPosition+dwTotalBytes+strlen("\r\n");
			break;			
		}

		::CopyMemory((pInBuffer+dwPosition), pBuffer, dwBufferSize);
		dwPosition+=dwBufferSize;

		if(pBuffer){
			::LocalFree(pBuffer);
			pBuffer=NULL;
		}
	}

	::CopyMemory((pInBuffer+dwPosition), "--", 2);
	::CopyMemory((pInBuffer+dwPosition+2), szBoundary, strlen(szBoundary));
	::CopyMemory((pInBuffer+dwPosition+2+strlen(szBoundary)), "--\r\n", 3);

	return dwPosition+5+strlen(szBoundary);
}

DWORD GenericHTTPClient::GetMultiPartsFormDataLength(){
	std::vector<GenericHTTPArgument>::iterator itArgv;

	DWORD	dwLength=0;

	for(itArgv=_vArguments.begin(); itArgv<_vArguments.end(); ++itArgv){

		switch(itArgv->dwType){
		case	GenericHTTPClient::TypeNormal:
			dwLength+=__SIZE_HTTP_HEAD_LINE*4;
			break;
		case	GenericHTTPClient::TypeBinary:
			HANDLE hFile=::CreateFileA(itArgv->szValue, 
									GENERIC_READ, // desired access
									FILE_SHARE_READ, // share mode
									NULL, // security attribute
									OPEN_EXISTING, // create disposition
									FILE_ATTRIBUTE_NORMAL, // flag and attributes
									NULL); // template file handle

			dwLength+=__SIZE_HTTP_HEAD_LINE*3+::GetFileSize(hFile, NULL);
			::CloseHandle(hFile);			
			break;
		}

	}

	return dwLength;
}

LPCSTR GenericHTTPClient::GetContentType(LPCSTR szName){

	static char	szReturn[1024]="";
	LONG	dwLen=1024;
	DWORD	dwDot=0;

	for(dwDot=strlen(szName);dwDot>0;dwDot--){
		if(!strncmp((szName+dwDot),".", 1))
			break;
	}

	HKEY	hKEY;
	LPSTR	szWord=(char*)(szName+dwDot);

	if(RegOpenKeyExA(HKEY_CLASSES_ROOT, szWord, 0, KEY_QUERY_VALUE, &hKEY)==ERROR_SUCCESS){
		if(RegQueryValueExA(hKEY, "Content Type", NULL, NULL, (LPBYTE)szReturn, (unsigned long*)&dwLen)!=ERROR_SUCCESS)
			strncpy(szReturn, "application/octet-stream", strlen("application/octet-stream"));
		RegCloseKey(hKEY);
	}else{
		strncpy(szReturn, "application/octet-stream", strlen("application/octet-stream"));
	}

	return szReturn;
}

DWORD GenericHTTPClient::GetLastError(){

	return _dwError;
}

VOID GenericHTTPClient::ParseURL(LPCSTR szURL, LPSTR szProtocol, LPSTR szAddress, DWORD &dwPort, LPSTR szURI){

	char szPort[__SIZE_BUFFER]="";
	DWORD dwPosition=0;
	BOOL bFlag=FALSE;

	while(strlen(szURL)>0 && dwPosition<strlen(szURL) && strncmp((szURL+dwPosition), ":", 1))
		++dwPosition;

	if(!strncmp((szURL+dwPosition+1), "/", 1)){	// is PROTOCOL
		if(szProtocol){
			strncpy(szProtocol, szURL, dwPosition);
			szProtocol[dwPosition]=0;
		}
		bFlag=TRUE;
	}else{	// is HOST 
		if(szProtocol){
			strncpy(szProtocol, "http", 4);
			szProtocol[5]=0;
		}
	}

	DWORD dwStartPosition=0;
	
	if(bFlag){
		dwStartPosition=dwPosition+=3;				
	}else{
		dwStartPosition=dwPosition=0;
	}

	bFlag=FALSE;
	while(strlen(szURL)>0 && dwPosition<strlen(szURL) && strncmp((szURL+dwPosition), "/", 1))
			++dwPosition;

	DWORD dwFind=dwStartPosition;

	for(;dwFind<=dwPosition;dwFind++){
		if(!strncmp((szURL+dwFind), ":", 1)){ // find PORT
			bFlag=TRUE;
			break;
		}
	}

	if(bFlag){
		char sztmp[__SIZE_BUFFER]="";
		strncpy(sztmp, (szURL+dwFind+1), dwPosition-dwFind);
		dwPort=atol(sztmp);
		strncpy(szAddress, (szURL+dwStartPosition), dwFind-dwStartPosition);
	}else if(!strcmp(szProtocol,"https")){
		dwPort=INTERNET_DEFAULT_HTTPS_PORT;
		strncpy(szAddress, (szURL+dwStartPosition), dwPosition-dwStartPosition);
	}else {
		dwPort=INTERNET_DEFAULT_HTTP_PORT;
		strncpy(szAddress, (szURL+dwStartPosition), dwPosition-dwStartPosition);
	}

	if(dwPosition<strlen(szURL)){ // find URI
		strncpy(szURI, (szURL+dwPosition), strlen(szURL)-dwPosition);
	}else{
		szURI[0]=0;
	}

	return;
}

LPCSTR GenericHTTPClient::QueryHTTPResponseHeader(){
	return _szHTTPResponseHeader;
}

LPCSTR GenericHTTPClient::QueryHTTPResponse(){
	return _szHTTPResponseHTML;
}


