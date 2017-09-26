#include "stdafx.h"
#include "Data_Format_Service.h"
#include "../public/FunctionInfo.h"
#include "../public/debug.h"
#include "../public/WEB.h"


struct st_Async_Thread_Callback 
{
	LONG ID_proc;
	int function_ID;
};
//����� �첽�ص�����,�߳�IDΪKey,Ҫ���͵�����Ϊvalue.
extern CSafeMap<DWORD,st_Async_Thread_Callback>* g_Async_Thread_Callback;

extern CFunctionInfo* g_CI_Service;
extern CData_Format_Service g_CDF;
extern CWEB* pCWEB;


CData_Format_Service::CData_Format_Service(void)
{
}


CData_Format_Service::~CData_Format_Service(void)
{
}



unsigned int WINAPI CData_Format_Service::Service_FlowToFormat_Execute(LPVOID lp)
{

	st_thread_FlowToFormat_Excute_argvs* p=(st_thread_FlowToFormat_Excute_argvs*) lp;

	if (nullptr == p)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:Input pointer==nullptr");
		return -1;
	}

	if (nullptr == p->flow)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:Input struct ponter==nullptr");
		return -2;
	}

	st_data_flow* pTmp_Flow = (st_data_flow*)p->flow;

	if (pTmp_Flow->length_of_this_struct!=p->flow_len)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:Input struct Format Error:flow_LEN!=length_of_this_struct");
		return -0x10;
	}

	//copy ָ��
	const st_data_flow* pFlowBase =(st_data_flow*) new char[p->flow_len]();

	int stat = memcpy_s((char*)pFlowBase,p->flow_len,p->flow,p->flow_len);
	if (stat)
	{
		throw("memcpy_s return err.");
	}

	//֪ͨ �������
	if(0==SetEvent(p->hdEvent_Copy_Finish))
	{
		throw("Event is invalid");
	}
	//��ָ������Ч
	pTmp_Flow = nullptr;
	p = nullptr;//p��Ч�ˡ�

	//////////////////////////////////////////////////////////////////////////
	/*Flow2Format*/

	char* pArgvCall  = nullptr;
	char* pSecondCopyArgv = nullptr;
	CSafeQueueAutoPointerManage* queue_memory_manage = nullptr;
	CSafeQueueAutoPointerManage* queue_memory_copy	 = nullptr;
	const bool bAsync = pFlowBase->async;
	try
	{
		int real_len=Flow2Format((char*)pFlowBase,pFlowBase->length_of_this_struct,nullptr,0,nullptr,nullptr,nullptr);
		if (real_len==0)
		{
			OutputDebug(L"real_len=0?");
			throw("real_len=0?");
		}

		
		//������
		pArgvCall = new char[real_len]();



		queue_memory_manage = new CSafeQueueAutoPointerManage();
		if (false == bAsync&& RECV_INFO==pFlowBase->work_type)
		{
			pSecondCopyArgv = new char[real_len]();
			queue_memory_copy = new CSafeQueueAutoPointerManage();
		}

		Flow2Format((char*)pFlowBase,pFlowBase->length_of_this_struct,pArgvCall,real_len,queue_memory_manage,pSecondCopyArgv,queue_memory_copy);


	}
	catch (char* string)
	{
		throw(string);//�ڲ��ṹ����
	}
	catch (int errCode)//�ļ���ʽ�������֮
	{
		OutputDebug(L"Flow Format Err,code:0x%x",errCode);
		return errCode;
	}
//////////////////////////////////////////////////////////////////////////


	/*
	0. ��ѯ�������ƣ����ú���
	1. �����ͬ����������޸�
	�ͷ���ֵ������������
	2. �ͷ�����ָ��ͨ��ջ
	*/
	int funName_Len = 0;
	funName_Len = g_CI_Service->QueryFuncName(pFlowBase->functionID);
	char* pFunctionName = new char[funName_Len]();
	g_CI_Service->QueryFuncName(pFlowBase->functionID,pFunctionName);


	if (pFunctionName==nullptr)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:query functionID Do'nt find Name,functionID:%d",pFlowBase->functionID);
		return -0x21;
	}
	


	//����Ĵ�����ҪǨ�Ƶ����߲�ȫ��ȥ��
	char* dllName = "ServiceDLL.dll";
	HMODULE hServiceDLL = LoadLibraryA(dllName);
	if (NULL==hServiceDLL)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:Load library fault,DLLName:%s",dllName);
		return -0x31;
	}

	//��ȡ������
	FARPROC CalledFunction = GetProcAddress(hServiceDLL,pFunctionName);
	
	if (nullptr == CalledFunction)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:GetProcAddress fault,DLLName:%s,functionName:%s",dllName,pFunctionName);
		delete(pFunctionName);
		pFunctionName = nullptr;
		return -0x41;
	}
	delete(pFunctionName);
	pFunctionName = nullptr;
	


	typedef void (*callback_fake)(char*,int);
	typedef int  (*called_function)(char*,callback_fake);

	called_function Service_Work_Function=(called_function)CalledFunction;

	if (bAsync)//�첽
	{
		
		//����callback;
		callback_fake callback=nullptr;
		if (pFlowBase->permit_callback)
		{
			callback = ServiceAsyncCallBack;
			/*��ջ��������ݣ���ʹ α�ص� �ܹ�֪������ID_proc,id��*/
			st_Async_Thread_Callback tmp = {pFlowBase->ID_proc,pFlowBase->functionID};
			g_Async_Thread_Callback->push(GetCurrentThreadId(),tmp);

		}

		Service_Work_Function(pArgvCall,callback);//�����ķ���ֵ			
	}
	else//ͬ����1.��鷵��ֵ��2.����Ƿ����޸Ĳ�����3.ת��������4.����
	{
		const int ret_value = Service_Work_Function(pArgvCall,nullptr);

		/*
		�ñ�����������

		����ģʽ
		1. ���Ա�:queue�ͱ���
		2. ���ͱ���
		��׼ģʽ
		1. ֱ�ӷ��ͱ���



		*/
		if (QUICK_FLOW_MODE==pFlowBase->argvTypeOption)
		{/*
		 ���õ��ò����Ľṹ��
		 1. ����޸ģ�
		 2. һ��û�޸ģ����ͷ�ָ�����0.	
		 */

			//���쳣������������������
			//ָ��Ϊ��,����Ϊ0,����ָ��ҲΪ�ա�����һ��Ϊ���Ǵ���

			if ((pFlowBase->number_Of_Argv_Pointer!=0)&&(pSecondCopyArgv==nullptr))
			{
				OutputDebug(L"Service_FlowToFormat_Execute:Error:Unexpected logic.");
				throw("Service_FlowToFormat_Execute:Error:Unexpected logic.");
			}


			//��תʹ�ý��˶���
			CSafeQueue<char*>* tmp_queue = new CSafeQueue<char*>();

			for (int offset = 0,i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
			{
				char* p_new_data = nullptr;
				try
				{ 
					p_new_data = queue_memory_manage->pop();
					tmp_queue->push(p_new_data);

					;}
				catch(...)
				{
					OutputDebug(L"Service_FlowToFormat_Execute:Error:Unexpected:\
								 queue_memory_manage's number is Incorrect is 0x%x,\
								 true number is:0x%x",i,pFlowBase->number_Of_Argv_Pointer);
					throw("Service_FlowToFormat_Execute:Error:Unexpected:\
						  queue_memory_manage's number is Incorrect");

				}
				char* p_old_data = (char*)(*(int*)(pSecondCopyArgv+offset));
				offset+=sizeof(int);
				int	data_len	 = *(int*)(pSecondCopyArgv+offset);


				if (!((p_new_data!=nullptr&&p_old_data!=nullptr&&data_len!=0)||(p_new_data==nullptr&&p_old_data==nullptr&&data_len==0)))
				{
					OutputDebug(L"Service_FlowToFormat_Execute:Error:Unexpected value,\
								 p_new_data:0x%x;p_old_data:0x%x;data_len:0x%x.",p_new_data,p_old_data,data_len);
					throw("Service_FlowToFormat_Execute:Error:Unexpected value.");
				}

				if (nullptr!=p_old_data)
				{
					int ret = memcmp(p_old_data,p_new_data,data_len);
					if (0==ret)//�޸Ķ�
					{
						delete(p_old_data);
						(*(int*)(pSecondCopyArgv+offset)) = 0;//ָ��
					}
				}


				offset+=sizeof(int);
			}

			//��ԭ����Ա������е��޸�ʹ��,Ϊ��δ�����ܵ�ʹ�á�
			for (int i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
			{
				try{queue_memory_manage->push(tmp_queue->pop());}
				catch(...)
				{
					OutputDebug(L"Service_FlowToFormat_Execute:Error:Unexpected2:\
								 queue_memory_manage's number is Incorrect is 0x%x,\
								 true number is:0x%x",i,pFlowBase->number_Of_Argv_Pointer);
					throw("Service_FlowToFormat_Execute:Error:Unexpected:\
						  queue_memory_manage's number is Incorrect.2");
				}

			}

			delete(tmp_queue);
			tmp_queue = nullptr;



		}
		else if (STANDARD_FLOW_MODE==pFlowBase->argvTypeOption)
		{
			/*
			ͬ�����õ��ò����Ľṹ
			*/
			;
		}
		else
		{
			OutputDebug(L"Service_FlowToFormat_Execute:Input struct Value Error:argvTypeOption value is unexpected");
			return -23;
		}
		/*
		1. ȥ��û�в���Ҫ�ش��Ĳ���  �ύ��Format2Flow
		2. Send
		*/
		{
			int ID_proc = pFlowBase->ID_proc;
			int SN		= pFlowBase->functionID;

			//û��ָ�����ݵ����
			char* pStruct=nullptr;
			if (0==pFlowBase->number_Of_Argv_Pointer)
			{
				pStruct = nullptr;
			}
			else
			{
				pStruct = pSecondCopyArgv;
			}
			//��С��������ָ��״̬
			int m_sizeOfStruct  = 2*sizeof(int)*pFlowBase->number_Of_Argv_Pointer;//=format_len;

			int m_pointerNumber = pFlowBase->number_Of_Argv_Pointer;
			int work_type = CDataFormat::RECV_INFO;
			int argvTypeOption = pFlowBase->argvTypeOption;
			int async = false;//ͬ��
			FARPROC callBack = nullptr;
			int ret2client_value = ret_value;


			//Format2Flow(LONG ID_proc,int SN,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,int work_type,int argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,int real_len=0)
			int realBufferLen = g_CDF.Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
				work_type,argvTypeOption,async,callBack);
			if (0==realBufferLen)
			{
				throw("Core: ToFlow return 0");
			}
			char* flowBuffer = new char[realBufferLen]();
			g_CDF.Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
				work_type,argvTypeOption,async,callBack,_Out_ flowBuffer,
				realBufferLen,ret2client_value);

			//����flowBuffer,realBufferLen

			pCWEB->Send(flowBuffer,realBufferLen);

			delete(flowBuffer);
			flowBuffer = nullptr;
		}



	}





	//�ͷŶԱ�ָ��ṹ - �������� �Ѿ��Զ�ʵ���ͷ���
	//if (pSecondCopyArgv!=nullptr)
	//{
	//	const int tmp_argv_number = pFlowBase->number_Of_Argv_Pointer;

	//	int offset = 0;
	//	for (int i=0;i<tmp_argv_number;i++)
	//	{
	//		char* p = (char*)(*(int*)(pSecondCopyArgv+offset));
	//		delete(p);
	//		offset+=sizeof(int)*2;
	//	}
	//}


	//�ͷű���ָ�������-�����ڱ��� ָ��ṹ �� �Ա�ָ��ṹ
	delete(queue_memory_manage);
	queue_memory_manage = nullptr;
	delete(queue_memory_copy);
	queue_memory_copy = nullptr;


	delete(pSecondCopyArgv);
	pSecondCopyArgv = nullptr;
	delete(pArgvCall);
	pArgvCall = nullptr;






	return 0;
	//////////////////////////////////////////////////////////////////////////
	//ͬ�첽����
	//���ڻص� �� �Ƿ�����Щָ�뷢���˸ı䡣���ڿ��ٻش���
	//�첽�ɻص���������
	//ͬ���ɱ��������������ͷ���ֵ���
}

struct argv_tmp 
{
	char* p;
	int p_len;
};

/*
��������fack�ص���

*/
void CData_Format_Service::ServiceAsyncCallBack(char* p,int p_len)
{
	/*
	1. ��ȡTID��Ӧ�Ĺ���ID_proc,function_ID;ʹ��ջ��ʽ
	�������߳�ǰ�����߳��ߣ����������:

	struct T()
	{
	ID_proc;
	function_ID
	}
	stack<T>.push(threadID);

	�������find����.����������û�д������̵ļһ����һ�Ӧ����WEB��Recive������
	������һ������д���Recive������


	2. ת����flow
	3. Web::Send

	*/
	char* pStruct = new char[2*sizeof(int)]();//struct {char* ,int len}
	((argv_tmp*)pStruct)->p = p;
	((argv_tmp*)pStruct)->p_len = p_len;

	{
		
		st_Async_Thread_Callback st_thread = g_Async_Thread_Callback->pop(GetCurrentThreadId());


		int ID_proc = st_thread.ID_proc;
		int SN		= st_thread.function_ID;



		//û��ָ�����ݵ����

		//��С��������ָ��״̬
		int number_Of_Argv_Pointer = 1;
		int m_sizeOfStruct  = 2*sizeof(int)*number_Of_Argv_Pointer;//=format_len;

		int m_pointerNumber = number_Of_Argv_Pointer;
		int work_type = CDataFormat::RECV_INFO;
		int argvTypeOption = CDataFormat::STANDARD_FLOW_MODE;
		int async = true;//ͬ��
		FARPROC callBack = nullptr;
		int ret2client_value = -1;//��Ч


		//Format2Flow(LONG ID_proc,int SN,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,int work_type,int argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,int real_len=0)
		int realBufferLen = g_CDF.Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
			work_type,argvTypeOption,async,callBack);
		if (0==realBufferLen)
		{
			throw("Core: ToFlow return 0");
		}
		char* flowBuffer = new char[realBufferLen]();
		g_CDF.Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
			work_type,argvTypeOption,async,callBack,_Out_ flowBuffer,
			realBufferLen,ret2client_value);

		//����flowBuffer,realBufferLen

		pCWEB->Send(flowBuffer,realBufferLen);

		delete(flowBuffer);
		flowBuffer = nullptr;
	}
}
