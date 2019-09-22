#ifndef _CURVEDSCISSORS_
#define _CURVEDSCISSORS_

#include "tool.h"
#include "MisCTool_PluginCut.h"
#include "MisCTool_PluginClamp.h"

class CXMLWrapperTool;

class CCurvedScissors : public CTool
{
public:
	
	CCurvedScissors(CXMLWrapperTool * pToolConfig);
	
	~CCurvedScissors();

	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	virtual bool Update(float dt);
	
	void onFrameUpdateStarted(float timeelapsed);

	void onFrameUpdateEnded();

	virtual float GetCutWidth();

	virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);
	
	virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);

	virtual void  onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair);
private:
	float m_timeSinceLastCut;

	float m_shaftInLastCut;

	bool  m_CanPerformCut;

	std::map<GFPhysSoftBodyFace* , int> m_LCollideFaces;
	std::map<GFPhysSoftBodyFace* , int> m_RCollideFaces;

    MisCTool_PluginCut * m_plugincut;
};
#endif