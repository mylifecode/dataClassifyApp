#ifndef _MISCTOOL_PLUGINBLUNDISSECT_
#define _MISCTOOL_PLUGINBLUNDISSECT_
#include "MisNewTraining.h"

#include "Instruments/MisMedicCToolPluginInterface.h"

class MisCTool_PluginBluntDissection : public MisMedicCToolPluginInterface
{
public:

	CTool* m_tool;
	float timeSinceLastCut;

	float forceThresholdOfPierce;
	float forceThresholdOfEnlarge;

	
	MisCTool_PluginBluntDissection(CTool * tool, float forceThresholdOfPierce_, float forceThresholdOfEnlarge_);
	~MisCTool_PluginBluntDissection();

	virtual void onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);
	virtual void PhysicsSimulationEnd(int currStep, int TotalStep, float dt);

	void distroyByEnlarge();
	void distroyByPierce();

};
#endif