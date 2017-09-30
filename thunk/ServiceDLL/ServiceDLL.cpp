// ServiceDLL.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ServiceDLL.h"


// 这是导出变量的一个示例
SERVICEDLL_API int nServiceDLL=0;

// 这是导出函数的一个示例。
SERVICEDLL_API int fnServiceDLL(void)
{
	return 42;
}

int Add(st_argv_Add* p,char* cb)
{
	if (cb!=nullptr)
	{
		OutputDebug("err:Add cb!=nullptr");
	}

	return p->firstNumber+p->secondNumber;
}

int Add_Async_NoCallback(st_argv_Add* p,RPC_CallBack cb)
{
	if (cb!=nullptr)
	{
		char str[]={'r','e','s','u','l','t','i','s',':','1','1','\0'};
		cb(str,strlen(str)+1);
	}
	else
	{
		OutputDebug("Test3:Pass:\r\n");
	}
	/*char str[10] ={0};
	sprintf_s(str,"%d",p->firstNumber+p->secondNumber);
	MessageBoxA(0,str,"Result is",MB_OK);
	*/

	return 0;
}

int Test2_Sync(st_argv_test2* p,RPC_CallBack cb)
{
	if (cb!=nullptr)
	{
		OutputDebug("Test3:Error:cb!=nullptr");
		return -1;
	}

	switch (p->other_argv_c)
	{
	case 1:
		if (p->firstStr!=nullptr||p->secondStr==nullptr)
		{
			return 11;
		}
		else
		{
			return 1;
		}
		break;
	case 2:
		if (p->firstStr!=nullptr||p->secondStr!=nullptr)
		{
			return 22;
		}
		else
		{
			return 2;
		}
		break;
	case 3:
		if (p->firstStr==nullptr||p->secondStr!=nullptr)
		{
			return 33;
		}
		else
		{
			return 3;
		}
		break;
	case 4:
		if (p->firstStr==nullptr||p->secondStr==nullptr)
		{
			return 44;
		}
		else
		{
			for (int i=0;i<p->firstStr_len;++i)
			{
				p->firstStr[i]=0;
			}
			return 4;
		}
		break;
	case 5:
		if (p->firstStr==nullptr||p->secondStr==nullptr)
		{
			return 55;
		}
		else
		{
			for (int i=0;i<p->secondStr_len;++i)
			{
				p->secondStr[i]=0;
			}
			return 5;
		}
		break;
	default:
		return 99;
		break;
	}
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 ServiceDLL.h
CServiceDLL::CServiceDLL()
{
	return;
}
