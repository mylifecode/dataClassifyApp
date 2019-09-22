/**³åÎüÆ÷**/
#pragma once
#include "tool.h"
#include	"Painting.h"

class CXMLWrapperTool;
class MisCTool_PluginSuction;

class SuctionAndIrrigationTube : public CTool , public GFPhysSoftBodyConstraint
{
public:
	SuctionAndIrrigationTube();
	SuctionAndIrrigationTube(CXMLWrapperTool * pToolConfig);
	~SuctionAndIrrigationTube();

	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	virtual bool Update(float dt);
	
	virtual void onFrameUpdateStarted(float timeelapsed);

	virtual void onFrameUpdateEnded();

	virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);
	
	virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);

	bool RemoveWaterColumn(void);

	virtual GFPhysVector3 CalculateToolCustomForceFeedBack();

	//@overridden GFPhysSoftBodyConstraint
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	//@overridden GFPhysSoftBodyConstraint
	void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysVector3 GetSuctionCenter(); 

	GFPhysVector3 GetSuctionInvDir(); 

	MisCTool_PluginSuction* GetPluginSuction() const { return m_pPluginSuction; }

private:
	void AddWaterToWaterPool(float dt);

	MisCTool_PluginSuction * m_pPluginSuction;

	bool m_LastLeftElecBtn;
	bool m_LastRightElecBtn;

	bool m_IsWaterColumnExistent;

	int m_WaterColumnID;

	PaintingTool m_painting;

};
