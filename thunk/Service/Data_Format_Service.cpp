#include "stdafx.h"
#include "Data_Format_Service.h"
#include "../public/FunctionInfo.h"
#include "../public/debug.h"
#include "../public/WEB.h"


struct st_Async_Thread_Callback 
{
	LONG ID_proc;
	LONG64 function_ID;
};
//服务端 异步回调任务,线程ID为Key,要发送的数据为value.
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
		OutputDebug("Service_FlowToFormat_Execute:Input pointer==nullptr");
		return -1;
	}

	if (nullptr == p->flow)
	{
		OutputDebug("Service_FlowToFormat_Execute:Input struct ponter==nullptr");
		return -2;
	}

	st_data_flow* pTmp_Flow = (st_data_flow*)p->flow;

	if (pTmp_Flow->length_of_this_struct!=p->flow_len)
	{
		OutputDebug("Service_FlowToFormat_Execute:Input struct Format Error:flow_LEN!=length_of_this_struct");
		return -0x10;
	}

	//copy 指针
	const st_data_flow* pFlowBase =(st_data_flow*) new char[p->flow_len]();

	LONG64 stat = memcpy_s((char*)pFlowBase,p->flow_len,p->flow,p->flow_len);
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

	//////////////////////////////////////////////////////////////////////////
	/*Flow2Format*/

	char* pArgvCall  = nullptr;
	char* pSecondCopyArgv = nullptr;
	CSafeQueueAutoPointerManage* queue_memory_manage = nullptr;
	CSafeQueueAutoPointerManage* queue_memory_copy	 = nullptr;
	const bool bAsync = pFlowBase->async;
	
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
		if (false == bAsync&&true)//就是这么奇怪。。。
		{
			pSecondCopyArgv = new char[real_len]();
			queue_memory_copy = new CSafeQueueAutoPointerManage();
		}

		Flow2Format((char*)pFlowBase,pFlowBase->length_of_this_struct,pArgvCall,real_len,queue_memory_manage,pSecondCopyArgv,queue_memory_copy);


	}
	catch (char* string)
	{
		throw(string);//内部结构错误
	}
	catch (LONG64 errCode)//文件格式错误忽略之
	{
		OutputDebug("Flow Format Err,code:0x%x",errCode);
		return errCode;
	}
//////////////////////////////////////////////////////////////////////////


	/*
	0. 查询函数名称，调用函数
	1. 如果是同步函数检查修改
	和返回值，并发送数据
	2. 释放所有指针通过栈
	*/
	LONG64 funName_Len = 0;
	funName_Len = g_CI_Service->QueryFuncName(pFlowBase->functionID);
	char* pFunctionName = new char[funName_Len]();
	g_CI_Service->QueryFuncName(pFlowBase->functionID,pFunctionName);


	if (pFunctionName==nullptr)
	{
		OutputDebug("Service_FlowToFormat_Execute:query functionID Do'nt find Name,functionID:%d",pFlowBase->functionID);
		return -0x21;
	}
	


	//这里的代码需要迁移到更高层全局去。
	char* dllName = "ServiceDLL.dll";
	HMODULE hServiceDLL = LoadLibraryA(dllName);
	if (NULL==hServiceDLL)
	{
		OutputDebug("Service_FlowToFormat_Execute:Load library fault,DLLName:%s",dllName);
		return -0x31;
	}

	//获取服务函数
	FARPROC CalledFunction = GetProcAddress(hServiceDLL,pFunctionName);
	
	if (nullptr == CalledFunction)
	{
		OutputDebug("Service_FlowToFormat_Execute:GetProcAddress fault,DLLName:%s,functionName:%s",dllName,pFunctionName);
		delete(pFunctionName);
		pFunctionName = nullptr;
		return -0x41;
	}
	delete(pFunctionName);
	pFunctionName = nullptr;
	


	typedef void (*callback_fake)(char*,LONG64);
	typedef LONG64  (*called_function)(char*,callback_fake);

	called_function Service_Work_Function=(called_function)CalledFunction;

	if (bAsync)//异步
	{
		
		//配置callback;
		callback_fake callback=nullptr;
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
		const LONG64 ret_value = Service_Work_Function(pArgvCall,nullptr);

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
				OutputDebug("Service_FlowToFormat_Execute:Error:Unexpected logic.");
				throw("Service_FlowToFormat_Execute:Error:Unexpected logic.");
			}


			//中转使用仅此而已
			CSafeQueue<char*>* tmp_queue = new CSafeQueue<char*>();

			for (LONG64 offset = 0,i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
			{
				char* p_new_data = nullptr;
				try
				{ 
					p_new_data = queue_memory_manage->pop();
					tmp_queue->push(p_new_data);

					;}
				catch(...)
				{
					OutputDebug("Service_FlowToFormat_Execute:Error:Unexpected:\
								 queue_memory_manage's number is Incorrect is 0x%x,\
								 true number is:0x%x",i,pFlowBase->number_Of_Argv_Pointer);
					throw("Service_FlowToFormat_Execute:Error:Unexpected:\
						  queue_memory_manage's number is Incorrect");

				}
				char* p_old_data = (char*)(*(LONG64*)(pSecondCopyArgv+offset));
				offset+=sizeof(LONG64);
				LONG64	data_len	 = *(LONG64*)(pSecondCopyArgv+offset);


				if (!((p_new_data!=nullptr&&p_old_data!=nullptr&&data_len!=0)||(p_new_data==nullptr&&p_old_data==nullptr&&data_len==0)))
				{
					OutputDebug("Service_FlowToFormat_Execute:Error:Unexpected value,\
								 p_new_data:0x%x;p_old_data:0x%x;data_len:0x%x.",p_new_data,p_old_data,data_len);
					throw("Service_FlowToFormat_Execute:Error:Unexpected value.");
				}

				if (nullptr!=p_old_data)
				{
					LONG64 ret = memcmp(p_old_data,p_new_data,data_len);
					if (0==ret)//无改动
					{
						(*(LONG64*)(pSecondCopyArgv+(offset-sizeof(LONG64)))) = 0;//指向数据的指针设为nullptr
						(*(LONG64*)(pSecondCopyArgv+offset)) = 0;				//长度也必须为0.！！
					}
					else
					{
						(*(LONG64*)(pSecondCopyArgv+(offset-sizeof(LONG64)))) = (LONG64)p_new_data;//有改动时，使用新数据
					}
				}


				offset+=sizeof(LONG64);
			}

			//复原上面对保护队列的修改使用,为了未来可能的使用。
			for (LONG64 i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
			{
				try{queue_memory_manage->push(tmp_queue->pop());}
				catch(...)
				{
					OutputDebug("Service_FlowToFormat_Execute:Error:Unexpected2:\
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
			OutputDebug("Service_FlowToFormat_Execute:Input struct Value Error:argvTypeOption value is unexpected");
			return -23;
		}
		/*
		1. 去掉没有不需要回传的参数  提交给Format2Flow
		2. Send
		*/
		{
			LONG64 ID_proc = pFlowBase->ID_proc;
			LONG64 SN		= pFlowBase->functionID;

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
			LONG64 m_sizeOfStruct  = 2*sizeof(LONG64)*pFlowBase->number_Of_Argv_Pointer;//=format_len;

			LONG64 m_pointerNumber = pFlowBase->number_Of_Argv_Pointer;
			LONG64 work_type = CDataFormat::RECV_INFO;
			LONG64 argvTypeOption = pFlowBase->argvTypeOption;
			LONG64 async = false;//同步
			FARPROC callBack = nullptr;
			LONG64 ret2client_value = ret_value;


			//Format2Flow(LONG ID_proc,LONG64 SN,char*pStruct,LONG64 sizeOfStruct,LONG64 ArgvPointerNumber,LONG64 work_type,LONG64 argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,LONG64 real_len=0)
			LONG64 realBufferLen = g_CDF.Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
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





	//释放对比指针结构 - 废弃代码 已经自动实现释放了
	//if (pSecondCopyArgv!=nullptr)
	//{
	//	const LONG64 tmp_argv_number = pFlowBase->number_Of_Argv_Pointer;

	//	LONG64 offset = 0;
	//	for (LONG64 i=0;i<tmp_argv_number;i++)
	//	{
	//		char* p = (char*)(*(LONG64*)(pSecondCopyArgv+offset));
	//		delete(p);
	//		offset+=sizeof(LONG64)*2;
	//	}
	//}


	//释放保护指针队列们-曾用于保存 指针结构 和 对比指针结构
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
	//同异步处理
	//在于回调 和 是否检查这些指针发生了改变。用于快速回传。
	//异步由回调发送数据
	//同步由本函数做参数检查和返回值检查
}

struct argv_tmp 
{
	char* p;
	LONG64 p_len;
};

/*
服务器上fack回调。

*/
void CData_Format_Service::ServiceAsyncCallBack(char* p,LONG64 p_len)
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
	char* pStruct = new char[2*sizeof(LONG64)]();//struct {char* ,LONG64 len}
	((argv_tmp*)pStruct)->p = p;
	((argv_tmp*)pStruct)->p_len = p_len;

	{
		
		st_Async_Thread_Callback st_thread = g_Async_Thread_Callback->pop(GetCurrentThreadId());


		LONG64 ID_proc = st_thread.ID_proc;
		LONG64 SN		= st_thread.function_ID;



		//没有指针数据的情况

		//大小调整到纯指针状态
		LONG64 number_Of_Argv_Pointer = 1;
		LONG64 m_sizeOfStruct  = 2*sizeof(LONG64)*number_Of_Argv_Pointer;//=format_len;

		LONG64 m_pointerNumber = number_Of_Argv_Pointer;
		LONG64 work_type = CDataFormat::RECV_INFO;
		LONG64 argvTypeOption = CDataFormat::STANDARD_FLOW_MODE;
		LONG64 async = true;//同步
		FARPROC callBack = nullptr;
		LONG64 ret2client_value = -1;//无效


		//Format2Flow(LONG ID_proc,LONG64 SN,char*pStruct,LONG64 sizeOfStruct,LONG64 ArgvPointerNumber,LONG64 work_type,LONG64 argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,LONG64 real_len=0)
		LONG64 realBufferLen = g_CDF.Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
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
