// testEverythin.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>

#include <iostream>
#include <queue>
using std::queue;
template <typename Ttype>
//��ȫ����
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
	Ttype pop()//��β
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

	//�Ƿ��첽
	char	async;
	//�Ƿ�����ص�
	char	permit_callback;
	//�����ṹ���ʽ��չ���ã���ǰΪ�ա�
	int		argvTypeOption;	

	/*�����ṹ���ܳ���*/
	int		length_Of_Argv_Struct;
	/*�����ṹ����ָ��ṹ�������*/
	int		number_Of_Argv_Pointer;

	char	argv_Struct[0];   

	/*
	ָ��ṹ�� ��������
		ָ���ǲ����е�ָ��ṹ�������.
		eg:
		struct argv
		{
			char* a
			int len_a;
			char* b
			int len_b;
			char c
		}	
		����ָ��ṹ���������2.

	


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

