#pragma once

#pragma warning (disable: 4200) 

#include "SafeQueue.h"


/*
�����ͷ�������ʽ֮���ת��

*/

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
		LONG64		length_of_this_struct;//�����ṹ�ĳ���
		LONG64		work_type;//���ι�������:e_work_type
		long	ID_proc;
		LONG64 	functionID;

		//�Ƿ��첽
		char	async;
		//�Ƿ�����ص�
		char	permit_callback;
		LONG64		return_value;//����ֵ,��ͬ���·���������Ч�����Ǹ���˳�۵�����
		//�����ṹ���ʽ��չ���ã���ǰΪ�ա�����ʽ��0:ȫ������ʽ;1:���ٲ�����ʽ,����������Ϊ0,��ʾ�޸Ķ���
		LONG64		argvTypeOption;	//e_argv_type_option

		/*�����ṹ���ܳ���*/
		LONG64		length_Of_Argv_Struct;
		/*�����ṹ����ָ��ṹ�������*/
		LONG64		number_Of_Argv_Pointer;

		char	argv_Struct[0];   

		/*
		ָ��ṹ�� ��������
		ָ���ǲ����е�ָ��ṹ�������.
		eg:
		struct argv
		{
		char* a
		LONG64 len_a;
		char* b
		LONG64 len_b;
		char c
		}	
		����ָ��ṹ���������2.




		*/

	};
	struct st_argv_Node_Struct
	{
		LONG64 length;
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
	work_type:		�������󣬻��ǻظ����ݣ��ͻ��ˣ����Ƿ���ˣ�����e_work_type
	argvTypeOption: �����ṹ���ʽ��չ����:����ģʽ�����Ǳ�׼������e_argv_type_option
	flowBuffer:		��������Ҫ�Ĵ洢��ָ��
	real_len:		������׼���Ĵ洢������
	ret_value:		����ֵ��������Ϊ���񷵻�׼���ġ��ɺ��ԡ�

	�쳣:
	����ṩ����ĳ��Ȼ�õ��쳣��

	*/
	static LONG64 Format2Flow(LONG ID_proc,LONG64 SN,char*pStruct,LONG64 sizeOfStruct,LONG64 ArgvPointerNumber,LONG64 work_type,LONG64 argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,LONG64 real_len=0,LONG64 ret_value=0);


	//��ת���ɸ�ʽ:�ͻ��˽��룺����һ���̺߳���,��Ϊ������������Ϊջ��������������ź��ǡ�
	/*

	*/
	static LONG64 Flow2Format(char *pFlow,LONG64 Flow_len, char* pArgvCall,LONG64 Real_Format_len, CSafeQueueAutoPointerManage* queue_memory_manage, char* pSecondCopyArgv,CSafeQueueAutoPointerManage* queue_memory_copy );
	/************************************************************************/
	/* �����ʹ��															*/
	/************************************************************************/

	struct  st_thread_FlowToFormat_Excute_argvs
	{
		char* flow;
		LONG64 flow_len;
		HANDLE hdEvent_Copy_Finish;//������������������Ͼʹ�������źš�
	};

	//��ת��Ԫ���Ժ���
	static bool Format2Flow_test();
	static bool Flow2Format_test();
	private:
		

};//�����ҵ�֪��ͬ�첽��������ǵķ������ݸ�ʽ�ǲ�ͬ�ģ�˭��������ָ���ڴ����������ٱȽϺ��ʣ�
/*
��������Щ���������˭����������ܺܶ���ڴ棿�͹������ǵ�ָ�룿
һ�ַ�������Service_FlowToFormat��������£�������ֱ�ӵ���Ŀ�����

Ŀ����񷵻ص�ʱ�򣬽������١�

Ŀ��������Ҫ���أ����ǣ������ܾܺòŷ��أ����������ʺ����߳�����

����Service_FlowToFormat��һ���̺߳������ȽϺ��ʡ�


*/