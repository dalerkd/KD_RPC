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

extern SERVICEDLL_API LONG64 nServiceDLL;

SERVICEDLL_API LONG64 fnServiceDLL(void);



//////////////////////////////////////////////////////////////////////////
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

typedef void (_cdecl* RPC_CallBack)(const char*,LONG64 len);//RPC回调原形
typedef  LONG64 (__cdecl *int_FUN_Standard)(char* ,RPC_CallBack callBack);//标准

#include <windows.h>
#include <stdio.h>
//测试1： 同步，无指针，返回值
extern"C" __declspec(dllexport)LONG64 Add(st_argv_Add* p,char* cb);


//测试3： 异步，无指针，无回调
extern"C" __declspec(dllexport)LONG64 Add_Async_NoCallback(st_argv_Add* p,RPC_CallBack cb);
////测试2 同步，各种情况,other_argv_c决定
/*  1. 第一指针为空，第二指针不为空。
	2. 第一指针为空，第二指针为空。
	3. 第一指针不为空-第二指针为空。
	4. 第一指针不为空-第一个指针数据有修改，第二个指针数据不为空的情况。
	5. 第一指针不为空-第二个指针数据有修改，第二个指针数据不为空的情况。
*/

extern"C" __declspec(dllexport)LONG64 Test2_Sync(st_argv_test2* p,RPC_CallBack cb);

//////////////////////////////////////////////////////////////////////////

//MessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType)
struct st_argv_MessageBoxA 
{
	_In_opt_ LPCSTR lpText;
	LONG64 lpText_len;
	_In_opt_ LPCSTR lpCaption;
	LONG64 lpCaption_len;

	_In_opt_ HWND hWnd;
	_In_ UINT uType;

};

extern"C" __declspec(dllexport)LONG64 RealMessageBoxA(st_argv_MessageBoxA* p,FARPROC callBack)
{
	return MessageBoxA(p->hWnd,p->lpCaption,p->lpText,p->uType);
}



/*服务器服务函数机制*/
/*

这里的真正情况应该是：
调用假回调函数													------服务器
由假回调函数 负责：捕获 参数 发送参数							------服务器

由接受函数 负责：创建“回调前环境处理”线程						------客户端

由 “回调前环境处理” 负责：用 捕获到的 参数 调用“回调函数”。	------客户端

*/


