#include "stdafx.h"
//#include "../Client/stdafx.h"
#include "WEB.h"
#include "../public/debug.h"
#include "../public/StateManage.h"

#include <process.h>




CWEB::CWEB()
{
	m_CSQ = new CSafeQueue<st_asysnc_queue_argv>();
}

CWEB::~CWEB()
{
	for (;;)
	{
		if (!m_CSQ->IsEmpty())
		{
			st_asysnc_queue_argv tmp = m_CSQ->pop();
			delete(tmp.data);

		}
		else
		{
			break;
		}
	}



	delete(m_CSQ);
}

void CWEB::Send(char* data,int data_len) //其实是个push而已.
{
	if (data_len<=0)
	{
		throw("CWEB::Send:data_len<=0");
	}
	char* p = new char[data_len]();
	int stat=memcpy_s(p,data_len,data,data_len);
	if (stat)
	{
		throw("memcpy_s return err.");
	}

	st_asysnc_queue_argv tmpArgv = {p,data_len};
	m_CSQ->push(tmpArgv);

	return;
}

bool CWEB::AllSendIsFinish()
{
	return m_CSQ->IsEmpty();
}

CWEB::st_asysnc_queue_argv CWEB::findAndPop()
{
	return m_CSQ->pop();
}
