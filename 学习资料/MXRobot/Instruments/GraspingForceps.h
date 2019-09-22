#ifndef _GRASPINGFORCEPS_
#define _GRASPINGFORCEPS_

#include <queue>
#include "Forceps.h"
#include "GoPhysContactListener.h"

class CXMLWrapperTool;
class MisCTool_PluginClamp;

class CGraspingForceps : public CForceps
{
public:
     CGraspingForceps();
    
	 CGraspingForceps(CXMLWrapperTool * pToolConfig);
     
	 virtual ~CGraspingForceps(void);

     virtual bool Initialize(CXMLWrapperTraining * pTraining);
	 std::string GetCollisionConfigEntryName();

     /*@Release All Organs Clamped By this Tool*/
     bool HasGraspSomeThing();	//抓钳是否抓住物体

	 MisCTool_PluginClamp * GetClampPlugin();

     virtual bool Update(float dt);

     //virtual void BiteFrom();
     //for new train
     virtual void onFrameUpdateStarted(float timeelapsed);
     virtual void onFrameUpdateEnded();

     virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);
     virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);

     //overridden
     virtual void onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);

private:
	
	 int m_GraspMode;

	 bool m_hasBitted;
};

#endif