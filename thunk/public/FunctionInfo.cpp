#include "stdafx.h"
//#include "../Client/stdafx.h"
#include "FunctionInfo.h"

HMODULE hServiceDLL = NULL;
const char* dllServiceName = "ServiceDLL.dll";

CFunctionInfo::CFunctionInfo(void) :m_safemap(nullptr)
{
	m_safemap = new CSafeMap<LONG64,CFunctionInfo::st_VALUE>();
	
	hServiceDLL = LoadLibraryA(dllServiceName);
}

CFunctionInfo::~CFunctionInfo(void)
{
	delete(m_safemap);
	m_safemap = nullptr;
	
	FreeLibrary(hServiceDLL);
	hServiceDLL = NULL;
}
//sizeOfStruct/*�����ṹ���С*/
void CFunctionInfo::AddInfo(LONG64 SN,char* funcName,bool async,LONG64 ArgvStringNumber,LONG64 sizeOfStruct/*�����ṹ���С*/)
{
	char* m_funcName = nullptr;
	if (funcName!=nullptr)
	{
		LONG64 NameLen = strlen(funcName);
		if (NameLen==0)
		{
			m_funcName = nullptr;
		}
		else
		{
			m_funcName = new char[NameLen+1]();
			LONG64 stat = memcpy_s(m_funcName,NameLen,funcName,NameLen);
			if (stat)
			{
				throw("memcpy_s return err.");
			}
		}
	}


	st_VALUE value = {m_funcName,async,ArgvStringNumber,sizeOfStruct};
	m_safemap->push(SN,value);
}

bool CFunctionInfo::QueryASync(LONG64 query_SN) //�Ƿ��첽,true �첽,false ͬ��
{
	st_VALUE value;
	value = m_safemap->pop(query_SN);
	bool async = value.async;
	m_safemap->push(query_SN,value);
	return async;
}

LONG64 CFunctionInfo::QueryArgvPointerNumber(LONG64 query_SN) //�����ṹ����ָ�����������:ָ��A������A��ָ��B������B......����Ϊ2
{
	st_VALUE value;
	value = m_safemap->pop(query_SN);
	LONG64 ArgvStringNumber = value.ArgvStringNumber;
	m_safemap->push(query_SN,value);
	return ArgvStringNumber;
}

LONG64 CFunctionInfo::QueryArgvStructSize(LONG64 query_SN) //�����ṹ���ܹ��೤�������������ڱ���ʱ���ṩ��
{
	st_VALUE value;
	value = m_safemap->pop(query_SN);
	LONG64 sizeOfStrcut = value.sizeOfStrcut;
	m_safemap->push(query_SN,value);
	return sizeOfStrcut;
}
/*return funcName len*/
LONG64 CFunctionInfo::QueryFuncName(LONG64 query_SN,_Out_ char* pfuncNameBuffer)
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

	LONG64 str_len = strlen(funcName)+1;
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
