// Client.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "Client.h"
#include "../public/debug.h"


#include "../public/WEB.h"
#include "WEB_Client.h"
#include "../public/FunctionInfo.h"
#include "../public/DataFormat.h"
#include "../public/StateManage.h"


// ���ǵ���������һ��ʾ��
CLIENT_API LONG64 nClient=0;

// ���ǵ���������һ��ʾ����
CLIENT_API LONG64 fnClient(void)
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
	LONG64 lpText_len;
	_In_opt_ LPCSTR lpCaption;
	LONG64 lpCaption_len;
	_In_ UINT uType;

};


CFunctionInfo* g_CI_Client = new CFunctionInfo();
//���ݸ�ʽ
CDataFormat g_CDF;
//�ͻ��� �첽״̬����
CAsyncStateManage* g_pasm = new CAsyncStateManage();
//�ͻ��� ͬ��״̬����
CSyncStateManage* g_pssm = new CSyncStateManage();
//�������
CWEB* pCWEB =new CWEB_Client();
//�Ƿ��������˳�״̬
HANDLE EXITING_EVENT = CreateEvent(NULL,TRUE,FALSE,NULL);;

/*
����Fake�����ļ��е�

1. ��ѯSN���첽�񣨵ڶ���������,���ݽṹ��С����һ��������
~~Ҫ֪�������е� struct������ָ���offset,�ͳ��ȵ�offset~~

0. ������(����)
1. SN
2. �첽��
3. �ṹ���м���ָ��(�涨ָ������ڽṹ���п�ǰ

*/
LONG64 Core(LONG64 SN,PVOID pStruct,FARPROC callBack)
{
	{
		DWORD dw = WaitForSingleObject(EXITING_EVENT,0);
		if (dw==WAIT_TIMEOUT)
		{
			//û�д���//��������
			;
		}
		else if (dw==WAIT_OBJECT_0)
		{
			//�Ѿ�����,�����˳���������ֱ�ӷ��ء�

			return -1;

		}
	}



	/*
	�������ƿ��� ��Ҫ�����ж�һ��ָ�������?�����ɸ�ʽ����������һվ����

	�������ؿ��� ֻ��ͬ��Ӵ

	�ȴ���ʽȡ���� �첽��


	ͬ�첽����Ժϳ�һ���������Ӱ�졣

	*/
	//���Ͳ�����ʽͳһ
	try
	{
		LONG64 m_pointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);
		LONG64 m_sizeOfStruct = g_CI_Client->QueryArgvStructSize(SN);


		LONG ID_proc = CID_Manager::GetNewID();

		//ͬ�첽������ͬ�ķ��ط�ʽ
		const bool async =g_CI_Client->QueryASync(SN);


		//���᷵��0����������һЩ��Ľṹ��
		LONG64 realBufferLen = g_CDF.Format2Flow(ID_proc,SN,(char*)pStruct,m_sizeOfStruct,m_pointerNumber,
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
			return TRUE;

		}
		else//ͬ������
		{
			LONG64 ret = 0;//����ֵ
			LONG64 PointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);

			//Event
			HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);


			g_pssm->push(ID_proc,&ret,(char*)pStruct,PointerNumber,hdEvent);

			WaitForSingleObject(hdEvent,INFINITE);
			CloseHandle(hdEvent);
			return ret;
		}
	}
	catch(char* s)
	{
		OutputDebug("Error:%s",s);
		throw("");
	}
	catch(...)
	{
		OutputDebug("Error:Happen some error.");
		throw("");
	}



}

//��CloseModuleǰ����Ҫ
//���ñ��������˳�����RPC����
extern"C" __declspec(dllexport)LONG64 ExitAllWork(PVOID,FARPROC)
{
	//1. �رձ������
	SetEvent(EXITING_EVENT);
	/*
	1. ���������,�ر�����ڣ�ֹͣRecive����Ȼ����һЩ�߼�©��������ʼ����ʱ���޷���֤û�����������м����С�
	2. ���ٴ����ͣ�pCWEB.push ջ
	3. �Ѿ��������ݴ�����Revice�գ�

	4. ����δ���յȴ�.������

	
	*/
	
	
	
	/*2. ���������Ѿ����ܵ�����ȫ����ɷ���*/
	
	while(!pCWEB->AllSendIsFinish())
	{
		Sleep(100);
	}

	/*3. MailSlot������ ��ȷ�����������½��յ����ݶ��Ѿ������߳̿�ʼ���ǵĹ�����*/
	//�������
	delete(pCWEB);
	pCWEB = nullptr;

	//�ͻ��� ������Ϣ
	delete(g_CI_Client);
	g_CI_Client = nullptr;

	/*4. ��΢�ȴ� �Ѿ������������е��߳���ɶ�������Ϣ�Ļ�ȡ�ʹ���
	��ȡ����Flow2Format���ٶȡ�
	FixMe: ��Ҫȷ�еص�֪�����Ѿ�����ʹ�ã��첽״̬��ͬ��״̬��ѯ��
	*/
	Sleep(1000);

	/*5. ����ʣ������*/
	//�ͻ��� �첽״̬����
	delete(g_pasm);
	g_pasm = nullptr;
	//�ͻ��� ͬ��״̬����
	delete(g_pssm);
	g_pssm = nullptr;

	CloseHandle(EXITING_EVENT);
	return true;
}

//0��SN��Ԥ�������½ӿڹ����õġ�

//Fake����
extern"C" __declspec(dllexport)LONG64 Add(PVOID pStruct,FARPROC callBack)
{
	LONG64 m_SN = 1;
	return Core(m_SN,pStruct,callBack);
}

extern"C" __declspec(dllexport)LONG64 Add_Async_NoCallback(PVOID pStruct,FARPROC callBack)
{
	LONG64 m_SN = 2;
	return Core(m_SN,pStruct,callBack);
}

extern"C" __declspec(dllexport)LONG64 Test2_Sync(PVOID pStruct,FARPROC callBack)
{
	LONG64 m_SN = 3;
	return Core(m_SN,pStruct,callBack);
}

extern"C" __declspec(dllexport)LONG64 RealMessageBoxA(PVOID pStruct,FARPROC callBack)
{
	LONG64 m_SN = 4;
	return Core(m_SN,pStruct,callBack);
}



