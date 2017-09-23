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
//目标电脑名字
wchar_t wcCPName[20]=L".";
//存放目标电脑名字拼接结果
wchar_t wcCPNameEnd[40]=L"\\\\";

unsigned int __stdcall CTempWeb::TrySend(void* pM)
{


	//wscanf_s(L"%s",wcCPName,20);//获取目标电脑名字
	wcscat_s(wcCPNameEnd,20,wcCPName);
	wcscat_s(wcCPNameEnd,40,L"\\mailslot\\KITTY");
	char* lpSendBuffer = nullptr;
	int SendBuffer_Len = 0;
	//获取数据-pop
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



	//1.创建邮槽对象
	HANDLE hSlot = CreateMailslot(L"\\\\.\\mailslot\\KITTY",0,MAILSLOT_WAIT_FOREVER,(LPSECURITY_ATTRIBUTES)NULL);
	//2.循环读取邮槽信息
	while (bIsTrue)
	{
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
			
			HexDump(L"数据内容:",lpReciveBuffer,dwMsgSize);

			//Recive((char*)lpReciveBuffer,dwMsgSize);
			


			//计算机剩余消息数
			GetMailslotInfo(hSlot,(LPDWORD)NULL,&dwMsgSize,&dwMsgCount,nullptr);
		}
		delete(lpReciveBuffer);
	}


	return 0;
}
