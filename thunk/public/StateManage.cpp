#include "stdafx.h"
//#include "../Client/stdafx.h"
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

void CSyncStateManage::push(LONG ID_proc,int* ret,char* pFormat,int PointerNumber,HANDLE hdEvent)
{
	st_CYSM tmp={ret,pFormat,PointerNumber,hdEvent};
	m_safemap->push(ID_proc,tmp);
	return;
}

int* CSyncStateManage::findAndPop(LONG ID_proc,char* pFormat,int& PointerNumber,HANDLE& hdEvent)
{
	st_CYSM tmp = m_safemap->pop(ID_proc);

	pFormat = tmp.pStruct;
	PointerNumber = tmp.PointerNumber;
	hdEvent = tmp.hdEvent;

	return tmp.ret;;
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
