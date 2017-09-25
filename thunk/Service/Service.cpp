// Service.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Service.h"




struct st_argv_Add
{
	int firstNumber;
	int secondNumber;
};


/*
1. 伪造一些函数数据到FunctionInfo中
2. 启动WEB

*/
void Start()
{

	const int funcNumber = 1;
	const int ArgvPointerNumber = 0;

	g_CI_Service->AddInfo(funcNumber,"Add",false,ArgvPointerNumber,sizeof(st_argv_Add));
	
	


}

void End()
{
	delete(pCWEB);
	delete(g_CI_Service);
	pCWEB = nullptr;
	g_CI_Service = nullptr;
}


int _tmain(int argc, _TCHAR* argv[])
{
	Start();
	
	SYSTEMTIME sysTime;
	GetLocalTime( &sysTime );
	printf("%02d:%02d:%02d.%03d",sysTime.wHour, sysTime.wMinute, sysTime.wSecond,sysTime.wMilliseconds);

	printf("Service running...Input any key exit.");

	getchar();
	
	End();
	printf("Service exiting.");
	return 0;
}

