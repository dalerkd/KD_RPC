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
	
	/*
	返回值	ret		用于接收返回值的指针
	ID_proc	唯一的号码用来区分本次调用。
	pFormat Format的指针
	PointerNumber	Format中指针的数量
	hdEvent			用于触发收到回复的事件。

	*/
	void push(LONG ID_proc,int* ret,char* pFormat,int PointerNumber,HANDLE hdEvent);

	int* findAndPop(LONG ID_proc,char*& pFormat,int& PointerNumber,HANDLE& hdEvent);
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
