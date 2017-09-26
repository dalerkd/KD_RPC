#include "stdafx.h"
//#include "../Client/stdafx.h"
#include "DataFormat.h"
#include "debug.h"
#include "SafeQueue.h"
#include "WEB.h"
#include "SafeMap.h"
#include "FunctionInfo.h"
#include "StateManage.h"







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
	if (sizeOfStruct<(int)(ArgvPointerNumber*sizeof(int)*2))
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
	int no_pointer_argv_length = sizeOfStruct - 2*sizeof(int)*ArgvPointerNumber;
	int m_real_length = sizeof(st_data_flow) + sizeof(int)*(ArgvPointerNumber+(no_pointer_argv_length?1:0));


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
		int* ptmp =(int*) pStruct;
		for (int i=0;i<ArgvPointerNumber;++i)
		{
			int* tmp;
			tmp = ptmp+(i+1);
			m_real_length+=*tmp;
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

	int* pBase =(int*) pStruct;
	int tmp_length = 0;//Argv_Struct��һ���ֵ��ܳ���

	int offset = 0;//��argv_Struct[]�е�ƫ��

	for (int i=0;i<ArgvPointerNumber;++i)
	{
		int* ptmp_length;
		int* ptmp_pointer;
		ptmp_pointer= pBase+i;
		ptmp_length	= pBase+(i+1);

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

	//ָ��������������ռ�ܳ���=ָ�����ݵĳ��Ⱥ�+���ȱ����Ϣ��ռλ��
	tmp_length += ArgvPointerNumber*sizeof(int);//+������Ϣ��ռλ��

	/*ע�⣺��ָ�����Ҳ��Ҫһ������λ�ñ���Լ��ĳ�����Flow�С�*/

	//��ָ���������=�ṹ���ܳ���-ָ�������ڽṹ������ռ����

	//�ܵĲ������ֳ���=��ָ���������+һ������λ�ñ��+ָ��������������ռ�ܳ���
	int t_ArgvLength = sizeOfStruct-ArgvPointerNumber*sizeof(int)*2;

	tmp_length += t_ArgvLength+(t_ArgvLength?sizeof(int):0);//�����ָ�����Ϊ0,�ǾͲ��ó�������ʾ��ָ������ˡ�

	psdf->length_Of_Argv_Struct = tmp_length;

	//д��ָ��������� �� ���ݵ�Flow����ȥ��




	if (no_pointer_argv_length)
	{
		*(int*)(psdf->argv_Struct+offset)=no_pointer_argv_length;
		offset+=sizeof(int);

		char* pArgv_Format = pStruct+ArgvPointerNumber*sizeof(int)*2;

		int stat = memcpy_s(psdf->argv_Struct+offset,no_pointer_argv_length,pArgv_Format,no_pointer_argv_length);
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
	if (pFlowBase->length_of_this_struct - sizeof(st_data_flow)!= pFlowBase->length_Of_Argv_Struct)
	{
		OutputDebug(L"FlowToFormat:Input struct Format Error:length_Of_Argv_Struct != length_of_this_struct-sizeof(struct head)");
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

		offset += sizeof(int)+m_pointer_len;//������+���Ƶĳ���
	}
	//���ķ�ָ������ĳ���
	int other_Length = 0;
	int tmp_argv_length =pFlowBase->length_Of_Argv_Struct;
	/*
	size > off	:
	size = off	:
	size < off	:Error
	*/
	if (tmp_argv_length>offset)
	{//�з�ָ�����
		other_Length = *(int*)(argv_Base+offset);
	}
	else if (tmp_argv_length==offset)
	{//û�з�ָ�����
		other_Length = 0;
	}
	else
	{
		OutputDebug(L"This is logic Error.");
		throw("This is logic Error.");
	}


	//�ܳ���=ָ���������+�������ķ�ָ������ĳ���+����ռ��
	real_argv_length +=other_Length;
	/*��ָ����ṹ�峤��Ϊ0��ʱ��ռ�ط�*/
	real_argv_length +=(pFlowBase->number_Of_Argv_Pointer + (other_Length?1:0))*sizeof(int);


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
					char* pointer_data =(char*)(argv_Base+argv_flow_offset+1*sizeof(int));//+���ݳ��ȵ�λ�ã������ڳ��Ⱥ���

					int stat=memcpy_s(pPointerData,m_pointer_len,pointer_data,m_pointer_len);
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

				argv_flow_offset += +sizeof(int)+m_pointer_len;
			}//-for-end

		//////////////////////////////////////////////////////////////////////////
			//���ķ�ָ������ĳ���
		
			int other_Length = 0;
			int tmp_argv_length =pFlowBase->length_Of_Argv_Struct;
			/*
			size > off	:
			size = off	:
			size < off	:Error
			*/
			if (tmp_argv_length>offset)
			{//�з�ָ�����
				other_Length = *(int*)(argv_Base+offset);
			}
			else if (tmp_argv_length==offset)
			{//û�з�ָ�����
				other_Length = 0;
			}
			else
			{
				OutputDebug(L"This is logic Error.");
				throw("This is logic Error.");
			}


			//copy
			argv_flow_offset+=sizeof(int);
			char* p_format_end_data = pArgvCall + argv_format_offset;
			char* p_flow_end_data = (char*)argv_Base+argv_flow_offset;

			if (other_Length!=0)
			{
				int stat = memcpy_s(p_format_end_data,other_Length,p_flow_end_data,other_Length);
				if (stat)
				{
					throw("memcpy_s p_format_end_data return err.");
				}
			}


			//�����ͬ������������ķ�ָ�����ҲҪ����һ�¡�
			if (nullptr!=pSecondCopyArgv/*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
			{
				char* p_format_end_data_copy = pSecondCopyArgv + argv_flow_offset;

				if (other_Length!=0)
				{
					int stat = memcpy_s(p_format_end_data_copy,other_Length,p_flow_end_data,other_Length);
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



