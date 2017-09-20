#pragma once

#include "SafeMap.h"

class CFunctionInfo
{
public:
	CFunctionInfo(void);
	~CFunctionInfo(void);

	void AddInfo(int SN,char* funcName,bool async,int ArgvStringNumber,int sizeOfStruct/*�����ṹ���С*/);

	bool QueryASync(int query_SN);//�Ƿ��첽,true �첽,false ͬ��
	int  QueryArgvPointerNumber(int query_SN);//�����ṹ����ָ�����������:ָ��A������A��ָ��B������B......����Ϊ2
	int  QueryArgvStructSize(int query_SN);//�����ṹ���ܹ��೤�������������ڱ���ʱ���ṩ��
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

