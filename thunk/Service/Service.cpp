// Service.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Service.h"




struct st_argv_Add
{
	LONG64 firstNumber;
	LONG64 secondNumber;
};
struct st_argv_test2
{
	char* firstStr;
	LONG64   firstStr_len;
	char* secondStr;
	LONG64   secondStr_len;
	char  other_argv_c;
	float f_f;
};

struct st_argv_MessageBoxA 
{
	_In_opt_ LPCSTR lpText;
	LONG64 lpText_len;
	_In_opt_ LPCSTR lpCaption;
	LONG64 lpCaption_len;

	_In_opt_ HWND hWnd;
	_In_ UINT uType;

};




/*
1. 伪造一些函数数据到FunctionInfo中
2. 启动WEB

*/
void Start()
{
	LONG64 ArgvPointerNumber = 0;

	g_CI_Service->AddInfo(1,"Add",false,ArgvPointerNumber,sizeof(st_argv_Add));
	g_CI_Service->AddInfo(2,"Add_Async_NoCallback",true,ArgvPointerNumber,sizeof(st_argv_Add));
	g_CI_Service->AddInfo(3,"Test2_Sync",false,2,sizeof(st_argv_test2));
	g_CI_Service->AddInfo(4,"RealMessageBoxA",false,2,sizeof(st_argv_MessageBoxA));
}

void End()
{
	delete(pCWEB);
	delete(g_CI_Service);
	pCWEB = nullptr;
	g_CI_Service = nullptr;
}


void printNowTime()
{
	SYSTEMTIME sysTime;
	GetLocalTime( &sysTime );
	printf("Now Time:%02d:%02d:%02d.%03d\r\n",sysTime.wHour, sysTime.wMinute, sysTime.wSecond,sysTime.wMilliseconds);

}

void _tmain(LONG64 argc, _TCHAR* argv[])
{
	Start();
	
	printNowTime();
	printf("Service running...Input any key exit!\r\n");

	getchar();
	
	End();

	printNowTime();
	printf("Service exiting.\r\n");
	

	return;
}

