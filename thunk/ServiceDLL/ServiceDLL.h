// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SERVICEDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SERVICEDLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
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
typedef void (_cdecl* RPC_CallBack)(const char*,int len);//RPC�ص�ԭ��
typedef  int (__cdecl *int_FUN_Standard)(char* ,RPC_CallBack callBack);//��׼

#include <windows.h>
#include <stdio.h>
//����1�� ͬ������ָ�룬����ֵ
extern"C" __declspec(dllexport)int Add(st_argv_Add* p,char* cb)
{
	if (cb!=nullptr)
	{
		printf("err:Add cb!=nullptr");
	}
	
	return p->firstNumber+p->secondNumber;

}


//����3�� �첽����ָ�룬�޻ص�
extern"C" __declspec(dllexport)int Add_Async_NoCallback(st_argv_Add* p,char* cb)
{
	if (cb!=nullptr)
	{
		printf("err:Add_Async_NoCallback cb!=nullptr");
	}
	char str[10] ={0};
	sprintf_s(str,"%d",p->firstNumber+p->secondNumber);
	MessageBoxA(0,str,"Result is",MB_OK);
	return 0;

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