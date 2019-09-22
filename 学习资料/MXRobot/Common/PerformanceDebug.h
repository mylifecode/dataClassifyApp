#pragma once
#include <fstream>
#include <string>
#include <map>
#include <windows.h>

/************************************************************************/
/* ★★★ 支持多线程。 ★★★                     */
/************************************************************************/

class CPerformanceDebug
{
public:
	CPerformanceDebug(std::string strFunctionName);
	~CPerformanceDebug(void);
	static void Init();
	static void Release();
	static void OutputString(std::string str);

private:
	static std::map<DWORD, int> s_mapRecursive;
	static CRITICAL_SECTION s_CriticalSection;
	static std::ofstream *s_pfOut;
	static LONGLONG s_nFreq;
	LARGE_INTEGER m_nBeginTime;
	LARGE_INTEGER m_nEndTime;
	std::string m_strFunctionName;
	DWORD m_dwThreadId;
};

#define PERFORMANCE_DEBUG CPerformanceDebug __pd__(__FUNCTION__);
#define PERFORMANCE_DEBUG_BEGIN CPerformanceDebug *__ppd__FUNCTION__ = new CPerformanceDebug(__FUNCTION__);
#define PERFORMANCE_DEBUG_END delete __ppd__FUNCTION__;
#define PERFORMANCE_DEBUG_BEGIN_BY_NAME(strFunctionName) CPerformanceDebug *__ppd_##strFunctionName##__ = new CPerformanceDebug(#strFunctionName);
#define PERFORMANCE_DEBUG_END_BY_NAME(strFunctionName) delete __ppd_##strFunctionName##__;