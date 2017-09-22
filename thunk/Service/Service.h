#pragma once

#include "../public/FunctionInfo.h"
#include "../public/SafeMap.h"
#include "../public/WEB.h"
#include "WEB_Service.h"
#include "Data_Format_Service.h"

#include <windows.h>



CData_Format_Service g_CDF;
CWEB* pCWEB =new CWEB_Service();


//服务器函数信息
CFunctionInfo* g_CI_Service = new CFunctionInfo();



//////////////////////////////////////////////////////////////////////////
//用于伪回调函数需要知道它的ID_proc,function_ID
struct st_Async_Thread_Callback 
{
	LONG ID_proc;
	int function_ID;
};

CSafeMap<DWORD,st_Async_Thread_Callback>* g_Async_Thread_Callback = 
	new CSafeMap<DWORD,st_Async_Thread_Callback>();
//////////////////////////////////////////////////////////////////////////