#include "stdafx.h"
#include "TempWeb.h"
#include <windows.h>
#include "../public/debug.h"

CTempWeb::CTempWeb(void)
{
}


CTempWeb::~CTempWeb(void)
{
}
//Ŀ���������
wchar_t wcCPName[20]=L".";
//���Ŀ���������ƴ�ӽ��
wchar_t wcCPNameEnd[40]=L"\\\\";

unsigned int __stdcall CTempWeb::TrySend(void* pM)
{


	//wscanf_s(L"%s",wcCPName,20);//��ȡĿ���������
	wcscat_s(wcCPNameEnd,20,wcCPName);
	wcscat_s(wcCPNameEnd,40,L"\\mailslot\\KITTY");
	char* lpSendBuffer = nullptr;
	int SendBuffer_Len = 0;
	//��ȡ����-pop
	for(;;)
	{
		try
		{
			lpSendBuffer = "abcd\0\1\2\3\4";
			SendBuffer_Len = 9;
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

		break;




	}

	return 0;



}

void CTempWeb::Recive(char* data,int data_len)
{

}
unsigned int __stdcall CTempWeb::ThreadReceive(PVOID pM)
//void CTempWeb::CheckRecive()
{
	bool bIsTrue = true;
	PBYTE lpReciveBuffer = nullptr;



	//1.�����ʲ۶���
	HANDLE hSlot = CreateMailslot(L"\\\\.\\mailslot\\KITTY",0,MAILSLOT_WAIT_FOREVER,(LPSECURITY_ATTRIBUTES)NULL);
	//2.ѭ����ȡ�ʲ���Ϣ
	while (bIsTrue)
	{
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
			
			HexDump(L"��������:",lpReciveBuffer,dwMsgSize);

			//Recive((char*)lpReciveBuffer,dwMsgSize);
			


			//�����ʣ����Ϣ��
			GetMailslotInfo(hSlot,(LPDWORD)NULL,&dwMsgSize,&dwMsgCount,nullptr);
		}
		delete(lpReciveBuffer);
	}


	return 0;
}
