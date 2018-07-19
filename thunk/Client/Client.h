// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� CLIENT_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// CLIENT_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#pragma once

#ifdef CLIENT_EXPORTS
#define CLIENT_API __declspec(dllexport)
#else
#define CLIENT_API __declspec(dllimport)
#endif

// �����Ǵ� Client.dll ������
class CLIENT_API CClient {
public:
	CClient(void);
	// TODO: �ڴ�������ķ�����
};

extern CLIENT_API LONG64 nClient;

CLIENT_API LONG64 fnClient(void);



extern"C" __declspec(dllexport)LONG64 aaaa(PVOID* p,FARPROC callBack);



//////////////////////////////////////////////////////////////////////////
/*
�������ΨһID����ID�����ڱ�������Ψһ���ϣ�
һ������ŵ��������ӷ�������������֪�������������
*/
class CID_Manager
{
public:
	static LONG GetNewID()
	{
		return InterlockedIncrement(&CID_Manager::m_ID);
	}
private:
	static LONG m_ID;
};
LONG CID_Manager::m_ID =0;