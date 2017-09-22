// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include "../public/FunctionInfo.h"


struct st_argv_Add
{
	int firstNumber;
	int secondNumber;
};

extern CFunctionInfo* g_CI_Client;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			/*
			初始化functionInfo
			g_CI_Client->AddInfo();
			*/

			const int funcNumber = 1;
			const int ArgvPointerNumber = 0;

			g_CI_Client->AddInfo(funcNumber,"Add",false,ArgvPointerNumber,sizeof(st_argv_Add));
			


			
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

