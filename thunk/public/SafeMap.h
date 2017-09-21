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

