// Client.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Client.h"
#include "../public/debug.h"


#include "../public/WEB.h"
#include "WEB_Client.h"
#include "../public/FunctionInfo.h"
#include "../public/DataFormat.h"
#include "../public/StateManage.h"


// 这是导出变量的一个示例
CLIENT_API LONG64 nClient=0;

// 这是导出函数的一个示例。
CLIENT_API LONG64 fnClient(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 Client.h
CClient::CClient()
{
	return;
}



/*
异		SN	Name不需要
1		1
0		2


用链表放？还是一个表？这个不重要。Key-Value
Map
SN 异




*/

struct st_argv 
{
	_In_opt_ HWND hWnd;
	_In_opt_ LPCSTR lpText;
	LONG64 lpText_len;
	_In_opt_ LPCSTR lpCaption;
	LONG64 lpCaption_len;
	_In_ UINT uType;

};


CFunctionInfo* g_CI_Client = new CFunctionInfo();
//数据格式
CDataFormat g_CDF;
//客户端 异步状态管理
CAsyncStateManage* g_pasm = new CAsyncStateManage();
//客户端 同步状态管理
CSyncStateManage* g_pssm = new CSyncStateManage();
//网络管理
CWEB* pCWEB =new CWEB_Client();
//是否处于正在退出状态
HANDLE EXITING_EVENT = CreateEvent(NULL,TRUE,FALSE,NULL);;

/*
所有Fake函数的集中点

1. 查询SN：异步否（第二个参数）,数据结构大小（第一个参数）
~~要知道参数中的 struct中所有指针的offset,和长度的offset~~

0. 函数名(无用)
1. SN
2. 异步否
3. 结构体有几个指针(规定指针必须在结构体中靠前

*/
LONG64 Core(LONG64 SN,PVOID pStruct,FARPROC callBack)
{
	{
		DWORD dw = WaitForSingleObject(EXITING_EVENT,0);
		if (dw==WAIT_TIMEOUT)
		{
			//没有触发//正常进行
			;
		}
		else if (dw==WAIT_OBJECT_0)
		{
			//已经触发,正在退出，新请求直接返回。

			return -1;

		}
	}



	/*
	参数复制拷贝 需要参数判定一下指针的数量?还是由格式化函数复制一站到底

	参数返回拷贝 只在同步哟

	等待方式取决于 异步否


	同异步不会对合成一个参数造成影响。

	*/
	//发送参数方式统一
	try
	{
		LONG64 m_pointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);
		LONG64 m_sizeOfStruct = g_CI_Client->QueryArgvStructSize(SN);


		LONG ID_proc = CID_Manager::GetNewID();

		//同异步——不同的返回方式
		const bool async =g_CI_Client->QueryASync(SN);


		//不会返回0，它会整合一些别的结构。
		LONG64 realBufferLen = g_CDF.Format2Flow(ID_proc,SN,(char*)pStruct,m_sizeOfStruct,m_pointerNumber,
			CDataFormat::Query_INFO,CDataFormat::QUICK_FLOW_MODE,async,callBack);
		if (0==realBufferLen)
		{
			throw("Core: ToFlow return 0");
		}
		char* flowBuffer = new char[realBufferLen]();
		g_CDF.Format2Flow(ID_proc,SN,(char*)pStruct,m_sizeOfStruct,m_pointerNumber,
			CDataFormat::Query_INFO,CDataFormat::QUICK_FLOW_MODE,async,callBack,_Out_ flowBuffer,realBufferLen);

		//发送flowBuffer,realBufferLen

		pCWEB->Send(flowBuffer,realBufferLen);

		delete(flowBuffer);
		flowBuffer = nullptr;


		if (async)//异步处理
		{
			if (nullptr!=callBack)
			{
				g_pasm->push(ID_proc,callBack);
			}
			else
			{
				;//空回调是不会返回的
			}
			return TRUE;

		}
		else//同步处理
		{
			LONG64 ret = 0;//返回值
			LONG64 PointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);

			//Event
			HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);


			g_pssm->push(ID_proc,&ret,(char*)pStruct,PointerNumber,hdEvent);

			WaitForSingleObject(hdEvent,INFINITE);
			CloseHandle(hdEvent);
			return ret;
		}
	}
	catch(char* s)
	{
		OutputDebug("Error:%s",s);
		throw("");
	}
	catch(...)
	{
		OutputDebug("Error:Happen some error.");
		throw("");
	}



}

//在CloseModule前你需要
//调用本函数来退出所有RPC工作
extern"C" __declspec(dllexport)LONG64 ExitAllWork(PVOID,FARPROC)
{
	//1. 关闭本地入口
	SetEvent(EXITING_EVENT);
	/*
	1. 关网络入口,关本地入口，停止Recive。显然还有一些逻辑漏洞，当开始销毁时，无法保证没有任务正在中间运行。
	2. 销毁待发送：pCWEB.push 栈
	3. 已经接收内容处理：等Revice空？

	4. 销毁未接收等待.触发。

	
	*/
	
	
	
	/*2. 本地数据已经接受的数据全部完成发送*/
	
	while(!pCWEB->AllSendIsFinish())
	{
		Sleep(100);
	}

	/*3. MailSlot的析构 可确保所有数据新接收的数据都已经创建线程开始它们的工作。*/
	//网络管理
	delete(pCWEB);
	pCWEB = nullptr;

	//客户端 函数信息
	delete(g_CI_Client);
	g_CI_Client = nullptr;

	/*4. 稍微等待 已经接收数据所有的线程完成对数据信息的获取和处理
	这取决于Flow2Format的速度。
	FixMe: 需要确切地得知它们已经不再使用，异步状态和同步状态查询。
	*/
	Sleep(1000);

	/*5. 销毁剩余数据*/
	//客户端 异步状态管理
	delete(g_pasm);
	g_pasm = nullptr;
	//客户端 同步状态管理
	delete(g_pssm);
	g_pssm = nullptr;

	CloseHandle(EXITING_EVENT);
	return true;
}

//0号SN是预备给更新接口功能用的。

//Fake函数
extern"C" __declspec(dllexport)LONG64 Add(PVOID pStruct,FARPROC callBack)
{
	LONG64 m_SN = 1;
	return Core(m_SN,pStruct,callBack);
}

extern"C" __declspec(dllexport)LONG64 Add_Async_NoCallback(PVOID pStruct,FARPROC callBack)
{
	LONG64 m_SN = 2;
	return Core(m_SN,pStruct,callBack);
}

extern"C" __declspec(dllexport)LONG64 Test2_Sync(PVOID pStruct,FARPROC callBack)
{
	LONG64 m_SN = 3;
	return Core(m_SN,pStruct,callBack);
}

extern"C" __declspec(dllexport)LONG64 RealMessageBoxA(PVOID pStruct,FARPROC callBack)
{
	LONG64 m_SN = 4;
	return Core(m_SN,pStruct,callBack);
}



