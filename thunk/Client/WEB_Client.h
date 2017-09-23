#pragma once
#include "..\public\MailSlot.h"
class CWEB_Client :
	public CMailSlot
{
private:
	void Recive_Data(char* flow,int flow_len);
	
};

