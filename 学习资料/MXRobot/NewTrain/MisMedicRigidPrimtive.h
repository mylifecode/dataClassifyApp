#pragma once

#include "MisMedicOrganInterface.h"
#include "collision/CollisionShapes/GoPhysTriangleMesh.h"
#include "MisMedicObjectSerializer.h"

class CBasicTraining;
class MisMedicRigidPrimtive : public MisMedicOrganInterface 
{ 
public:
	enum RigidType
	{
		RT_UNKNOW,
		RT_BALL,
		RT_BOX,
		RT_SPHERE,
		RT_CYLINDER,
		RT_TRIMESH,
	};
	
	MisMedicRigidPrimtive(int index , CBasicTraining * ownertrain);
	
	~MisMedicRigidPrimtive(void);

	// function 
public:
	virtual void Create(MisMedicDynObjConstructInfo & constructInfo)
	{
		assert(0 "not implemented!!");
	}
	virtual void Create(MisMedicDynObjConstructInfo & constructInfo , 
		                Ogre::SceneManager * sceneMgr ,
						Ogre::String & entityName);

	void CreateRigidSphere(Ogre::String meshName, 
		                   Ogre::SceneManager * scenemgr, 
						   Ogre::Vector3 center, 
						   float radius, 
						   float mass);

	void CreateRigidBox(Ogre::String entityName, Ogre::SceneManager * scenemgr, Ogre::Vector3 center, Ogre::Vector3 size, float mass);
	
	void CreateRigidCylinderY(Ogre::String entityName, Ogre::SceneManager * scenemgr, Ogre::Vector3 center, Ogre::Vector3 size, float mass);

	void CreateRigidMeshFromSerialize(MisMedicObjetSerializer& serializer, 
		                              Ogre::SceneManager * scenemgr,
									  GFPhysVector3 & position);

	void CreateRigidMeshFromMMSFile(Ogre::SceneManager * scenemgr,
		                            const std::string & mmsfilename,
									GFPhysVector3 & position);

	void SetPosition(const GFPhysVector3 & pos);

	void SetTransform(const GFPhysTransform & trans);

	GFPhysBvhTriMeshShape * GetCollisionTriMesh();

	// data record
	void SetEntityMaterial(Ogre::String materialName);
	void SetMass(float mass);
	void SetEntityPos(Ogre::Vector3 pos);
	void SetEntityRot(Ogre::Quaternion rot);
	void SetEntitySize(Ogre::Vector3 size);
   
protected:

	RigidType m_RigidType;

	//overridden
	void RemovePhysicsPart();
	//overridden
	void RemoveGraphicPart();
public:
	GFPhysRigidBody* m_body;
	Ogre::SceneNode* m_node;
	Ogre::Entity*	m_entity;
    Ogre::ManualObject * m_RendObject;
    MisMedicObjetSerializer* m_serializer;

	GFPhysVector3 * m_MeshVertex;
	GFPhysVector3 * m_MeshLocalVertex;
	GFPhysMeshDataTriangle * m_MeshTriangles;
	GFPhysTriangleMesh * m_MeshData;

	// inherit base class MisMedicOrganInterface
public:
	virtual Ogre::Vector2 GetTextureCoord(GFPhysSoftBody * sb ,GFPhysSoftBodyFace * face , float weights[3]);

	virtual void  Update(float dt , Ogre::SceneManager * scenemgr , Ogre::Camera * camera);	// for old training
	virtual void  UpdateScene(float dt , Ogre::Camera * camera);	// new training

	virtual bool  CanBeGrasp();

	virtual float GetForceFeedBackRation();

	virtual void  SetForceFeedBackRation(float ration);

	virtual void ToolPunctureSurface(ITool * tool , GFPhysSoftBodyFace * face , const float weights[3]);

	virtual void NotifyRigidBodyRemovedFromWorld(GFPhysRigidBody * rb);
};
