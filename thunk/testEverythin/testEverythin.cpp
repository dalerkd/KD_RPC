// testEverythin.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include "SafeQueue.h"
#include "../public/debug.h"



int _tmain(int argc, _TCHAR* argv[])
{
	CSafeQueueAutoPointerManage* p =new CSafeQueueAutoPointerManage();
	delete(p);

	OutputDebug("Hi Good morning!");





	return 0;
}

