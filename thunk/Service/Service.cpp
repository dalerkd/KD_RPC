// Service.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "../public/FunctionInfo.h"
#include "../public/SafeMap.h"
#include "../public/WEB.h"
#include "WEB_Service.h"
#include "Data_Format_Service.h"

#include <windows.h>


CData_Format_Service g_CDF;
CWEB* pCWEB =new CWEB_Service();
//////////////////////////////////////////////////////////////////////////
//����α�ص�������Ҫ֪������ID_proc,function_ID
struct st_Async_Thread_Callback 
{
	LONG ID_proc;
	int function_ID;
};

CSafeMap<DWORD,st_Async_Thread_Callback>* g_Async_Thread_Callback = 
	new CSafeMap<DWORD,st_Async_Thread_Callback>();
//////////////////////////////////////////////////////////////////////////

//������������Ϣ
CFunctionInfo* g_CI_Service = new CFunctionInfo();

void Start()
{
	
}


int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}

