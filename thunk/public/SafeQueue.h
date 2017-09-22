#pragma once


#include <windows.h>
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
	CSafeQueue();
	~CSafeQueue();

public:
	void push(TSafeQueue data);
	TSafeQueue pop();//队尾
	/*
	某一时刻队列是否为空.
	用于在退出前检查自己线程的任务是否发送完毕了。
	当然不能保证别的线程再往里面塞。
	*/
	bool IsEmpty();
private:
	queue<TSafeQueue> m_queue;
	CRITICAL_SECTION g_csSafeThread;
};
//只适合new出来的指针，不能指针数组和指针对象做参数。
class CSafeQueueAutoPointerManage:public CSafeQueue<char*>
{
public:
	~CSafeQueueAutoPointerManage();
};



template <typename TSafeQueue>
CSafeQueue<TSafeQueue>::CSafeQueue()
{
	InitializeCriticalSection(&g_csSafeThread);
}

template <typename TSafeQueue>
CSafeQueue<TSafeQueue>::~CSafeQueue()
{
	DeleteCriticalSection(&g_csSafeThread);
}

template <typename TSafeQueue>
void CSafeQueue<TSafeQueue>::push(TSafeQueue data)
{
	EnterCriticalSection(&g_csSafeThread);
	m_queue.push(data);
	LeaveCriticalSection(&g_csSafeThread);
}

template <typename TSafeQueue>
TSafeQueue CSafeQueue<TSafeQueue>::pop() //队尾
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

template <typename TSafeQueue>
bool CSafeQueue<TSafeQueue>::IsEmpty()
{
	bool ret;
	EnterCriticalSection(&g_csSafeThread);
	ret = m_queue.empty();
	LeaveCriticalSection(&g_csSafeThread);
	return ret;
}

CSafeQueueAutoPointerManage::~CSafeQueueAutoPointerManage(void)
{
	for (;;)
	{
		if(IsEmpty())
		{
			break;
		}
		delete (pop());
	}
}
