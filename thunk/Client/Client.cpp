// Client.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Client.h"
#include "../public/debug.h"


#include "../public/WEB.h"
#include "../public/FunctionInfo.h"
#include "../public/DataFormat.h"
#include "../public/StateManage.h"


// 这是导出变量的一个示例
CLIENT_API int nClient=0;

// 这是导出函数的一个示例。
CLIENT_API int fnClient(void)
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
	int lpText_len;
	_In_opt_ LPCSTR lpCaption;
	int lpCaption_len;
	_In_ UINT uType;

};


CFunctionInfo* g_CI_Client = new CFunctionInfo();
CDataFormat g_CDF;
//客户端 异步状态管理
CAsyncStateManage* g_pasm = new CAsyncStateManage();
//客户端 同步状态管理
CSyncStateManage* g_pssm = new CSyncStateManage();
CWEB* pCWEB =new CClientWeb();


/*
所有Fake函数的集中点

1. 查询SN：异步否（第二个参数）,数据结构大小（第一个参数）
~~要知道参数中的 struct中所有指针的offset,和长度的offset~~

0. 函数名(无用)
1. SN
2. 异步否
3. 结构体有几个指针(规定指针必须在结构体中靠前

*/
int Core(int SN,PVOID pStruct,FARPROC callBack)
{
	/*
	参数复制拷贝 需要参数判定一下指针的数量?还是由格式化函数复制一站到底

	参数返回拷贝 只在同步哟

	等待方式取决于 异步否


	同异步不会对合成一个参数造成影响。

	*/
	//发送参数方式统一
	int m_pointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);
	int m_sizeOfStruct = g_CI_Client->QueryArgvStructSize(SN);


	LONG ID_proc = CID_Manager::GetNewID();

	//同异步——不同的返回方式
	const bool async =g_CI_Client->QueryASync(SN);


	//不会返回0，它会整合一些别的结构。
	int realBufferLen = g_CDF.Format2Flow(ID_proc,SN,(char*)pStruct,m_sizeOfStruct,m_pointerNumber,
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
		return true;
		
	}
	else//同步处理
	{
		int ret;//返回值
		int PointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);

		//Event
		HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);


		g_pssm->push(ID_proc,&ret,(char*)pStruct,PointerNumber,hdEvent);

		WaitForSingleObject(hdEvent,INFINITE);
		CloseHandle(hdEvent);
		return ret;
	}



}
//Fake函数
int aaa1(PVOID pStruct,FARPROC callBack)
{
	int m_SN = 1;
	return Core(m_SN,pStruct,callBack);
}






//////////////////////////////////////////////////////////////////////////
/*
负责产生唯一ID。该ID被用在标记任务的唯一性上：
一个被标号的数据流从服务器回来还能知道它来自于哪里。
*/
class CID_Manager
{
public:
	static LONG GetNewID()
	{
		return InterlockedIncrement(&CID_Manager::m_ID);
	}
private:
	static LONG m_ID;
};
LONG CID_Manager::m_ID =0;


