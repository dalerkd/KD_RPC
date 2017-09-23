#pragma once
#include "..\public\web.h"
class CMailSlot :
	public CWEB
{
public:
	CMailSlot():m_Exit_Recive_Thread_EVENT(0),m_Exit_Recive_Thread_EVENT_OK(0),m_Exit_Send_Thread_EVENT(0),m_Exit_Send_Thread_EVENT_OK(0)
	{
		StartWork();

	};
	~CMailSlot();

	void StartWork();

	static unsigned int __stdcall TrySend(void* pM);
	void Recive(char* data,int data_len);

	static unsigned int __stdcall ThreadReceive(void* pM);


private:
	HANDLE m_Exit_Recive_Thread_EVENT;		//通知线程退出时激活
	HANDLE m_Exit_Recive_Thread_EVENT_OK;	//线程获知退出时激活

	HANDLE m_Exit_Send_Thread_EVENT;		//通知线程退出时激活
	HANDLE m_Exit_Send_Thread_EVENT_OK;		//线程获知退出时激活
};

