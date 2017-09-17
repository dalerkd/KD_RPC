#pragma once


/*
参数和发送流格式之间的转换

*/

class CDataFormat
{
public:
	enum e_work_type
	{
		Query_INFO,//请求-客户端发送
		RECV_INFO,//回复-服务端发送
	};
	enum e_argv_type_option
	{
		STANDARD_FLOW_MODE,//标准模式
		QUICK_FLOW_MODE//快速参数模式，即参数长度为0,表示无改动。

	};
	struct st_data_flow
	{
		int		length_of_this_struct;//整个结构的长度
		int		work_type;//本次工作类型:e_work_type
		long	ID_proc;
		int 	functionID;

		//是否异步
		char	async;
		//是否允许回调
		char	permit_callback;
		int		return_value;//返回值,在同步下服务器端有效：这是个不顺眼的设置
		//参数结构体格式扩展配置，当前为空。仅格式。0:全参数格式;1:快速参数格式,即参数长度为0,表示无改动。
		int		argvTypeOption;	//e_argv_type_option

		/*参数结构体总长度*/
		int		length_Of_Argv_Struct;
		/*参数结构体中指针结构体的数量*/
		int		number_Of_Argv_Pointer;

		char	argv_Struct[0];   

		/*
		指针结构体 的数量：
		指的是参数中的指针结构体的数量.
		eg:
		struct argv
		{
		char* a
		int len_a;
		char* b
		int len_b;
		char c
		}	
		其中指针结构体的数量是2.




		*/

	};
	struct st_argv_Node_Struct
	{
		int length;
		char data[0];
	};


	/************************************************************************/
	/* 客户端使用                                                           */
	/************************************************************************/
	//转换成流：客户端编码
	/*
	返回值:			实际需要的buffer长度
	ID_proc:		每个调用过程的唯一标记
	SN:				每个函数的唯一标记
	pStruct:		指向参数结构体的指针
	sizeOfStruct:	结构体的长度
	ArgvPointerNumber:结构体中指针的数量,格式为最前排列:{指针,长度}{指针,长度}
	async:			是否异步
	callBack:		原调用函数的回调函数
	work_type:		发送请求，还是回复数据？客户端，还是服务端？见：e_work_type
	argvTypeOption: 参数结构体格式扩展配置:快速模式，还是标准？见：e_argv_type_option
	flowBuffer:		生成流需要的存储区指针
	real_len:		生成流准备的存储区长度
	ret_value:		返回值：看来是为服务返回准备的。可忽略。

	异常:
	如果提供错误的长度会得到异常。

	*/
	static int Format2Flow(LONG ID_proc,int SN,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,int work_type,int argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,int real_len=0,int ret_value=0);


	//流转换成格式:客户端解码：不是一个线程函数,因为它不阻塞：因为栈管理程序管理的是信号们。
	/*

	*/
	//void Client_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int& structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	static void Client_FlowToFormat_Execute(char* flow,int flow_len);//由它来自行区分是否异步,并走不同的流程。
	int Flow2Format(char *pFlow,int Flow_len,_Out_ char*& pFormat,_Out_ int&m_Format_len, _Out_ CSafeQueueAutoPointerManage*& queue_memory_manage, _Out_ char*& pSecondCopyArgv );
	/************************************************************************/
	/* 服务端使用															*/
	/************************************************************************/

	struct  st_thread_Service_FlowToFormat_Excute
	{
		char* flow;
		int flow_len;
		HANDLE hdEvent_Copy_Finish;//上面两个参数复制完毕就触发这个信号。
	};

	//流转换成格式:服务端解码：这是一个线程函数。
	//参数：st_thread_Service_FlowToFormat_Excute* p
	static  unsigned int WINAPI Service_FlowToFormat_Execute(LPVOID p);
	/*
	提供给异步服务器方法的回调函数

	用于获取返回的数据

	并在这里发送
	*/
	static void ServiceAsyncCallBack(char* p,int p_len);

	private:
		

};//首先我得知道同异步情况，它们的返回数据格式是不同的？谁负责其中指针内存的申请和销毁比较合适？
/*
上面遇到些许的困境：谁来申请这可能很多的内存？和管理它们的指针？
一种方案是由Service_FlowToFormat来做这件事，并由它直接调用目标服务。

目标服务返回的时候，进行销毁。

目标服务迟早要返回，但是：它可能很久才返回，所以这里适合用线程做。

所以Service_FlowToFormat是一个线程函数，比较合适。


*/