// Client.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "Client.h"
#include "../public/debug.h"


#include "../public/WEB.h"
#include "../public/FunctionInfo.h"
#include "../public/DataFormat.h"
#include "../public/StateManage.h"


// ���ǵ���������һ��ʾ��
CLIENT_API int nClient=0;

// ���ǵ���������һ��ʾ����
CLIENT_API int fnClient(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� Client.h
CClient::CClient()
{
	return;
}



/*
��		SN	Name����Ҫ
1		1
0		2


������ţ�����һ�����������Ҫ��Key-Value
Map
SN ��




*/

struct st_argv 
{
	_In_opt_ HWND hWnd;
	_In_opt_ LPCSTR lpText;
	int lpText_len;
	_In_opt_ LPCSTR lpCaption;
	int lpCaption_len;
	_In_ UINT uType;

};


CFunctionInfo* g_CI_Client = new CFunctionInfo();
CDataFormat g_CDF;
//�ͻ��� �첽״̬����
CAsyncStateManage* g_pasm = new CAsyncStateManage();
//�ͻ��� ͬ��״̬����
CSyncStateManage* g_pssm = new CSyncStateManage();
CWEB* pCWEB =new CClientWeb();


/*
����Fake�����ļ��е�

1. ��ѯSN���첽�񣨵ڶ���������,���ݽṹ��С����һ��������
~~Ҫ֪�������е� struct������ָ���offset,�ͳ��ȵ�offset~~

0. ������(����)
1. SN
2. �첽��
3. �ṹ���м���ָ��(�涨ָ������ڽṹ���п�ǰ

*/
int Core(int SN,PVOID pStruct,FARPROC callBack)
{
	/*
	�������ƿ��� ��Ҫ�����ж�һ��ָ�������?�����ɸ�ʽ����������һվ����

	�������ؿ��� ֻ��ͬ��Ӵ

	�ȴ���ʽȡ���� �첽��


	ͬ�첽����Ժϳ�һ���������Ӱ�졣

	*/
	//���Ͳ�����ʽͳһ
	int m_pointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);
	int m_sizeOfStruct = g_CI_Client->QueryArgvStructSize(SN);


	LONG ID_proc = CID_Manager::GetNewID();

	//ͬ�첽������ͬ�ķ��ط�ʽ
	const bool async =g_CI_Client->QueryASync(SN);


	//���᷵��0����������һЩ��Ľṹ��
	int realBufferLen = g_CDF.Format2Flow(ID_proc,SN,(char*)pStruct,m_sizeOfStruct,m_pointerNumber,
		CDataFormat::Query_INFO,CDataFormat::QUICK_FLOW_MODE,async,callBack);
	if (0==realBufferLen)
	{
		throw("Core: ToFlow return 0");
	}
	char* flowBuffer = new char[realBufferLen]();
	g_CDF.Format2Flow(ID_proc,SN,(char*)pStruct,m_sizeOfStruct,m_pointerNumber,
		CDataFormat::Query_INFO,CDataFormat::QUICK_FLOW_MODE,async,callBack,_Out_ flowBuffer,realBufferLen);

	//����flowBuffer,realBufferLen

	pCWEB->Send(flowBuffer,realBufferLen);

	delete(flowBuffer);
	flowBuffer = nullptr;


	if (async)//�첽����
	{
		if (nullptr!=callBack)
		{
			g_pasm->push(ID_proc,callBack);
		}
		else
		{
			;//�ջص��ǲ��᷵�ص�
		}
		return true;
		
	}
	else//ͬ������
	{
		int ret;//����ֵ
		int PointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);

		//Event
		HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);


		g_pssm->push(ID_proc,&ret,(char*)pStruct,PointerNumber,hdEvent);

		WaitForSingleObject(hdEvent,INFINITE);
		CloseHandle(hdEvent);
		return ret;
	}



}
//Fake����
int aaa1(PVOID pStruct,FARPROC callBack)
{
	int m_SN = 1;
	return Core(m_SN,pStruct,callBack);
}






//////////////////////////////////////////////////////////////////////////
/*
�������ΨһID����ID�����ڱ�������Ψһ���ϣ�
һ������ŵ��������ӷ�������������֪�������������
*/
class CID_Manager
{
public:
	static LONG GetNewID()
	{
		return InterlockedIncrement(&CID_Manager::m_ID);
	}
private:
	static LONG m_ID;
};
LONG CID_Manager::m_ID =0;


