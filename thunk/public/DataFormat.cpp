#include "stdafx.h"
//#include "../Client/stdafx.h"
#include "DataFormat.h"
#include "debug.h"
#include "SafeQueue.h"
#include "WEB.h"
#include "SafeMap.h"
#include "FunctionInfo.h"
#include "StateManage.h"







LONG64 CDataFormat::Format2Flow(LONG ID_proc,LONG64 SN,char*pStruct,LONG64 sizeOfStruct,LONG64 ArgvPointerNumber,LONG64 work_type,LONG64 argvTypeOption,bool async,FARPROC callback,char* flowBuffer/*=nullptr*/,LONG64 real_len/*=0*/,LONG64 ret_value/*=0*/)
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
	if (sizeOfStruct<(LONG64)(ArgvPointerNumber*sizeof(LONG64)*2))
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


	//�ȼ��㳤����Ϣ��ռ�ռ�
	/*
	�����ָ���������Ϊ0,�������ڷ�ָ����������ָ��������� ���������Flow�С�
	*/
	LONG64 no_pointer_argv_length = sizeOfStruct - 2*sizeof(LONG64)*ArgvPointerNumber;
	LONG64 m_real_length = sizeof(st_data_flow) + sizeof(LONG64)*(ArgvPointerNumber+(no_pointer_argv_length?1:0));


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
	{//����ָ�����ݳ���
		LONG64* ptmp =(LONG64*) pStruct;
		for (LONG64 i=0;i<ArgvPointerNumber;++i)
		{
			LONG64* tmp;
			if (nullptr!=(char*)*(ptmp+(i*2)))//����ָ��Ϊ�գ��򲻼��㳤�ȣ����ڿ���ģʽ��
			{
				tmp = ptmp+(i*2+1);//���ݳ���
				m_real_length+=*tmp;
			}

		}
	}

	m_real_length += no_pointer_argv_length;//���Ϸ�ָ����������ݳ���



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

	psdf->length_of_this_struct = m_real_length;
	psdf->work_type = work_type;
	psdf->ID_proc	= ID_proc;
	psdf->functionID= SN;
	psdf->async		= async;
	psdf->permit_callback = callback?true:false;
	psdf->return_value = ret_value;
	psdf->argvTypeOption = argvTypeOption;

	//psdf->length_Of_Argv_Struct;//�����и�ֵ
	psdf->number_Of_Argv_Pointer = ArgvPointerNumber;

	LONG64* pBase =(LONG64*) pStruct;
	LONG64 tmp_length = 0;//Argv_Struct��һ���ֵ��ܳ���

	LONG64 offset = 0;//��argv_Struct[]�е�ƫ��

	for (LONG64 i=0;i<ArgvPointerNumber;++i)
	{
		LONG64* ptmp_length;
		LONG64* ptmp_pointer;
		ptmp_pointer= pBase+2*i;
		ptmp_length	= pBase+(2*i+1);

		//check
		if (nullptr == (char*)*ptmp_pointer&&0!=*ptmp_length)
		{
			;//˵�����ڽ��п��ٴ���ģʽ
		}
		//copy
		LONG64* plength = (LONG64*)(psdf->argv_Struct+offset);

		*plength = *ptmp_length;

		offset += sizeof(LONG64);

		if (nullptr != (char*)*ptmp_pointer)//��ָ�벻Ϊ��ʱ�����ȱ��벻Ϊ0
		{
			if (*ptmp_length==0)
			{
				throw("Format2Flow:This argv pointer!=nullptr,but length call me ==0");
			}
			LONG64 stat = memcpy_s(psdf->argv_Struct+offset,*ptmp_length,(char*)*ptmp_pointer,*ptmp_length);
			if (stat)
			{
				throw("memcpy_s return err.");
			}

			offset+=*ptmp_length;

			tmp_length+=*ptmp_length;
		}
		else//���ٴ���ģʽ,�����Ʋ����ĳ���
		{
			offset+=0;
			tmp_length+=0;
		}


	}

	//ָ��������������ռ�ܳ���=ָ�����ݵĳ��Ⱥ�+���ȱ����Ϣ��ռλ��
	tmp_length += ArgvPointerNumber*sizeof(LONG64);//+������Ϣ��ռλ��

	/*ע�⣺��ָ�����Ҳ��Ҫһ������λ�ñ���Լ��ĳ�����Flow�С�*/

	//��ָ���������=�ṹ���ܳ���-ָ�������ڽṹ������ռ����

	//�ܵĲ������ֳ���=��ָ���������+һ������λ�ñ��+ָ��������������ռ�ܳ���
	LONG64 t_ArgvLength = sizeOfStruct-ArgvPointerNumber*sizeof(LONG64)*2;

	tmp_length += t_ArgvLength+(t_ArgvLength?sizeof(LONG64):0);//�����ָ�����Ϊ0,�ǾͲ��ó�������ʾ��ָ������ˡ�

	psdf->length_Of_Argv_Struct = tmp_length;

	//д��ָ��������� �� ���ݵ�Flow����ȥ��




	if (no_pointer_argv_length)
	{
		*(LONG64*)(psdf->argv_Struct+offset)=no_pointer_argv_length;
		offset+=sizeof(LONG64);

		char* pArgv_Format = pStruct+ArgvPointerNumber*sizeof(LONG64)*2;

		LONG64 stat = memcpy_s(psdf->argv_Struct+offset,no_pointer_argv_length,pArgv_Format,no_pointer_argv_length);
		if (stat)
		{
			throw("memcpy_s return err.");
		}
	}



	return m_real_length;
}







/*

_In_:
char*	pFlow				��������ָ��
LONG64		Flow_len			������������

_Out_:
char*	pArgvCall			����Format�����ṹ�Ĵ��Buffer,���������롣
CSafeQueueAutoPointerManage* queue_memory_manage ֻ����Format����Ĳ���ָ���ڴ�ָ��
LONG64		Real_Format_len		��Ҫ��Buffer�ĳ���,�����֪����д0.���᷵����ȷ�ĳ���ֵд�뼴�ɡ�����ֻ��ΪУ�鳤���á�
-----------------------
���½����ڷ������ˣ�
char*	pSecondCopyArgv		����ͬ�������޸�״̬�Ա�-ֻ���ڷ���˵�ͬ���Ա�ʱ�����������롣���Ⱥ������һ����
CSafeQueueAutoPointerManage* queue_memory_copy ֻ����Format�����ڴ�ָ��,

Exception:
����	����			����
----------------------------
char*	|�ڲ��߼�����|	����
LONG64		|���ݸ�ʽ����|	�ǵ���״̬���Ժ���


return ռ�õ����ݳ���
*/


LONG64 CDataFormat::Flow2Format(char *pFlow,LONG64 Flow_len,
							 char* pArgvCall,LONG64 Real_Format_len, CSafeQueueAutoPointerManage* queue_memory_manage,
							 char* pSecondCopyArgv,CSafeQueueAutoPointerManage* queue_memory_copy
							 )
{

	if (nullptr==pFlow||Flow_len<sizeof(st_data_flow))
	{
		OutputDebug("FlowToFormat:Argv look like is false");
		throw(0x10);
	}
	const st_data_flow* pFlowBase = (st_data_flow*)pFlow;


	//////////////////////////////////////////////////////////////////////////
	//�����Ϸ��Լ��


	/*��������ʵ���ȼ���
	�ܳ��� = ͷ + β
	length_of_this_struct = sizeof(st_data_flow) + length_Of_Argv_Struct
	*/
	if (pFlowBase->length_of_this_struct - sizeof(st_data_flow)!= pFlowBase->length_Of_Argv_Struct)
	{
		OutputDebug("FlowToFormat:Input struct Format Error:length_Of_Argv_Struct != length_of_this_struct-sizeof(struct head)");
		throw(0x11);
	}




	//����Ӧ�����������,����ʵ�������������
	const LONG64 tmp_number_Of_Argv_Pointer = pFlowBase->length_of_this_struct - sizeof(st_data_flow);

	//����ָ��ṹ�������

	const bool bAsync = (char)pFlowBase->async;
	const char* argv_Base = pFlowBase->argv_Struct;

	LONG64 offset=0;
	//��ȡ�ܳ��Ȳ�У�飬���ͨ�������µĳ��������ڴ�

	LONG64 real_argv_length = 0;//ʵ�ʼ���õ����ܳ���

	//��������ָ�����ݵĳ���
	for (LONG64 i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
	{
		LONG64 m_pointer_len = *(LONG64*)(argv_Base+offset);
		real_argv_length += m_pointer_len;

		offset += sizeof(LONG64)+m_pointer_len;//������+���Ƶĳ���
	}
	//���ķ�ָ������ĳ���
	LONG64 other_Length = 0;
	LONG64 tmp_argv_length =pFlowBase->length_Of_Argv_Struct;
	/*
	size > off	:
	size = off	:
	size < off	:Error
	*/
	if (tmp_argv_length>offset)
	{//�з�ָ�����
		other_Length = *(LONG64*)(argv_Base+offset);
	}
	else if (tmp_argv_length==offset)
	{//û�з�ָ�����
		other_Length = 0;
	}
	else
	{
		OutputDebug("This is logic Error.");
		throw("This is logic Error.");
	}


	//�ܳ���=ָ���������+�������ķ�ָ������ĳ���+����ռ��
	real_argv_length +=other_Length;
	/*��ָ����ṹ�峤��Ϊ0��ʱ��ռ�ط�*/
	real_argv_length +=(pFlowBase->number_Of_Argv_Pointer + (other_Length?1:0))*sizeof(LONG64);


	if (real_argv_length!=pFlowBase->length_Of_Argv_Struct)
	{
		OutputDebug("FlowToFormat:Input struct Format Error:length_Of_Argv_Struct is UnReal");
		throw(0x12);
	}

	//������ȱ�������֪��Ӵ��ֻ�����ǲ�֪��Ӵ���������Ǹ���ָ������¡�
	const LONG64 format_len = pFlowBase->number_Of_Argv_Pointer*2*sizeof(LONG64)+other_Length;

	if(0==Real_Format_len)
	{
		return format_len;
	}
	else if (format_len != Real_Format_len || pArgvCall==nullptr)
	{
		OutputDebug("input the len of format is fault.check it.You input:0x%x,calc length is:0x%x",
			Real_Format_len,format_len);
		throw("input the len of format is fault.");
	}	




	/*ԭ�ͣ�LONG64 aaa1(PVOID* pStruct,FARPROC callBack)*/

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
			LONG64 argv_flow_offset = 0;
			LONG64 argv_format_offset=0;
			for (LONG64 i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
			{
				LONG64 m_pointer_len = *(LONG64*)(argv_Base+argv_flow_offset);

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
					char* pointer_data =(char*)(argv_Base+argv_flow_offset+1*sizeof(LONG64));//+���ݳ��ȵ�λ�ã������ڳ��Ⱥ���

					LONG64 stat=memcpy_s(pPointerData,m_pointer_len,pointer_data,m_pointer_len);
					if (stat)
					{
						throw("memcpy_s pPointerData return err.");
					}
					//			pArgvCallû�б�������ݣ���ȥ��������



					//����Ƿ���ͬ�����ǵĻ�������ָ�룬�Ա���ú�ԱȽ����
					if (nullptr!=pSecondCopyArgv /*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
					{
						pSecondPointerData = new char[m_pointer_len]();
						if (queue_memory_copy)
						{
							queue_memory_copy->push(pSecondPointerData);
						}

						LONG64 stat=memcpy_s(pSecondPointerData,m_pointer_len,pointer_data,m_pointer_len);
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
				*(LONG64*)point= (LONG64)pPointerData;//����ָ�����
				point+=sizeof(LONG64);
				*(LONG64*)point= m_pointer_len;//�������

				//����Ƿ���ͬ�����ǵĻ�������ָ�룬�Ա���ú�ԱȽ����
				if (nullptr!=pSecondCopyArgv/*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
				{
					char* t_point = pSecondCopyArgv + argv_format_offset;
					*(LONG64*)t_point= (LONG64)pSecondPointerData;//����ָ�����
					t_point+=sizeof(LONG64);
					*(LONG64*)t_point= m_pointer_len;//�������

				}
				argv_format_offset+=sizeof(LONG64)+sizeof(LONG64);//ָ��+����

				argv_flow_offset += +sizeof(LONG64)+m_pointer_len;
			}//-for-end

			//////////////////////////////////////////////////////////////////////////
			//���ķ�ָ������ĳ���

			LONG64 other_Length = 0;
			LONG64 tmp_argv_length =pFlowBase->length_Of_Argv_Struct;
			/*
			size > off	:
			size = off	:
			size < off	:Error
			*/
			if (tmp_argv_length>offset)
			{//�з�ָ�����
				other_Length = *(LONG64*)(argv_Base+offset);
			}
			else if (tmp_argv_length==offset)
			{//û�з�ָ�����
				other_Length = 0;
			}
			else
			{
				OutputDebug("This is logic Error.");
				throw("This is logic Error.");
			}


			//copy
			argv_flow_offset+=sizeof(LONG64);
			char* p_format_end_data = pArgvCall + argv_format_offset;
			char* p_flow_end_data = (char*)argv_Base+argv_flow_offset;

			if (other_Length!=0)
			{
				LONG64 stat = memcpy_s(p_format_end_data,other_Length,p_flow_end_data,other_Length);
				if (stat)
				{
					throw("memcpy_s p_format_end_data return err.");
				}
			}


			//�����ͬ������������ķ�ָ�����ҲҪ����һ�¡�
			if (nullptr!=pSecondCopyArgv/*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
			{
				char* p_format_end_data_copy = pSecondCopyArgv + argv_format_offset;

				if (other_Length!=0)
				{
					LONG64 stat = memcpy_s(p_format_end_data_copy,other_Length,p_flow_end_data,other_Length);
					if (stat)
					{
						throw("memcpy_s p_format_end_data return err.");
					}
				}

			}

		}


	}
	else//����ָ���ǽ�Ϊ��,ֱ�Ӵ���
	{
		;
	}
	return real_argv_length;
}



//UNDONE: Need Finishe Format2Flow
bool CDataFormat::Format2Flow_test()
{
	{
		const LONG64 STRUCT_SIZE = 5;
		char* pStruct = new char[STRUCT_SIZE]();//struct {char* ,LONG64 len}

		LONG64 ID_proc = 0;//st_thread.ID_proc;
		LONG64 SN		= 0;//st_thread.function_ID;



		//û��ָ�����ݵ����

		//��С��������ָ��״̬
		LONG64 number_Of_Argv_Pointer = 1;
		LONG64 m_sizeOfStruct  = 2*sizeof(LONG64)*number_Of_Argv_Pointer;//=format_len;

		LONG64 m_pointerNumber = number_Of_Argv_Pointer;
		LONG64 work_type = CDataFormat::RECV_INFO;
		LONG64 argvTypeOption = CDataFormat::STANDARD_FLOW_MODE;
		LONG64 async = true;//ͬ��
		FARPROC callBack = nullptr;
		LONG64 ret2client_value = -1;//��Ч


		//Format2Flow(LONG ID_proc,LONG64 SN,char*pStruct,LONG64 sizeOfStruct,LONG64 ArgvPointerNumber,LONG64 work_type,LONG64 argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,LONG64 real_len=0)
		LONG64 realBufferLen = Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
			work_type,argvTypeOption,async,callBack);
		if (0==realBufferLen)
		{
			throw("Core: ToFlow return 0");
		}
		char* flowBuffer = new char[realBufferLen]();
		Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
			work_type,argvTypeOption,async,callBack,_Out_ flowBuffer,
			realBufferLen,ret2client_value);

		//����flowBuffer,realBufferLen

		//pCWEB->Send(flowBuffer,realBufferLen);

		delete(flowBuffer);
		flowBuffer = nullptr;
		delete(pStruct);
	}
	return true;
}
//UNDONE: Need Finishe Flow2Format
bool CDataFormat::Flow2Format_test()
{
	/*
	���� ��-ת��ʽ�����ظ�ʽ����+���ݸ�ʽ���С�
		Async�������Ƿ���ǲ��ñ��ݣ����ݸ�ʽ���е���;��������������Ա��û���ָ�����ݵ��޸ġ�
		������Զ�ָ������Ǹ������ѡ�񡣽���ˡ��ṹ��ӵ���ӽṹ��ָ��ṹ���Ĺ������ͷ����⡣
	*/
	{
		//////////////////////////////////////////////////////////////////////////
		/*Flow2Format*/
		LONG64 flow_len = 0;
		const st_data_flow* pFlowBase =(st_data_flow*) new char[flow_len]();

		char* pArgvCall  = nullptr;
		char* pSecondCopyArgv = nullptr;
		CSafeQueueAutoPointerManage* queue_memory_manage = nullptr;
		CSafeQueueAutoPointerManage* queue_memory_copy	 = nullptr;
		const bool bAsync = true;//pFlowBase->async;

		try
		{
			LONG64 real_len=Flow2Format((char*)pFlowBase,pFlowBase->length_of_this_struct,nullptr,0,nullptr,nullptr,nullptr);
			if (real_len==0)
			{
				OutputDebug("real_len=0?");
				throw("real_len=0?");
			}


			pArgvCall = new char[real_len]();



			queue_memory_manage = new CSafeQueueAutoPointerManage();
			if (false == bAsync&&true)//������ô��֡�����
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
		catch (LONG64 errCode)//�ļ���ʽ�������֮
		{
			OutputDebug("Flow Format Err,code:0x%x",errCode);
			return errCode;
		}
		//////////////////////////////////////////////////////////////////////////
		delete(pFlowBase);
		delete(pArgvCall);
		delete(queue_memory_manage);
		delete(pSecondCopyArgv);
		delete(queue_memory_copy);
	}


	return true;
}

