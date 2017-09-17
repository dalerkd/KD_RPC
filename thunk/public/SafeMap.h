#pragma once


//////////////////////////////////////////////////////////////////////////
//处理map<ID_proc,struct>
//异常抛出：在意外情况，此时应当调试。
#include <iostream>
#include <map>
using std::map;
template <typename TSafeMapIndex,typename TSafeMapData>
//安全map
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
