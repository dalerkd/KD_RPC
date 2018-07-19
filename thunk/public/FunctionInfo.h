#pragma once

#include "SafeMap.h"

class CFunctionInfo
{
public:
	CFunctionInfo(void);
	~CFunctionInfo(void);

	void AddInfo(LONG64 SN,char* funcName,bool async,LONG64 ArgvStringNumber,LONG64 sizeOfStruct/*�����ṹ���С*/);

	bool QueryASync(LONG64 query_SN);//�Ƿ��첽,true �첽,false ͬ��
	LONG64  QueryArgvPointerNumber(LONG64 query_SN);//�����ṹ����ָ�����������:ָ��A������A��ָ��B������B......����Ϊ2
	LONG64  QueryArgvStructSize(LONG64 query_SN);//�����ṹ���ܹ��೤�������������ڱ���ʱ���ṩ��
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

