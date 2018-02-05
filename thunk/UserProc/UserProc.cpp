// UserProc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string.h>
#include <windows.h>
#include "../public/debug.h"

/*
解析头文件也很简单，什么时候解析？
总之初始化程序运行起来会向服务器请求数据获得头文件并写在本地(或者内存)。

我们运行时读取这个头文件，完成获取以下内容：
序号，函数名(初始化时用)， 异步情况(区别处理)，参数信息（用做解析）

1. 方案1添加结构进去
2. 方案2以无效的变量进去
3. 方案3以将信息附加到结构体名称上-hack语法...
用注释的方式表示序号

所有的函数在客户端都是相同的，除了它们每个函数有一个独特的序号。

序号-函数名-异步情况-参数信息

序号是不是自动生成的?可以，也可以手动

序号：方便传输，和字符串一一对应。否则就得用char*函数名和len来表示了.
我觉得这种字符串不在网络上传输，就是函数调用和函数名的一种分离，是好的。

能否动态生成自由参数的调用呢？eg：
int MessageBoxA(HANDLE,char*,char*,int);
答案是不可以：因为字符串多长我不知道。所以还是需要修改参数。
所以不如让其参数全部统一到我们的新格式来。

*/
void printNowTime()
{
	SYSTEMTIME sysTime;
	GetLocalTime( &sysTime );
	printf("Now Time:%02d:%02d:%02d.%03d\r\n",sysTime.wHour, sysTime.wMinute, sysTime.wSecond,sysTime.wMilliseconds);

}


struct st_argv_QueryWeather
{
	const char*	string;
	int			string_len;

};


struct st_argv_Add
{
	int firstNumber;
	int secondNumber;
};
//int Add(PVOID pStruct,FARPROC callBack)

struct st_argv_test2
{
	char* firstStr;
	int   firstStr_len;
	char* secondStr;
	int   secondStr_len;
	char  other_argv_c;
	float f_f;
};

struct st_argv_MessageBoxA 
{
	_In_opt_ LPCSTR lpText;
	int lpText_len;
	_In_opt_ LPCSTR lpCaption;
	int lpCaption_len;

	_In_opt_ HWND hWnd;
	_In_ UINT uType;

};


typedef void (_cdecl* RPC_CallBack)(const char*,int len);//RPC回调原形

typedef  int (__cdecl *int_FUN_ADD)(int,int);

typedef  int (__cdecl *int_FUN_Standard)(char* ,RPC_CallBack callBack);//标准





/*
添加框架函数的步骤：
1. 按照规范添加服务器导出函数
2. 添加本地调用函数

3. 添加Client函数注册代码
4. 添加Service函数注册代码


*/

/*
用前需知

1. 在客户端中调用以下析构函数前，不要直接使用FreeLibrary(test)。


CFunctionInfo* g_CI_Client;	//客户端 接口信息管理		,不处理会引起内存泄漏
CAsyncStateManage* g_pasm;	//客户端 异步状态管理		,不处理会引起内存泄漏
CSyncStateManage* g_pssm;	//客户端 同步状态管理		,不处理会引起内存泄漏
CWEB* pCWEB;				//客户端 网络管理收发线程	,不处理会引起线程问题和内存问题

2. 建议将网络模块替换成你自己的高速模块。我为了方便使用了MailSlot网络这种天底下最慢的网络模块。

*/




void Test4Callback(const char* cp,int len);


int TestErrNumber = 0;


void Work_Test()
{
	{
		TestErrNumber = 0;
		/*
		1. 无指针参数：检查返回值
		*/
		HMODULE  test = LoadLibraryA("Client_fack.dll");

		const bool excuteMessageBoxA = false;
		if (excuteMessageBoxA)
		{
			OutputDebug("Execute function MessageBoxA test.\r\n");
			if (test==0)
			{
				OutputDebug("UserProc:Don't find the DLL");
				++TestErrNumber;
				goto End_Test_Go;

			}
			FARPROC fpAdd = GetProcAddress(test,"RealMessageBoxA");		//导入算术测试
			if (fpAdd==nullptr)
			{
				OutputDebug("UserProc:Havn't the function");
				++TestErrNumber;
				goto End_Test_Go;

			}


			int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

			st_argv_MessageBoxA tmp_Message;
			tmp_Message.hWnd = NULL;
			tmp_Message.lpText = "I'm Text.";
			tmp_Message.lpText_len = strlen(tmp_Message.lpText)+1;
			tmp_Message.lpCaption = "Caption Happy";
			tmp_Message.lpCaption_len = strlen(tmp_Message.lpCaption)+1;
			tmp_Message.uType = MB_OKCANCEL;

			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			OutputDebug("Execute function MessageBoxA test result:%d",RealResult);
		}
		else
		{

			{
				OutputDebug("Test1:Starting:Sync:No Pointe,Check return value.\r\n");


				if (test==0)
				{
					OutputDebug("UserProc:Don't find the DLL");
					++TestErrNumber;
					goto End_Test_Go;

				}
				FARPROC fpAdd = GetProcAddress(test,"Add");		//导入算术测试
				if (fpAdd==nullptr)
				{
					OutputDebug("UserProc:Havn't the function");
					++TestErrNumber;
					goto End_Test_Go;

				}


				int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

				st_argv_Add tmp_Message;
				tmp_Message.firstNumber = 5;
				tmp_Message.secondNumber= 6;
				int RealResult = funAdd((char*)&tmp_Message,nullptr);
				if (RealResult==11)
				{
					OutputDebug("Test1:Pass:\r\n");
				}
				else
				{
					++TestErrNumber;
					OutputDebug("Test1:Fault:");

				}
				//FreeLibrary(test);
				//test = 0;

			}
			/*
			2. 有指针参数：检查返回值和指针修改情况
			- 标准模式
			- 快速模式
			*///
			/*
			test 2，other_c决定什么值，什么情况。
			1. 第一指针为空，第二指针不为空。
			2. 第一指针为空，第二指针为空。
			3. 第一指针不为空-第二指针为空。
			4. 第一指针不为空-第一个指针数据有修改，第二个指针数据不为空的情况。
			5. 第一指针不为空-第二个指针数据有修改，第二个指针数据不为空的情况。
			*/

			{
				OutputDebug("Test2:Sync,Starting:Have Pointe Argv:Check return value......\r\n");


				if (test==0)
				{
					OutputDebug("UserProc:Don't find the DLL");
					++TestErrNumber;
					goto End_Test_Go;
				}
				FARPROC fpAdd = GetProcAddress(test,"Test2_Sync");		//导入算术测试
				if (fpAdd==nullptr)
				{
					OutputDebug("UserProc:Havn't the function");
					++TestErrNumber;
					goto End_Test_Go;
				}


				int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;
				//1. 指针为空，普通参数为空的情况。
				st_argv_test2 tmp_Message;
				tmp_Message.firstStr = nullptr;
				tmp_Message.firstStr_len = 0;
				tmp_Message.secondStr= "SecondStr";
				tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
				tmp_Message.other_argv_c = 1;
				tmp_Message.f_f = (float)1.1;


				OutputDebug("Test2:1:Starting:First String Pointe Argv is empty.\r\n");
				int RealResult = funAdd((char*)&tmp_Message,nullptr);
				if (RealResult==1)
				{
					OutputDebug("Test2:1:Pass:\r\n");
				}
				else
				{
					++TestErrNumber;
					OutputDebug("Test2:1:Fault:");
				}
				//////////////////////////////////////////////////////////////////////////
				//2. 第一指针为空，第二指针为空。
				tmp_Message.firstStr = nullptr;
				tmp_Message.firstStr_len = 0;
				tmp_Message.secondStr= nullptr;
				tmp_Message.secondStr_len= 0;
				tmp_Message.other_argv_c = 2;
				tmp_Message.f_f = (float)2.2;


				OutputDebug("Test2:2:Starting:Second Pointe is empty.\r\n");
				RealResult = funAdd((char*)&tmp_Message,nullptr);
				if (RealResult==2)
				{
					OutputDebug("Test2:2:Pass:\r\n");
				}
				else
				{
					++TestErrNumber;
					OutputDebug("Test2:2:Fault:");
				}

				//////////////////////////////////////////////////////////////////////////
				//3. 第一指针不为空-第二指针为空。
				tmp_Message.firstStr = "FirstStr";
				tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
				tmp_Message.secondStr= nullptr;
				tmp_Message.secondStr_len= 0;
				tmp_Message.other_argv_c = 3;
				tmp_Message.f_f = (float)3.3;


				OutputDebug("Test2:3:Starting:First Pointe is empty,Second Pointe is empty.\r\n");
				RealResult = funAdd((char*)&tmp_Message,nullptr);
				if (RealResult==3)
				{
					OutputDebug("Test2:3:Pass:\r\n");
				}
				else
				{
					++TestErrNumber;
					OutputDebug("Test2:3:Fault:");
				}
				//////////////////////////////////////////////////////////////////////////
				//4. 第一指针不为空-第一个指针数据有修改，第二个指针数据不为空的情况。

				//修改你得给个能修改的量
				char* pf4= new char[0x10]();
				char* tpFirstStr = "FirstStr";

				for (int i=0;i<0x10;++i)
				{
					pf4[i] = tpFirstStr[i];
				}
				tmp_Message.firstStr = pf4;
				tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
				tmp_Message.secondStr= "SecondStr";
				tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
				tmp_Message.other_argv_c = 4;
				tmp_Message.f_f = (float)4.4;


				OutputDebug("Test2:4:Staring:First Pointe isn't empty,change by Service.Seoncd Pointe isn't empty.\r\n");
				RealResult = funAdd((char*)&tmp_Message,nullptr);
				if (RealResult==4)
				{
					for (int i=0;i<tmp_Message.firstStr_len;++i)
					{
						if (tmp_Message.firstStr[i]!=0)
						{
							++TestErrNumber;
							OutputDebug("Test2:4:Fault:Change");
							goto hah__;
						}

					}
					OutputDebug("Test2:4:Pass:\r\n");
hah__:;

				}
				else
				{
					++TestErrNumber;
					OutputDebug("Test2:4:Fault:");
				}
				delete(pf4);
				pf4 = nullptr;

				//////////////////////////////////////////////////////////////////////////
				//5. 第一指针不为空-第二个指针数据有修改，第二个指针数据不为空的情况。
				//修改你得给个能修改的量

				tmp_Message.firstStr = "FirstStr";
				tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
				char* ps5 =  new char[0x10]();
				char* tpStr = "SecondStr";

				for (int i=0;i<0x10;++i)
				{
					ps5[i] = tpStr[i];
				}
				tmp_Message.secondStr= ps5;
				tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
				tmp_Message.other_argv_c = 5;
				tmp_Message.f_f = 5.5;


				OutputDebug("Test2:5:Staring:First Pointe isn't empty,change by Service.Seoncd Pointe isn't empty.\r\n");
				RealResult = funAdd((char*)&tmp_Message,nullptr);
				if (RealResult==5)
				{
					for (int i=0;i<tmp_Message.secondStr_len;++i)
					{
						if (tmp_Message.secondStr[i]!=0)
						{
							++TestErrNumber;
							OutputDebug("Test2:5:Fault:Change");
							goto h__ah__;
						}

					}
					OutputDebug("Test2:5:Pass:.\r\n");
h__ah__:;
				}
				else
				{
					++TestErrNumber;
					OutputDebug("Test2:5:Fault:");
				}
				delete(ps5);
				ps5 = nullptr;
				//////////////////////////////////////////////////////////////////////////

			}
			/*
			- 异步：
			1. 无回调
			*/
			{
				OutputDebug("Test3:Starting:Async.Havn't Callback.\r\n");
				OutputDebug("Please see the server test display for success.\r\n");
				if (test==0)
				{
					MessageBoxA(0,"不存在dll","",MB_OK);
				}
				FARPROC fpAdd = GetProcAddress(test,"Add_Async_NoCallback");
				if (fpAdd==nullptr)
				{
					MessageBoxA(0,"存在dll,但不存在指定函数","",MB_OK);

				}


				int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

				st_argv_Add tmp_Message;
				tmp_Message.firstNumber = 5;
				tmp_Message.secondNumber= 6;
				int RealResult = funAdd((char*)&tmp_Message,nullptr);
				if (RealResult==TRUE)
				{
					;
				}
				else
				{
					OutputDebug("Test3:Fault:");
					MessageBoxA(0,"测试3返回值错误","",MB_OK);
				}
				//FreeLibrary(test);

			}
			/*
			2. 有回调：检查回调信息情况
			*/		
			{
				OutputDebug("Test4:Start:Async.Callback testing.\r\n");

				if (test==0)
				{
					++TestErrNumber;
					OutputDebug("UserProc:Don't find the DLL");
					goto End_Test_Go;
				}
				FARPROC fpAdd = GetProcAddress(test,"Add_Async_NoCallback");		//导入算术测试
				if (fpAdd==nullptr)
				{
					++TestErrNumber;
					OutputDebug("UserProc:Havn't the function");

				}


				int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

				st_argv_Add tmp_Message;
				tmp_Message.firstNumber = 5;
				tmp_Message.secondNumber= 6;
				int RealResult = funAdd((char*)&tmp_Message,Test4Callback);
				if (RealResult==TRUE)
				{
					;
				}
				else
				{
					++TestErrNumber;
					OutputDebug("Test4:Fault:");

				}
				/*
				在客户端中调用以下析构函数前，不要直接使用FreeLibrary(test)。


				CFunctionInfo* g_CI_Client;	//客户端 接口信息管理		,不处理会引起内存泄漏
				CAsyncStateManage* g_pasm;	//客户端 异步状态管理		,不处理会引起内存泄漏
				CSyncStateManage* g_pssm;	//客户端 同步状态管理		,不处理会引起内存泄漏
				CWEB* pCWEB;				//客户端 网络管理收发线程	,不处理会引起线程问题和内存问题


				*/
			}

End_Test_Go:
			if (0==TestErrNumber)
			{
				//Sleep(5000);
				OutputDebug("Congratulations：All pass:)\r\n");
			}
			else
			{
				//Sleep(5000);
				OutputDebug("There are %d failed tests\r\n",TestErrNumber);
			}

		}

		//卸载代码
		if (test!=0)
		{

			FARPROC fpExit = GetProcAddress(test,"ExitAllWork");		//导入算术测试
			if (fpExit==nullptr)
			{
				OutputDebug("UserProc:Havn't the function");

			}


			int_FUN_Standard funExit =(int_FUN_Standard)fpExit;

			int RealResult = funExit(nullptr,nullptr);



			FreeLibrary(test);
			
			printf("\r\n\r\nModule Unload Success!\r\n");
		}

	}

}


int _tmain(int argc, _TCHAR* argv[])
{
	printNowTime();
	
	Work_Test();
	
	printNowTime();
	printf("Input Any key exit:");
	getchar();

	Work_Test();
	printNowTime();
	printf("Input Any key exit:");
	getchar();


	return 0;
}

void Test4Callback(const char* cp,int len)
{
	char p[] = {'r','e','s','u','l','t','i','s',':','1','1','\0'};
	if (len!=(strlen(p)+1))
	{
		++TestErrNumber;
		OutputDebug("Test4:Fault:length");
		return;
	}
	int stat = memcmp(p,cp,len);
	if (stat==0)
	{
		OutputDebug("Test4:Pass:\r\n");
	}
	else
	{
		++TestErrNumber;
		OutputDebug("Test4:Fault:memcpy");
	}
	return;
}

