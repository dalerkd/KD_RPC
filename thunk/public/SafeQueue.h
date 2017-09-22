#pragma once


#include <windows.h>
//////////////////////////////////////////////////////////////////////////
//�̰߳�ȫ�Ķ���,push��β����,pop���ײ���
//�쳣�׳����ڶ����Ѿ��յ�����»�Ҫpop��
#include <iostream>
#include <queue>
using std::queue;
template <typename TSafeQueue>
//��ȫ����
class CSafeQueue
{
public:
	CSafeQueue();
	~CSafeQueue();

public:
	void push(TSafeQueue data);
	TSafeQueue pop();//��β
	/*
	ĳһʱ�̶����Ƿ�Ϊ��.
	�������˳�ǰ����Լ��̵߳������Ƿ�������ˡ�
	��Ȼ���ܱ�֤����߳�������������
	*/
	bool IsEmpty();
private:
	queue<TSafeQueue> m_queue;
	CRITICAL_SECTION g_csSafeThread;
};
//ֻ�ʺ�new������ָ�룬����ָ�������ָ�������������
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
TSafeQueue CSafeQueue<TSafeQueue>::pop() //��β
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
