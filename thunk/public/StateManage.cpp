#include "../Client/stdafx.h"
#include "StateManage.h"


CStateManage::CStateManage(void)
{
}


CStateManage::~CStateManage(void)
{
}

CSyncStateManage::CSyncStateManage()
{
	m_safemap = new CSafeMap<LONG,st_CYSM>();
}

CSyncStateManage::~CSyncStateManage()
{
	delete(m_safemap);
}

void CSyncStateManage::push(LONG ID_proc,int* ret,char* pStruct,int PointerNumber,HANDLE hdEvent)
{
	st_CYSM tmp={ret,pStruct,PointerNumber,hdEvent};
	m_safemap->push(ID_proc,tmp);
	return;
}

void CSyncStateManage::findAndPop(LONG ID_proc,int* ret,char* pStruct,int& PointerNumber,HANDLE& hdEvent)
{
	st_CYSM tmp = m_safemap->pop(ID_proc);

	ret = tmp.ret;
	pStruct = tmp.pStruct;
	PointerNumber = tmp.PointerNumber;
	hdEvent = tmp.hdEvent;

	return;
}

CAsyncStateManage::CAsyncStateManage()
{
	m_safemap = new CSafeMap<LONG,FARPROC>();
}

CAsyncStateManage::~CAsyncStateManage()
{
	delete(m_safemap);
}

void CAsyncStateManage::push(LONG ID_proc,FARPROC p)
{
	m_safemap->push(ID_proc,p);
}

void CAsyncStateManage::findAndPop(LONG ID_proc,FARPROC& p)
{
	p = m_safemap->pop(ID_proc);
}
