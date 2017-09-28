#include "stdafx.h"
#include "MailSlot.h"
#include "debug.h"
#include <process.h>
#include <windows.h>

CMailSlot::~CMailSlot()
{
	SetEvent(m_Exit_Recive_Thread_EVENT);
	SetEvent(m_Exit_Send_Thread_EVENT);



	WaitForSingleObject(m_Exit_Recive_Thread_EVENT_OK,INFINITE);
	WaitForSingleObject(m_Exit_Send_Thread_EVENT_OK,INFINITE);


	CloseHandle(m_Exit_Recive_Thread_EVENT);
	CloseHandle(m_Exit_Recive_Thread_EVENT_OK);
	CloseHandle(m_Exit_Send_Thread_EVENT);
	CloseHandle(m_Exit_Send_Thread_EVENT_OK);

	m_Exit_Recive_Thread_EVENT = 0;
	m_Exit_Recive_Thread_EVENT_OK = 0;
	m_Exit_Send_Thread_EVENT = 0;
	m_Exit_Send_Thread_EVENT_OK = 0;

}

void CMailSlot::StartWork()
{
	/*
	启动接收机制
	启动发送机制
	*/
	m_Exit_Recive_Thread_EVENT		= CreateEvent(NULL,TRUE,FALSE,NULL);
	m_Exit_Recive_Thread_EVENT_OK	= CreateEvent(NULL,TRUE,FALSE,NULL);
	m_Exit_Send_Thread_EVENT		= CreateEvent(NULL,TRUE,FALSE,NULL);
	m_Exit_Send_Thread_EVENT_OK		= CreateEvent(NULL,TRUE,FALSE,NULL);


	_beginthreadex(NULL, 0, ThreadReceive, this, 0, NULL);
	_beginthreadex(NULL, 0, TrySend, this, 0, NULL);
	


}

//目标电脑名字
wchar_t wcCPName[20]=L".";
//存放目标电脑名字拼接结果
wchar_t wcCPNameEnd[40]=L"\\\\";

unsigned int __stdcall CMailSlot::TrySend(void* pM)
{
	CMailSlot* CMS = (CMailSlot* )pM;

	wcscat_s(wcCPNameEnd,20,wcCPName);
#ifdef SERVICE
	wcscat_s(wcCPNameEnd,40,L"\\mailslot\\_RECV");
#endif // SERVICE
#ifdef CLIENT
	wcscat_s(wcCPNameEnd,40,L"\\mailslot\\_QUERY");
#endif //CLIENT
	
	
	char* lpSendBuffer = nullptr;
	int SendBuffer_Len = 0;
	//获取数据-pop
	for(;;)
	{
		{//检查是否要求退出任务
			DWORD dw = WaitForSingleObject(CMS->m_Exit_Send_Thread_EVENT,0);
			if (dw==WAIT_TIMEOUT)
			{
				//没有触发
				;
			}
			else if (dw==WAIT_OBJECT_0)
			{
				//已经触发
				SetEvent(CMS->m_Exit_Send_Thread_EVENT_OK);
				OutputDebug("TrySend Thread Exiting");
				break;
			}
		}

		try
		{
			//lpSendBuffer = "abcd\0\1\2\3\4";
			//SendBuffer_Len = 9;
			CWEB::st_asysnc_queue_argv pST = CMS->findAndPop();
			lpSendBuffer = pST.data;
			SendBuffer_Len = pST.data_len;

		}
		catch(...)
		{
			Sleep(1000);
			continue;
		}
		//1.打开邮槽对象
		HANDLE hFile=CreateFileW(
			wcCPNameEnd,//邮槽名称
			GENERIC_WRITE,					//读写属性
			FILE_SHARE_READ,				//共享属性
			NULL,							//安全属性
			OPEN_EXISTING,					//打开方式
			FILE_ATTRIBUTE_NORMAL,			//标志位
			NULL							//文件模块(默认留空)
			);
		//2.向mailslot写入
		DWORD dwWritten;
		LPSTR lpMessage =lpSendBuffer;/*"UDP风暴来袭>>>>>>>>>>>"*/
		DWORD dwMegLen =SendBuffer_Len;
		WriteFile(hFile,lpMessage,dwMegLen,&dwWritten,NULL);
		//3.结束

		CloseHandle(hFile);
		hFile = 0;

	}

	return 0;



}



unsigned int __stdcall CMailSlot::ThreadReceive(PVOID pM)
{
	CMailSlot* CMS = (CMailSlot* )pM;


	bool bIsTrue = true;
	PBYTE lpReciveBuffer = nullptr;



	//1.创建邮槽对象
	
#ifdef SERVICE
	HANDLE hSlot = CreateMailslot(L"\\\\.\\mailslot\\_QUERY",0,MAILSLOT_WAIT_FOREVER,(LPSECURITY_ATTRIBUTES)NULL);
#endif // SERVICE//服务端接收
#ifdef CLIENT
	HANDLE hSlot = CreateMailslot(L"\\\\.\\mailslot\\_RECV",0,MAILSLOT_WAIT_FOREVER,(LPSECURITY_ATTRIBUTES)NULL);
#endif //CLIENT //客户端接受
	//2.循环读取邮槽信息
	while (bIsTrue)
	{
		{//检查是否要求退出任务
			DWORD dw = WaitForSingleObject(CMS->m_Exit_Recive_Thread_EVENT,0);
			if (dw==WAIT_TIMEOUT)
			{
				//没有触发
				;
			}
			else if (dw==WAIT_OBJECT_0)
			{
				//已经触发
				SetEvent(CMS->m_Exit_Recive_Thread_EVENT_OK);
				break;
			}
		}
		//2.1获取邮槽消息数量
		DWORD dwMsgCount = 0,dwMsgSize=0;
		GetMailslotInfo(hSlot,(LPDWORD)NULL,&dwMsgSize,&dwMsgCount,(LPDWORD)NULL);
		if (dwMsgSize==MAILSLOT_NO_MESSAGE)
		{
			Sleep(2000);
			continue;
		}
		//2.2循环获取全部消息
		if (dwMsgSize==0)
		{
			Sleep(2000);
			continue;
		}
		lpReciveBuffer = new BYTE[dwMsgSize]();
		while (dwMsgCount)
		{
			DWORD dwRet;
			ZeroMemory(lpReciveBuffer,dwMsgSize);
			if (!ReadFile(hSlot,lpReciveBuffer,dwMsgSize,&dwRet,NULL))
			{
				printf("ReadFile执行失败，错误码:%d",GetLastError());
				return 0;
			}
			//显示信息

			HexDump(L"Recive数据内容:",lpReciveBuffer,dwMsgSize);

			CMS->Recive_Data((char*)lpReciveBuffer,dwMsgSize);//未做数据长度等多次等待处理



			//计算机剩余消息数
			GetMailslotInfo(hSlot,(LPDWORD)NULL,&dwMsgSize,&dwMsgCount,nullptr);
		}
		delete(lpReciveBuffer);
		lpReciveBuffer = nullptr;

	}


	return 0;
}
