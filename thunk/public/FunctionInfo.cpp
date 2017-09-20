#include "stdafx.h"
//#include "../Client/stdafx.h"
#include "FunctionInfo.h"



CFunctionInfo::CFunctionInfo(void) :m_safemap(nullptr)
{
	m_safemap = new CSafeMap<int,CFunctionInfo::st_VALUE>();
}

CFunctionInfo::~CFunctionInfo(void)
{
	delete(m_safemap);
	m_safemap = nullptr;
}

void CFunctionInfo::AddInfo(int SN,char* funcName,bool async,int ArgvStringNumber,int sizeOfStruct/*�����ṹ���С*/)
{
	st_VALUE value = {funcName,async,ArgvStringNumber,sizeOfStruct};
	m_safemap->push(SN,value);
}

bool CFunctionInfo::QueryASync(int query_SN) //�Ƿ��첽,true �첽,false ͬ��
{
	st_VALUE value;
	value = m_safemap->pop(query_SN);
	bool async = value.async;
	m_safemap->push(query_SN,value);
	return async;
}

int CFunctionInfo::QueryArgvPointerNumber(int query_SN) //�����ṹ����ָ�����������:ָ��A������A��ָ��B������B......����Ϊ2
{
	st_VALUE value;
	value = m_safemap->pop(query_SN);
	int ArgvStringNumber = value.ArgvStringNumber;
	m_safemap->push(query_SN,value);
	return ArgvStringNumber;
}

int CFunctionInfo::QueryArgvStructSize(int query_SN) //�����ṹ���ܹ��೤�������������ڱ���ʱ���ṩ��
{
	st_VALUE value;
	value = m_safemap->pop(query_SN);
	int sizeOfStrcut = value.sizeOfStrcut;
	m_safemap->push(query_SN,value);
	return sizeOfStrcut;
}
/*return funcName len*/
int CFunctionInfo::QueryFuncName(int query_SN,_Out_ char* pfuncNameBuffer)
{
	st_VALUE value;
	try
	{
		value = m_safemap->pop(query_SN);
	}
	catch(...)
	{
		return 0;
	}
	char* funcName = value.funcName;
	m_safemap->push(query_SN,value);

	int str_len = strlen(funcName)+1;
	if (pfuncNameBuffer)
	{
		strcpy_s(pfuncNameBuffer,str_len,funcName);
		return str_len;
	}
	else
	{
		return str_len;
	}
}
