// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include "../public/FunctionInfo.h"


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
			char*strName = "Add";
			

			g_CI_Client->AddInfo(0,,);
			


			
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

