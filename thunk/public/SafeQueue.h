#pragma once



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

