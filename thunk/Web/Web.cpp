// Web.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "TempWeb.h"
#include <process.h>

int _tmain(int argc, _TCHAR* argv[])
{

	CTempWeb* p = new CTempWeb();
	
	_beginthreadex(NULL, 0, CTempWeb::ThreadReceive, NULL, 0, NULL);
	
	_beginthreadex(NULL, 0, CTempWeb::TrySend, NULL, 0, NULL);

	Sleep(-1);

	return 0;
}

