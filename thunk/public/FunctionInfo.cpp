#include "../Client/stdafx.h"
#include "FunctionInfo.h"



void CFunctionInfo::AddInfo(int SN,char* funcName,bool asyn,int ArgvStringNumber,int sizeOfStruct/*参数结构体大小*/)
{

}

bool CFunctionInfo::QueryASync(int query_SN) //是否异步,true 异步,false 同步
{

}

int CFunctionInfo::QueryArgvPointerNumber(int query_SN) //参数结构体中指针个数，例如:指针A，长度A，指针B，长度B......个数为2
{

}

int CFunctionInfo::QueryArgvStructSize(int query_SN) //参数结构体总共多长，服务器必须在编译时刻提供。
{

}

char* CFunctionInfo::QueryFuncName(int query_SN)
{

}
