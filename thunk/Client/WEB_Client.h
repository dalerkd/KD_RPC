#pragma once
#include "..\public\web.h"
class CWEB_Client :
	public CWEB
{
public:
	CWEB_Client(void);
	~CWEB_Client(void);

public:
	void Recive_Data(char* flow,int flow_len);
	
};

