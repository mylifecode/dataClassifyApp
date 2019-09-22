/**³¡¾°±ê±¾´ü**/
#ifndef _SCENESPECBAG_
#define _SCENESPECBAG_ 
#include <Ogre.h>
#include "MisMedicOrganInterface.h"
class MisMedicOrgan_Ordinary;
class MisMedicRigidPrimtive;
class MisMedicDynObjConstructInfo;
class MisNewTraining;
class SceneSpeciBag
{
public:
	SceneSpeciBag();
	
	~SceneSpeciBag();

	//after create manually add the return organ to your train!!
	bool Create( const MisMedicDynObjConstructInfo & cs,
		                             const Ogre::Vector3 & pos,
		                             MisNewTraining * hostTrain ,
									 Ogre::SceneManager * sceneMgr,
									 Ogre::String  rendMeshName,
									 Ogre::String  colMeshName,
									 Ogre::String  rendMaterial);

	void ResetPosition(const Ogre::Vector3 & pos);

	void ScaleBag(float ScaleX, float ScaleY, float ScaleZ);

	bool IsClusetInBag(const std::set<GFPhysSoftBodyNode*> & nodeCluster);

	int m_ColCupCollisionCat;

private:
	MisMedicDynObjConstructInfo mCopyCS;

	MisMedicOrgan_Ordinary * m_SoftPart;

	Ogre::SceneNode * m_RigidRendNode;
	Ogre::Entity * m_RigidRendEntity;

	MisMedicRigidPrimtive * m_Collision;

	GFPhysVector3 m_HoleCenter;
	GFPhysVector3 m_HoleNormal;
	float         m_HoleRadius;

	int m_SoftID;
	int m_RigidID;

	MisNewTraining * m_HostTrain;
};
#endif