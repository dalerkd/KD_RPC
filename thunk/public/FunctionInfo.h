#pragma once

#include "SafeMap.h"

class CFunctionInfo
{
public:
	CFunctionInfo(void);
	~CFunctionInfo(void);

	void AddInfo(LONG64 SN,char* funcName,bool async,LONG64 ArgvStringNumber,LONG64 sizeOfStruct/*参数结构体大小*/);

	bool QueryASync(LONG64 query_SN);//是否异步,true 异步,false 同步
	LONG64  QueryArgvPointerNumber(LONG64 query_SN);//参数结构体中指针个数，例如:指针A，长度A，指针B，长度B......个数为2
	LONG64  QueryArgvStructSize(LONG64 query_SN);//参数结构体总共多长，服务器必须在编译时刻提供。
	LONG64	 QueryFuncName(LONG64 query_SN,_Out_ char* pfuncNameBuffer=nullptr);

	struct  st_VALUE
	{
		char*	funcName;
		bool	async;
		LONG64		ArgvStringNumber;
		LONG64		sizeOfStrcut;
	};


private:

	//CSafeMap<LONG,st_CYSM>* m_safemap;
	CSafeMap<LONG64,CFunctionInfo::st_VALUE>* m_safemap;

};

