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


	//先计算长度信息所占空间
	/*
	如果非指针参数长度为0,即不存在非指针参数，则非指针参数长度 不会出现在Flow中。
	*/
	int no_pointer_argv_length = sizeOfStruct - 2*sizeof(int)*ArgvPointerNumber;
	int m_real_length = sizeof(st_data_flow) + sizeof(int)*(ArgvPointerNumber+(no_pointer_argv_length?1:0));


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
		int* ptmp =(int*) pStruct;
		for (int i=0;i<ArgvPointerNumber;++i)
		{
			int* tmp;
			tmp = ptmp+(i+1);
			m_real_length+=*tmp;
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

	int* pBase =(int*) pStruct;
	int tmp_length = 0;//Argv_Struct这一部分的总长度

	int offset = 0;//在argv_Struct[]中的偏移

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

	//指针数据在流中所占总长度=指针数据的长度和+长度标记信息所占位置
	tmp_length += ArgvPointerNumber*sizeof(int);//+长度信息所占位置

	/*注意：非指针参数也需要一个长度位置标记自己的长度在Flow中。*/

	//非指针参数长度=结构体总长度-指针数量在结构体中所占长度

	//总的参数部分长度=非指针参数长度+一个长度位置标记+指针数据在流中所占总长度
	int t_ArgvLength = sizeOfStruct-ArgvPointerNumber*sizeof(int)*2;

	tmp_length += t_ArgvLength+(t_ArgvLength?sizeof(int):0);//如果非指针参数为0,那就不用长度来表示非指针参数了。

	psdf->length_Of_Argv_Struct = tmp_length;

	//写非指针参数长度 和 数据到Flow里面去。




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
	if (pFlowBase->length_of_this_struct - sizeof(st_data_flow)!= pFlowBase->length_Of_Argv_Struct)
	{
		OutputDebug(L"FlowToFormat:Input struct Format Error:length_Of_Argv_Struct != length_of_this_struct-sizeof(struct head)");
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

		offset += sizeof(int)+m_pointer_len;//本身长度+宣称的长度
	}
	//最后的非指针参数的长度
	int other_Length = 0;
	int tmp_argv_length =pFlowBase->length_Of_Argv_Struct;
	/*
	size > off	:
	size = off	:
	size < off	:Error
	*/
	if (tmp_argv_length>offset)
	{//有非指针参数
		other_Length = *(int*)(argv_Base+offset);
	}
	else if (tmp_argv_length==offset)
	{//没有非指针参数
		other_Length = 0;
	}
	else
	{
		OutputDebug(L"This is logic Error.");
		throw("This is logic Error.");
	}


	//总长度=指针参数长度+加上最后的非指针参数的长度+长度占地
	real_argv_length +=other_Length;
	/*非指针类结构体长度为0的时候不占地方*/
	real_argv_length +=(pFlowBase->number_Of_Argv_Pointer + (other_Length?1:0))*sizeof(int);


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
					char* pointer_data =(char*)(argv_Base+argv_flow_offset+1*sizeof(int));//+数据长度的位置，数据在长度后面

					int stat=memcpy_s(pPointerData,m_pointer_len,pointer_data,m_pointer_len);
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

				argv_flow_offset += +sizeof(int)+m_pointer_len;
			}//-for-end

		//////////////////////////////////////////////////////////////////////////
			//最后的非指针参数的长度
		
			int other_Length = 0;
			int tmp_argv_length =pFlowBase->length_Of_Argv_Struct;
			/*
			size > off	:
			size = off	:
			size < off	:Error
			*/
			if (tmp_argv_length>offset)
			{//有非指针参数
				other_Length = *(int*)(argv_Base+offset);
			}
			else if (tmp_argv_length==offset)
			{//没有非指针参数
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


			//如果是同步函数，后面的非指针参数也要备份一下。
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
	else//参数指针们皆为空,直接传入
	{
		;
	}
	return real_argv_length;
}



