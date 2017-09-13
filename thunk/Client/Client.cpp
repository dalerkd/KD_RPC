// Client.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Client.h"


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
int Core(int SN,PVOID* pStruct,FARPROC callBack)
{
	/*
		参数复制拷贝 需要参数判定一下指针的数量?还是由格式化函数复制一站到底

		参数返回拷贝 只在同步哟

		等待方式取决于 异步否


		同异步不会对合成一个参数造成影响。

	*/
//发送参数方式统一
	int m_pointerNumber = g_CI->QueryArgvPointerNumber(SN);
	int m_sizeOfStruct = g_CI->QueryArgvStructSize(SN);


	LONG ID_proc = CID_Manager::GetNewID();

	//应该不会返回0，它会整合一些别的结构。
	int realBufferLen = g_CDF.ToFlow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber);
	if (0==realBufferLen)
	{
		throw("Core: ToFlow return 0");
	}
	
	char* flowBuffer = new char[realBufferLen]();
	
	

	g_CDF.ToFlow(ID_proc,SN,pStruct,m_sizeOfStruct,m_pointerNumber,_Out_ flowBuffer,realBufferLen);

	//发送flowBuffer,realBufferLen
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
	pCWEB->Send(flowBuffer,realBufferLen);

	delete(flowBuffer);
	flowBuffer = nullptr;


//同异步——不同的返回方式
	bool async =g_CI->QueryASync(SN);

	if (async)//异步处理
	{
		pasm->push(ID_proc,callBack);
		return true;
	}
	else//同步处理
	{
		int ret;//返回值
		int PointerNumber = g_CI->QueryArgvPointerNumber(SN);
		
		//Event
		HANDLE hdEvent =CreateEvent(NULL,FALSE,FALSE,NULL);
		
		
		pssm->push(ID_proc,&ret,pStruct,PointerNumber,hdEvent);

		WaitForSingleObject(hdEvent,INFINITE);
		CloseHandle(hdEvent);
		return ret;
	}



}
//Fake函数
int aaa1(PVOID* pStruct,FARPROC callBack)
{
	int m_SN = 1;
	Core(m_SN,pStruct,callBack);
}

CFunctionInfo* g_CI = new CFunctionInfo();

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
private:
};

/*
参数和发送流格式之间的转换

*/
CDataFormat g_CDF;
class CDataFormat
{
public:
/************************************************************************/
/* 客户端使用                                                           */
/************************************************************************/
	//转换成流：客户端编码
	/*
	返回值:			实际需要的buffer长度
	pStruct:		指向参数结构体的指针
	sizeOfStruct:	结构体的长度
	ArgvPointerNumber:结构体中指针的数量,格式为最前排列:{指针,长度}{指针,长度}
	flowBuffer:		生成流需要的存储区指针
	real_len:		生成流准备的存储区长度

	异常:
	如果提供错误的长度会得到异常。

	*/
	int Client_FormatToFlow(LONG ID_proc,char*pStruct,int sizeOfStruct,int ArgvPointerNumber,char* flowBuffer=nullptr,int real_len=0)
	{
		/*返回真正需要的长度给外部
		如果real_len!=真实长度的话,就不拷贝,而是返回真正的长度.
		如果参数长度!=0,且与真正计算长度,抛异常.-这意味着发生了某种变动在两次计算之间。

		这种解决方案,真是巧妙啊。防止缓冲区不足的情况。设计灵感来自于微软的宽窄转换系列。
		如果不这样做的话，就得以下三种方案：
		1. 由掌握长度的函数被迫自行分配，调用者得释放一个不是它申请的内存，权责不明：结果导致内存泄漏。
		2. 每次不足就得抛异常或者错误值：不容易接啊，操作繁琐。
		3. 由其他函数负责获取长度。分离导致容易出现次序错误。

		*/
	}
	
	//流转换成格式:客户端解码：不是一个线程函数,因为它不阻塞：因为栈管理程序管理的是信号们。
	/*
		
	*/
	//void Client_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int& structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	void Client_FlowToFormat_Execute(char* flow,int flow_len);//由它来自行区分是否异步,并走不同的流程。

/************************************************************************/
/* 服务端使用															*/
/************************************************************************/
	
	//异步格式转换成流:服务端编码
	void Service_FormatToFlow_Async();

	//同步格式转换成流:服务端编码
	void Service_FormatToFlow_Sync();



	
	struct  st_thread_Service_FlowToFormat_Excute
	{
		char* flow;
		int flow_len;
		HANDLE 信号;//上面两个参数复制完毕就触发这个信号。
	};

	//流转换成格式:服务端解码：这是一个线程函数。
	//void Service_FlowToFormat_Execute(char* flow,int flow_len,_Out_ int& ID_proc,_Out_ char *pStruct ,_Out_ int structLen,_Out_ int& ArgvPointerNumber,_Out_ bool& async);
	DWORD Service_FlowToFormat_Execute(st_thread_Service_FlowToFormat_Excute* p)
	{

	}
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
		memcpy_s(p,data_len,data,data_len);

		st_asysnc_queue_argv tmpArgv = {p,data_len};
		m_CSQ->push(tmpArgv);

		return;
	}
	/*当前时刻，是否全部发送完毕
	可被用做退出前确认数据是否发送完毕。
	注意：无法保证别的线程不来发数据，除非每个线程都做检查哈哈。
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
#include <iostream>
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


