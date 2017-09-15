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

	//ͬ�첽������ͬ�ķ��ط�ʽ
	const bool async =g_CI->QueryASync(SN);


	//Ӧ�ò��᷵��0����������һЩ��Ľṹ��
	int realBufferLen = g_CDF.ToFlow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,async,callBack);
	if (0==realBufferLen)
	{
		throw("Core: ToFlow return 0");
	}
	char* flowBuffer = new char[realBufferLen]();
	g_CDF.ToFlow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,async,callBack,_Out_ flowBuffer,realBufferLen);

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
	enum e_work_type
	{
		Query_INFO,//����-�ͻ��˷���
		RECV_INFO,//�ظ�-����˷���
	};
	enum e_argv_type_option
	{
		STANDARD_FLOW_MODE,//��׼ģʽ
		QUICK_FLOW_MODE//���ٲ���ģʽ������������Ϊ0,��ʾ�޸Ķ���

	};
	struct st_data_flow
	{
		int		length_of_this_struct;//�����ṹ�ĳ���
		int		work_type;//���ι�������:e_work_type
		long	ID_proc;
		int 	functionID;

		//�Ƿ��첽
		char	async;
		//�Ƿ�����ص�
		char	permit_callback;
		//�����ṹ���ʽ��չ���ã���ǰΪ�ա�����ʽ��0:ȫ������ʽ;1:���ٲ�����ʽ,����������Ϊ0,��ʾ�޸Ķ���
		int		argvTypeOption;	//e_argv_type_option

		/*�����ṹ���ܳ���*/
		int		length_Of_Argv_Struct;
		/*�����ṹ����ָ��ṹ�������*/
		int		number_Of_Argv_Pointer;

		char	argv_Struct[0];   

		/*
		ָ��ṹ�� ��������
		ָ���ǲ����е�ָ��ṹ�������.
		eg:
		struct argv
		{
		char* a
		int len_a;
		char* b
		int len_b;
		char c
		}	
		����ָ��ṹ���������2.




		*/

	};
	struct st_argv_Node_Struct
	{
		int length;
		char data[0];
	};


	/************************************************************************/
	/* �ͻ���ʹ��                                                           */
	/************************************************************************/
	//ת���������ͻ��˱���
	/*
	����ֵ:			ʵ����Ҫ��buffer����
	ID_proc:		ÿ�����ù��̵�Ψһ���
	SN:				ÿ��������Ψһ���
	pStruct:		ָ������ṹ���ָ��
	sizeOfStruct:	�ṹ��ĳ���
	ArgvPointerNumber:�ṹ����ָ�������,��ʽΪ��ǰ����:{ָ��,����}{ָ��,����}
	async:			�Ƿ��첽
	callBack:		ԭ���ú����Ļص�����
	flowBuffer:		��������Ҫ�Ĵ洢��ָ��
	real_len:		������׼���Ĵ洢������

	�쳣:
	����ṩ����ĳ��Ȼ�õ��쳣��

	*/
	int Client_FormatToFlow(LONG ID_proc,int SN,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,bool async,FARPROC callback,char* flowBuffer=nullptr,int real_len=0)
	{
		/*����������Ҫ�ĳ��ȸ��ⲿ
		���real_len!=��ʵ���ȵĻ�,�Ͳ�����,���Ƿ��������ĳ���.
		�����������!=0,�����������㳤��,���쳣.-����ζ�ŷ�����ĳ�ֱ䶯�����μ���֮�䡣

		���ֽ������,�����������ֹ����������������������������΢��Ŀ�խת��ϵ�С�
		������������Ļ����͵��������ַ�����
		1. �����ճ��ȵĺ����������з��䣬�����ߵ��ͷ�һ��������������ڴ棬Ȩ��������������ڴ�й©��
		2. ÿ�β���͵����쳣���ߴ���ֵ�������׽Ӱ�������������
		3. ���������������ȡ���ȡ����뵼�����׳��ִ������


		���裺
		1. ��ȡ��Ҫ�ĳ��ȣ���֪������
		2. У������߳���
		3. �����

		*/
		if (sizeOfStruct<(ArgvPointerNumber*sizeof(int)*2))
		{
			throw("Client_FormatToFlow:Error:argv is not true,...");
		}
		if (ArgvPointerNumber<0)
		{
			throw("Client_FormatToFlow:Error:argv is not true,ArgvPointerNumber<0");
		}

		if (async==false&&callback!=nullptr)
		{
			throw("Client_FormatToFlow: Warming: Sync function can't permit have callback");
		}

		//Ϊʲô��ô�㣿��st_data_flow��st_argv_Node_Struct�ṹ
		const int NO_POINTER_NUMBER = 1;
		int m_real_length = sizeof(st_data_flow) + sizeof(int)*(ArgvPointerNumber+NO_POINTER_NUMBER);

		if (nullptr==pStruct)//�޲���
		{
			if (ArgvPointerNumber!=0||sizeOfStruct!=0)
			{
				throw("Client_FormatToFlow:Error:pStruct==nullptr,and ArgvPointerNumber!=0");
			}
			else
			{
				;
			}
		}
		else
		{
			int* ptmp =(int*) pStruct;
			for (int i=0;i<ArgvPointerNumber;++i)
			{
				int* tmp;
				tmp = ptmp+(i+1)*sizeof(int);
				m_real_length+=*tmp;
			}
		}
		if (0==real_len)
		{
			return m_real_length;
		}
		else if (m_real_length!=real_len)
		{
			throw("Client_FormatToFlow:RealLength != your input length:Data change?");
		}

		if (flowBuffer==nullptr)
		{
			throw("Client_FormatToFlow:FlowBuffer is nullptr");
		}

//�ṹ��ֵ
		st_data_flow* psdf =(st_data_flow*) flowBuffer;

		psdf->length_Of_Argv_Struct = m_real_length;
		psdf->work_type = Query_INFO;
		psdf->ID_proc	= ID_proc;
		psdf->functionID= SN;
		psdf->async		= async;
		psdf->permit_callback = callback?true:false;
		psdf->argvTypeOption = STANDARD_FLOW_MODE;
		
		psdf->number_Of_Argv_Pointer = ArgvPointerNumber;
		/*
		int* ptmp =(int*) pStruct;
		ArgvPointerNumber;
		for (int i=0;i<ArgvPointerNumber;++i)
		{
		int* tmp;
		tmp = ptmp+(i+1)*sizeof(int);
		m_real_length+=*tmp;
		}
		*/
		int* pBase =(int*) pStruct;
		int tmp_length = 0;//Argv_Struct��һ���ֵ��ܳ���
		
		int offset = 0;//��argv_Struct[]�е�ƫ��

		for (int i=0;i<ArgvPointerNumber;++i)
		{
			int* ptmp_length;
			int* ptmp_pointer;
			ptmp_pointer= pBase+i*sizeof(int);
			ptmp_length	= pBase+(i+1)*sizeof(int);

			//check
			if (nullptr == (char*)*ptmp_pointer&&0!=*ptmp_length)
			{
				throw("Client_FormatToFlow:This argv pointer==nullptr,but length call me !=0");
			}
			//copy

			int* plength = (int*)psdf->argv_Struct[offset];
			*plength = *ptmp_length;

			offset += sizeof(int);
			memcpy_s((void*)psdf->argv_Struct[offset],*ptmp_length,(char*)*ptmp_pointer,*ptmp_length);
			
			offset+=*ptmp_length;

			tmp_length+=*ptmp_length;
		}

		tmp_length += ArgvPointerNumber*sizeof(int);//+������Ϣ��ռλ��
		tmp_length += sizeOfStruct-ArgvPointerNumber*sizeof(int)*2+sizeof(int);//�Ͼ���ָ�������ʱҪ��ʼռ����λ�ã����һ���Ӳ���⿴�����ṹ��

		psdf->length_Of_Argv_Struct = tmp_length;

		return m_real_length;


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

	��ʹ������Ҳ���ܱ�֤�����Ѿ������ͣ��������Ǹձ���ջ��ȡ�����ˡ�
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


