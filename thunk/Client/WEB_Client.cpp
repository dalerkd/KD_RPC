#include "stdafx.h"
#include <process.h>

#include "WEB_Client.h"
#include "Data_Format_Client.h"


void CWEB_Client::Recive_Data(char* flow,int flow_len)
{
	//Event�����ã����������Ƶ��ڲ�����Ҫ֪ͨ���������������������Ҫ��Ҫ�������̡�
	HANDLE  hdEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	CDataFormat::st_thread_FlowToFormat_Excute_argvs
		tmp={flow,flow_len,hdEvent};

	_beginthreadex(NULL,0,CData_Format_Client::Client_FlowToFormat_Execute,(void*)&tmp,0,NULL);	

	WaitForSingleObject(hdEvent,INFINITE);
	CloseHandle(hdEvent);
	hdEvent = NULL;
}
