// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include "../public/FunctionInfo.h"


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
			LONG64 ArgvPointerNumber = 0;

			g_CI_Client->AddInfo(1,"Add",false,ArgvPointerNumber,sizeof(st_argv_Add));
			g_CI_Client->AddInfo(2,"Add_Async_NoCallback",true,ArgvPointerNumber,sizeof(st_argv_Add));
			g_CI_Client->AddInfo(3,"Test2_Sync",false,2,sizeof(st_argv_test2));
			g_CI_Client->AddInfo(4,"RealMessageBoxA",false,2,sizeof(st_argv_MessageBoxA));
			
			//pCWEB->Recive_Data()

			
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

