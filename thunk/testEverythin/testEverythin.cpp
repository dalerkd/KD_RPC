// testEverythin.cpp : �������̨Ӧ�ó������ڵ㡣
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
//	int		work_type;//���ι������ͣ���ȡ������Ϣ,�ظ���Ϣ,��������
//	long	ID_proc;
//	int 	functionID;
//
//	//�Ƿ��첽
//	char	async;
//	//�Ƿ�����ص�
//	char	permit_callback;
//	int		return_value;//����ֵ,��ͬ���·���������Ч
//	//�����ṹ���ʽ��չ���ã���ǰΪ�ա�
//	int		argvTypeOption;	
//
//	/*�����ṹ���ܳ���*/
//	int		length_Of_Argv_Struct;
//	/*�����ṹ����ָ��ṹ�������*/
//	int		number_Of_Argv_Pointer;
//
//	char	argv_Struct[0];   
//
//	/*
//	ָ��ṹ�� ��������
//		ָ���ǲ����е�ָ��ṹ�������.
//		eg:
//		struct argv
//		{
//			char* a
//			int len_a;
//			char* b
//			int len_b;
//			char c
//		}	
//		����ָ��ṹ���������2.
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
////�̰߳�ȫ�Ķ���,push��β����,pop���ײ���
////�쳣�׳����ڶ����Ѿ��յ�����»�Ҫpop��
//#include <iostream>
//#include <queue>
//using std::queue;
//template <typename TSafeQueue>
////��ȫ����
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
//	TSafeQueue pop()//��β
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
//	ĳһʱ�̶����Ƿ�Ϊ��.
//	�������˳�ǰ����Լ��̵߳������Ƿ�������ˡ�
//	��Ȼ���ܱ�֤����߳�������������
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

