// Service.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "../public/FunctionInfo.h"
#include "../public/SafeMap.h"

#include <windows.h>


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

//服务器函数信息
CFunctionInfo* g_CI_Service = new CFunctionInfo();

void Start()
{
	
}


int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}

