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


class 
{
public:
protected:
private:
};



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

