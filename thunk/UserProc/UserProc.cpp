// UserProc.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <string.h>
#include <windows.h>
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


typedef void (_cdecl* RPC_CallBack)(const char*,int len);//RPC�ص�ԭ��

typedef  int (__cdecl *int_FUN_ADD)(int,int);
typedef  int (__cdecl *int_FUN_REAL)(st_argv_MessageBoxA* p,RPC_CallBack callBack);
typedef  int (__cdecl *int_FUN_Standard)(st_argv_QueryWeather* ,RPC_CallBack callBack);//��׼




void WeatherCallBack(const char*,int len);

int _tmain(int argc, _TCHAR* argv[])
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

	funQueryWeather(&tmp_we,WeatherCallBack);//


	Sleep(6000);

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