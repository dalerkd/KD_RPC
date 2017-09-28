// testEverythin.cpp : 定义控制台应用程序的入口点。
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

