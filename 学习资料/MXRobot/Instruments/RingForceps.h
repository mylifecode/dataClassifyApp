
#pragma once
#include <queue>
#include "Forceps.h"
#include "GoPhysContactListener.h"
class CXMLWrapperTool;
class MisCTool_PluginClamp;


class CRingForceps : public CForceps
{
public:
    CRingForceps();

    CRingForceps(CXMLWrapperTool * pToolConfig);

    virtual ~CRingForceps(void);

    virtual bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

  
    virtual bool Update(float dt);

    //for new train
	virtual float GetMaxShaftSpeed();
    virtual void onFrameUpdateStarted(float timeelapsed);
    virtual void onFrameUpdateEnded();



    virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);
    virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);


    virtual void onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);

private:
	int m_GraspMode;

	bool m_hasBitted;
	std::map<GFPhysSoftBodyNode* , GFPhysVector3> m_NodesImpluse;
	
};