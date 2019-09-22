// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料

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

// TODO: 在此处引用程序需要的其他头文件
#define SY_ASSERT(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )
