// testEverythin.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>

#include <iostream>
#include <queue>
using std::queue;
template <typename Ttype>
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

	void push(Ttype data)
	{
		EnterCriticalSection(&g_csSafeThread);
		m_queue.push(data);
		LeaveCriticalSection(&g_csSafeThread);
	}
	Ttype pop()//队尾
	{
		EnterCriticalSection(&g_csSafeThread);
		if (m_queue.empty())
		{
			LeaveCriticalSection(&g_csSafeThread);
			throw("CSafeQueue:empty");
		}
		Ttype temp = m_queue.front();
		m_queue.pop();
		LeaveCriticalSection(&g_csSafeThread);

		return temp;
	}

private:
	queue<Ttype> m_queue;
	CRITICAL_SECTION g_csSafeThread;
};



struct st_data_flow
{
	int		length_of_this_struct;
	long	ID_proc;
	int 	functionID;

	//是否异步
	char	async;
	//是否允许回调
	char	permit_callback;
	//参数结构体格式扩展配置，当前为空。
	int		argvTypeOption;	

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
}


















int _tmain(int argc, _TCHAR* argv[])
{
	CSafeQueue<int>* p =new CSafeQueue<int>();

	
	try
	{
		int a = p->pop();
	}
	catch(...)
	{
		;
	}
	p->push(2);
	int b = p->pop();
	return 0;
}

