#pragma once

#include "SafeQueue.h"
#include "DataFormat.h"

/*
����㣺
ʵ�����¹��ܣ�
���첽�������ݡ�����������-������ģ�ͣ�ֻ��ջ����ʱ�������û���
�ȴ�ȫ���������:�ýӿ��ṩһ��֪ͨ�����������ݷ�����ϵ�ʱ�򴥷������ڳ����˳��ȡ�

����Ϣʱ,�����ͬ�������𡣡�����
����Ϣ��ʱ������Ǹ�����Ϣ
����CFunctionInfo����.
�����������Ϣ
push��Stack���������Stack�����������������


*/

	/*
	���ͻ���������Щ��ġ����˴�ͳ��ֱ�ӵ��÷��ͣ�

	��һ���뷨�ǽ����ݷ��ڷ���ջ��,���ڷ�����������ǻ���ģ��Ҽ��ջ��û������
	�������ߣ�������ģ�͡���������
	�ô��ǣ�

	1. ���緢�Ͳ����ĵ��õ�ʱ�䡣�÷��ط��ء�
	2. ʹ�����á��͡����緢�͡����������໥����������֮���Ҳ������й������ɵĲ㡣
	3. ����������ٶȣ�������ʵ�Ǹ���ռ���̣�����ʱ���ڵ������ӣ��ᵼ�´�Ҷ��ȴ���

	ȱ���ǣ�
	1. ����һ�θ�����������Ҫ���ڴ治���͡�����
	2. ������ôЩ�����裬�ƺ�Ч���������͡�


	*/

//
class CWEB
{
public:
	CWEB();
	~CWEB();

public:
	struct st_asysnc_queue_argv 
	{
		char* data;
		int data_len;
	};


	/*�첽����
	1. �����ݸ��Ƶ��Լ�����Ŀռ�
	2. ���浽ջ��
	3. ����
	*/
	void Send(char* data,int data_len);//��ʵ�Ǹ�push����.
	/*��ǰʱ�̣��Ƿ�ȫ���������
	�ɱ������˳�ǰȷ�������Ƿ�����ϡ�
	ע�⣺�޷���֤����̲߳��������ݣ�����ÿ���̶߳�����������

	��ʹ������Ҳ���ܱ�֤�����Ѿ������ͣ��������Ǹձ���ջ��ȡ�����ˡ�
	*/
	bool AllSendIsFinish();

	/*
	�����ͨ���߳������������Ĳ���
	�ͻ���ͨ��ֱ�ӵ�������

	*/
	virtual void Recive_Data(char* flow,int flow_len)=0;

private:
	/*������******�����쳣��˵������ exception::char*
	��;������һ����������
	*/
	st_asysnc_queue_argv findAndPop();

private:
	CSafeQueue<st_asysnc_queue_argv>* m_CSQ;//��������ջ
};

class CClientWeb:public CWEB
{
public:
	void Recive_Data(char* flow,int flow_len)
	{
		/*Flow2Format*/
		if (nullptr==flow||0==flow_len)
		{
			return;
		}
		static CDataFormat::st_data_flow* pFlowBase = (CDataFormat::st_data_flow*)flow;

		char* pArgvCall  = nullptr;
		CSafeQueueAutoPointerManage* queue_memory_manage = nullptr;

		try
		{
			int real_len=CDataFormat::Flow2Format((char*)pFlowBase,pFlowBase->length_of_this_struct,nullptr,0,nullptr,nullptr,nullptr);
			if (real_len==0)
			{
				OutputDebug(L"real_len=0?");
				throw("real_len=0?");
			}

			pArgvCall = new char[real_len]();
			queue_memory_manage = new CSafeQueueAutoPointerManage();
	

			CDataFormat::Flow2Format((char*)pFlowBase,pFlowBase->length_of_this_struct,pArgvCall,real_len,queue_memory_manage,nullptr,nullptr);


			?????

		}
		catch (char* string)
		{
			throw(string);//�ڲ��ṹ����
		}
		catch (int errCode)//�ļ���ʽ�������֮
		{
			OutputDebug(L"Flow Format Err,code:0x%x",errCode);
			return;
		}
		//////////////////////////////////////////////////////////////////////////
	}
};
#include <process.h>
class CServiceWeb:public CWEB
{
	/*
	�����߳�
	���߳�ID����ĳ��XX Stack ��
	˭���ӣ�thread_id ID_proc,function_ID
	�Ƿ񱾺�����������
	˭���ͷţ�
	
	*/
	void Recive_Data(char* flow,int flow_len)
	{


		//Event�����ã����������Ƶ��ڲ�����Ҫ֪ͨ���������������������Ҫ��Ҫ�������̡�
		HANDLE  hdEvent=(NULL,TRUE,FALSE,NULL);

		CDataFormat::st_thread_Service_FlowToFormat_Excute
			tmp={flow,flow_len,hdEvent};

		_beginthreadex(NULL,0,CDataFormat::Service_FlowToFormat_Execute,(void*)&tmp,0,NULL);	

		WaitForSingleObject(hdEvent,INFINITE);
		CloseHandle(hdEvent);
		hdEvent = NULL;
	}
};
