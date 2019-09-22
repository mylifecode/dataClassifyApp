/**Author:zx**/
/**Ότµ¶**/
#pragma once
#include "tool.h"
#include "MisCTool_PluginCut.h"
#include "MisCTool_PluginClamp.h"
#include "MisCTool_PluginPricker.h"
#include "MisCTool_PluginBluntDissection.h"

class CXMLWrapperTool;



class CStraightScissors : public CTool
{
public:
	CStraightScissors();
	CStraightScissors(CXMLWrapperTool * pToolConfig);
	~CStraightScissors();

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
    
	MisCTool_PluginPricker * m_pluginpricker;
	MisCTool_PluginBluntDissection* m_pluginbluntdissection;
};
