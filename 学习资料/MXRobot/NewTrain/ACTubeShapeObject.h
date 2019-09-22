#ifndef _ACTUBESHAPEOBJECT_
#define _ACTUBESHAPEOBJECT_

#include "MisMedicOrganInterface.h"
#include "MisMedicTubeShapeRendObject.h"

class SkinedVertex
{
public:
	Ogre::Vector3 m_RestPos;
	Ogre::Vector3 m_RestNormal;

	Ogre::Vector3 m_SkinedPos;
	Ogre::Vector3 m_SKinedNorm;

	Ogre::Vector3 m_BoneCoord[4];
	Ogre::Vector3 m_NormCoord[4];

	int m_BoneSegIndex[4];
	int m_NumBoneSeg;
};
class ACTubeShapeObject : public MisMedicOrganInterface, public GFPhysPositionConstraint
{
public:
	ACTubeShapeObject(int index, CBasicTraining * ownertrain);
	~ACTubeShapeObject();

	void Create(MisMedicDynObjConstructInfo &);
	
public:
	void CreateToturs(Ogre::SceneManager * sceneMgr,
		              const GFPhysVector3 & circlecenter,
		              const GFPhysVector3 & circleNormal, 
					  float radius);

	
	void InternalSimulateStart(int currStep, int TotalStep, Real dt);

	void InternalSimulateEnd(int currStep, int TotalStep, Real dt);

	void PrepareSolveConstraint(Real Stiffness, Real TimeStep);

	void SolveConstraint(Real Stiffness, Real TimeStep);
	
	
	void UpdateScene(float dt, Ogre::Camera * camera);

	GFPhysSoftTube * GetPhysicsBody()
	{
		return m_PhysicsBody;
	}
	
protected:
	
	void CreateTotursMesh(float radius, int circlesegnum);

	void AnimatedSkinMesh();

	void UpdateRendBuffer(int circleSegnum);

	GFPhysSoftTube * m_PhysicsBody;

	MisMedicTubeShapeRendObject m_RendObject;

	float m_RendRadius;

	GFPhysVectorObj<SkinedVertex> m_SkinedVertex;

	Ogre::ManualObject * m_SkinRendObject;

	Ogre::SceneNode * m_Node;


};
#endif