#pragma once

#include "SafeMap.h"

class CFunctionInfo
{
public:
	CFunctionInfo(void);
	~CFunctionInfo(void);

	void AddInfo(int SN,char* funcName,bool async,int ArgvStringNumber,int sizeOfStruct/*参数结构体大小*/);

	bool QueryASync(int query_SN);//是否异步,true 异步,false 同步
	int  QueryArgvPointerNumber(int query_SN);//参数结构体中指针个数，例如:指针A，长度A，指针B，长度B......个数为2
	int  QueryArgvStructSize(int query_SN);//参数结构体总共多长，服务器必须在编译时刻提供。
	int	 QueryFuncName(int query_SN,_Out_ char* pfuncNameBuffer=nullptr);

	struct  st_VALUE
	{
		char*	funcName;
		bool	async;
		int		ArgvStringNumber;
		int		sizeOfStrcut;
	};


private:

	//CSafeMap<LONG,st_CYSM>* m_safemap;
	CSafeMap<int,CFunctionInfo::st_VALUE>* m_safemap;

};

