#pragma once
#include "..\public\web.h"
class CWEB_Service :
	public CWEB
{
public:
	void Recive_Data(char* flow,int flow_len);

};

