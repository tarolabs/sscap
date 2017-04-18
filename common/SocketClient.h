#if (!defined(__NEURO__SOCKET__CLIENT__))
#define __NEURO__SOCKET__CLIENT__

#include "SocketEx.h"
/************************************************************************
		 
		 Name   : CSocketClient
		 Type   : Class
------------------------------------------------------------------------
		 Author : Akash Kava
		 Purpose: Client socket derived from CSocketEx and introduces 
		          Exception to rise when Data is unavailable or timedout

				  CSocketEx does not throw any exception at all.
************************************************************************/

#define _DNS_LIB_

class _DNS_LIB_ CSocketClient : public CSocketEx
{
public:

	CSocketClient()
		: CSocketEx()
	{
		Create(0,NULL,SOCK_STREAM );
	}

	void Close();

	BOOL ConnectTo(LPCTSTR lpszHost , int nPort);
	BOOL ReadInt8( unsigned __int8 &d );

	BOOL ReadInt16( unsigned __int16 &d);

	BOOL ReadInt32( unsigned __int32  &d );

	BOOL WriteInt8(unsigned __int8 data);

	BOOL WriteInt16(unsigned __int16 data);

	BOOL WriteInt32(unsigned __int32 data);

	BOOL WriteBytes (void * Buffer, int Length);

	int ReadBytes (void * Buffer , int Length);
};

class _DNS_LIB_ CUDPSocketClient : public CSocketEx
{
public:

	CUDPSocketClient();

	void Close();

	BOOL ConnectTo(LPCTSTR lpszHost , int nPort);

	BOOL WriteBytes ( void * Buffer, int Length );

	int ReadBytes (void * Buffer , int Length);

};
#endif