// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
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
			��ʼ��functionInfo
			g_CI_Client->AddInfo();
			*/
			int ArgvPointerNumber = 0;

			g_CI_Client->AddInfo(1,"Add",false,ArgvPointerNumber,sizeof(st_argv_Add));
			g_CI_Client->AddInfo(2,"Add_Async_NoCallback",true,ArgvPointerNumber,sizeof(st_argv_Add));

			
			
			//pCWEB->Recive_Data()

			
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

