#pragma once

#include <windows.h>
//////////////////////////////////////////////////////////////////////////
//����map<ID_proc,struct>
//�쳣�׳����������������ʱӦ�����ԡ�
#include <iostream>
#include <map>
using std::map;
template <typename TSafeMapIndex,typename TSafeMapData>
//��ȫmap
class CSafeMap
{
public:
	CSafeMap();
	~CSafeMap();

public:
	void push(TSafeMapIndex id,TSafeMapData data);
	TSafeMapData pop(TSafeMapIndex id);
	TSafeMapIndex Explorer();

private:
	map<TSafeMapIndex,TSafeMapData> m_map;
	CRITICAL_SECTION g_csSafeThread;
};





template <typename TSafeMapIndex,typename TSafeMapData>
CSafeMap<TSafeMapIndex,TSafeMapData>::CSafeMap(void)
{
	InitializeCriticalSection(&g_csSafeThread);
}


template <typename TSafeMapIndex,typename TSafeMapData>
CSafeMap<TSafeMapIndex,TSafeMapData>::~CSafeMap(void)
{
	DeleteCriticalSection(&g_csSafeThread);
}

template <typename TSafeMapIndex,typename TSafeMapData>
void CSafeMap<TSafeMapIndex,TSafeMapData>
	::push(TSafeMapIndex id,TSafeMapData data)
{
	EnterCriticalSection(&g_csSafeThread);
	m_map[id]=data;
	LeaveCriticalSection(&g_csSafeThread);
}

template <typename TSafeMapIndex,typename TSafeMapData>
TSafeMapData CSafeMap<TSafeMapIndex,TSafeMapData> 
	::pop(TSafeMapIndex id)
{
	EnterCriticalSection(&g_csSafeThread);
	map<TSafeMapIndex,TSafeMapData>::iterator pos = m_map.find(id);
	if (pos != m_map.end())
	{
		TSafeMapData ret= m_map[id];
		m_map.erase(pos);
		LeaveCriticalSection(&g_csSafeThread);
		return ret;
	}
	else
	{
		LeaveCriticalSection(&g_csSafeThread);
		throw("CSafeMap: key is non-existent!");
	}


}
//��������һ������
template <typename TSafeMapIndex,typename TSafeMapData>
TSafeMapIndex CSafeMap<TSafeMapIndex,TSafeMapData> 
	::Explorer()
{
	EnterCriticalSection(&g_csSafeThread);
	

	map<TSafeMapIndex,TSafeMapData>::iterator it;

	it = m_map.begin();

	TSafeMapIndex ret_value=0;
	if(it != m_map.end())
	{
		ret_value = it->first;
	}
	else
	{
		ret_value = 0;
	}

	LeaveCriticalSection(&g_csSafeThread);
	return ret_value;
}