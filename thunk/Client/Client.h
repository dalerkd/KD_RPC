// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� CLIENT_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// CLIENT_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
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

extern CLIENT_API int nClient;

CLIENT_API int fnClient(void);



extern"C" __declspec(dllexport)int aaaa(PVOID* p,FARPROC callBack);