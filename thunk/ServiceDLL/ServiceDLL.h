// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SERVICEDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SERVICEDLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#include "../public/debug.h"



#ifdef SERVICEDLL_EXPORTS
#define SERVICEDLL_API __declspec(dllexport)
#else
#define SERVICEDLL_API __declspec(dllimport)
#endif

// �����Ǵ� ServiceDLL.dll ������
class SERVICEDLL_API CServiceDLL {
public:
	CServiceDLL(void);
	// TODO: �ڴ�������ķ�����
};

extern SERVICEDLL_API int nServiceDLL;

SERVICEDLL_API int fnServiceDLL(void);



//////////////////////////////////////////////////////////////////////////
struct st_argv_Add
{
	int firstNumber;
	int secondNumber;
};

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
typedef  int (__cdecl *int_FUN_Standard)(char* ,RPC_CallBack callBack);//��׼

#include <windows.h>
#include <stdio.h>
//����1�� ͬ������ָ�룬����ֵ
extern"C" __declspec(dllexport)int Add(st_argv_Add* p,char* cb)
{
	if (cb!=nullptr)
	{
		OutputDebug(L"err:Add cb!=nullptr");
	}
	
	return p->firstNumber+p->secondNumber;

}


//����3�� �첽����ָ�룬�޻ص�
extern"C" __declspec(dllexport)int Add_Async_NoCallback(st_argv_Add* p,RPC_CallBack cb)
{
	if (cb!=nullptr)
	{
		char str[]={'r','e','s','u','l','t','i','s',':','1','1','\0'};
		cb(str,strlen(str)+1);
	}
	else
	{
		OutputDebug(L"Test3:Pass:\r\n");//MessageBoxA(0,"����ͨ��","",MB_OK);
	}
	/*char str[10] ={0};
	sprintf_s(str,"%d",p->firstNumber+p->secondNumber);
	MessageBoxA(0,str,"Result is",MB_OK);
	*/

	return 0;

}
////����2 ͬ�����������,other_argv_c����
/*  1. ��һָ��Ϊ�գ��ڶ�ָ�벻Ϊ�ա�
	2. ��һָ��Ϊ�գ��ڶ�ָ��Ϊ�ա�
	3. ��һָ�벻Ϊ��-�ڶ�ָ��Ϊ�ա�
	4. ��һָ�벻Ϊ��-��һ��ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
	5. ��һָ�벻Ϊ��-�ڶ���ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
*/

extern"C" __declspec(dllexport)int Test2_Sync(st_argv_test2* p,RPC_CallBack cb)
{
	if (cb!=nullptr)
	{
		OutputDebug(L"Test3:Error:cb!=nullptr");
		return -1;
	}

	switch (p->other_argv_c)
	{
	case 1:
		if (p->firstStr!=nullptr||p->secondStr==nullptr)
		{
			return 11;
		}
		else
		{
			return 1;
		}
		break;
	case 2:
		if (p->firstStr!=nullptr||p->secondStr!=nullptr)
		{
			return 22;
		}
		else
		{
			return 2;
		}
		break;
	case 3:
		if (p->firstStr==nullptr||p->secondStr!=nullptr)
		{
			return 33;
		}
		else
		{
			return 3;
		}
		break;
	case 4:
		if (p->firstStr==nullptr||p->secondStr==nullptr)
		{
			return 4;
		}
		else
		{
			for (int i=0;i<p->firstStr_len;++i)
			{
				p->firstStr[i]=0;
			}
		}
		break;
	case 5:
		if (p->firstStr==nullptr||p->secondStr==nullptr)
		{
			return 5;
		}
		else
		{
			for (int i=0;i<p->secondStr_len;++i)
			{
				p->secondStr[i]=0;
			}
		}
		break;
	default:
		return 99;
		break;
	}


}

//////////////////////////////////////////////////////////////////////////

//MessageBoxA(0,"","",MB_OK)
//MessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType)
struct st_argv 
{
	_In_opt_ HWND hWnd;
	_In_opt_ LPCSTR lpText;
	int lpText_len;
	_In_opt_ LPCSTR lpCaption;
	int lpCaption_len;
	_In_ UINT uType;

};

extern"C" __declspec(dllexport)int Real(st_argv* p,FARPROC callBack)
{
	return MessageBoxA(p->hWnd,p->lpText,p->lpText,p->uType);
}



struct st_argv_QueryWeather
{
	const char*	string;
	int						string_len;

};



typedef void (_cdecl* RPC_CallBack)(const char*,int len);
DWORD WINAPI RPC_Async_Thread(LPVOID);


struct weatch_thread_arg
{
	st_argv_QueryWeather qw;
	RPC_CallBack callback;
};

weatch_thread_arg g_weather_thread_arg;

/*������������*/
/*
����Ĵ��벻����ʵ�ĵ���Ӵ

������������Ӧ���ǣ�
���üٻص�����													------������
�ɼٻص����� ���𣺲��� ���� ���Ͳ���							------������

�ɽ��ܺ��� ���𣺴������ص�ǰ���������߳�						------�ͻ���

�� ���ص�ǰ�������� ������ ���񵽵� ���� ���á��ص���������	------�ͻ���

*/
extern"C" __declspec(dllexport)int QueryWeather(st_argv_QueryWeather* p,FARPROC/*fake*/ callBack)
{
	g_weather_thread_arg.callback =(RPC_CallBack)callBack;
	g_weather_thread_arg.qw = *p;

	CreateThread(0,0,RPC_Async_Thread,&g_weather_thread_arg,0,0);

	return 0;
}


void fakeCallBack()
{
	
}


/*
���ص�ǰ��������

��ָ���������лص�*/
DWORD WINAPI RPC_Async_Thread(LPVOID m_argv)
{
	weatch_thread_arg* argv = (weatch_thread_arg*)m_argv;

	RPC_CallBack proc = argv->callback;
	const char*  result = argv->qw.string;


	Sleep(2000);


	proc(result,strlen(result));

	return 0;
	
}