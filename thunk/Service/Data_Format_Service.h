#pragma once

#include <windows.h>
#include "..\public\dataformat.h"
class CData_Format_Service :
	public CDataFormat
{
public:
	CData_Format_Service(void);
	~CData_Format_Service(void);

public:
	//流转换成格式:服务端解码：这是一个线程函数。
	//参数：st_thread_Service_FlowToFormat_Excute* p
	static  unsigned int WINAPI Service_FlowToFormat_Execute(LPVOID p);
	/*
	提供给异步服务器方法的回调函数

	用于获取返回的数据

	并在这里发送
	*/
	static void ServiceAsyncCallBack(char* p,int p_len);
};

