// UserProc.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <string.h>
#include <windows.h>
#include "../public/debug.h"

/*
����ͷ�ļ�Ҳ�ܼ򵥣�ʲôʱ�������
��֮��ʼ������������������������������ݻ��ͷ�ļ���д�ڱ���(�����ڴ�)��

��������ʱ��ȡ���ͷ�ļ�����ɻ�ȡ�������ݣ�
��ţ�������(��ʼ��ʱ��)�� �첽���(������)��������Ϣ������������

1. ����1��ӽṹ��ȥ
2. ����2����Ч�ı�����ȥ
3. ����3�Խ���Ϣ���ӵ��ṹ��������-hack�﷨...
��ע�͵ķ�ʽ��ʾ���

���еĺ����ڿͻ��˶�����ͬ�ģ���������ÿ��������һ�����ص���š�

���-������-�첽���-������Ϣ

����ǲ����Զ����ɵ�?���ԣ�Ҳ�����ֶ�

��ţ����㴫�䣬���ַ���һһ��Ӧ������͵���char*��������len����ʾ��.
�Ҿ��������ַ������������ϴ��䣬���Ǻ������úͺ�������һ�ַ��룬�Ǻõġ�

�ܷ�̬�������ɲ����ĵ����أ�eg��
int MessageBoxA(HANDLE,char*,char*,int);
���ǲ����ԣ���Ϊ�ַ����೤�Ҳ�֪�������Ի�����Ҫ�޸Ĳ�����
���Բ����������ȫ��ͳһ�����ǵ��¸�ʽ����

*/

//!!���!!�첽���!!
//!!SN!!1234
//!!async!!0
struct st_argv_MessageBoxA
{
	_In_opt_ HWND hWnd;
	_In_opt_ LPCSTR lpText;
	int lpText_len;
	_In_opt_ LPCSTR lpCaption;
	int lpCaption_len;
	_In_ UINT uType;

};

struct st_argv_QueryWeather
{
	const char*	string;
	int			string_len;

};


struct st_argv_Add
{
	int firstNumber;
	int secondNumber;
};
//int Add(PVOID pStruct,FARPROC callBack)





typedef void (_cdecl* RPC_CallBack)(const char*,int len);//RPC�ص�ԭ��

typedef  int (__cdecl *int_FUN_ADD)(int,int);
typedef  int (__cdecl *int_FUN_REAL)(st_argv_MessageBoxA* p,RPC_CallBack callBack);

typedef  int (__cdecl *int_FUN_Standard)(char* ,RPC_CallBack callBack);//��׼

//typedef  int (__cdecl *int_FUN_Standard)(st_argv_QueryWeather* ,RPC_CallBack callBack);//��׼




void WeatherCallBack(const char*,int len);

/*
��ӿ�ܺ����Ĳ��裺
1. ���չ淶��ӷ�������������
2. ��ӱ��ص��ú���

3. ���Client����ע�����
4. ���Service����ע�����


*/



int _tmain(int argc, _TCHAR* argv[])
{
	bool Yuan = false;

	if (Yuan)
	{


		HMODULE  test = LoadLibraryA("ServiceDLL.dll");

		FARPROC fpAdd = GetProcAddress(test,"Add");		//������������
		FARPROC fpReal = GetProcAddress(test,"Real");	//����MessageBoxA����
		FARPROC fpQueryWeather = GetProcAddress(test,"QueryWeather");//����ص�����

		int_FUN_ADD funAdd =(int_FUN_ADD)fpAdd;
		int_FUN_REAL funReal =(int_FUN_REAL)fpReal;
		int_FUN_Standard funQueryWeather =(int_FUN_Standard)fpQueryWeather;



		int AddResult  = funAdd(5,6);


		st_argv_MessageBoxA tmp_Message;
		tmp_Message.hWnd = NULL;
		tmp_Message.lpText = "Hi";
		tmp_Message.lpCaption = "title";
		tmp_Message.uType = MB_OK;
		int RealResult = funReal(&tmp_Message,nullptr);


		st_argv_QueryWeather tmp_we;
		tmp_we.string = "It's raining today";
		tmp_we.string_len = strlen(tmp_we.string);

		funQueryWeather((char*)&tmp_we,WeatherCallBack);//


		Sleep(6000);
	}
	else
	{
/*
1. ��ָ���������鷵��ֵ
*/
		HMODULE  test = LoadLibraryA("Client_fack.dll");
		{
			OutputDebug(L"Test1:Start:��ָ�����:��鷵��ֵ");

			
			if (test==0)
			{
				MessageBoxA(0,"������dll","",MB_OK);
			}
			FARPROC fpAdd = GetProcAddress(test,"Add");		//������������
			if (fpAdd==nullptr)
			{
				MessageBoxA(0,"����dll,��������ָ������","",MB_OK);

			}


			int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

			st_argv_Add tmp_Message;
			tmp_Message.firstNumber = 5;
			tmp_Message.secondNumber= 6;
			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==11)
			{
				OutputDebug(L"Test1:Pass:");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug(L"Test1:Fault:");
				MessageBoxA(0,"���Է���ֵ����","",MB_OK);
			}
			//FreeLibrary(test);
			//test = 0;
			
		}
/*
2. ��ָ���������鷵��ֵ��ָ���޸����
- ��׼ģʽ
- ����ģʽ
*/
		{
			
		}
/*
- �첽��
1. �޻ص�
*/
		{
			OutputDebug(L"Test3:Start:�첽.�޻ص�");

			if (test==0)
			{
				MessageBoxA(0,"������dll","",MB_OK);
			}
			FARPROC fpAdd = GetProcAddress(test,"Add_Async_NoCallback");		//������������
			if (fpAdd==nullptr)
			{
				MessageBoxA(0,"����dll,��������ָ������","",MB_OK);

			}


			int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

			st_argv_Add tmp_Message;
			tmp_Message.firstNumber = 5;
			tmp_Message.secondNumber= 6;
			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==true)
			{
				OutputDebug(L"Test3:Pass:");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug(L"Test3:Fault:");
				MessageBoxA(0,"���Է���ֵ����","",MB_OK);
			}
			//FreeLibrary(test);
			test = 0;
		}
/*
2. �лص������ص���Ϣ���
*/
		{

		}

	}


	return 0;
}

void WeatherCallBack(const char* cp,int len)
{
	if (len==0)
	{
		return;
	}
	int destlen = len+1;
	char* p = new char[destlen]();
	strcpy_s(p,destlen,cp);
	MessageBoxA(0,p,"�������",MB_OK);

	return ;


}