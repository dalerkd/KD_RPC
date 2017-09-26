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

struct st_argv_test2
{
	char* firstStr;
	int   firstStr_len;
	char* secondStr;
	int   secondStr_len;
	char  other_argv_c;
	float f_f;
};



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

void Test4Callback(const char* cp,int len);

int _tmain(int argc, _TCHAR* argv[])
{
	bool Yuan = false;//ԭʼ���ԣ�ԭʼ�����Ѿ������ˡ�

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
			OutputDebug(L"Test1:Start:��ָ�����:��鷵��ֵ.\r\n");


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
				OutputDebug(L"Test1:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
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
		*///
		/*
		test 2��other_c����ʲôֵ��ʲô�����
		1. ��һָ��Ϊ�գ��ڶ�ָ�벻Ϊ�ա�
		2. ��һָ��Ϊ�գ��ڶ�ָ��Ϊ�ա�
		3. ��һָ�벻Ϊ��-�ڶ�ָ��Ϊ�ա�
		4. ��һָ�벻Ϊ��-��һ��ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
		5. ��һָ�벻Ϊ��-�ڶ���ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
		*/

		{
			OutputDebug(L"Test2:Start:��ָ�����:��鷵��ֵ.\r\n");


			if (test==0)
			{
				MessageBoxA(0,"������dll","",MB_OK);
			}
			FARPROC fpAdd = GetProcAddress(test,"Test2_Sync");		//������������
			if (fpAdd==nullptr)
			{
				MessageBoxA(0,"����dll,��������ָ������","",MB_OK);

			}


			int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;
			//1. ָ��Ϊ�գ���ͨ����Ϊ�յ������
			st_argv_test2 tmp_Message;
			tmp_Message.firstStr = nullptr;
			tmp_Message.firstStr_len = 0;
			tmp_Message.secondStr= "SecondStr";
			tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
			tmp_Message.other_argv_c = 1;
			tmp_Message.f_f = 1.1;



			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==1)
			{
				OutputDebug(L"Test2:1:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug(L"Test2:1:Fault:");
			}
			//////////////////////////////////////////////////////////////////////////
			//2. �ڶ�ָ��Ϊ�գ��ڶ�ָ��Ϊ�ա�
			tmp_Message.firstStr = nullptr;
			tmp_Message.firstStr_len = 0;
			tmp_Message.secondStr= nullptr;
			tmp_Message.secondStr_len= 0;
			tmp_Message.other_argv_c = 2;
			tmp_Message.f_f = 2.2;


			
			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==2)
			{
				OutputDebug(L"Test2:2:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug(L"Test2:2:Fault:");
			}

			//////////////////////////////////////////////////////////////////////////
			//3. ��һָ�벻Ϊ��-�ڶ�ָ��Ϊ�ա�
			tmp_Message.firstStr = "FirstStr";
			tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
			tmp_Message.secondStr= nullptr;
			tmp_Message.secondStr_len= 0;
			tmp_Message.other_argv_c = 3;
			tmp_Message.f_f = 3.3;



			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==3)
			{
				OutputDebug(L"Test2:3:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug(L"Test2:3:Fault:");
			}
			//////////////////////////////////////////////////////////////////////////
			//4. ��һָ�벻Ϊ��-��һ��ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
			tmp_Message.firstStr = "FirstStr";
			tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
			tmp_Message.secondStr= "SecondStr";
			tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
			tmp_Message.other_argv_c = 4;
			tmp_Message.f_f = 4.4;



			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==4)
			{
				for (int i=0;i<tmp_Message.firstStr_len;++i)
				{
					if (tmp_Message.firstStr[i]!=0)
					{
						OutputDebug(L"Test2:4:Fault:Change");
						goto hah__;
					}
					
				}
				OutputDebug(L"Test2:4:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
				hah__:
			}
			else
			{
				OutputDebug(L"Test2:4:Fault:");
			}
			//////////////////////////////////////////////////////////////////////////
			//5. ��һָ�벻Ϊ��-�ڶ���ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
			tmp_Message.firstStr = "FirstStr";
			tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
			tmp_Message.secondStr= "SecondStr";
			tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
			tmp_Message.other_argv_c = 5;
			tmp_Message.f_f = 5.5;



			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==5)
			{
				for (int i=0;i<tmp_Message.secondStr_len;++i)
				{
					if (tmp_Message.secondStr[i]!=0)
					{
						OutputDebug(L"Test2:5:Fault:Change");
						goto h__ah__;
					}

				}
				OutputDebug(L"Test2:5:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
				h__ah__:
			}
			else
			{
				OutputDebug(L"Test2:2:Fault:");
			}
			//////////////////////////////////////////////////////////////////////////

		}
		/*
		- �첽��
		1. �޻ص�
		*/
		{
			OutputDebug(L"Test3:Start:�첽.�޻ص�.\r\n");

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
				;
			}
			else
			{
				OutputDebug(L"Test3:Fault:");
				MessageBoxA(0,"����3����ֵ����","",MB_OK);
			}
			//FreeLibrary(test);
			
		}
		/*
		2. �лص������ص���Ϣ���
		*/		
		{
			OutputDebug(L"Test4:Start:�첽.�ص�.\r\n");

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
			int RealResult = funAdd((char*)&tmp_Message,Test4Callback);
			if (RealResult==true)
			{
				;
			}
			else
			{
				OutputDebug(L"Test4:Fault:");
				MessageBoxA(0,"����4����ֵ����","",MB_OK);
			}
			//FreeLibrary(test);
			
		}

	}
	Sleep(-1);

	return 0;
}

void Test4Callback(const char* cp,int len)
{
	char p[] = {'r','e','s','u','l','t','i','s',':','1','1','\0'};
	if (len!=(strlen(p)+1))
	{
		OutputDebug(L"Test4:Fault:length");
		return;
	}
	int stat = memcmp(p,cp,len);
	if (stat==0)
	{
		OutputDebug(L"Test4:Pass:");
	}
	else
	{
		OutputDebug(L"Test4:Fault:memcpy");
	}
	return;
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