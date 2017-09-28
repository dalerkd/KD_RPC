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
	�������ջ���
	�������ͻ���
	*/
	m_Exit_Recive_Thread_EVENT		= CreateEvent(NULL,TRUE,FALSE,NULL);
	m_Exit_Recive_Thread_EVENT_OK	= CreateEvent(NULL,TRUE,FALSE,NULL);
	m_Exit_Send_Thread_EVENT		= CreateEvent(NULL,TRUE,FALSE,NULL);
	m_Exit_Send_Thread_EVENT_OK		= CreateEvent(NULL,TRUE,FALSE,NULL);


	_beginthreadex(NULL, 0, ThreadReceive, this, 0, NULL);
	_beginthreadex(NULL, 0, TrySend, this, 0, NULL);
	


}

//Ŀ���������
wchar_t wcCPName[20]=L".";
//���Ŀ���������ƴ�ӽ��
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
	//��ȡ����-pop
	for(;;)
	{
		{//����Ƿ�Ҫ���˳�����
			DWORD dw = WaitForSingleObject(CMS->m_Exit_Send_Thread_EVENT,0);
			if (dw==WAIT_TIMEOUT)
			{
				//û�д���
				;
			}
			else if (dw==WAIT_OBJECT_0)
			{
				//�Ѿ�����
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
		//1.���ʲ۶���
		HANDLE hFile=CreateFileW(
			wcCPNameEnd,//�ʲ�����
			GENERIC_WRITE,					//��д����
			FILE_SHARE_READ,				//��������
			NULL,							//��ȫ����
			OPEN_EXISTING,					//�򿪷�ʽ
			FILE_ATTRIBUTE_NORMAL,			//��־λ
			NULL							//�ļ�ģ��(Ĭ������)
			);
		//2.��mailslotд��
		DWORD dwWritten;
		LPSTR lpMessage =lpSendBuffer;/*"UDP�籩��Ϯ>>>>>>>>>>>"*/
		DWORD dwMegLen =SendBuffer_Len;
		WriteFile(hFile,lpMessage,dwMegLen,&dwWritten,NULL);
		//3.����

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



	//1.�����ʲ۶���
	
#ifdef SERVICE
	HANDLE hSlot = CreateMailslot(L"\\\\.\\mailslot\\_QUERY",0,MAILSLOT_WAIT_FOREVER,(LPSECURITY_ATTRIBUTES)NULL);
#endif // SERVICE//����˽���
#ifdef CLIENT
	HANDLE hSlot = CreateMailslot(L"\\\\.\\mailslot\\_RECV",0,MAILSLOT_WAIT_FOREVER,(LPSECURITY_ATTRIBUTES)NULL);
#endif //CLIENT //�ͻ��˽���
	//2.ѭ����ȡ�ʲ���Ϣ
	while (bIsTrue)
	{
		{//����Ƿ�Ҫ���˳�����
			DWORD dw = WaitForSingleObject(CMS->m_Exit_Recive_Thread_EVENT,0);
			if (dw==WAIT_TIMEOUT)
			{
				//û�д���
				;
			}
			else if (dw==WAIT_OBJECT_0)
			{
				//�Ѿ�����
				SetEvent(CMS->m_Exit_Recive_Thread_EVENT_OK);
				break;
			}
		}
		//2.1��ȡ�ʲ���Ϣ����
		DWORD dwMsgCount = 0,dwMsgSize=0;
		GetMailslotInfo(hSlot,(LPDWORD)NULL,&dwMsgSize,&dwMsgCount,(LPDWORD)NULL);
		if (dwMsgSize==MAILSLOT_NO_MESSAGE)
		{
			Sleep(2000);
			continue;
		}
		//2.2ѭ����ȡȫ����Ϣ
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
				printf("ReadFileִ��ʧ�ܣ�������:%d",GetLastError());
				return 0;
			}
			//��ʾ��Ϣ

			HexDump(L"Recive��������:",lpReciveBuffer,dwMsgSize);

			CMS->Recive_Data((char*)lpReciveBuffer,dwMsgSize);//δ�����ݳ��ȵȶ�εȴ�����



			//�����ʣ����Ϣ��
			GetMailslotInfo(hSlot,(LPDWORD)NULL,&dwMsgSize,&dwMsgCount,nullptr);
		}
		delete(lpReciveBuffer);
		lpReciveBuffer = nullptr;

	}


	return 0;
}
