// Client.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "Client.h"
#include "debug.h"


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
		int PointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);

		//Event
		HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);


		pssm->push(ID_proc,&ret,pStruct,PointerNumber,hdEvent);

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

CFunctionInfo* g_CI_Client = new CFunctionInfo();

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
	char* QueryFuncName(int query_SN)
	{


	}
private:
};


CFunctionInfo* g_CI_Service = new CFunctionInfo();

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
		int		return_value;//����ֵ,��ͬ���·���������Ч�����Ǹ���˳�۵�����
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
		psdf->return_value = 0;
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

			//int* plength = (int*)psdf->argv_Struct[offset];

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


	//��ת���ɸ�ʽ:�ͻ��˽��룺����һ���̺߳���,��Ϊ������������Ϊջ��������������ź��ǡ�
	/*

	*/
	//void Client_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int& structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	void Client_FlowToFormat_Execute(char* flow,int flow_len)//���������������Ƿ��첽,���߲�ͬ�����̡�
	{

	}
	/************************************************************************/
	/* �����ʹ��															*/
	/************************************************************************/

	//�첽��ʽת������:����˱���
	void Service_FormatToFlow_Async();

	//ͬ����ʽת������:����˱���
	void Service_FormatToFlow_Sync();




#include <iostream>

	struct  st_thread_Service_FlowToFormat_Excute
	{
		char* flow;
		int flow_len;
		HANDLE hdEvent_Copy_Finish;//������������������Ͼʹ�������źš�
	};

	//��ת���ɸ�ʽ:����˽��룺����һ���̺߳�����
	//void Service_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	DWORD Service_FlowToFormat_Execute(st_thread_Service_FlowToFormat_Excute* p)
	{
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

		//////////////////////////////////////////////////////////////////////////
		//�����Ϸ��Լ��


		/*��������ʵ���ȼ���
		�ܳ��� = ͷ + β
		length_of_this_struct = sizeof(st_data_flow) + length_Of_Argv_Struct
		*/
		if (pFlowBase->length_of_this_struct - sizeof(st_data_flow)!= pFlowBase->number_Of_Argv_Pointer)
		{
			OutputDebug(L"Service_FlowToFormat_Execute:Input struct Format Error:number_Of_Argv_Pointer != length_of_this_struct-sizeof(struct head)");
			return -0x11;
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
			OutputDebug(L"Service_FlowToFormat_Execute:Input struct Format Error:length_Of_Argv_Struct is UnReal");
			return -0x12;
		}

		//������ȱ�������֪��Ӵ��ֻ�����ǲ�֪��Ӵ���������Ǹ���ָ������¡�
		int format_len = pFlowBase->number_Of_Argv_Pointer*2*sizeof(int)+other_Length;

		/*ԭ�ͣ�int aaa1(PVOID* pStruct,FARPROC callBack)*/

		//////////////////////////////////////////////////////////////////////////
		//���Ʋ���
		//���ڲ���
		char* pArgvCall = nullptr;
		//����ͬ�������޸�״̬�Ա�
		char* pSecondCopyArgv = nullptr;
		//ֻ��������Ĳ���ָ���ڴ�ָ��
		CSafeQueue<char*>* stack_memory_manage = nullptr;

		if (format_len!=0)
		{
			pArgvCall = new char[format_len]();
			
			if (false == bAsync)
			{//ͬ���򹹽��ڶ������������ڶԱȷ���
				pSecondCopyArgv = new char[format_len]();
			}

			/*
				���Ʋ���
			*/

			
			stack_memory_manage = new CSafeQueue<char*>();

			

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
					char* pSecondPointerData = nullptr;

					if (0!=m_pointer_len)
					{
						pPointerData = new char[m_pointer_len]();

						//���û������ݿ��ܱ��û��ƻ�������Ϊ�˱�֤�ͷ�������ջ����ָ��
						stack_memory_manage->push(pPointerData);
						
						char* pointer_data =(char*)(argv_Base+argv_flow_offset);

						int stat=memcpy_s(pPointerData,m_pointer_len,pointer_data,m_pointer_len);
						if (stat)
						{
							throw("memcpy_s pPointerData return err.");
						}

						//����Ƿ���ͬ�����ǵĻ�������ָ�룬�Ա���ú�ԱȽ����
						if (nullptr!=pSecondCopyArgv)
						{
							pSecondPointerData = new char[m_pointer_len]();

							int stat=memcpy_s(pSecondPointerData,m_pointer_len,pointer_data,m_pointer_len);
							if (stat)
							{
								throw("memcpy_s pSecondPointerData return err.");
							}


						}
					}
					char* point = pArgvCall + argv_format_offset;
					*(int*)point= (int)pPointerData;//����ָ�����
					point+=sizeof(int);
					*(int*)point= m_pointer_len;//�������

					//����Ƿ���ͬ�����ǵĻ�������ָ�룬�Ա���ú�ԱȽ����
					if (nullptr!=pSecondCopyArgv)
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

				//�����ͬ������Ҳ����һ�¡�����
				char* p_format_end_data_copy = pSecondCopyArgv + argv_flow_offset;

				int stat = memcpy_s(p_format_end_data_copy,other_Length,p_flow_end_data,other_Length);
				if (stat)
				{
					throw("memcpy_s p_format_end_data return err.");
				}

			}


		}
		else//����ָ���ǽ�Ϊ��,ֱ�Ӵ���
		{
			;
		}
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


		typedef void (CDataFormat::*callback_fake)(char*,int);
		typedef int  (*called_function)(char*,callback_fake);

		called_function Service_Work_Function=(called_function)CalledFunction;

		if (bAsync)//�첽
		{
			callback_fake callback;
			if (pFlowBase->permit_callback)
			{
				callback = ServiceAsyncCallBack;
			}

			Service_Work_Function(pArgvCall,callback);//�����ķ���ֵ			
		}
		else//ͬ����1.��鷵��ֵ��2.����Ƿ����޸Ĳ�����3.ת��������4.����
		{
			int ret_value = Service_Work_Function(pArgvCall,nullptr);

			/*
			һ���޸������ݾͣ�����Ҫ�齨һ������
			һ����ȫ��ֱ�ӷ��ء�
			һ���ǿ��ٷ�ʽ������⡣ֻ���ر�Ҫ�����ݡ�

			*/
			if (QUICK_FLOW_MODE==pFlowBase->argvTypeOption)
			{/*
				���õ��ò����Ľṹ��
				1. �޸���ָ��,��ָֹ�뱻�û��㻵
				2. ��������û�б��޸ĵĽ�ָ����0(��Ϊ�б���ָ�룬���Ա�֤�ͷš�)
				3. �����ò����ṹ�ύ��Format2Flow��(��ȻҪ��֤�޸�һ�·�����
				4. Send			


				*/
				?
			}
			else if (STANDARD_FLOW_MODE==pFlowBase->argvTypeOption)
			{
				/*
				ͬ�����õ��ò����Ľṹ
				1. �ö��нṹ�޸���ָ�룬��ָֹ�뱻�û�����
				2. �����ò����ṹ�ύ��Format2Flow
				3. Send
				*/
				?
			}
			else
			{
				OutputDebug(L"Service_FlowToFormat_Execute:Input struct Value Error:argvTypeOption value is unexpected");
				return -23;
			}


		}


		//�ͷű���ָ�����,�ն��л��쳣
		try{
			for (;;)
			{
				char* p = stack_memory_manage->pop();
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
	/*
	�ṩ���첽�����������Ļص�����

	���ڻ�ȡ���ص�����

	�������﷢��
	*/
	void ServiceAsyncCallBack(char* p,int p_len)
	{
		/*
		1. ��ȡTID��Ӧ�Ĺ���ID_proc;
		2. ת����flow
		3. Web::Send

		*/
	}

	private:
		

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
		int stat=memcpy_s(p,data_len,data,data_len);
		if (stat)
		{
			throw("memcpy_s return err.");
		}

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


