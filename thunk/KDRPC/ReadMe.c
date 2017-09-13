//#include <windows.h>

/*
服务器提供的内容：
	至少要注明一些东西对吗
	1. async
	2. argv		return value;返回值为空的话，就在客户端返回状态码？还是void。
	3. funcName
	
	看一下需求：
这里有一个问题，同步异步的问题：
	同步异步是谁决定的？是服务器决定的。不由客户端决定。
	所以异步是否允许是无用的。但是后面的回调地址根据具体内容来定。
	
	其实是我们的客户程序是知道是否异步的，因为它要检查回调。如果没有回调，也没有事。


##体系结构
	所有的DLL函数都会回归一点：携带他们的序号信息和回调参数以及多参数指针。
	一个列表等待它们：这里将会得知它们是同步函数还是异步函数。
--------
序号|异步|名
0 true	函数名|
1 false	函数名|
......		  |
--------

同步函数和异步函数是两种东西：

	同步函数使用单独的栈并返回值和修改某些内容。
	
	如何将参数返回？由接收者提供指针。
	int function x()
	{
		x.SendMessage(a,b,c,&X);
		return X;
	}//一个朴素的建议是支持返回值，以适应直观的看法，其实这里的return只是徒增烦恼。
	//因为这样会造成参数的不统一，所有的参数应该都走参数指针。
	//当然这里的烦恼只会限制在 **解析和构造层** ，因为在服务端这种用法也不是错误的。
	//它们是直观的。另一个想废弃这种直观方法的原因是我只能在这里定义一种返回类型
	//比如是int。
		同步栈：
		struct Stack_Node{
			//int* ret;//返回值 == len=4;这是p[0];
			s_p* x;
			counteof(x);
			Signal s;//用于通知那些工作。
		}
		struct s_p{//可能被修改的数据串，即用于发送内容，也用于接收内容。
		//只要告诉我需要把数据给哪个数据即可。
			BYTE* p;
			int	  len;
		}
		stack<Stack_Node> p;




	异步函数使用单独的栈使用指定参数调用指定栈中回调函数。
		异步回调函数：
		void Function(char* p,int len);
	
		谁来提供这个参数的内容?一种方法是通过对象来处理：
		因为对象可以自动释放？够呛。先把数据接过来再说？
		由新回调线程负责申请,复制,释放。


		异步栈：
		struct xxx{
			functionAddress;//真简洁

		}
		stack<xxx> p;
##注意
		跨机器的int是可能不一致的。所以数据类型需要新的妥善方案。

*/


//bool functionName(BYTE* argv,/*BOOL async =true ,*/char* callback = nullptr); 
/*
struct FunctionName_Argv{




};
struct argc{
	char a;
	int    b;
	_In_   BYTE* x;
	int    len_x;
	_Out_   BYTE* y;
	int  len_y;
};
*/


/*
1. 制作一个服务端函数DLL
怎么获取服务端的函数列表呢？

应当首先建立一个列表：
--------
序号|异步|名
0 true	函数名 |
1 false	函数名 |
......		   |
--------
“序号”是用来在网络上传递的调用号。
获取函数名的目的是能够动态调用。
此外还需要一个参数struct,这些只能由.h文件提供：

参数信息应该由谁解读？
如果客户端提供了错误的参数信息怎么办？没有关系。这只是一个参数指针。服务端怎么处理由
自己决定。所以服务端提供一个.h文件给客户端是OK的。但不是必须的。

***********！！！
服务端自己只要保有 : 序号，函数名， 异步情况，参数信息(用做解析复原调用信息，打包反馈信息)
客户端保有：序号，函数名(初始化时用)， 异步情况(区别处理)，参数信息（用做打包原调用信息，复原反馈）
***********！！！
这样看两者是相同的，但是对于struct参数，因为涉及拷贝指针参数，就需要客户端知悉：
	参数结构体。
否则就不能区分复制参数了。
eg:
	HANDLE open(s_x* p,callback*);

	struct s_x{
		BYTE* a;
		int len_a;
		int x;
		BYTE* b;
		int len_b;
	}
-------------------------------------------
这个结构体编译DLL时候写的。显然它需要规范以方便我们对它的识别。

BYTE* a 和 int len_a 两者是对应的关系，后者标志了前者的长度。
在传输前这个结构会被流化，在传输到服务端后，这个结构会被复原。当然指针会变得。

！！！！所以我们的程序应当具备读.h的能力。
但这个解析能力需要预估时间。


2. 制作一个本地调用函数DLL
3. 制作它们的头文件

*/

/*
任务：
	客户端：
		4网络层
		2+?数据格式打包，解包层：头文件解析层：这个可能有一定的工作量哟
		2异步回调启动
		2同步启动
		3根据传回信息修改DLL

	服务端：
		4网络层
		2数据格式打包，解包层
		1回调参数捕获
		1同步参数改动检测
		1同步返回值捕获

*/
/*
具体设计：
	实施顺序是：
		1. 不带网络层的客户端直接使用服务端的DLL测试 - 定结构体格式	√ 
		2. 假定已经有存在.h文件并能解析且导出函数已经被初始化完毕：
			用代码尝试打通 “异步和同步区分与分别注册”过程封装类。√ 
			**写测试函数。如果不想一直重看这里的话。
		3. “数据格式打包”“数据格式解包”过程封装类。要客户端和服务端都能用的。
			**写测试函数。
共11项	4. 格式解析
			**写测试函数。
		4. 回调参数捕获-服务端
		4. 参数修改侦测-服务端
		4. 参数回写-客户端
		4. 服务端初始化程序
		4. 客户端初始化程序-导出函数修改
		5. 完成除网络外的所有内容 并能够在单机运行。
		6. 网络层
*/



