// testEverythin.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>



DWORD WINAPI x(LPVOID  p)
{

	HANDLE dwGood = (HANDLE)p;

	CloseHandle(dwGood);
	
	return 0;
}

//�¼���������

void _tmain(LONG64 argc, _TCHAR* argv[])
{




	HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);
	CreateThread(NULL,0,x,(LPVOID)hdEvent,0,NULL);



	DWORD y = WaitForSingleObject(hdEvent,INFINITE);
	CloseHandle(hdEvent);

	return;
}

