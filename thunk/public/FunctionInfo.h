#pragma once



class CFunctionInfo
{
public:

	void AddInfo(int SN,char* funcName,bool asyn,int ArgvStringNumber,int sizeOfStruct/*�����ṹ���С*/);

	bool QueryASync(int query_SN);//�Ƿ��첽,true �첽,false ͬ��
	int  QueryArgvPointerNumber(int query_SN);//�����ṹ����ָ�����������:ָ��A������A��ָ��B������B......����Ϊ2
	int  QueryArgvStructSize(int query_SN);//�����ṹ���ܹ��೤�������������ڱ���ʱ���ṩ��
	char* QueryFuncName(int query_SN);
private:
};

