#pragma once
#include "..\public\MailSlot.h"
class CWEB_Service :
	public CMailSlot
{
private:
	void Recive_Data(char* flow,LONG64 flow_len);

};

