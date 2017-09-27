// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 SERVICEDLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// SERVICEDLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#include "../public/debug.h"



#ifdef SERVICEDLL_EXPORTS
#define SERVICEDLL_API __declspec(dllexport)
#else
#define SERVICEDLL_API __declspec(dllimport)
#endif

// 此类是从 ServiceDLL.dll 导出的
class SERVICEDLL_API CServiceDLL {
public:
	CServiceDLL(void);
	// TODO: 在此添加您的方法。
};

extern SERVICEDLL_API int nServiceDLL;

SERVICEDLL_API int fnServiceDLL(void);



//////////////////////////////////////////////////////////////////////////
struct st_argv_Add
{
	int firstNumber;
	int secondNumber;
};

struct st_argv_test2
{
	char* firstStr;
	int   firstStr_len;
	char* secondStr;
	int   secondStr_len;
	char  other_argv_c;
	float f_f;
};

typedef void (_cdecl* RPC_CallBack)(const char*,int len);//RPC回调原形
typedef  int (__cdecl *int_FUN_Standard)(char* ,RPC_CallBack callBack);//标准

#include <windows.h>
#include <stdio.h>
//测试1： 同步，无指针，返回值
extern"C" __declspec(dllexport)int Add(st_argv_Add* p,char* cb)
{
	if (cb!=nullptr)
	{
		OutputDebug(L"err:Add cb!=nullptr");
	}
	
	return p->firstNumber+p->secondNumber;

}


//测试3： 异步，无指针，无回调
extern"C" __declspec(dllexport)int Add_Async_NoCallback(st_argv_Add* p,RPC_CallBack cb)
{
	if (cb!=nullptr)
	{
		char str[]={'r','e','s','u','l','t','i','s',':','1','1','\0'};
		cb(str,strlen(str)+1);
	}
	else
	{
		OutputDebug(L"Test3:Pass:\r\n");//MessageBoxA(0,"测试通过","",MB_OK);
	}
	/*char str[10] ={0};
	sprintf_s(str,"%d",p->firstNumber+p->secondNumber);
	MessageBoxA(0,str,"Result is",MB_OK);
	*/

	return 0;

}
////测试2 同步，各种情况,other_argv_c决定
/*  1. 第一指针为空，第二指针不为空。
	2. 第一指针为空，第二指针为空。
	3. 第一指针不为空-第二指针为空。
	4. 第一指针不为空-第一个指针数据有修改，第二个指针数据不为空的情况。
	5. 第一指针不为空-第二个指针数据有修改，第二个指针数据不为空的情况。
*/

extern"C" __declspec(dllexport)int Test2_Sync(st_argv_test2* p,RPC_CallBack cb)
{
	if (cb!=nullptr)
	{
		OutputDebug(L"Test3:Error:cb!=nullptr");
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
			return 4;
		}
		else
		{
			for (int i=0;i<p->firstStr_len;++i)
			{
				p->firstStr[i]=0;
			}
		}
		break;
	case 5:
		if (p->firstStr==nullptr||p->secondStr==nullptr)
		{
			return 5;
		}
		else
		{
			for (int i=0;i<p->secondStr_len;++i)
			{
				p->secondStr[i]=0;
			}
		}
		break;
	default:
		return 99;
		break;
	}


}

//////////////////////////////////////////////////////////////////////////

//MessageBoxA(0,"","",MB_OK)
//MessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType)
struct st_argv 
{
	_In_opt_ HWND hWnd;
	_In_opt_ LPCSTR lpText;
	int lpText_len;
	_In_opt_ LPCSTR lpCaption;
	int lpCaption_len;
	_In_ UINT uType;

};

extern"C" __declspec(dllexport)int Real(st_argv* p,FARPROC callBack)
{
	return MessageBoxA(p->hWnd,p->lpText,p->lpText,p->uType);
}



struct st_argv_QueryWeather
{
	const char*	string;
	int						string_len;

};



typedef void (_cdecl* RPC_CallBack)(const char*,int len);
DWORD WINAPI RPC_Async_Thread(LPVOID);


struct weatch_thread_arg
{
	st_argv_QueryWeather qw;
	RPC_CallBack callback;
};

weatch_thread_arg g_weather_thread_arg;

/*服务器服务函数*/
/*
下面的代码不是真实的调用哟

这里的真正情况应该是：
调用假回调函数													------服务器
由假回调函数 负责：捕获 参数 发送参数							------服务器

由接受函数 负责：创建“回调前环境处理”线程						------客户端

由 “回调前环境处理” 负责：用 捕获到的 参数 调用“回调函数”。	------客户端

*/
extern"C" __declspec(dllexport)int QueryWeather(st_argv_QueryWeather* p,FARPROC/*fake*/ callBack)
{
	g_weather_thread_arg.callback =(RPC_CallBack)callBack;
	g_weather_thread_arg.qw = *p;

	CreateThread(0,0,RPC_Async_Thread,&g_weather_thread_arg,0,0);

	return 0;
}


void fakeCallBack()
{
	
}


/*
“回调前环境处理”

用指定参数进行回调*/
DWORD WINAPI RPC_Async_Thread(LPVOID m_argv)
{
	weatch_thread_arg* argv = (weatch_thread_arg*)m_argv;

	RPC_CallBack proc = argv->callback;
	const char*  result = argv->qw.string;


	Sleep(2000);


	proc(result,strlen(result));

	return 0;
	
}