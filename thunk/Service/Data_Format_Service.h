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
	//��ת���ɸ�ʽ:����˽��룺����һ���̺߳�����
	//������st_thread_Service_FlowToFormat_Excute* p
	static  unsigned int WINAPI Service_FlowToFormat_Execute(LPVOID p);
	/*
	�ṩ���첽�����������Ļص�����

	���ڻ�ȡ���ص�����

	�������﷢��
	*/
	static void ServiceAsyncCallBack(char* p,int p_len);
};

