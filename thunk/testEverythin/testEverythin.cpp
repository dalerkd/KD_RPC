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

int _tmain(int argc, _TCHAR* argv[])
{




	HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);
	CreateThread(NULL,0,x,(LPVOID)hdEvent,0,NULL);



	DWORD y = WaitForSingleObject(hdEvent,INFINITE);
	CloseHandle(hdEvent);

	return 0;
}

