#pragma once
#include "..\public\web.h"
class CWEB_Service :
	public CWEB
{
public:
	CWEB_Service(void);
	~CWEB_Service(void);
public:
	void Recive_Data(char* flow,int flow_len);

};

