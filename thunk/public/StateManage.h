#pragma once

#include "SafeMap.h"

class CStateManage
{
public:
	CStateManage(void);
	~CStateManage(void);

};

//�ͻ��� ͬ��״̬����
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
	����ֵ	ret		���ڽ��շ���ֵ��ָ��
	ID_proc	Ψһ�ĺ����������ֱ��ε��á�
	pFormat Format��ָ��
	PointerNumber	Format��ָ�������
	hdEvent			���ڴ����յ��ظ����¼���

	*/
	void push(LONG ID_proc,int* ret,char* pFormat,int PointerNumber,HANDLE hdEvent);

	int* findAndPop(LONG ID_proc,char*& pFormat,int& PointerNumber,HANDLE& hdEvent);
private:
	CSafeMap<LONG,st_CYSM>* m_safemap;
};


//�ͻ��� �첽״̬����
class CAsyncStateManage
{
public:
	CAsyncStateManage();
	~CAsyncStateManage();

public:
	//ID_proc:Ψһ�ĺ����������ֱ��ε��á�
	void push(LONG ID_proc,FARPROC p);
	void findAndPop(LONG ID_proc,FARPROC& p);
private:
	CSafeMap<LONG,FARPROC>* m_safemap;
};
