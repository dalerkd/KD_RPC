// Client.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "Client.h"


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

/*
����Fake�����ļ��е�

1. ��ѯSN���첽�񣨵ڶ���������,���ݽṹ��С����һ��������
~~Ҫ֪�������е� struct������ָ���offset,�ͳ��ȵ�offset~~

0. ������(����)
1. SN
2. �첽��
3. �ṹ���м���ָ��(�涨ָ������ڽṹ���п�ǰ

*/
int Core(int SN,PVOID* pStruct,FARPROC callBack)
{
	/*
		�������ƿ��� ��Ҫ�����ж�һ��ָ�������?�����ɸ�ʽ����������һվ����

		�������ؿ��� ֻ��ͬ��Ӵ

		�ȴ���ʽȡ���� �첽��


		ͬ�첽����Ժϳ�һ���������Ӱ�졣

	*/
//���Ͳ�����ʽͳһ
	int m_pointerNumber = g_CI->QueryArgvPointerNumber(SN);
	int m_sizeOfStruct = g_CI->QueryArgvStructSize(SN);


	LONG ID_proc = CID_Manager::GetNewID();

	//Ӧ�ò��᷵��0����������һЩ��Ľṹ��
	int realBufferLen = g_CDF.ToFlow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber);
	if (0==realBufferLen)
	{
		throw("Core: ToFlow return 0");
	}
	
	char* flowBuffer = new char[realBufferLen]();
	
	

	g_CDF.ToFlow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,_Out_ flowBuffer,realBufferLen);

	//����flowBuffer,realBufferLen
	/*
	���ͻ���������Щ��ġ����˴�ͳ��ֱ�ӵ��÷��ͣ�
	
	��һ���뷨�ǽ����ݷ��ڷ���ջ��,���ڷ�����������ǻ���ģ��Ҽ��ջ��û������
	�������ߣ�������ģ�͡���������
	�ô��ǣ�
	
	1. ���緢�Ͳ����ĵ��õ�ʱ�䡣�÷��ط��ء�
	2. ʹ�����á��͡����緢�͡����������໥����������֮���Ҳ������й������ɵĲ㡣
	3. ����������ٶȣ�������ʵ�Ǹ���ռ���̣�����ʱ���ڵ������ӣ��ᵼ�´�Ҷ��ȴ���

	ȱ���ǣ�
	1. ����һ�θ�����������Ҫ���ڴ治���͡�����
	2. ������ôЩ�����裬�ƺ�Ч���������͡�

	
	*/
	pCWEB->Send(flowBuffer,realBufferLen);

	delete(flowBuffer);
	flowBuffer = nullptr;


//ͬ�첽������ͬ�ķ��ط�ʽ
	bool async =g_CI->QueryASync(SN);

	if (async)//�첽����
	{
		pasm->push(ID_proc,callBack);
		return true;
	}
	else//ͬ������
	{
		int ret;//����ֵ
		int PointerNumber = g_CI->QueryArgvPointerNumber(SN);
		
		//Event
		HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);
		
		
		pssm->push(ID_proc,&ret,pStruct,PointerNumber,hdEvent);

		WaitForSingleObject(hdEvent,INFINITE);
		CloseHandle(hdEvent);
		return ret;
	}



}
//Fake����
int aaa1(PVOID* pStruct,FARPROC callBack)
{
	int m_SN = 1;
	Core(m_SN,pStruct,callBack);
}

CFunctionInfo* g_CI = new CFunctionInfo();

class CFunctionInfo
{
public:
	
	void AddInfo(int SN,char* funcName,bool asyn,int ArgvStringNumber,int sizeOfStruct/*�����ṹ���С*/)
	{

	}

	bool QueryASync(int query_SN)//�Ƿ��첽,true �첽,false ͬ��
	{

	}
	int  QueryArgvPointerNumber(int query_SN)//�����ṹ����ָ�����������:ָ��A������A��ָ��B������B......����Ϊ2
	{

	}
	int  QueryArgvStructSize(int query_SN)//�����ṹ���ܹ��೤�������������ڱ���ʱ���ṩ��
	{

	}
private:
};

/*
�����ͷ�������ʽ֮���ת��

*/
CDataFormat g_CDF;
class CDataFormat
{
public:
/************************************************************************/
/* �ͻ���ʹ��                                                           */
/************************************************************************/
	//ת���������ͻ��˱���
	/*
	����ֵ:			ʵ����Ҫ��buffer����
	pStruct:		ָ������ṹ���ָ��
	sizeOfStruct:	�ṹ��ĳ���
	ArgvPointerNumber:�ṹ����ָ�������,��ʽΪ��ǰ����:{ָ��,����}{ָ��,����}
	flowBuffer:		��������Ҫ�Ĵ洢��ָ��
	real_len:		������׼���Ĵ洢������

	�쳣:
	����ṩ����ĳ��Ȼ�õ��쳣��

	*/
	int Client_FormatToFlow(LONG ID_proc,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,char* flowBuffer=nullptr,int real_len=0)
	{
		/*����������Ҫ�ĳ��ȸ��ⲿ
		���real_len!=��ʵ���ȵĻ�,�Ͳ�����,���Ƿ��������ĳ���.
		�����������!=0,�����������㳤��,���쳣.-����ζ�ŷ�����ĳ�ֱ䶯�����μ���֮�䡣

		���ֽ������,�����������ֹ����������������������������΢��Ŀ�խת��ϵ�С�
		������������Ļ����͵��������ַ�����
		1. �����ճ��ȵĺ����������з��䣬�����ߵ��ͷ�һ��������������ڴ棬Ȩ��������������ڴ�й©��
		2. ÿ�β���͵����쳣���ߴ���ֵ�������׽Ӱ�������������
		3. ���������������ȡ���ȡ����뵼�����׳��ִ������

		*/
	}
	
	//��ת���ɸ�ʽ:�ͻ��˽��룺����һ���̺߳���,��Ϊ������������Ϊջ��������������ź��ǡ�
	/*
		
	*/
	//void Client_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int& structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	void Client_FlowToFormat_Execute(char* flow,int flow_len);//���������������Ƿ��첽,���߲�ͬ�����̡�

/************************************************************************/
/* �����ʹ��															*/
/************************************************************************/
	
	//�첽��ʽת������:����˱���
	void Service_FormatToFlow_Async();

	//ͬ����ʽת������:����˱���
	void Service_FormatToFlow_Sync();



	
	struct  st_thread_Service_FlowToFormat_Excute
	{
		char* flow;
		int flow_len;
		HANDLE �ź�;//������������������Ͼʹ�������źš�
	};

	//��ת���ɸ�ʽ:����˽��룺����һ���̺߳�����
	//void Service_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	DWORD Service_FlowToFormat_Execute(st_thread_Service_FlowToFormat_Excute* p)
	{

	}
};//�����ҵ�֪��ͬ�첽��������ǵķ������ݸ�ʽ�ǲ�ͬ�ģ�˭��������ָ���ڴ����������ٱȽϺ��ʣ�
/*
��������Щ���������˭����������ܺܶ���ڴ棿�͹������ǵ�ָ�룿
һ�ַ�������Service_FlowToFormat��������£�������ֱ�ӵ���Ŀ�����

Ŀ����񷵻ص�ʱ�򣬽������١�

Ŀ��������Ҫ���أ����ǣ������ܾܺòŷ��أ����������ʺ����߳�����

����Service_FlowToFormat��һ���̺߳������ȽϺ��ʡ�


*/







/*
����㣺
ʵ�����¹��ܣ�
	���첽�������ݡ�����������-������ģ�ͣ�ֻ��ջ����ʱ�������û���
	�ȴ�ȫ���������:�ýӿ��ṩһ��֪ͨ�����������ݷ�����ϵ�ʱ�򴥷������ڳ����˳��ȡ�
	
����Ϣʱ,�����ͬ�������𡣡�����
	����Ϣ��ʱ������Ǹ�����Ϣ
							����CFunctionInfo����.
				�����������Ϣ
							push��Stack���������Stack�����������������

*/
CWEB* pCWEB =new CWEB();
class CWEB
{
public:
	CWEB()
	{
		m_CSQ = new CSafeQueue<st_asysnc_queue_argv>();
	}
	~CWEB()
	{
		delete(m_CSQ);
	}

public:
	struct st_asysnc_queue_argv 
	{
		char* data;
		int data_len;
	};


	/*�첽����
	1. �����ݸ��Ƶ��Լ�����Ŀռ�
	2. ���浽ջ��
	3. ����
	*/
	void Send(char* data,int data_len)//��ʵ�Ǹ�push����.
	{
		if (data_len<=0)
		{
			throw("CWEB::Send:data_len<=0");
		}
		char* p = new char[data_len]();
		memcpy_s(p,data_len,data,data_len);

		st_asysnc_queue_argv tmpArgv = {p,data_len};
		m_CSQ->push(tmpArgv);

		return;
	}
	/*��ǰʱ�̣��Ƿ�ȫ���������
	�ɱ������˳�ǰȷ�������Ƿ�����ϡ�
	ע�⣺�޷���֤����̲߳��������ݣ�����ÿ���̶߳�����������
	*/
	bool AllSendIsFinish()
	{
		return m_CSQ->IsEmpty();
	}
	
private:
	/*������******�����쳣��˵������ exception::char*
	��;������һ����������
	*/
	st_asysnc_queue_argv findAndPop()
	{
		return m_CSQ->pop();
	}

private:
	CSafeQueue<st_asysnc_queue_argv>* m_CSQ;//��������ջ
};

//�ͻ��� �첽״̬����
CAsyncStateManage* pasm = new CAsyncStateManage();

class CAsyncStateManage
{
public:
	CAsyncStateManage()
	{
		m_safemap = new CSafeMap<FARPROC>;
	}
	~CAsyncStateManage()
	{
		delete(m_safemap);
	}

public:
	//ID_proc:Ψһ�ĺ����������ֱ��ε��á�
	void push(LONG ID_proc,FARPROC p)
	{
		m_safemap->push(ID_proc,p);
	}
	void findAndPop(LONG ID_proc,FARPROC& p)
	{
		p = m_safemap->pop(ID_proc);
	}
private:
	CSafeMap<FARPROC>* m_safemap;
};


//�ͻ��� ͬ��״̬����
CSyncStateManage* pssm = new CSyncStateManage();

class CSyncStateManage
{
public:
	CSyncStateManage()
	{
		m_safemap = new CSafeMap<st_CYSM>();
	}
	~CSyncStateManage()
	{
		delete(m_safemap);
	}
public:
	struct  st_CYSM
	{
		int* ret;
		char* pStruct;
		int PointerNumber;
		HANDLE hdEvent;
	};
	//ID_proc:Ψһ�ĺ����������ֱ��ε��á�
	void push(LONG ID_proc,int* ret,char* pStruct,int PointerNumber,HANDLE hdEvent)
	{
		st_CYSM tmp={ret,pStruct,PointerNumber,hdEvent};
		m_safemap->push(ID_proc,tmp);
		return;
	}

	void findAndPop(LONG ID_proc,int* ret,char* pStruct,int& PointerNumber,HANDLE& hdEvent)
	{
		st_CYSM tmp = m_safemap->pop(ID_proc);

		ret = tmp.ret;
		pStruct = tmp.pStruct;
		PointerNumber = tmp.PointerNumber;
		hdEvent = tmp.hdEvent;

		return;
	}
private:
	CSafeMap<st_CYSM>* m_safemap;
};

//////////////////////////////////////////////////////////////////////////
//�̰߳�ȫ�Ķ���,push��β����,pop���ײ���
//�쳣�׳����ڶ����Ѿ��յ�����»�Ҫpop��
#include <iostream>
#include <queue>
using std::queue;
template <typename TSafeQueue>
//��ȫ����
class CSafeQueue
{
public:
	CSafeQueue()
	{
		InitializeCriticalSection(&g_csSafeThread);
	};
	~CSafeQueue()
	{
		DeleteCriticalSection(&g_csSafeThread);
	}

public:
	void push(TSafeQueue data)
	{
		EnterCriticalSection(&g_csSafeThread);
		m_queue.push(data);
		LeaveCriticalSection(&g_csSafeThread);
	}
	TSafeQueue pop()//��β
	{
		EnterCriticalSection(&g_csSafeThread);
		if (m_queue.empty())
		{
			LeaveCriticalSection(&g_csSafeThread);
			throw("CSafeQueue:empty");
		}
		TSafeQueue temp = m_queue.front();
		m_queue.pop();
		LeaveCriticalSection(&g_csSafeThread);

		return temp;
	}
	/*
	ĳһʱ�̶����Ƿ�Ϊ��.
		�������˳�ǰ����Լ��̵߳������Ƿ�������ˡ�
		��Ȼ���ܱ�֤����߳�������������
	*/
	bool IsEmpty()
	{
		bool ret;
		EnterCriticalSection(&g_csSafeThread);
		ret = m_queue.empty();
		LeaveCriticalSection(&g_csSafeThread);
		return ret;
	}
private:
	queue<TSafeQueue> m_queue;
	CRITICAL_SECTION g_csSafeThread;
};


//////////////////////////////////////////////////////////////////////////
//����map<ID_proc,struct>
//�쳣�׳����������������ʱӦ�����ԡ�
#include <map>
using std::map;
template <typename TSafeMapData>
//��ȫmap
class CSafeMap
{
public:
	CSafeMap()
	{
		InitializeCriticalSection(&g_csSafeThread);
	};
	~CSafeMap()
	{
		DeleteCriticalSection(&g_csSafeThread);
	}

public:
	void push(LONG id,TSafeMapData data)
	{
		EnterCriticalSection(&g_csSafeThread);
		m_map[id]=data;

	}
	TSafeMapData pop(LONG id)
	{
		EnterCriticalSection(&g_csSafeThread);
		map<LONG,TSafeMapData>::iterator pos = m_map.find(id);
		if (pos != m_map.end())
		{
			TSafeMapData ret= m_map[id];
			m_map.erase(pos);
			LeaveCriticalSection(&g_csSafeThread);
			return ret;
		}
		else
		{
			LeaveCriticalSection(&g_csSafeThread);
			throw("CSafeMap: key is non-existent!");
		}
	}

private:
	map<LONG,TSafeMapData> m_map;
	CRITICAL_SECTION g_csSafeThread;
};

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


