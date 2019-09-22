#ifndef  _HYSTERECTOMYTRAINING_
#define  _HYSTERECTOMYTRAINING_

#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicObjectUnion.h"

class MisMedicOrgan_Ordinary;
class CElectricHook;
class DrawObject;


class WombManipulator : public GFPhysSoftBodyConstraint
{
public:
	class  NodeBeManipulated
	{
	public:
		GFPhysVector3 m_PointLocalCoord;
		GFPhysSoftBodyNode * m_Node;
		float m_Lambda;
	};

	struct ManipulatorCupPoint
	{
		GFPhysSoftBodyNode * m_NodeInM;
		GFPhysSoftBodyNode * m_NodeInWomb;
	};

	WombManipulator(MisMedicOrgan_Ordinary * womb, const GFPhysVector3 & rootPos, const GFPhysVector3 & headPos);

	~WombManipulator();

	void Rotate(const GFPhysQuaternion & rot);

	GFPhysVector3 m_RootInitPos;
	GFPhysVector3 m_HeadInitPos;
	GFPhysVector3 m_CoordVec[3];
	GFPhysVector3 m_InitCoordVec[3];

	float m_Radius;

	MisMedicOrgan_Ordinary * m_Womb;

	GFPhysAlignedVectorObj<NodeBeManipulated> m_TetrasManipulated;

	virtual void PrepareSolveConstraint(Real Stiffness, Real TimeStep);

	virtual void SolveConstraint(Real Stiffness, Real TimeStep);

	void CalculateTetrasBeManipulated();

	std::vector<ManipulatorCupPoint> m_ManipulatorCupPoints;
};
class HysterectomyTraining :public MisNewTraining
{
public:
	
	
	HysterectomyTraining(const Ogre::String & strName);

	virtual ~HysterectomyTraining(void);

	MisMedicOrganInterface * LoadOrganism( MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain);

	bool Update(float dt);


	bool ReadTrainParam(const std::string& strFileName);
	virtual void KeyPress(QKeyEvent * event);
protected:
	virtual void NodesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes, MisMedicOrgan_Ordinary * orgn);//GFPhysSoftBodyShape * hostshape);

	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	virtual void CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig);

	void OnTrainingIlluminated();

	void DisplaceWombManipulate();

	void BuildManipulatorTouchPoints(MisMedicOrgan_Ordinary * manipulator , MisMedicOrgan_Ordinary * womb);
private:
	bool BeginRendOneFrame(float timeelapsed);

	WombManipulator * m_wombManitor;

	MisMedicOrgan_Ordinary * m_Womb;
	MisMedicOrgan_Ordinary * m_ManiCup;

};

#endif