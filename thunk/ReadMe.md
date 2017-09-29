

## 项目介绍：
主页:http://github.com/dalerkd/KD-RPC

本项目开始于2017年9月8日
全部测试通过于2017年9月29日星期五 20:06

## 功能
支持同步，异步调用。
支持指针数据传输，较大限度的提供符合C语言本地调用的网络调用体验。

未来支持：动态修改端口。但说实话，我觉得这样做的意义一般。



## 工程
一共七个子工程：
- Client
客户端代理程序
- KDRPC
设计前期方案文本。
- Service
服务端代理程序
- ServiceDLL
服务器上工作的用户模块
- testEverythin
无用，这是我用来测试各种小技术的。
- UserProc
用户程序
- WEB
无用，设计时，用来测试MailSlot。


UserProc->Client->......网络......->Service->ServiceDLL


我已经添加了8种情况的测试用例。涵盖了绝大多数提供服务的情况。直接编译执行Servic后执行UserProc即可测试.





## 添加框架函数的步骤：
1. 按照规范添加服务器导出函数
2. 添加本地调用函数

3. 添加Client函数注册代码
4. 添加Service函数注册代码





## 用前需知

1. 在客户端中调用以下析构函数前，不要直接使用FreeLibrary(test)。


CFunctionInfo* g_CI_Client;	//客户端 接口信息管理		,不处理会引起内存泄漏
CAsyncStateManage* g_pasm;	//客户端 异步状态管理		,不处理会引起内存泄漏
CSyncStateManage* g_pssm;	//客户端 同步状态管理		,不处理会引起内存泄漏
CWEB* pCWEB;				//客户端 网络管理收发线程	,不处理会引起线程问题和内存问题

2. 建议将网络模块替换成你自己的高速模块。因为我为了方便使用了MailSlot网络这种天底下最慢的网络模块。

