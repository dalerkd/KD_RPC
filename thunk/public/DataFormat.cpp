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


	//先计算长度信息所占空间
	/*
	如果非指针参数长度为0,即不存在非指针参数，则非指针参数长度 不会出现在Flow中。
	*/
	LONG64 no_pointer_argv_length = sizeOfStruct - 2*sizeof(LONG64)*ArgvPointerNumber;
	LONG64 m_real_length = sizeof(st_data_flow) + sizeof(LONG64)*(ArgvPointerNumber+(no_pointer_argv_length?1:0));


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
	{//计算指针数据长度
		LONG64* ptmp =(LONG64*) pStruct;
		for (LONG64 i=0;i<ArgvPointerNumber;++i)
		{
			LONG64* tmp;
			if (nullptr!=(char*)*(ptmp+(i*2)))//数据指针为空，则不计算长度，用于快速模式。
			{
				tmp = ptmp+(i*2+1);//数据长度
				m_real_length+=*tmp;
			}

		}
	}

	m_real_length += no_pointer_argv_length;//加上非指针参数的数据长度



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

	psdf->length_of_this_struct = m_real_length;
	psdf->work_type = work_type;
	psdf->ID_proc	= ID_proc;
	psdf->functionID= SN;
	psdf->async		= async;
	psdf->permit_callback = callback?true:false;
	psdf->return_value = ret_value;
	psdf->argvTypeOption = argvTypeOption;

	//psdf->length_Of_Argv_Struct;//后面有赋值
	psdf->number_Of_Argv_Pointer = ArgvPointerNumber;

	LONG64* pBase =(LONG64*) pStruct;
	LONG64 tmp_length = 0;//Argv_Struct这一部分的总长度

	LONG64 offset = 0;//在argv_Struct[]中的偏移

	for (LONG64 i=0;i<ArgvPointerNumber;++i)
	{
		LONG64* ptmp_length;
		LONG64* ptmp_pointer;
		ptmp_pointer= pBase+2*i;
		ptmp_length	= pBase+(2*i+1);

		//check
		if (nullptr == (char*)*ptmp_pointer&&0!=*ptmp_length)
		{
			;//说明正在进行快速传递模式
		}
		//copy
		LONG64* plength = (LONG64*)(psdf->argv_Struct+offset);

		*plength = *ptmp_length;

		offset += sizeof(LONG64);

		if (nullptr != (char*)*ptmp_pointer)//当指针不为空时，长度必须不为0
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
		else//快速传输模式,不复制不关心长度
		{
			offset+=0;
			tmp_length+=0;
		}


	}

	//指针数据在流中所占总长度=指针数据的长度和+长度标记信息所占位置
	tmp_length += ArgvPointerNumber*sizeof(LONG64);//+长度信息所占位置

	/*注意：非指针参数也需要一个长度位置标记自己的长度在Flow中。*/

	//非指针参数长度=结构体总长度-指针数量在结构体中所占长度

	//总的参数部分长度=非指针参数长度+一个长度位置标记+指针数据在流中所占总长度
	LONG64 t_ArgvLength = sizeOfStruct-ArgvPointerNumber*sizeof(LONG64)*2;

	tmp_length += t_ArgvLength+(t_ArgvLength?sizeof(LONG64):0);//如果非指针参数为0,那就不用长度来表示非指针参数了。

	psdf->length_Of_Argv_Struct = tmp_length;

	//写非指针参数长度 和 数据到Flow里面去。




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
char*	pFlow				待解析流指针
LONG64		Flow_len			待解析流长度

_Out_:
char*	pArgvCall			用于Format参数结构的存放Buffer,调用者申请。
CSafeQueueAutoPointerManage* queue_memory_manage 只管理Format申请的参数指针内存指针
LONG64		Real_Format_len		需要的Buffer的长度,如果不知道就写0.将会返回正确的长度值写入即可。这里只作为校验长度用。
-----------------------
以下仅用于服务器端：
char*	pSecondCopyArgv		用于同步参数修改状态对比-只用在服务端的同步对比时。调用者申请。长度和上面的一样。
CSafeQueueAutoPointerManage* queue_memory_copy 只管理Format备份内存指针,

Exception:
类型	介绍			建议
----------------------------
char*	|内部逻辑问题|	挂起
LONG64		|数据格式问题|	非调试状态可以忽略


return 占用的数据长度
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
	//参数合法性检测


	/*测验以真实长度计算
	总长度 = 头 + 尾
	length_of_this_struct = sizeof(st_data_flow) + length_Of_Argv_Struct
	*/
	if (pFlowBase->length_of_this_struct - sizeof(st_data_flow)!= pFlowBase->length_Of_Argv_Struct)
	{
		OutputDebug("FlowToFormat:Input struct Format Error:length_Of_Argv_Struct != length_of_this_struct-sizeof(struct head)");
		throw(0x11);
	}




	//长度应当是这个数字,检验实际是这个数字吗
	const LONG64 tmp_number_Of_Argv_Pointer = pFlowBase->length_of_this_struct - sizeof(st_data_flow);

	//检验指针结构体的数量

	const bool bAsync = (char)pFlowBase->async;
	const char* argv_Base = pFlowBase->argv_Struct;

	LONG64 offset=0;
	//获取总长度并校验，如果通过就用新的长度申请内存

	LONG64 real_argv_length = 0;//实际计算得到的总长度

	//计算所有指针数据的长度
	for (LONG64 i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
	{
		LONG64 m_pointer_len = *(LONG64*)(argv_Base+offset);
		real_argv_length += m_pointer_len;

		offset += sizeof(LONG64)+m_pointer_len;//本身长度+宣称的长度
	}
	//最后的非指针参数的长度
	LONG64 other_Length = 0;
	LONG64 tmp_argv_length =pFlowBase->length_Of_Argv_Struct;
	/*
	size > off	:
	size = off	:
	size < off	:Error
	*/
	if (tmp_argv_length>offset)
	{//有非指针参数
		other_Length = *(LONG64*)(argv_Base+offset);
	}
	else if (tmp_argv_length==offset)
	{//没有非指针参数
		other_Length = 0;
	}
	else
	{
		OutputDebug("This is logic Error.");
		throw("This is logic Error.");
	}


	//总长度=指针参数长度+加上最后的非指针参数的长度+长度占地
	real_argv_length +=other_Length;
	/*非指针类结构体长度为0的时候不占地方*/
	real_argv_length +=(pFlowBase->number_Of_Argv_Pointer + (other_Length?1:0))*sizeof(LONG64);


	if (real_argv_length!=pFlowBase->length_Of_Argv_Struct)
	{
		OutputDebug("FlowToFormat:Input struct Format Error:length_Of_Argv_Struct is UnReal");
		throw(0x12);
	}

	//这个长度被调用者知道哟，只是我们不知道哟，所以我们给个指针就完事。
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




	/*原型：LONG64 aaa1(PVOID* pStruct,FARPROC callBack)*/

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
			LONG64 argv_flow_offset = 0;
			LONG64 argv_format_offset=0;
			for (LONG64 i=0;i<pFlowBase->number_Of_Argv_Pointer;++i)
			{
				LONG64 m_pointer_len = *(LONG64*)(argv_Base+argv_flow_offset);

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
					char* pointer_data =(char*)(argv_Base+argv_flow_offset+1*sizeof(LONG64));//+数据长度的位置，数据在长度后面

					LONG64 stat=memcpy_s(pPointerData,m_pointer_len,pointer_data,m_pointer_len);
					if (stat)
					{
						throw("memcpy_s pPointerData return err.");
					}
					//			pArgvCall没有被填充内容，我去。！！！



					//检查是否是同步，是的话：备份指针，以便调用后对比结果。
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
				*(LONG64*)point= (LONG64)pPointerData;//数据指针填充
				point+=sizeof(LONG64);
				*(LONG64*)point= m_pointer_len;//长度填充

				//检查是否是同步，是的话：备份指针，以便调用后对比结果。
				if (nullptr!=pSecondCopyArgv/*false == bAsync&& RECV_INFO==pFlowBase->work_type*/)
				{
					char* t_point = pSecondCopyArgv + argv_format_offset;
					*(LONG64*)t_point= (LONG64)pSecondPointerData;//数据指针填充
					t_point+=sizeof(LONG64);
					*(LONG64*)t_point= m_pointer_len;//长度填充

				}
				argv_format_offset+=sizeof(LONG64)+sizeof(LONG64);//指针+长度

				argv_flow_offset += +sizeof(LONG64)+m_pointer_len;
			}//-for-end

			//////////////////////////////////////////////////////////////////////////
			//最后的非指针参数的长度

			LONG64 other_Length = 0;
			LONG64 tmp_argv_length =pFlowBase->length_Of_Argv_Struct;
			/*
			size > off	:
			size = off	:
			size < off	:Error
			*/
			if (tmp_argv_length>offset)
			{//有非指针参数
				other_Length = *(LONG64*)(argv_Base+offset);
			}
			else if (tmp_argv_length==offset)
			{//没有非指针参数
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


			//如果是同步函数，后面的非指针参数也要备份一下。
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
	else//参数指针们皆为空,直接传入
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

		//发送flowBuffer,realBufferLen

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
	测试 流-转格式，返回格式队列+备份格式队列。
		Async决定了是否就是采用备份，备份格式队列的用途：在设计中用做对比用户对指针内容的修改。
		设计中自动指针管理是个不错的选择。解决了“结构中拥有子结构是指针结构”的构造与释放问题。
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
		delete(pFlowBase);
		delete(pArgvCall);
		delete(queue_memory_manage);
		delete(pSecondCopyArgv);
		delete(queue_memory_copy);
	}


	return true;
}

