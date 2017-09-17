#pragma once

#include "SafeMap.h"

class CStateManage
{
public:
	CStateManage(void);
	~CStateManage(void);

};

//客户端 同步状态管理
class CSyncStateManage:public CStateManage
{
public:
	CSyncStateManage();
	~CSyncStateManage();
public:
	struct  st_CYSM
	{
		int* ret;
		char* pStruct;
		int PointerNumber;
		HANDLE hdEvent;
	};
	//ID_proc:唯一的号码用来区分本次调用。
	void push(LONG ID_proc,int* ret,char* pStruct,int PointerNumber,HANDLE hdEvent);

	void findAndPop(LONG ID_proc,int* ret,char* pStruct,int& PointerNumber,HANDLE& hdEvent);
private:
	CSafeMap<LONG,st_CYSM>* m_safemap;
};


//客户端 异步状态管理
class CAsyncStateManage
{
public:
	CAsyncStateManage();
	~CAsyncStateManage();

public:
	//ID_proc:唯一的号码用来区分本次调用。
	void push(LONG ID_proc,FARPROC p);
	void findAndPop(LONG ID_proc,FARPROC& p);
private:
	CSafeMap<LONG,FARPROC>* m_safemap;
};
