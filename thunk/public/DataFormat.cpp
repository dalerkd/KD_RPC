#include "../Client/stdafx.h"
#include "DataFormat.h"
#include "debug.h"
#include "SafeQueue.h"



int CDataFormat::Format2Flow(LONG ID_proc,int SN,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,int work_type,int argvTypeOption,bool async,FARPROC callback,char* flowBuffer/*=nullptr*/,int real_len/*=0*/,int ret_value/*=0*/)
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
		throw("Format2Flow:Error:argv is not true,...");
	}
	if (ArgvPointerNumber<0)
	{
		throw("Format2Flow:Error:argv is not true,ArgvPointerNumber<0");
	}

	if (async==false&&callback!=nullptr)
	{
		throw("Format2Flow: Warming: Sync function can't permit have callback");
	}

	//Ϊʲô��ô�㣿��st_data_flow��st_argv_Node_Struct�ṹ
	const int NO_POINTER_NUMBER = 1;
	int m_real_length = sizeof(st_data_flow) + sizeof(int)*(ArgvPointerNumber+NO_POINTER_NUMBER);

	if (nullptr==pStruct)//�޲���
	{
		if (ArgvPointerNumber!=0||sizeOfStruct!=0)
		{
			throw("Format2Flow:Error:pStruct==nullptr,and ArgvPointerNumber!=0");
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
		throw("Format2Flow:RealLength != your input length:Data change?");
	}

	if (flowBuffer==nullptr)
	{
		throw("Format2Flow:FlowBuffer is nullptr");
	}

	//�ṹ��ֵ
	st_data_flow* psdf =(st_data_flow*) flowBuffer;

	psdf->length_Of_Argv_Struct = m_real_length;
	psdf->work_type = work_type;
	psdf->ID_proc	= ID_proc;
	psdf->functionID= SN;
	psdf->async		= async;
	psdf->permit_callback = callback?true:false;
	psdf->return_value = ret_value;
	psdf->argvTypeOption = argvTypeOption;

	psdf->number_Of_Argv_Pointer = ArgvPointerNumber;

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
			throw("Format2Flow:This argv pointer==nullptr,but length call me !=0");
		}
		//copy
		int* plength = (int*)(psdf->argv_Struct+offset);

		*plength = *ptmp_length;

		offset += sizeof(int);
		int stat = memcpy_s(psdf->argv_Struct+offset,*ptmp_length,(char*)*ptmp_pointer,*ptmp_length);
		if (stat)
		{
			throw("memcpy_s return err.");
		}

		offset+=*ptmp_length;

		tmp_length+=*ptmp_length;
	}

	tmp_length += ArgvPointerNumber*sizeof(int);//+������Ϣ��ռλ��
	tmp_length += sizeOfStruct-ArgvPointerNumber*sizeof(int)*2+sizeof(int);//�Ͼ���ָ�������ʱҪ��ʼռ����λ�ã����һ���Ӳ���⿴�����ṹ��

	psdf->length_Of_Argv_Struct = tmp_length;

	return m_real_length;
}

void CDataFormat::Client_FlowToFormat_Execute(char* flow,int flow_len) //���������������Ƿ��첽,���߲�ͬ�����̡�
{

}
struct st_Async_Thread_Callback 
{
	LONG ID_proc;
	int function_ID;
};
#include "SafeMap.h"
extern CSafeMap<DWORD,st_Async_Thread_Callback>* g_Async_Thread_Callback;


/*

_In_:
char*	pFlow				��������ָ��
int		Flow_len			������������

_Out_:
char*	pArgvCall			����Format�����ṹ�Ĵ��Buffer,���������롣
CSafeQueueAutoPointerManage* queue_memory_manage ֻ����Format����Ĳ���ָ���ڴ�ָ��
int		Real_Format_len		��Ҫ��Buffer�ĳ���,�����֪����д0.���᷵����ȷ�ĳ���ֵд�뼴�ɡ�����ֻ��ΪУ�鳤���á�
-----------------------
���½����ڷ������ˣ�
char*	pSecondCopyArgv		����ͬ�������޸�״̬�Ա�-ֻ���ڷ���˵�ͬ���Ա�ʱ�����������롣���Ⱥ������һ����
CSafeQueueAutoPointerManage* queue_memory_copy ֻ����Format�����ڴ�ָ��,

Exception:
	����	����			����
	----------------------------
	char*	|�ڲ��߼�����|	����
	int		|���ݸ�ʽ����|	�ǵ���״̬���Ժ���


return ռ�õ����ݳ���
*/


int CDataFormat::Flow2Format(char *pFlow,int Flow_len,
							 char* pArgvCall,int Real_Format_len, CSafeQueueAutoPointerManage* queue_memory_manage,
							 char* pSecondCopyArgv,CSafeQueueAutoPointerManage* queue_memory_copy
							  )
{

	if (nullptr==pFlow||Flow_len<sizeof(st_data_flow))
	{
		OutputDebug(L"FlowToFormat:Argv look like is false");
		throw(0x10);
	}
	const st_data_flow* pFlowBase = (st_data_flow*)pFlow;


	//////////////////////////////////////////////////////////////////////////
	//�����Ϸ��Լ��


	/*��������ʵ���ȼ���
	�ܳ��� = ͷ + β
	length_of_this_struct = sizeof(st_data_flow) + length_Of_Argv_Struct
	*/
	if (pFlowBase->length_of_this_struct - sizeof(st_data_flow)!= pFlowBase->number_Of_Argv_Pointer)
	{
		OutputDebug(L"FlowToFormat:Input struct Format Error:number_Of_Argv_Pointer != length_of_this_struct-sizeof(struct head)");
		throw(0x11);
	}




	//����Ӧ�����������,����ʵ�������������
	const int tmp_number_Of_Argv_Pointer = pFlowBase->length_of_this_struct - sizeof(st_data_flow);

	//����ָ��ṹ�������

	const bool bAsync = (char)pFlowBase->async;
	const char* argv_Base = pFlowBase->argv_Struct;

	int offset=0;
	//��ȡ�ܳ��Ȳ�У�飬���ͨ�������µĳ��������ڴ�

	int real_argv_length = 0;//ʵ�ʼ���õ����ܳ���

	//��������ָ�����ݵĳ���
	for (int i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
	{
		int m_pointer_len = *(int*)(argv_Base+offset);
		real_argv_length += m_pointer_len;

		offset += m_pointer_len+sizeof(int);
	}
	//���ķ�ָ������ĳ���
	int other_Length = *(int*)(argv_Base+offset);

	//�ܳ���=ָ���������+�������ķ�ָ������ĳ���+����ռ��
	real_argv_length +=other_Length;
	const int SIZEOF_NON_POINTER_LENGTH=1;/*��ָ����ṹ�峤��*/
	real_argv_length +=(pFlowBase->number_Of_Argv_Pointer + SIZEOF_NON_POINTER_LENGTH)*sizeof(int);


	if (real_argv_length!=pFlowBase->length_Of_Argv_Struct)
	{
		OutputDebug(L"FlowToFormat:Input struct Format Error:length_Of_Argv_Struct is UnReal");
		throw(0x12);
	}

	//������ȱ�������֪��Ӵ��ֻ�����ǲ�֪��Ӵ���������Ǹ���ָ������¡�
	const int format_len = pFlowBase->number_Of_Argv_Pointer*2*sizeof(int)+other_Length;
	
	if(0==Real_Format_len)
	{
		return format_len;
	}
	else if (format_len != Real_Format_len || pArgvCall==nullptr)
	{
		OutputDebug(L"input the len of format is fault.check it.You input:0x%x,calc length is:0x%x",
			Real_Format_len,format_len);
		throw("input the len of format is fault.");
	}	




	/*ԭ�ͣ�int aaa1(PVOID* pStruct,FARPROC callBack)*/

	//////////////////////////////////////////////////////////////////////////
	//���Ʋ���
	//���ڲ���
	//char* pArgvCall = nullptr;�ⲿ�ṩ

	//����ͬ�������޸�״̬�Ա�-ֻ���ڷ���˵�ͬ���Ա�ʱ
	//char* pSecondCopyArgv = nullptr;�ⲿ�ṩ
	
	//ֻ��������Ĳ���ָ���ڴ�ָ��
	//CSafeQueueAutoPointerManage* queue_memory_manage = nullptr;���ⲿ�ṩ

	//ֻ�������ڴ�ָ��
	//CSafeQueueAutoPointerManage* queue_memory_copy = nullptr;���ⲿ�ṩ

	if (format_len!=0)
	{
		//pArgvCall = new char[format_len]();�ⲿ�ṩ

		if (nullptr!=pSecondCopyArgv /*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
		{//ͬ���򹹽��ڶ������������ڶԱȷ���
			//pSecondCopyArgv = new char[format_len]();�ⲿ�ṩ
			//queue_memory_copy = new CSafeQueueAutoPointerManage();�ⲿ�ṩ
		}

		/*
		���Ʋ���
		*/


		//queue_memory_manage = new CSafeQueueAutoPointerManage();�ⲿ�ṩ


		//��������ָ�����ݵĳ���
		{
			int argv_flow_offset = 0;
			int argv_format_offset=0;
			for (int i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
			{
				int m_pointer_len = *(int*)(argv_Base+argv_flow_offset);

				/*
				Ҫ��Ҫnew����ôȷ��new��Ķ����ܹ�����ȫ�ͷţ�
				�����û���ȫ�𣿻������Ᵽ��һ�ݣ�
				���������û��Ǻܲ����׵ġ��ֲ���constָ�롣
				*/
				char* pPointerData = nullptr;
				//���ݺ��ָ��
				char* pSecondPointerData = nullptr;

				if (0!=m_pointer_len)
				{
					pPointerData = new char[m_pointer_len]();

					//���û������ݿ��ܱ��û��ƻ�������Ϊ�˱�֤�ͷ�������ջ����ָ��
					if (queue_memory_manage)
					{
						queue_memory_manage->push(pPointerData);
					}
					
					//�û�������flow�е�ָ��
					char* pointer_data =(char*)(argv_Base+argv_flow_offset);

					int stat=memcpy_s(pPointerData,m_pointer_len,pointer_data,m_pointer_len);
					if (stat)
					{
						throw("memcpy_s pPointerData return err.");
					}

					//����Ƿ���ͬ�����ǵĻ�������ָ�룬�Ա���ú�ԱȽ����
					if (nullptr!=pSecondCopyArgv /*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
					{
						pSecondPointerData = new char[m_pointer_len]();
						if (queue_memory_copy)
						{
							queue_memory_copy->push(pSecondPointerData);
						}

						int stat=memcpy_s(pSecondPointerData,m_pointer_len,pointer_data,m_pointer_len);
						if (stat)
						{
							throw("memcpy_s pSecondPointerData return err.");
						}


					}
				}//if(0!=m_pointer_len)else
				else
				{
					if (queue_memory_manage)
					{
						queue_memory_manage->push(nullptr);//��ָ��ҲҪ�밡����ȻӰ�����
					}
					
					if (nullptr!=pSecondCopyArgv/*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
					{
						if (queue_memory_copy)
						{
							queue_memory_copy->push(nullptr);
						}
					}
				}
				/*
				
				���Format�ṹ
				*/
				char* point = pArgvCall + argv_format_offset;
				*(int*)point= (int)pPointerData;//����ָ�����
				point+=sizeof(int);
				*(int*)point= m_pointer_len;//�������

				//����Ƿ���ͬ�����ǵĻ�������ָ�룬�Ա���ú�ԱȽ����
				if (nullptr!=pSecondCopyArgv/*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
				{
					char* t_point = pSecondCopyArgv + argv_format_offset;
					*(int*)t_point= (int)pSecondPointerData;//����ָ�����
					t_point+=sizeof(int);
					*(int*)t_point= m_pointer_len;//�������

				}
				argv_format_offset+=sizeof(int)+sizeof(int);//ָ��+����

				argv_flow_offset += m_pointer_len+sizeof(int);
			}//-for-end


			//���ķ�ָ������ĳ���
			int other_Length = *(int*)(argv_Base+argv_flow_offset);

			//copy
			argv_flow_offset+=sizeof(int);
			char* p_format_end_data = pArgvCall + argv_format_offset;
			char* p_flow_end_data = (char*)argv_Base+argv_flow_offset;

			int stat = memcpy_s(p_format_end_data,other_Length,p_flow_end_data,other_Length);
			if (stat)
			{
				throw("memcpy_s p_format_end_data return err.");
			}

			
			
			//�����ͬ������������ķ�ָ�����ҲҪ����һ�¡�
			if (nullptr!=pSecondCopyArgv/*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
			{
				char* p_format_end_data_copy = pSecondCopyArgv + argv_flow_offset;

				int stat = memcpy_s(p_format_end_data_copy,other_Length,p_flow_end_data,other_Length);
				if (stat)
				{
					throw("memcpy_s p_format_end_data return err.");
				}
			}

		}


	}
	else//����ָ���ǽ�Ϊ��,ֱ�Ӵ���
	{
		;
	}
	return 0;
}

unsigned int WINAPI CDataFormat::Service_FlowToFormat_Execute(LPVOID lp)
{

	st_thread_Service_FlowToFormat_Excute* p=(st_thread_Service_FlowToFormat_Excute*) lp;

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
	const st_data_flow* pFlowBase =(st_data_flow*) new char(p->flow_len);

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

????????????
	/*
	0. ��ѯ�������ƣ����ú���
	1. �����ͬ����������޸�
	�ͷ���ֵ������������
	2. �ͷ�����ָ��ͨ��ջ
	*/
	char* pfunName = g_CI_Service->QueryFuncName(pFlowBase->functionID);

	if (pfunName==nullptr)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:query functionID Do'nt find Name,functionID:%d",pFlowBase->functionID);
		return -0x21;
	}

	const int MAX_funName_Len = 256;

	char* m_functionName = new char[MAX_funName_Len]();
	strcpy_s(m_functionName,MAX_funName_Len-1,pfunName);


	//����Ĵ�����ҪǨ�Ƶ����߲�ȫ��ȥ��
	char* dllName = "Service.dll";
	HMODULE hServiceDLL = LoadLibraryA(dllName);
	if (NULL==hServiceDLL)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:Load library fault,DLLName:%s",dllName);
		return -0x31;
	}

	//��ȡ������
	FARPROC CalledFunction = GetProcAddress(hServiceDLL,m_functionName);

	if (nullptr == CalledFunction)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:GetProcAddress fault,DLLName:%s,functionName:%s",dllName,m_functionName);
		return -0x41;
	}

	typedef void (*callback_fake)(char*,int);
	typedef int  (*called_function)(char*,callback_fake);

	called_function Service_Work_Function=(called_function)CalledFunction;

	if (bAsync)//�첽
	{
		
		//����callback;
		callback_fake callback;
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

			for (int offset = 0,int i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
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


	//�ͷű���ָ�����,�ն��л��쳣
	try{
		for (;;)
		{
			char* p = queue_memory_manage->pop();
			delete(p);
		}

	}
	catch(char*)
	{
		;
	}

	//�ͷŶԱ�ָ��ṹ
	if (pSecondCopyArgv!=nullptr)
	{
		const int tmp_argv_number = pFlowBase->number_Of_Argv_Pointer;

		int offset = 0;
		for (int i=0;i<tmp_argv_number;i++)
		{
			char* p = (char*)(*(int*)(pSecondCopyArgv+offset));
			delete(p);
			offset+=sizeof(int)*2;
		}
	}



	delete(pSecondCopyArgv);
	delete(pArgvCall);
	pSecondCopyArgv = nullptr;
	pArgvCall = nullptr;







	//////////////////////////////////////////////////////////////////////////
	//ͬ�첽����
	//���ڻص� �� �Ƿ�����Щָ�뷢���˸ı䡣���ڿ��ٻش���
	//�첽�ɻص���������
	//ͬ���ɱ��������������ͷ���ֵ���
}

void CDataFormat::ServiceAsyncCallBack(char* p,int p_len)
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
