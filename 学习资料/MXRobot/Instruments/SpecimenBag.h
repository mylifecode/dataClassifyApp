/**±ê±¾´ü**/
#pragma once
#include "tool.h"
#include	"Painting.h"

class CXMLWrapperTool;

class MisMedicRigidPrimtive;

class SpecimenBag : public CTool , public GFPhysSoftBodyConstraint
{
public:
	SpecimenBag();
	SpecimenBag(CXMLWrapperTool * pToolConfig);
	~SpecimenBag();

	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	virtual bool Update(float dt);
	
	virtual void onFrameUpdateStarted(float timeelapsed);

	virtual void onFrameUpdateEnded();

	virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);
	
	virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);

	virtual GFPhysVector3 CalculateToolCustomForceFeedBack();

	//@overridden GFPhysSoftBodyConstraint
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	//@overridden GFPhysSoftBodyConstraint
	void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysVector3 GetSuctionCenter(); 

	GFPhysVector3 GetSuctionInvDir(); 
private:
	PaintingTool m_painting;
	//MisMedicRigidPrimtive * m_pBagCollision;
};
