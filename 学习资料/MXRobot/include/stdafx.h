// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <ctype.h>
#include <tchar.h>
#include "CommonDataStruct.h"
using namespace::cds;
using namespace::std;


#ifdef  __cplusplus
extern "C" {
#endif

	_CRTIMP void __cdecl _wassert(_In_z_ const wchar_t * _Message, _In_z_ const wchar_t *_File, _In_ unsigned _Line);

#ifdef  __cplusplus
}
#endif

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
#define SY_ASSERT(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )
