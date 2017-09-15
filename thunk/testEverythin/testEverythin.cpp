// testEverythin.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>





struct st_data_flow
{
	int		length_of_this_struct;
	int		work_type;//本次工作类型？获取函数信息,回复信息,发送请求
	long	ID_proc;
	int 	functionID;

	//是否异步
	char	async;
	//是否允许回调
	char	permit_callback;
	int		return_value;//返回值,在同步下服务器端有效
	//参数结构体格式扩展配置，当前为空。
	int		argvTypeOption;	

	/*参数结构体总长度*/
	int		length_Of_Argv_Struct;
	/*参数结构体中指针结构体的数量*/
	int		number_Of_Argv_Pointer;

	char	argv_Struct[0];   

	/*
	指针结构体 的数量：
		指的是参数中的指针结构体的数量.
		eg:
		struct argv
		{
			char* a
			int len_a;
			char* b
			int len_b;
			char c
		}	
		其中指针结构体的数量是2.

	


	*/

};
struct st_argv_Node_Struct
{
	int length;
	char data[0];
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

