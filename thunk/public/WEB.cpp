#include "stdafx.h"
//#include "../Client/stdafx.h"
#include "WEB.h"
#include "../public/debug.h"
#include "../public/StateManage.h"

#include <process.h>




CWEB::CWEB()
{
	m_CSQ = new CSafeQueue<st_asysnc_queue_argv>();
}

CWEB::~CWEB()
{
	delete(m_CSQ);
}

void CWEB::Send(char* data,int data_len) //其实是个push而已.
{
	if (data_len<=0)
	{
		throw("CWEB::Send:data_len<=0");
	}
	char* p = new char[data_len]();
	int stat=memcpy_s(p,data_len,data,data_len);
	if (stat)
	{
		throw("memcpy_s return err.");
	}

	st_asysnc_queue_argv tmpArgv = {p,data_len};
	m_CSQ->push(tmpArgv);

	return;
}

bool CWEB::AllSendIsFinish()
{
	return m_CSQ->IsEmpty();
}

CWEB::st_asysnc_queue_argv CWEB::findAndPop()
{
	return m_CSQ->pop();
}

void CClientWeb::Recive_Data(char* flow,int flow_len)
{
	//Event的作用：将参数复制到内部后，需要通知本函数。另外参数传入需要需要拷贝过程。
	HANDLE  hdEvent=(NULL,TRUE,FALSE,NULL);

	CDataFormat::st_thread_Service_FlowToFormat_Excute
		tmp={flow,flow_len,hdEvent};

	_beginthreadex(NULL,0,CDataFormat::Client_FlowToFormat_Execute,(void*)&tmp,0,NULL);	

	WaitForSingleObject(hdEvent,INFINITE);
	CloseHandle(hdEvent);
	hdEvent = NULL;
}

void CServiceWeb::Recive_Data(char* flow,int flow_len)
{
	//Event的作用：将参数复制到内部后，需要通知本函数。另外参数传入需要需要拷贝过程。
	HANDLE  hdEvent=(NULL,TRUE,FALSE,NULL);

	CDataFormat::st_thread_Service_FlowToFormat_Excute
		tmp={flow,flow_len,hdEvent};

	_beginthreadex(NULL,0,CDataFormat::Service_FlowToFormat_Execute,(void*)&tmp,0,NULL);	

	WaitForSingleObject(hdEvent,INFINITE);
	CloseHandle(hdEvent);
	hdEvent = NULL;
}
