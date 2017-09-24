#include "stdafx.h"
#include <process.h>

#include "WEB_Client.h"
#include "Data_Format_Client.h"


void CWEB_Client::Recive_Data(char* flow,int flow_len)
{
	//Event的作用：将参数复制到内部后，需要通知本函数。另外参数传入需要需要拷贝过程。
	HANDLE  hdEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	CDataFormat::st_thread_FlowToFormat_Excute_argvs
		tmp={flow,flow_len,hdEvent};

	_beginthreadex(NULL,0,CData_Format_Client::Client_FlowToFormat_Execute,(void*)&tmp,0,NULL);	

	WaitForSingleObject(hdEvent,INFINITE);
	CloseHandle(hdEvent);
	hdEvent = NULL;
}
