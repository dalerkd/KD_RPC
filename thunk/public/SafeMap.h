#pragma once


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
