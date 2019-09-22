#include "PerformanceDebug.h"

CPerformanceDebug::CPerformanceDebug(std::string strFunctionName)
{
	m_dwThreadId = GetCurrentThreadId();
	m_strFunctionName = strFunctionName;

	EnterCriticalSection(&s_CriticalSection);
	if (s_mapRecursive.find(m_dwThreadId) == s_mapRecursive.end())
	{
		s_mapRecursive[m_dwThreadId] = 0;
	}
	*s_pfOut << "[" << m_dwThreadId << "]";
	int nRecursive = s_mapRecursive[m_dwThreadId]++;
	for (int i = 0; i < nRecursive; i++)
	{
		*s_pfOut << "--";
	}
	*s_pfOut << "-->" << strFunctionName << " (Begin)" << std::endl;
	LeaveCriticalSection(&s_CriticalSection);

	QueryPerformanceCounter(&m_nBeginTime);
}

CPerformanceDebug::~CPerformanceDebug(void)
{
	QueryPerformanceCounter(&m_nEndTime);
	double fTime = (double)(m_nEndTime.QuadPart - m_nBeginTime.QuadPart) / (double)s_nFreq * 1000;

	EnterCriticalSection(&s_CriticalSection);
	*s_pfOut << "[" << m_dwThreadId << "]";
	int nRecursive = --s_mapRecursive[m_dwThreadId];
	for (int i = 0; i < nRecursive; i++)
	{
		*s_pfOut << "--";
	}
	*s_pfOut << "-->" << m_strFunctionName << " (End): " << fTime << std::endl;
	LeaveCriticalSection(&s_CriticalSection);
}

void CPerformanceDebug::Init()
{
	LARGE_INTEGER nFreq;
	QueryPerformanceFrequency(&nFreq);
	s_nFreq = nFreq.QuadPart;
	s_pfOut = new std::ofstream;
	s_pfOut->open("log.txt", std::ios_base::out);

	InitializeCriticalSection(&s_CriticalSection);
	*s_pfOut << __FUNCTION__ << std::endl;
}

void CPerformanceDebug::Release()
{
	*s_pfOut << __FUNCTION__ << std::endl;
	DeleteCriticalSection(&s_CriticalSection);

	if (s_pfOut)
	{
		s_pfOut->close();
		delete s_pfOut;
		s_pfOut = NULL;
	}
}

void CPerformanceDebug::OutputString(std::string str)
{
	EnterCriticalSection(&s_CriticalSection);
	*s_pfOut << "[" << GetCurrentThreadId() << "]";
	*s_pfOut << str;
	LeaveCriticalSection(&s_CriticalSection);
}

std::map<DWORD, int> CPerformanceDebug::s_mapRecursive;
CRITICAL_SECTION CPerformanceDebug::s_CriticalSection;
LONGLONG CPerformanceDebug::s_nFreq = (LONGLONG)0;
std::ofstream *CPerformanceDebug::s_pfOut = (std::ofstream *)NULL;