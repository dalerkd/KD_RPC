// ServiceDLL.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "ServiceDLL.h"


// ���ǵ���������һ��ʾ��
SERVICEDLL_API int nServiceDLL=0;

// ���ǵ���������һ��ʾ����
SERVICEDLL_API int fnServiceDLL(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� ServiceDLL.h
CServiceDLL::CServiceDLL()
{
	return;
}
