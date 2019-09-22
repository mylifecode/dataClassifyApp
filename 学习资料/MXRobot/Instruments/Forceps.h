/**Author:zx**/
/**ЧЏзг**/
#pragma once
#include "tool.h"
#include "MXEvent.h"

class CXMLWrapperTool;
class CForceps : public CTool
{
public:
	CForceps();
	CForceps(CXMLWrapperTool * pToolConfig);
	virtual ~CForceps(void);		
	
protected:
	bool            m_bClipClamp;
	bool            m_bCheckClipClamp;
	double          m_fClipAccumTime;
	double          m_fClipLastTickCount;
	bool            m_bUseClampThread;
	bool            m_bCanCreateClip;
	bool            m_bHasCandiPoint;   
};