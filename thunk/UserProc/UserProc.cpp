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

/*
��ǰ��֪

1. �ڿͻ����е���������������ǰ����Ҫֱ��ʹ��FreeLibrary(test)��


CFunctionInfo* g_CI_Client;	//�ͻ��� �ӿ���Ϣ����		,������������ڴ�й©
CAsyncStateManage* g_pasm;	//�ͻ��� �첽״̬����		,������������ڴ�й©
CSyncStateManage* g_pssm;	//�ͻ��� ͬ��״̬����		,������������ڴ�й©
CWEB* pCWEB;				//�ͻ��� ��������շ��߳�	,������������߳�������ڴ�����

2. ���齫����ģ���滻�����Լ��ĸ���ģ�顣��Ϊ�˷���ʹ����MailSlot�����������������������ģ�顣

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
			OutputDebug("Test1:Start:��ָ�����:��鷵��ֵ.\r\n");


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
				OutputDebug("Test1:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug("Test1:Fault:");
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
			OutputDebug("Test2:Start:��ָ�����:��鷵��ֵ.\r\n");


			if (test==0)
			{
				MessageBoxA(0,"������dll","",MB_OK);
				return -1;
			}
			FARPROC fpAdd = GetProcAddress(test,"Test2_Sync");		//������������
			if (fpAdd==nullptr)
			{
				MessageBoxA(0,"����dll,��������ָ������","",MB_OK);
				return -2;
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
				OutputDebug("Test2:1:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug("Test2:1:Fault:");
				system("pause");
			}
			//////////////////////////////////////////////////////////////////////////
			//2. �ڶ�ָ��Ϊ�գ��ڶ�ָ��Ϊ�ա�
			tmp_Message.firstStr = nullptr;
			tmp_Message.firstStr_len = 0;
			tmp_Message.secondStr= nullptr;
			tmp_Message.secondStr_len= 0;
			tmp_Message.other_argv_c = 2;
			tmp_Message.f_f = 2.2;


			
			RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==2)
			{
				OutputDebug("Test2:2:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug("Test2:2:Fault:");
				system("pause");
			}

			//////////////////////////////////////////////////////////////////////////
			//3. ��һָ�벻Ϊ��-�ڶ�ָ��Ϊ�ա�
			tmp_Message.firstStr = "FirstStr";
			tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
			tmp_Message.secondStr= nullptr;
			tmp_Message.secondStr_len= 0;
			tmp_Message.other_argv_c = 3;
			tmp_Message.f_f = 3.3;



			RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==3)
			{
				OutputDebug("Test2:3:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
			}
			else
			{
				OutputDebug("Test2:3:Fault:");
				system("pause");
			}
			//////////////////////////////////////////////////////////////////////////
			//4. ��һָ�벻Ϊ��-��һ��ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
			
			//�޸���ø������޸ĵ���
			char* pf4= new char[0x10]();
			char* tpFirstStr = "FirstStr";

			for (int i=0;i<0x10;++i)
			{
				pf4[i] = tpFirstStr[i];
			}
			tmp_Message.firstStr = pf4;
			tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
			tmp_Message.secondStr= "SecondStr";
			tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
			tmp_Message.other_argv_c = 4;
			tmp_Message.f_f = 4.4;
			


			RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==4)
			{
				for (int i=0;i<tmp_Message.firstStr_len;++i)
				{
					if (tmp_Message.firstStr[i]!=0)
					{
						OutputDebug("Test2:4:Fault:Change");
						system("pause");
						goto hah__;
					}
					
				}
				OutputDebug("Test2:4:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
hah__:;
				
			}
			else
			{
				OutputDebug("Test2:4:Fault:");
				system("pause");
			}
			delete(pf4);
			pf4 = nullptr;

			//////////////////////////////////////////////////////////////////////////
			//5. ��һָ�벻Ϊ��-�ڶ���ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
			//�޸���ø������޸ĵ���
			
			tmp_Message.firstStr = "FirstStr";
			tmp_Message.firstStr_len = strlen(tmp_Message.firstStr)+1;
			char* ps5 =  new char[0x10]();
			char* tpStr = "SecondStr";
			
			for (int i=0;i<0x10;++i)
			{
				ps5[i] = tpStr[i];
			}
			tmp_Message.secondStr= ps5;
			tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
			tmp_Message.other_argv_c = 5;
			tmp_Message.f_f = 5.5;



			RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==5)
			{
				for (int i=0;i<tmp_Message.secondStr_len;++i)
				{
					if (tmp_Message.secondStr[i]!=0)
					{
						OutputDebug("Test2:5:Fault:Change");
						system("pause");
						goto h__ah__;
					}

				}
				OutputDebug("Test2:5:Pass:.\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
				h__ah__:;
			}
			else
			{
				OutputDebug("Test2:5:Fault:");
				system("pause");
			}
			delete(ps5);
			ps5 = nullptr;
			//////////////////////////////////////////////////////////////////////////
			
		}
		/*
		- �첽��
		1. �޻ص�
		*/
		{
			OutputDebug("Test3:Start:�첽.�޻ص�.\r\n");

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
				OutputDebug("Test3:Fault:");
				MessageBoxA(0,"����3����ֵ����","",MB_OK);
			}
			//FreeLibrary(test);
			
		}
		/*
		2. �лص������ص���Ϣ���
		*/		
		{
			OutputDebug("Test4:Start:�첽.�ص�.\r\n");

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
				OutputDebug("Test4:Fault:");
				MessageBoxA(0,"����4����ֵ����","",MB_OK);
			}
			/*
			�ڿͻ����е���������������ǰ����Ҫֱ��ʹ��FreeLibrary(test)��
			

			CFunctionInfo* g_CI_Client;	//�ͻ��� �ӿ���Ϣ����		,������������ڴ�й©
			CAsyncStateManage* g_pasm;	//�ͻ��� �첽״̬����		,������������ڴ�й©
			CSyncStateManage* g_pssm;	//�ͻ��� ͬ��״̬����		,������������ڴ�й©
			CWEB* pCWEB;				//�ͻ��� ��������շ��߳�	,������������߳�������ڴ�����
			
			
			*/
			
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
		OutputDebug("Test4:Fault:length");
		return;
	}
	int stat = memcmp(p,cp,len);
	if (stat==0)
	{
		OutputDebug("Test4:Pass:\r\n");
		OutputDebug("All pass.:)��ϲ.");
	}
	else
	{
		OutputDebug("Test4:Fault:memcpy");
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