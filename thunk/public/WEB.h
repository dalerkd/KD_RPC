#pragma once

#include "SafeQueue.h"
#include "DataFormat.h"

/*
网络层：
实现以下功能：
半异步发送数据。运用生产者-消费者模型，只在栈满的时候阻塞用户。
等待全部发送完毕:该接口提供一个通知，当所有数据发送完毕的时候触发，用于程序退出等。

有消息时,如果是同步就消灭。。。？
有消息来时，如果是更新消息
交给CFunctionInfo处理.
如果是其他消息
push给Stack管理程序：由Stack管理程序来具体解包。


*/

	/*
	发送机制我想玩些别的。除了传统的直接调用发送，

	另一种想法是将数据放在发送栈中,而在放入这个操作是互斥的，且检查栈有没有满。
	“生产者，消费者模型”，这样的
	好处是：

	1. 网络发送不消耗调用的时间。该返回返回。
	2. 使“调用”和“网络发送”两个过程相互独立，错误之类的也不会外泄到不相干的层。
	3. 提高了吞吐速度，发送其实是个抢占过程，而短时间内调用增加，会导致大家都等待。

	缺点是：
	1. 多了一次复制手续，这要是内存不够就。。。
	2. 多了那么些管理步骤，似乎效率有所降低。


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


	/*异步发送
	1. 将数据复制到自己申请的空间
	2. 保存到栈里
	3. 返回
	*/
	void Send(char* data,int data_len);//其实是个push而已.
	/*当前时刻，是否全部发送完毕
	可被用做退出前确认数据是否发送完毕。
	注意：无法保证别的线程不来发数据，除非每个线程都做检查哈哈。

	即使这样，也不能保证数据已经被发送，它可能是刚被从栈中取出来了。
	*/
	bool AllSendIsFinish();

	/*
	服务端通过线程来做接下来的操作
	客户端通过直接调用来做

	*/
	virtual void Recive_Data(char* flow,int flow_len)=0;

private:
	/*！！！******产生异常：说明空了 exception::char*
	用途：弹出一条发送任务
	*/
	st_asysnc_queue_argv findAndPop();

private:
	CSafeQueue<st_asysnc_queue_argv>* m_CSQ;//发送数据栈
};

