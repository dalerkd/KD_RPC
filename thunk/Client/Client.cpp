// Client.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Client.h"
#include "debug.h"
#include <iostream>


// 这是导出变量的一个示例
CLIENT_API int nClient=0;

// 这是导出函数的一个示例。
CLIENT_API int fnClient(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 Client.h
CClient::CClient()
{
	return;
}



/*
异		SN	Name不需要
1		1
0		2


用链表放？还是一个表？这个不重要。Key-Value
Map
SN 异




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
所有Fake函数的集中点

1. 查询SN：异步否（第二个参数）,数据结构大小（第一个参数）
~~要知道参数中的 struct中所有指针的offset,和长度的offset~~

0. 函数名(无用)
1. SN
2. 异步否
3. 结构体有几个指针(规定指针必须在结构体中靠前

*/
int Core(int SN,PVOID pStruct,FARPROC callBack)
{
	/*
	参数复制拷贝 需要参数判定一下指针的数量?还是由格式化函数复制一站到底

	参数返回拷贝 只在同步哟

	等待方式取决于 异步否


	同异步不会对合成一个参数造成影响。

	*/
	//发送参数方式统一
	int m_pointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);
	int m_sizeOfStruct = g_CI_Client->QueryArgvStructSize(SN);


	LONG ID_proc = CID_Manager::GetNewID();

	//同异步——不同的返回方式
	const bool async =g_CI_Client->QueryASync(SN);


	//应该不会返回0，它会整合一些别的结构。
	int realBufferLen = g_CDF.Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
		CDataFormat::Query_INFO,CDataFormat::QUICK_FLOW_MODE,async,callBack);
	if (0==realBufferLen)
	{
		throw("Core: ToFlow return 0");
	}
	char* flowBuffer = new char[realBufferLen]();
	g_CDF.Format2Flow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,
		CDataFormat::Query_INFO,CDataFormat::QUICK_FLOW_MODE,async,callBack,_Out_ flowBuffer,realBufferLen);

	//发送flowBuffer,realBufferLen

	pCWEB->Send(flowBuffer,realBufferLen);

	delete(flowBuffer);
	flowBuffer = nullptr;


	if (async)//异步处理
	{
		pasm->push(ID_proc,callBack);
		return true;
	}
	else//同步处理
	{
		int ret;//返回值
		int PointerNumber = g_CI_Client->QueryArgvPointerNumber(SN);

		//Event
		HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);


		pssm->push(ID_proc,&ret,pStruct,PointerNumber,hdEvent);

		WaitForSingleObject(hdEvent,INFINITE);
		CloseHandle(hdEvent);
		return ret;
	}



}
//Fake函数
int aaa1(PVOID pStruct,FARPROC callBack)
{
	int m_SN = 1;
	return Core(m_SN,pStruct,callBack);
}

CFunctionInfo* g_CI_Client = new CFunctionInfo();

class CFunctionInfo
{
public:

	void AddInfo(int SN,char* funcName,bool asyn,int ArgvStringNumber,int sizeOfStruct/*参数结构体大小*/)
	{

	}

	bool QueryASync(int query_SN)//是否异步,true 异步,false 同步
	{

	}
	int  QueryArgvPointerNumber(int query_SN)//参数结构体中指针个数，例如:指针A，长度A，指针B，长度B......个数为2
	{

	}
	int  QueryArgvStructSize(int query_SN)//参数结构体总共多长，服务器必须在编译时刻提供。
	{

	}
	char* QueryFuncName(int query_SN)
	{


	}
private:
};


CFunctionInfo* g_CI_Service = new CFunctionInfo();

/*
参数和发送流格式之间的转换

*/
CDataFormat g_CDF;
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
	int Format2Flow(LONG ID_proc,int SN,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,int work_type,int argvTypeOption,bool async,FARPROC callback,char* flowBuffer=nullptr,int real_len=0,int ret_value=0)
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


	//流转换成格式:客户端解码：不是一个线程函数,因为它不阻塞：因为栈管理程序管理的是信号们。
	/*

	*/
	//void Client_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int& structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	void Client_FlowToFormat_Execute(char* flow,int flow_len)//由它来自行区分是否异步,并走不同的流程。
	{

	}
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

		//////////////////////////////////////////////////////////////////////////
		//参数合法性检测


		/*测验以真实长度计算
		总长度 = 头 + 尾
		length_of_this_struct = sizeof(st_data_flow) + length_Of_Argv_Struct
		*/
		if (pFlowBase->length_of_this_struct - sizeof(st_data_flow)!= pFlowBase->number_Of_Argv_Pointer)
		{
			OutputDebug(L"Service_FlowToFormat_Execute:Input struct Format Error:number_Of_Argv_Pointer != length_of_this_struct-sizeof(struct head)");
			return -0x11;
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
			OutputDebug(L"Service_FlowToFormat_Execute:Input struct Format Error:length_Of_Argv_Struct is UnReal");
			return -0x12;
		}

		//这个长度被调用者知道哟，只是我们不知道哟，所以我们给个指针就完事。
		const int format_len = pFlowBase->number_Of_Argv_Pointer*2*sizeof(int)+other_Length;

		/*原型：int aaa1(PVOID* pStruct,FARPROC callBack)*/

		//////////////////////////////////////////////////////////////////////////
		//复制操作
		//用于参数
		char* pArgvCall = nullptr;
		//用于同步参数修改状态对比
		char* pSecondCopyArgv = nullptr;
		//只管理申请的参数指针内存指针
		CSafeQueue<char*>* queue_memory_manage = nullptr;

		if (format_len!=0)
		{
			pArgvCall = new char[format_len]();
			
			if (false == bAsync)
			{//同步则构建第二个保留区用于对比服务
				pSecondCopyArgv = new char[format_len]();
			}

			/*
				复制操作
			*/

			
			queue_memory_manage = new CSafeQueue<char*>();

			

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
					char* pSecondPointerData = nullptr;

					if (0!=m_pointer_len)
					{
						pPointerData = new char[m_pointer_len]();

						//给用户的数据可能被用户破坏，所以为了保证释放所以用栈备份指针
						queue_memory_manage->push(pPointerData);
						
						char* pointer_data =(char*)(argv_Base+argv_flow_offset);

						int stat=memcpy_s(pPointerData,m_pointer_len,pointer_data,m_pointer_len);
						if (stat)
						{
							throw("memcpy_s pPointerData return err.");
						}

						//检查是否是同步，是的话：备份指针，以便调用后对比结果。
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
					*(int*)point= (int)pPointerData;//数据指针填充
					point+=sizeof(int);
					*(int*)point= m_pointer_len;//长度填充

					//检查是否是同步，是的话：备份指针，以便调用后对比结果。
					if (nullptr!=pSecondCopyArgv)
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

				//如果是同步函数也备份一下。。。
				char* p_format_end_data_copy = pSecondCopyArgv + argv_flow_offset;

				int stat = memcpy_s(p_format_end_data_copy,other_Length,p_flow_end_data,other_Length);
				if (stat)
				{
					throw("memcpy_s p_format_end_data return err.");
				}

			}


		}
		else//参数指针们皆为空,直接传入
		{
			;
		}
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


		typedef void (CDataFormat::*callback_fake)(char*,int);
		typedef int  (*called_function)(char*,callback_fake);

		called_function Service_Work_Function=(called_function)CalledFunction;

		if (bAsync)//异步
		{
			callback_fake callback;
			if (pFlowBase->permit_callback)
			{
				callback = ServiceAsyncCallBack;
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
	/*
	提供给异步服务器方法的回调函数

	用于获取返回的数据

	并在这里发送
	*/
	void ServiceAsyncCallBack(char* p,int p_len)
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
			int ID_proc = ?->ID_proc;
			int SN		= ?->functionID;



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

	private:
		

};//首先我得知道同异步情况，它们的返回数据格式是不同的？谁负责其中指针内存的申请和销毁比较合适？
/*
上面遇到些许的困境：谁来申请这可能很多的内存？和管理它们的指针？
一种方案是由Service_FlowToFormat来做这件事，并由它直接调用目标服务。

目标服务返回的时候，进行销毁。

目标服务迟早要返回，但是：它可能很久才返回，所以这里适合用线程做。

所以Service_FlowToFormat是一个线程函数，比较合适。


*/







/*
网络层：
实现以下功能：
半异步发送数据。运用生产者-消费者模型，只在栈满的时候阻塞用户。
等待全部发送完毕:该接口提供一个通知，当所有数据发送完毕的时候触发，用于程序退出等。

有消息时,如果是同步就消灭。。。？
有消息来时，如果是更新消息
交给CFunctionInfo处理.
如果是其他消息
push给Stack管理程序：由Stack管理程序来具体解包。


*/

	/*
	发送机制我想玩些别的。除了传统的直接调用发送，

	另一种想法是将数据放在发送栈中,而在放入这个操作是互斥的，且检查栈有没有满。
	“生产者，消费者模型”，这样的
	好处是：

	1. 网络发送不消耗调用的时间。该返回返回。
	2. 使“调用”和“网络发送”两个过程相互独立，错误之类的也不会外泄到不相干的层。
	3. 提高了吞吐速度，发送其实是个抢占过程，而短时间内调用增加，会导致大家都等待。

	缺点是：
	1. 多了一次复制手续，这要是内存不够就。。。
	2. 多了那么些管理步骤，似乎效率有所降低。


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


	/*异步发送
	1. 将数据复制到自己申请的空间
	2. 保存到栈里
	3. 返回
	*/
	void Send(char* data,int data_len)//其实是个push而已.
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
	/*当前时刻，是否全部发送完毕
	可被用做退出前确认数据是否发送完毕。
	注意：无法保证别的线程不来发数据，除非每个线程都做检查哈哈。

	即使这样，也不能保证数据已经被发送，它可能是刚被从栈中取出来了。
	*/
	bool AllSendIsFinish()
	{
		return m_CSQ->IsEmpty();
	}

private:
	/*！！！******产生异常：说明空了 exception::char*
	用途：弹出一条发送任务
	*/
	st_asysnc_queue_argv findAndPop()
	{
		return m_CSQ->pop();
	}

private:
	CSafeQueue<st_asysnc_queue_argv>* m_CSQ;//发送数据栈
};

//客户端 异步状态管理
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
	//ID_proc:唯一的号码用来区分本次调用。
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


//客户端 同步状态管理
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
	//ID_proc:唯一的号码用来区分本次调用。
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
//线程安全的队列,push队尾插入,pop队首插入
//异常抛出：在队列已经空的情况下还要pop。
#include <queue>
using std::queue;
template <typename TSafeQueue>
//安全队列
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
	TSafeQueue pop()//队尾
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
	某一时刻队列是否为空.
	用于在退出前检查自己线程的任务是否发送完毕了。
	当然不能保证别的线程再往里面塞。
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
//处理map<ID_proc,struct>
//异常抛出：在意外情况，此时应当调试。
#include <map>
using std::map;
template <typename TSafeMapData>
//安全map
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
负责产生唯一ID。该ID被用在标记任务的唯一性上：
一个被标号的数据流从服务器回来还能知道它来自于哪里。
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


