#include "stdafx.h"
#include <process.h>
#include <windows.h>

#include "Data_Format_Service.h"
#include "WEB_Service.h"


CWEB_Service::CWEB_Service(void)
{
}


CWEB_Service::~CWEB_Service(void)
{
}

void CWEB_Service::Recive_Data(char* flow,int flow_len)
{
	//Event�����ã����������Ƶ��ڲ�����Ҫ֪ͨ���������������������Ҫ��Ҫ�������̡�
	HANDLE  hdEvent=(NULL,TRUE,FALSE,NULL);

	CDataFormat::st_thread_Service_FlowToFormat_Excute
		tmp={flow,flow_len,hdEvent};

	_beginthreadex(NULL,0,CData_Format_Service::Service_FlowToFormat_Execute,(void*)&tmp,0,NULL);	

	WaitForSingleObject(hdEvent,INFINITE);
	CloseHandle(hdEvent);
	hdEvent = NULL;
}

