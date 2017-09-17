#pragma once


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
	work_type:		�������󣬻��ǻظ����ݣ��ͻ��ˣ����Ƿ���ˣ�����e_work_type
	argvTypeOption: �����ṹ���ʽ��չ����:����ģʽ�����Ǳ�׼������e_argv_type_option
	flowBuffer:		��������Ҫ�Ĵ洢��ָ��
	real_len:		������׼���Ĵ洢������
	ret_value:		����ֵ��������Ϊ���񷵻�׼���ġ��ɺ��ԡ�

	�쳣:
	����ṩ����ĳ��Ȼ�õ��쳣��

	*/
	static int Format2Flow(LONG ID_proc,int SN,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,int work_type,int argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,int real_len=0,int ret_value=0);


	//��ת���ɸ�ʽ:�ͻ��˽��룺����һ���̺߳���,��Ϊ������������Ϊջ��������������ź��ǡ�
	/*

	*/
	//void Client_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int& structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	static void Client_FlowToFormat_Execute(char* flow,int flow_len);//���������������Ƿ��첽,���߲�ͬ�����̡�
	int Flow2Format(char *pFlow,int Flow_len,_Out_ char*& pFormat,_Out_ int&m_Format_len, _Out_ CSafeQueueAutoPointerManage*& queue_memory_manage, _Out_ char*& pSecondCopyArgv );
	/************************************************************************/
	/* �����ʹ��															*/
	/************************************************************************/

	struct  st_thread_Service_FlowToFormat_Excute
	{
		char* flow;
		int flow_len;
		HANDLE hdEvent_Copy_Finish;//������������������Ͼʹ�������źš�
	};

	//��ת���ɸ�ʽ:����˽��룺����һ���̺߳�����
	//������st_thread_Service_FlowToFormat_Excute* p
	static  unsigned int WINAPI Service_FlowToFormat_Execute(LPVOID p);
	/*
	�ṩ���첽�����������Ļص�����

	���ڻ�ȡ���ص�����

	�������﷢��
	*/
	static void ServiceAsyncCallBack(char* p,int p_len);

	private:
		

};//�����ҵ�֪��ͬ�첽��������ǵķ������ݸ�ʽ�ǲ�ͬ�ģ�˭��������ָ���ڴ����������ٱȽϺ��ʣ�
/*
��������Щ���������˭����������ܺܶ���ڴ棿�͹������ǵ�ָ�룿
һ�ַ�������Service_FlowToFormat��������£�������ֱ�ӵ���Ŀ�����

Ŀ����񷵻ص�ʱ�򣬽������١�

Ŀ��������Ҫ���أ����ǣ������ܾܺòŷ��أ����������ʺ����߳�����

����Service_FlowToFormat��һ���̺߳������ȽϺ��ʡ�


*/