#if(!defined(__NEURO__DNS__CLIENT__))
#define __NEURO__DNS__CLIENT__

//#include "SocketClient.h"
//#include "NeuroBuffer.h"
#include <afxtempl.h>
#include <WinSock2.h>

/** default dns port is 53 */
#define DEFAULT_DNS_PORT 53

#define _DNS_LIB_
/************************************************************************
		 
		 Name   : CNeuroBuffer
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Bytes Buffer to store, allocate, reallocate etc.
		          Append etc
************************************************************************/

class _DNS_LIB_ CNeuroBuffer
{
protected:

	BYTE *	m_pBuffer;
	int		m_nAllocatedSize;
	int		m_nSize;
	int		m_nGrowBy;
	int		m_nPointer;

	void UnsafeClear();
public:

	CNeuroBuffer();
	CNeuroBuffer(BYTE * pBuffer,int nSize);
	CNeuroBuffer(CNeuroBuffer & Buffer);
	virtual ~CNeuroBuffer();
	int GetSize();
	BYTE * GetBuffer();
	BYTE * GetBufferSetLength(int nSize);
	BOOL Allocate(int nSize);
	BOOL Append(BYTE * pBuffer,int nSize);
	int Copy(BYTE * pBuffer,int nSize);
	void Clear();
	void operator = (CNeuroBuffer & Buffer)
	{
		//UnsafeClear();
		Clear();
		Allocate(Buffer.GetSize());
		Copy(Buffer.GetBuffer(),Buffer.GetSize());
	}

	void operator += (CNeuroBuffer & Buffer)
	{
		Append(Buffer.GetBuffer(),Buffer.GetSize());
	}

	CString Detach();
};

/***********************************************************************
LICENSE AGREEMENT AND POLICY

The following code is provided on AS IS basis, no claim should be made
if the code does any damage, the user using and modifying this code will 
be responsible for any kind of damange this code produces.

You can freely distribute, use this code as long as you insert this 
Agreement with each distribution.

The code should not be sold at any cases.

-Akash Kava
ackava@hotmail.com
************************************************************************/


/************************************************************************
		 
		 Name   : CDnsBuffer
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Dns Results come in a binary format, length followed
		          by the data buffer. Complete data buffer is needed
		          to search Compressed Domain Names in buffer
		          
		          Dns protocol uses pointers in response to represent
		          repeated data, by expanding this pointers you will
		          get full names
************************************************************************/
class _DNS_LIB_ CDnsBuffer : public CNeuroBuffer {
public:


	// Buffer Pointer Management
	void Reset();
	int GetPointer();
	int Reset(int n);
	void operator = (CDnsBuffer & Buffer)
	{
		UnsafeClear();
		Allocate(Buffer.GetSize());
		Copy(Buffer.GetBuffer(),Buffer.GetSize());
	}


	// Appending Buffer Functions
	void operator << (unsigned __int8 c)
	{
		Append((BYTE *)&c,1);
	}

	void operator << (unsigned __int16 c)
	{
		c = htons(c);
		Append((BYTE *)&c,2);
	}

	void operator << (unsigned __int32 c)
	{
		c = htonl(c);
		Append((BYTE*)&c,4);
	}



	// Extracting Buffer Data, pointer moves ahead
	// Throws DNS Exception if pointer out of range
	void operator >> (unsigned __int8 & c)
	{
		Get(&c,1);
	}

	void operator >> (unsigned __int16 & c)
	{
		Get(&c,2);
		c = ntohs(c);
	}

	void operator >> (unsigned __int32 & c)
	{
		Get(&c,4);
		c = ntohl(c);
	}
	void operator ++()
	{
		m_nPointer ++;
	}

	void operator += (int n)
	{
		m_nPointer += n;
	}

	BOOL Get(LPVOID Buffer,int size);
	void operator += (CDnsBuffer & Buffer)
	{
		Append(Buffer.GetBuffer(),Buffer.GetSize());
	}

	// DNS Name Extraction from buffer , Message Pointer Expansion
	// Call this member function with nPointer=0 while reading buffer
	// to get domain name.

	// This function calls same function recursively to extract message
	// from pointers and also sets pointer to next resource data to be read

	// This function is carefully designed and implemented, its tested 
	// perfectly, use it without modifying single line
	virtual CString GetName(int nPointer);
	// 单独写个函数来获取Question里边的Domain Name, 这里边的Domain Name不会有名称的地址偏移
	virtual CString GetQestionName(int nPointer);
};

/************************************************************************
		 
		 Name   : DnsHeaderFlags
		 Type   : Union
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: DNS Header Flags, its 16 bit value
		          Every bit field is explained below
************************************************************************/
union DnsHeaderFlags{
	unsigned __int16 Flag;
	/* 
	 |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
	  1       4       1  1 1  1   3         4
	 */
	struct{
		unsigned int RCODE	: 4 ;
		unsigned int Z		: 3 ;
		unsigned int RA		: 1 ;
		unsigned int RD		: 1 ;
		unsigned int TC		: 1 ;
		unsigned int AA		: 1 ;
		unsigned int OPCODE	: 4 ;
		unsigned int QR		: 1 ;
	}Flags;
};
/************************************************************************
		 
		 Name   : CDnsHeader
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: DNS Header Class which holds initial
		          header information

				  The pack and unpack method interacts with CDnsBuffer
				  and exchanges data according to protocol in every class
				  defined below

				  pack and unpack methods moves buffer pointer as well,
				  make sure you never write function which passes copy of
				  buffer rather then reference of buffer, always pass
				  reference of buffer to maintain integrity
************************************************************************/
class _DNS_LIB_ CDnsHeader{
public:
	unsigned __int16	ID;
	DnsHeaderFlags		Flags;
	unsigned __int16	QDCount; // 请求段中的问题记录数。
	unsigned __int16	ANCount; // 回答段中的回答记录数。
	unsigned __int16	NSCount; // 授权段中的授权记录数。
	unsigned __int16	ARCount; // 附加段中的附加记录数。

	void operator = ( CDnsHeader & Header )
	{
		ID = Header.ID;
		Flags.Flag = Header.Flags.Flag;
		QDCount = Header.QDCount;
		ANCount = Header.ANCount;
		NSCount = Header.NSCount;
		ARCount = Header.ARCount;
	}

	CDnsHeader();
	CDnsBuffer Pack();
	void Unpack( CDnsBuffer & Buffer );
	CString ToString();
};
/************************************************************************
		 
		 Name   : CDnsRR
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: DNS Response's Resource Record
		          This class holds Name,Type,Class,TTL and 
		          Length of RData, RData is Type dependent
		          which then appears in every derived class
		          from CDnsRR
		          
		          The pack and unpack method interacts with CDnsBuffer 
		          and exchanges data according to protocol in every class
		          defined below.

				  Please note, pack and unpack methods of all derived 
				  classes of CDnsRR does not call pack and unpack method
				  of CDnsRR anytime, its your responsibility to first
				  make object of CDnsRR and then process it with buffer,
				  make type dependent class of Resource Record and then
				  process it again for only RData.

				  Such complex method is designed only to get the 
				  compressed domain names in RData section of Resource
				  Record
************************************************************************/
class _DNS_LIB_ CDnsRR{
public:
	CString				Name;
	unsigned __int16	Type;
	unsigned __int16	Class;
	unsigned __int32	TTL;
	unsigned __int16	Length;

	CDnsRR(){}
	CDnsRR(CDnsRR * pRR);
	virtual ~CDnsRR(){}

	CString GetString(int Number);
	virtual void Unpack(CDnsBuffer & Buffer);
	virtual CString ToString();
	virtual void CopyRData(CDnsRR* pRR){
	};
};
/************************************************************************
		 
		 Name   : CDnsRRDefault
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: All Resource Record types are not programmed here
		          thats why This class serves as substitute for non-implemented
		          class types.
************************************************************************/

class _DNS_LIB_ CDnsRRDefault: public CDnsRR{
public:
	BYTE *			pRData;

	CDnsRRDefault();
	CDnsRRDefault(CDnsRR * pRR);
	virtual ~CDnsRRDefault();

	virtual void Unpack(CDnsBuffer & Buffer);

	virtual void CopyRData(CDnsRR* pRR);
};
/************************************************************************
		 
		 Name   : CDnsRRMX
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Most widely used DNS class type, for mail exchange
************************************************************************/
class _DNS_LIB_ CDnsRRMX : public CDnsRR {
public:
	unsigned __int16	Preference;
	CString				Exchange;

	CDnsRRMX(CDnsRR * pRR) : CDnsRR(pRR){}
	virtual ~CDnsRRMX(){}

	void Unpack(CDnsBuffer & Buffer);
	virtual CString ToString();
	virtual void CopyRData(CDnsRR* pRR);
};
/************************************************************************
		 
		 Name   : CDnsRRCNAME
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Encapsulates Resource Record for Type CNAME
************************************************************************/
class _DNS_LIB_ CDnsRRCNAME : public CDnsRR {
public:
	CString Name;
	CDnsRRCNAME(CDnsRR * pRR): CDnsRR(pRR){}
	virtual ~CDnsRRCNAME(){}

	void Unpack(CDnsBuffer & Buffer);
	virtual CString ToString();
	virtual void CopyRData(CDnsRR* pRR);
};

/************************************************************************
		 
		 Name   : CDnsRRA
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Encapsulates Resource Record for Type A
		          CString A  of this class holds text
		          representation of IP address like 127.0.0.1
************************************************************************/
class _DNS_LIB_ CDnsRRA : public CDnsRR {
public:
	CString A;
	CDnsRRA(CDnsRR * pRR): CDnsRR(pRR){}
	virtual ~CDnsRRA(){}

	void Unpack(CDnsBuffer & Buffer);
	virtual CString ToString();
	BOOL operator == (LPCTSTR lpszName) const
	{
		return (Name.CompareNoCase(lpszName)==0);
	}

	virtual void CopyRData(CDnsRR* pRR);
};

/************************************************************************
		 
		 Name   : CDnsRRNS
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Encapsulates Resource Record for Type NS
		          CString NS  of this class holds domain
		          name of Authorized Name Server 
************************************************************************/
class _DNS_LIB_ CDnsRRNS : public CDnsRR {
public:
	CString NS;
	CDnsRRNS( CDnsRR * pRR ): CDnsRR(pRR){}
	virtual ~CDnsRRNS(){}

	void Unpack(CDnsBuffer & Buffer);
	virtual CString ToString();
	virtual void CopyRData(CDnsRR* pRR);
	
};

/************************************************************************
		 
		 Name   : CDnsQueryType
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Returns int type number from string representation of type
		          Also returns new Resource Record Class Pointer
		          
		          This static function makes new object of specified type
		          and also copies contents of CDnsRR to type dependent
		          resource record object
************************************************************************/
class _DNS_LIB_ CDnsQueryType{
public:
	static int GetType( CString Name );
	static CDnsRR * GetNewRecord(CDnsRR * pRR);
};
/************************************************************************
		 
		 Name   : CDnsQuery
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Dns Query Class
************************************************************************/
class _DNS_LIB_ CDnsQuery{
public:
	CStringList			Names;
	unsigned __int16	Type;
	unsigned __int16	Class;

	CDnsQuery( LPCTSTR lpszHost , LPCTSTR lpszType );
	~CDnsQuery();
	CDnsBuffer Pack();
	void Unpack( CDnsBuffer & Buffer );
};
/************************************************************************
		 
		 Name   : CDnsRRPtrList
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: All Resource Records are hold in a CList class which holds 
		          pointers of all objects. And also implements CHostSearch 
		          and returns IP address found in A records if any

				  Users who use the record pointer are requested to keep
				  copy of Resource Record according to type because when
				  This clas gets destroys it deletes its pointers !!
************************************************************************/
class _DNS_LIB_ CDnsRRPtrList :public  CList<CDnsRR*,CDnsRR*>{
public:
	~CDnsRRPtrList ();
};
/************************************************************************
		 
		 Name   : CDnsClient
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: The final DnsClient class
		          
		          It can be used in 2 ways.
		          
		          1) Directly Query MX records and CStringList by calling 
		          static member function. No hastle of pointer management.
		          
		          2) Call another static function Get to get result of type 
				  requested The CDnsRRPtrList supplied gets all required 
				  resource records. It searches recursivly of found names 
				  servers so you dont need to worry about nameservers 
				  returned in query. What you will get is the final result 
				  after 10 tries. Or else domain does not exist.
		              The list returned has pointers to CDnsRR derived class
		          depending upon type. While using it, please copy the 
				  relevant content and then processing because everything 
				  will get destroyed as soon as CDnsRRPtrList goes out of 
				  scope.
		          
		          2) Create Object of class and query, you will need to do all 
		          recursive nameserver processing etc so just forget it.
************************************************************************/
class _DNS_LIB_ CDnsClient
{
public:

	CString				Log;
	//CSocketClient		Client;
	CDnsBuffer			Buffer;

	int					QueryType;
	CDnsHeader			Header;
	CDnsRRPtrList		Questions;
	CDnsRRPtrList		Answers;
	CDnsRRPtrList		Authorities;
	CDnsRRPtrList		Additionals;

	
	virtual BOOL Query(LPCTSTR lpszServer, LPCTSTR lpszHost, LPCTSTR lpszType);
	virtual void GetRecords( CDnsRRPtrList & List , int Total );
	/************************************************************************
			 FUNCTION
			 Name   : Get
			 Author : Akash Kava
			 Purpose: Call this function to retrive answers of query, this function
					  does recursive processing upto specified number of trials
					  
					  While calling this function, please pass the Reference parameters
					  as explained
	========================================================================
			 Params : Server	: Name of the name server to connect
					  Name		: Domain name to query
					  Type		: Type of query, case insensitive, MX or mx
					  Answers	: Resource Record Object List , please pass empty 
									List here, it will return all answers
					  Additional: Resource Record Object List, please pass empty
									List here , it will pass additional A record entries
					  NSTried	: Please pass empty CStringList here
					  Log		: Please pass empty CString here
					  nTries	: Make sure you pass atleast 5 trials
	------------------------------------------------------------------------
			 Returns: True if successful , false if failed
	************************************************************************/
	virtual BOOL Get(
					LPCTSTR lpszServer,		
					LPCTSTR lpszName,		
					LPCTSTR lpszType,		
				CString&	Log,		
					int&	nTries		
					);
	// whooooooo baby, the most important , MX search
	// Just searches the MX of domain, returns IP address only,
	// however you can use Get function to get detailed MX records
	// CStringList receives all host addresses for MX
	virtual CString GetMX(LPCTSTR lpszServer,LPCTSTR lpszHost,CStringList & List);
	virtual CString GetA(LPCTSTR lpszServer,LPCTSTR lpszHost,CStringList & List, int nTries = 2 );
	virtual void SearchARecords( CString & Domain , CStringList & IPList);
};

class _DNS_LIB_ CDnsUDPClient : public CDnsClient 
{
public:
	virtual BOOL Query(LPCTSTR lpszServer, LPCTSTR lpszHost, LPCTSTR lpszType);
};
#endif