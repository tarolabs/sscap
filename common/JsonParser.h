#pragma once
/** \brief
* 封装的用于解析json结构的类
* 需要使用到的json数据的类继承自此类
*
* 用法:
* 1 . 继承自此类的子类先要实现虚函数:printValueTree 用于解析自己的数据
* 
* \code
* 
	CItemCatsParser<CXPtrList *> parser;
	parser.Parse(string(strResponse.GetBuffer()));
	strResponse.ReleaseBuffer();

	return parser.GetResult();
*
*/
#include <string>
#include <json/json.h>
#include <algorithm> // sort
#include "Debug.h"
//#include "../lib.h"

#define BASICLIB_API

using namespace debug;
using namespace std;


class BASICLIB_API CJsonParser
{
public:
	CJsonParser(void)
	{
		nDisplaied= 0;
	}
	~CJsonParser(void)
	{

	}

	/** 解析json格式的数据
	*
	* \returns
	* 0 失败
	* 1 成功
	*/
	BOOL Parse(const string &input)
	{
		Json::Reader reader;
		Json::Value root;

		nDisplaied = 0;
		bool parsingSuccessful = reader.parse( input, root );
		if ( !parsingSuccessful )
		{
			PrintfA(LEVEL_ERROR,"Failed to parse input string <%s>",reader.getFormatedErrorMessages().c_str());

			return FALSE;
		}

		printValueTree( root );
		if( nDisplaied > 100 )
			PrintfA(LEVEL_KERNEL,"要打印的Json Tree太大,已被忽略输出");
	
		//Json::Value rst = root.get(string("rsp"),string("nofound"));

		return ParseValueTree( root );

//		return 1;
	}

	/** 解析JSON树*/
	virtual BOOL ParseValueTree(Json::Value &value) =0;
	/** 获取解析的结果*/
	//virtual _Type GetResult() = 0;
protected:

	void printValueTree(Json::Value &value, const std::string &path = "." )
	{
		if( nDisplaied > 100 )
			return;

		nDisplaied ++;
		switch ( value.type() )
		{
		case Json::nullValue:
			PrintfA(LEVEL_KERNEL,"%s=null", path.c_str() );
			break;
		case Json::intValue:
			PrintfA(LEVEL_KERNEL,"%s=%d", path.c_str(), value.asInt() );
			break;
		case Json::uintValue:
			PrintfA(LEVEL_KERNEL,"%s=%u", path.c_str(), value.asUInt() );
			break;
		case Json::realValue:
			PrintfA(LEVEL_KERNEL, "%s=%.16g", path.c_str(), value.asDouble() );
			break;
		case Json::stringValue:
			{
				char *pGb2312 = conv_utf82ascii_1((char *) value.asString().c_str());
				if(pGb2312)
				{
					PrintfA(LEVEL_KERNEL, "%s=\"%s\"", path.c_str(), pGb2312);
					delete pGb2312;
					pGb2312 = NULL;
				}
			}
			break;
		case Json::booleanValue:
			PrintfA(LEVEL_KERNEL,"%s=%s", path.c_str(), value.asBool() ? "true" : "false" );
			break;
		case Json::arrayValue:
			{
				PrintfA(LEVEL_KERNEL,"%s=[]", path.c_str() );
				int size = value.size();
				for ( int index =0; index < size; ++index )
				{
					static char buffer[16];
					sprintf_s( buffer , 16 , "[%d]", index );
					//printValueTree( value[index], path );
					printValueTree( value[index], path + buffer );
				}
			}
			break;
		case Json::objectValue:
			{
				PrintfA(LEVEL_KERNEL,"%s={}", path.c_str() );
				Json::Value::Members members( value.getMemberNames() );
				std::sort( members.begin(), members.end() );
				std::string suffix = *(path.end()-1) == '.' ? "" : ".";
				for ( Json::Value::Members::iterator it = members.begin(); 
					it != members.end(); 
					++it )
				{
					const std::string &name = *it;
					printValueTree( value[name], path + suffix + name );
					//printValueTree( value[name], name );
				}
			}
			break;
		default:
			break;
		}
	}
protected:
	/***********************************************************************
	UTF8->ASCII
	org：需要转换成ascii的原字符串
	return：转换成ascii后的字符串，需要调用者自己释放内存
	***********************************************************************/
	char*  conv_utf82ascii_1(char *org)
	{
		int len1=0,len2,l;
		wchar_t *buffer=NULL;

		//	char *out=NULL;

		if(org==NULL)
			return NULL;

		len2=(int)strlen(org);

		len1=MultiByteToWideChar(CP_UTF8,0,org,len2,NULL,0);

		buffer=new wchar_t [len1+2];
		if(!buffer)
			return NULL;

		memset( buffer, 0, sizeof( wchar_t ) * (len1 + 2) );
		//wcscpy(buffer,L"");
		l=MultiByteToWideChar(CP_UTF8,0,org,len2,buffer,len1);

		l=WideCharToMultiByte(CP_ACP,0,buffer,len1,NULL,0,NULL,NULL);
		char *pOut = new char[l+2];
		if(pOut)
		{
			l=WideCharToMultiByte(CP_ACP,0,buffer,len1,pOut,l,NULL,NULL);
			if (l>=0)
				pOut[l]=0;
		}

		delete [] buffer;
		return pOut;
	}
//	_Type nResult;
protected:
	int nDisplaied;
private:
};

