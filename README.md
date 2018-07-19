# KD_RPC



## 项目介绍：
主页:https://github.com/dalerkd/KD_RPC

代码量4k+

本项目开始于2017年9月8日

于2017年10月7日解决了已知的所有问题。达到可用阶段。

标志性测试通过于2017年9月29日星期五 20:06


## 功能
支持同步，异步调用。
支持指针数据传输，较大限度的提供符合C语言本地调用的网络调用体验。

未来支持：动态修改接口。但说实话，我觉得这样做的优先级不高。



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


我已经添加了9种情况的测试用例。涵盖了绝大多数提供服务的情况。直接编译执行Servic后执行UserProc即可测试.





## 添加框架函数的步骤：
1. 按照规范添加服务器导出函数
2. 添加本地调用函数

3. 添加Client函数注册代码
4. 添加Service函数注册代码





## 用前需知

1. 如果你需要FreeLibrary(test)卸载模块，请先调用以下DLL接口来通知RPC自动合理释放空间和结束工作线程：
```
ExitAllWork()

2. 建议将网络模块替换成你自己的高速模块。因为我为了方便使用了MailSlot网络这种天底下最慢的网络模块。
```


## 用途
- 支持同步和回调

- 支持指针数据修改
将指针作为数据传输给远程，支持远程主机将数据修改结果回传在同步下。
支持形如下面的调用形式：

```
struct Argvs
{
  char* data;
  char data_len;
  float other;
}
int Function(Argvs* pArgvs,FARRPOC Callback);
```

## 优势
- 和本地调用没有差别的使用体验

流畅的接口，不需要额外显式初始化工作。

- 接口动态更新-待增加

可以对接口功能能按照需要进行动态更新。


## 未来
- 更好的接口
  - 无缝替换
最新的计划是设计出能够无缝替换本地服务的框架:wink:这非常吊哈哈。
  - 类支持
  
- 网络
  - 多机协商
这才是RPC更广阔的天地，比如和爬虫结合。
  - 长连接保持

- 对64位和32位相互通讯进行支持
因为本程序设计初期在对32位进行了支持。
我现在在做这个迁移工作。

## 为什么设计该项目？
为了自我提升。
我最早听说RPC是在两年前。
后随系统架构学习的深入，了解到Windows，Linux在本地，远程调用方面的设计。

近年来较深入了解网络开发和前端技术。认识到网络应用的巨大作用。

[知乎:如何通过自学找到一份开发的工作？](https://www.zhihu.com/question/26421707)

发现有推荐设计RPC项目。将机器和机器相互连接起来。同样也是为了方便自己未来使用。因原团队解散，这次正有时间。
