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
extern"C" __declspec(dllexport)int Add(st_argv_Add* p,char* cb);


//����3�� �첽����ָ�룬�޻ص�
extern"C" __declspec(dllexport)int Add_Async_NoCallback(st_argv_Add* p,RPC_CallBack cb);
////����2 ͬ�����������,other_argv_c����
/*  1. ��һָ��Ϊ�գ��ڶ�ָ�벻Ϊ�ա�
	2. ��һָ��Ϊ�գ��ڶ�ָ��Ϊ�ա�
	3. ��һָ�벻Ϊ��-�ڶ�ָ��Ϊ�ա�
	4. ��һָ�벻Ϊ��-��һ��ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
	5. ��һָ�벻Ϊ��-�ڶ���ָ���������޸ģ��ڶ���ָ�����ݲ�Ϊ�յ������
*/

extern"C" __declspec(dllexport)int Test2_Sync(st_argv_test2* p,RPC_CallBack cb);

//////////////////////////////////////////////////////////////////////////

//MessageBoxA(_In_opt_ HWND hWnd, _In_opt_ LPCSTR lpText, _In_opt_ LPCSTR lpCaption, _In_ UINT uType)
struct st_argv_MessageBoxA 
{
	_In_opt_ LPCSTR lpText;
	int lpText_len;
	_In_opt_ LPCSTR lpCaption;
	int lpCaption_len;

	_In_opt_ HWND hWnd;
	_In_ UINT uType;

};

extern"C" __declspec(dllexport)int RealMessageBoxA(st_argv_MessageBoxA* p,FARPROC callBack)
{
	return MessageBoxA(p->hWnd,p->lpCaption,p->lpText,p->uType);
}



/*����������������*/
/*

������������Ӧ���ǣ�
���üٻص�����													------������
�ɼٻص����� ���𣺲��� ���� ���Ͳ���							------������

�ɽ��ܺ��� ���𣺴������ص�ǰ���������߳�						------�ͻ���

�� ���ص�ǰ�������� ������ ���񵽵� ���� ���á��ص���������	------�ͻ���

*/


