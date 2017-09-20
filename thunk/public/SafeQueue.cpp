#include "stdafx.h"

//#include "../Client/stdafx.h"
#include "SafeQueue.h"



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
TSafeQueue CSafeQueue<TSafeQueue>::pop() //∂”Œ≤
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

CSafeQueueAutoPointerManage::~CSafeQueueAutoPointerManage()
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
