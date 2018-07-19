// testEverythin.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>



DWORD WINAPI x(LPVOID  p)
{

	HANDLE dwGood = (HANDLE)p;

	CloseHandle(dwGood);
	
	return 0;
}

//事件不能遗弃

void _tmain(LONG64 argc, _TCHAR* argv[])
{




	HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);
	CreateThread(NULL,0,x,(LPVOID)hdEvent,0,NULL);



	DWORD y = WaitForSingleObject(hdEvent,INFINITE);
	CloseHandle(hdEvent);

	return;
}

