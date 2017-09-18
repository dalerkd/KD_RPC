#include "../Client/stdafx.h"
#include "DataFormat.h"
#include "debug.h"
#include "SafeQueue.h"



int CDataFormat::Format2Flow(LONG ID_proc,int SN,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,int work_type,int argvTypeOption,bool async,FARPROC callback,char* flowBuffer/*=nullptr*/,int real_len/*=0*/,int ret_value/*=0*/)
{
	/*返回真正需要的长度给外部
	如果real_len!=真实长度的话,就不拷贝,而是返回真正的长度.
	如果参数长度!=0,且与真正计算长度,抛异常.-这意味着发生了某种变动在两次计算之间。

	这种解决方案,真是巧妙啊。防止缓冲区不足的情况。设计灵感来自于微软的宽窄转换系列。
	如果不这样做的话，就得以下三种方案：
	1. 由掌握长度的函数被迫自行分配，调用者得释放一个不是它申请的内存，权责不明：结果导致内存泄漏。
	2. 每次不足就得抛异常或者错误值：不容易接啊，操作繁琐。
	3. 由其他函数负责获取长度。分离导致容易出现次序错误。


	步骤：
	1. 获取需要的长度，告知调用者
	2. 校验调用者长度
	3. 填充流

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

	//为什么这么算？看st_data_flow和st_argv_Node_Struct结构
	const int NO_POINTER_NUMBER = 1;
	int m_real_length = sizeof(st_data_flow) + sizeof(int)*(ArgvPointerNumber+NO_POINTER_NUMBER);

	if (nullptr==pStruct)//无参数
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

	//结构赋值
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
	int tmp_length = 0;//Argv_Struct这一部分的总长度

	int offset = 0;//在argv_Struct[]中的偏移

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

	tmp_length += ArgvPointerNumber*sizeof(int);//+长度信息所占位置
	tmp_length += sizeOfStruct-ArgvPointerNumber*sizeof(int)*2+sizeof(int);//毕竟非指针参数此时要开始占长度位置，如果一下子不理解看两个结构。

	psdf->length_Of_Argv_Struct = tmp_length;

	return m_real_length;
}

void CDataFormat::Client_FlowToFormat_Execute(char* flow,int flow_len) //由它来自行区分是否异步,并走不同的流程。
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
char*	pFlow				待解析流指针
int		Flow_len			待解析流长度

_Out_:
char*	pArgvCall			用于Format参数结构的存放Buffer,调用者申请。
CSafeQueueAutoPointerManage* queue_memory_manage 只管理Format申请的参数指针内存指针
int		Real_Format_len		需要的Buffer的长度,如果不知道就写0.将会返回正确的长度值写入即可。这里只作为校验长度用。
-----------------------
以下仅用于服务器端：
char*	pSecondCopyArgv		用于同步参数修改状态对比-只用在服务端的同步对比时。调用者申请。长度和上面的一样。
CSafeQueueAutoPointerManage* queue_memory_copy 只管理Format备份内存指针,

Exception:
	类型	介绍			建议
	----------------------------
	char*	|内部逻辑问题|	挂起
	int		|数据格式问题|	非调试状态可以忽略


return 占用的数据长度
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
	//参数合法性检测


	/*测验以真实长度计算
	总长度 = 头 + 尾
	length_of_this_struct = sizeof(st_data_flow) + length_Of_Argv_Struct
	*/
	if (pFlowBase->length_of_this_struct - sizeof(st_data_flow)!= pFlowBase->number_Of_Argv_Pointer)
	{
		OutputDebug(L"FlowToFormat:Input struct Format Error:number_Of_Argv_Pointer != length_of_this_struct-sizeof(struct head)");
		throw(0x11);
	}




	//长度应当是这个数字,检验实际是这个数字吗
	const int tmp_number_Of_Argv_Pointer = pFlowBase->length_of_this_struct - sizeof(st_data_flow);

	//检验指针结构体的数量

	const bool bAsync = (char)pFlowBase->async;
	const char* argv_Base = pFlowBase->argv_Struct;

	int offset=0;
	//获取总长度并校验，如果通过就用新的长度申请内存

	int real_argv_length = 0;//实际计算得到的总长度

	//计算所有指针数据的长度
	for (int i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
	{
		int m_pointer_len = *(int*)(argv_Base+offset);
		real_argv_length += m_pointer_len;

		offset += m_pointer_len+sizeof(int);
	}
	//最后的非指针参数的长度
	int other_Length = *(int*)(argv_Base+offset);

	//总长度=指针参数长度+加上最后的非指针参数的长度+长度占地
	real_argv_length +=other_Length;
	const int SIZEOF_NON_POINTER_LENGTH=1;/*非指针类结构体长度*/
	real_argv_length +=(pFlowBase->number_Of_Argv_Pointer + SIZEOF_NON_POINTER_LENGTH)*sizeof(int);


	if (real_argv_length!=pFlowBase->length_Of_Argv_Struct)
	{
		OutputDebug(L"FlowToFormat:Input struct Format Error:length_Of_Argv_Struct is UnReal");
		throw(0x12);
	}

	//这个长度被调用者知道哟，只是我们不知道哟，所以我们给个指针就完事。
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




	/*原型：int aaa1(PVOID* pStruct,FARPROC callBack)*/

	//////////////////////////////////////////////////////////////////////////
	//复制操作
	//用于参数
	//char* pArgvCall = nullptr;外部提供

	//用于同步参数修改状态对比-只用在服务端的同步对比时
	//char* pSecondCopyArgv = nullptr;外部提供
	
	//只管理申请的参数指针内存指针
	//CSafeQueueAutoPointerManage* queue_memory_manage = nullptr;由外部提供

	//只管理备份内存指针
	//CSafeQueueAutoPointerManage* queue_memory_copy = nullptr;由外部提供

	if (format_len!=0)
	{
		//pArgvCall = new char[format_len]();外部提供

		if (nullptr!=pSecondCopyArgv /*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
		{//同步则构建第二个保留区用于对比服务
			//pSecondCopyArgv = new char[format_len]();外部提供
			//queue_memory_copy = new CSafeQueueAutoPointerManage();外部提供
		}

		/*
		复制操作
		*/


		//queue_memory_manage = new CSafeQueueAutoPointerManage();外部提供


		//计算所有指针数据的长度
		{
			int argv_flow_offset = 0;
			int argv_format_offset=0;
			for (int i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
			{
				int m_pointer_len = *(int*)(argv_Base+argv_flow_offset);

				/*
				要不要new，怎么确保new后的东西能够被安全释放，
				交给用户安全吗？还是另外保存一份？
				反正交给用户是很不靠谱的。又不是const指针。
				*/
				char* pPointerData = nullptr;
				//备份后的指针
				char* pSecondPointerData = nullptr;

				if (0!=m_pointer_len)
				{
					pPointerData = new char[m_pointer_len]();

					//给用户的数据可能被用户破坏，所以为了保证释放所以用栈备份指针
					if (queue_memory_manage)
					{
						queue_memory_manage->push(pPointerData);
					}
					
					//用户数据在flow中的指针
					char* pointer_data =(char*)(argv_Base+argv_flow_offset);

					int stat=memcpy_s(pPointerData,m_pointer_len,pointer_data,m_pointer_len);
					if (stat)
					{
						throw("memcpy_s pPointerData return err.");
					}

					//检查是否是同步，是的话：备份指针，以便调用后对比结果。
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
						queue_memory_manage->push(nullptr);//空指针也要入啊，不然影响次序
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
				
				填充Format结构
				*/
				char* point = pArgvCall + argv_format_offset;
				*(int*)point= (int)pPointerData;//数据指针填充
				point+=sizeof(int);
				*(int*)point= m_pointer_len;//长度填充

				//检查是否是同步，是的话：备份指针，以便调用后对比结果。
				if (nullptr!=pSecondCopyArgv/*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
				{
					char* t_point = pSecondCopyArgv + argv_format_offset;
					*(int*)t_point= (int)pSecondPointerData;//数据指针填充
					t_point+=sizeof(int);
					*(int*)t_point= m_pointer_len;//长度填充

				}
				argv_format_offset+=sizeof(int)+sizeof(int);//指针+长度

				argv_flow_offset += m_pointer_len+sizeof(int);
			}//-for-end


			//最后的非指针参数的长度
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

			
			
			//如果是同步函数，后面的非指针参数也要备份一下。
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
	else//参数指针们皆为空,直接传入
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

	//copy 指针
	const st_data_flow* pFlowBase =(st_data_flow*) new char(p->flow_len);

	int stat = memcpy_s((char*)pFlowBase,p->flow_len,p->flow,p->flow_len);
	if (stat)
	{
		throw("memcpy_s return err.");
	}

	//通知 拷贝完成
	if(0==SetEvent(p->hdEvent_Copy_Finish))
	{
		throw("Event is invalid");
	}
	//让指针们无效
	pTmp_Flow = nullptr;
	p = nullptr;//p无效了。

????????????
	/*
	0. 查询函数名称，调用函数
	1. 如果是同步函数检查修改
	和返回值，并发送数据
	2. 释放所有指针通过栈
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


	//这里的代码需要迁移到更高层全局去。
	char* dllName = "Service.dll";
	HMODULE hServiceDLL = LoadLibraryA(dllName);
	if (NULL==hServiceDLL)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:Load library fault,DLLName:%s",dllName);
		return -0x31;
	}

	//获取服务函数
	FARPROC CalledFunction = GetProcAddress(hServiceDLL,m_functionName);

	if (nullptr == CalledFunction)
	{
		OutputDebug(L"Service_FlowToFormat_Execute:GetProcAddress fault,DLLName:%s,functionName:%s",dllName,m_functionName);
		return -0x41;
	}

	typedef void (*callback_fake)(char*,int);
	typedef int  (*called_function)(char*,callback_fake);

	called_function Service_Work_Function=(called_function)CalledFunction;

	if (bAsync)//异步
	{
		
		//配置callback;
		callback_fake callback;
		if (pFlowBase->permit_callback)
		{
			callback = ServiceAsyncCallBack;
			/*向栈中添加数据，以使 伪回调 能够知道它的ID_proc,id。*/
			st_Async_Thread_Callback tmp = {pFlowBase->ID_proc,pFlowBase->functionID};
			g_Async_Thread_Callback->push(GetCurrentThreadId(),tmp);

		}

		Service_Work_Function(pArgvCall,callback);//不关心返回值			
	}
	else//同步，1.检查返回值，2.检查是否有修改参数。3.转换成流。4.发送
	{
		const int ret_value = Service_Work_Function(pArgvCall,nullptr);

		/*
		用备份做操作体

		快速模式
		1. 做对比:queue和备份
		2. 发送备份
		标准模式
		1. 直接发送备份



		*/
		if (QUICK_FLOW_MODE==pFlowBase->argvTypeOption)
		{/*
		 借用调用参数的结构，
		 1. 检测修改，
		 2. 一旦没修改，则释放指针和置0.	
		 */

			//抛异常，不正常，数量问题
			//指针为空,长度为0,备用指针也为空。否则一则为空是错误。

			if ((pFlowBase->number_Of_Argv_Pointer!=0)&&(pSecondCopyArgv==nullptr))
			{
				OutputDebug(L"Service_FlowToFormat_Execute:Error:Unexpected logic.");
				throw("Service_FlowToFormat_Execute:Error:Unexpected logic.");
			}


			//中转使用仅此而已
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
					if (0==ret)//无改动
					{
						delete(p_old_data);
						(*(int*)(pSecondCopyArgv+offset)) = 0;//指针
					}
				}


				offset+=sizeof(int);
			}

			//复原上面对保护队列的修改使用,为了未来可能的使用。
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
			同样借用调用参数的结构
			*/
			;
		}
		else
		{
			OutputDebug(L"Service_FlowToFormat_Execute:Input struct Value Error:argvTypeOption value is unexpected");
			return -23;
		}
		/*
		1. 去掉没有不需要回传的参数  提交给Format2Flow
		2. Send
		*/
		{
			int ID_proc = pFlowBase->ID_proc;
			int SN		= pFlowBase->functionID;

			//没有指针数据的情况
			char* pStruct=nullptr;
			if (0==pFlowBase->number_Of_Argv_Pointer)
			{
				pStruct = nullptr;
			}
			else
			{
				pStruct = pSecondCopyArgv;
			}
			//大小调整到纯指针状态
			int m_sizeOfStruct  = 2*sizeof(int)*pFlowBase->number_Of_Argv_Pointer;//=format_len;

			int m_pointerNumber = pFlowBase->number_Of_Argv_Pointer;
			int work_type = CDataFormat::RECV_INFO;
			int argvTypeOption = pFlowBase->argvTypeOption;
			int async = false;//同步
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

			//发送flowBuffer,realBufferLen

			pCWEB->Send(flowBuffer,realBufferLen);

			delete(flowBuffer);
			flowBuffer = nullptr;
		}



	}


	//释放保护指针队列,空队列会异常
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

	//释放对比指针结构
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
	//同异步处理
	//在于回调 和 是否检查这些指针发生了改变。用于快速回传。
	//异步由回调发送数据
	//同步由本函数做参数检查和返回值检查
}

void CDataFormat::ServiceAsyncCallBack(char* p,int p_len)
{
	/*
	1. 获取TID对应的过程ID_proc,function_ID;使用栈方式
	在启动线程前启动线程者，做这个操作:

	struct T()
	{
	ID_proc;
	function_ID
	}
	stack<T>.push(threadID);

	这里进行find操作.问题是现在没有创建进程的家伙。这个家伙应该是WEB的Recive函数。
	所以下一步就是写这个Recive函数。


	2. 转换成flow
	3. Web::Send

	*/
	char* pStruct = new char[2*sizeof(int)]();//struct {char* ,int len}


	{
		
		st_Async_Thread_Callback st_thread = g_Async_Thread_Callback->pop(GetCurrentThreadId());


		int ID_proc = st_thread.ID_proc;
		int SN		= st_thread.function_ID;



		//没有指针数据的情况

		//大小调整到纯指针状态
		int number_Of_Argv_Pointer = 1;
		int m_sizeOfStruct  = 2*sizeof(int)*number_Of_Argv_Pointer;//=format_len;

		int m_pointerNumber = number_Of_Argv_Pointer;
		int work_type = CDataFormat::RECV_INFO;
		int argvTypeOption = CDataFormat::STANDARD_FLOW_MODE;
		int async = true;//同步
		FARPROC callBack = nullptr;
		int ret2client_value = -1;//无效


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

		//发送flowBuffer,realBufferLen

		pCWEB->Send(flowBuffer,realBufferLen);

		delete(flowBuffer);
		flowBuffer = nullptr;
	}
}
