
#include "MisNewTraining.h"

#include "Instruments/MisMedicCToolPluginInterface.h"

class MisCTool_PluginPricker_FixConstraint : public GFPhysPositionConstraint {
public:
	float coord[3];
	float coordOrig[3];
	GFPhysSoftBodyNode* pNode1;
	GFPhysSoftBodyNode* pNode2;
	GFPhysSoftBodyNode* pNode3;
	float weights[3];
	float f[3];
	GFPhysTransform* transformOrig;
	MisCTool_PluginPricker_FixConstraint(MisMedicOrgan_Ordinary* organ, int faceIdx, float* weights_, GFPhysTransform* transform_, const float* coord0);
	void PrepareSolveConstraint(Real Stiffness, Real TimeStep);
	void SolveConstraint(Real Stiffniss, Real TimeStep);
	void SetCoord(const float* coord_);
	void SetCoord(GFPhysTransform* newTransform);
	~MisCTool_PluginPricker_FixConstraint();

};

class MisCTool_PluginPricker : public MisMedicCToolPluginInterface
{
public:

	CTool* m_tool;
	char m_part[10];	// left, right
	char m_side[10];	// left, right
	float m_resistOfPrick;
	MisCTool_PluginPricker_FixConstraint* m_constraint;
	float forceDir[3];

	std::vector<int> disabledFaces;
	std::vector<MisMedicOrgan_Ordinary*> disabledOrgans;

	MisCTool_PluginPricker(CTool * tool, float resistOfPrick);
	~MisCTool_PluginPricker();

	virtual void PhysicsSimulationEnd(int currStep, int TotalStep, float dt);
	

	MisMedicOrgan_Ordinary* pHitOrgan;
	int hitFaceID;
	float hitPoint[3];

	virtual void OneFrameUpdateEnded();

	

private:
	void setFaceEx(MisMedicOrgan_Ordinary* pOrgan, int faceUID, int collisionEnabled);
	int getFaceEx(MisMedicOrgan_Ordinary* organ, int faceUID, float* pCoord);
	void getStatusEx(char* pStatus, MisMedicOrgan_Ordinary** ppOrgan, int* pFaceID, float* pFaceWeight, float* force, GFPhysTransform* transforms);
	void tryReset();
};
