// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 CLIENT_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// CLIENT_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#pragma once

#ifdef CLIENT_EXPORTS
#define CLIENT_API __declspec(dllexport)
#else
#define CLIENT_API __declspec(dllimport)
#endif

// 此类是从 Client.dll 导出的
class CLIENT_API CClient {
public:
	CClient(void);
	// TODO: 在此添加您的方法。
};

extern CLIENT_API LONG64 nClient;

CLIENT_API LONG64 fnClient(void);



extern"C" __declspec(dllexport)LONG64 aaaa(PVOID* p,FARPROC callBack);



//////////////////////////////////////////////////////////////////////////
/*
负责产生唯一ID。该ID被用在标记任务的唯一性上：
一个被标号的数据流从服务器回来还能知道它来自于哪里。
*/
class CID_Manager
{
public:
	static LONG GetNewID()
	{
		return InterlockedIncrement(&CID_Manager::m_ID);
	}
private:
	static LONG m_ID;
};
LONG CID_Manager::m_ID =0;