#pragma once
#include "..\public\dataformat.h"
class CData_Format_Client :
	public CDataFormat
{
public:
	CData_Format_Client(void);
	~CData_Format_Client(void);
public:
	static unsigned int WINAPI  Client_FlowToFormat_Execute(LPVOID lp);
};

