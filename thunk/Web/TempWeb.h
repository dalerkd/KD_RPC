#pragma once

#include <windows.h>

class CTempWeb
{
public:
	CTempWeb(void);
	~CTempWeb(void);

public:
	static unsigned int __stdcall TrySend(void* pM);
	void Recive(char* data,int data_len);

	static unsigned int __stdcall ThreadReceive(void* pM);

	//void CheckRecive();

};

