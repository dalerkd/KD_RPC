// testEverythin.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include "SafeQueue.h"


//
//
//
//
//struct st_data_flow
//{
//	int		length_of_this_struct;
//	int		work_type;//本次工作类型？获取函数信息,回复信息,发送请求
//	long	ID_proc;
//	int 	functionID;
//
//	//是否异步
//	char	async;
//	//是否允许回调
//	char	permit_callback;
//	int		return_value;//返回值,在同步下服务器端有效
//	//参数结构体格式扩展配置，当前为空。
//	int		argvTypeOption;	
//
//	/*参数结构体总长度*/
//	int		length_Of_Argv_Struct;
//	/*参数结构体中指针结构体的数量*/
//	int		number_Of_Argv_Pointer;
//
//	char	argv_Struct[0];   
//
//	/*
//	指针结构体 的数量：
//		指的是参数中的指针结构体的数量.
//		eg:
//		struct argv
//		{
//			char* a
//			int len_a;
//			char* b
//			int len_b;
//			char c
//		}	
//		其中指针结构体的数量是2.
//
//	
//
//
//	*/
//
//};
//struct st_argv_Node_Struct
//{
//	int length;
//	char data[0];
//};
//
//
//
//
////////////////////////////////////////////////////////////////////////////
////线程安全的队列,push队尾插入,pop队首插入
////异常抛出：在队列已经空的情况下还要pop。
//#include <iostream>
//#include <queue>
//using std::queue;
//template <typename TSafeQueue>
////安全队列
//class CSafeQueue
//{
//public:
//	CSafeQueue()
//	{
//		InitializeCriticalSection(&g_csSafeThread);
//	};
//	~CSafeQueue()
//	{
//		DeleteCriticalSection(&g_csSafeThread);
//	}
//
//public:
//	void push(TSafeQueue data)
//	{
//		EnterCriticalSection(&g_csSafeThread);
//		m_queue.push(data);
//		LeaveCriticalSection(&g_csSafeThread);
//	}
//	TSafeQueue pop()//队尾
//	{
//		EnterCriticalSection(&g_csSafeThread);
//		if (m_queue.empty())
//		{
//			LeaveCriticalSection(&g_csSafeThread);
//			throw("CSafeQueue:empty");
//		}
//		TSafeQueue temp = m_queue.front();
//		m_queue.pop();
//		LeaveCriticalSection(&g_csSafeThread);
//
//		return temp;
//	}
//	/*
//	某一时刻队列是否为空.
//	用于在退出前检查自己线程的任务是否发送完毕了。
//	当然不能保证别的线程再往里面塞。
//	*/
//	bool IsEmpty()
//	{
//		bool ret;
//		EnterCriticalSection(&g_csSafeThread);
//		ret = m_queue.empty();
//		LeaveCriticalSection(&g_csSafeThread);
//		return ret;
//	}
//protected:
//	queue<TSafeQueue> m_queue;
//	CRITICAL_SECTION g_csSafeThread;
//};
//class CSafeQueueAutoPointerManage:public CSafeQueue<char*>
//{
//public:
//	~CSafeQueueAutoPointerManage()
//	{
//		for (;;)
//		{
//			if(IsEmpty())
//			{
//				break;
//			}
//			delete (pop());
//		}
//	}
//};





int _tmain(int argc, _TCHAR* argv[])
{
	CSafeQueueAutoPointerManage* p =new CSafeQueueAutoPointerManage();

	/*CSafeQueue<char*>* p = new CSafeQueue<char*>();


	char* a = new char[256]();
	p->push(a);
	delete(p);*/
	delete(p);

	//bool b = 1;
	//printf("%s",b?"Hi":"Will");








	return 0;
}

