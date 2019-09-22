#ifndef _MISMEDICCTOOLPLUGININTERFACE_
#define _MISMEDICCTOOLPLUGININTERFACE_
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Topology/GoPhysSoftBodyCutter.h"
using namespace GoPhys;

class CTool;
class VeinConnectObject;
class MisMedicOrganInterface;
class MisMedicOrgan_Ordinary;
class MisMedicThreadRope;
class MisCustomSimObj;
class MisMedicCToolPluginInterface
{
public:
	MisMedicCToolPluginInterface(CTool * tool) : m_ToolObject(tool)
	{}

	virtual ~MisMedicCToolPluginInterface()
	{}

	virtual void OnOrganBeRemovedFromWorld(MisMedicOrganInterface * organif){}

	virtual void OnCustomSimObjBeRemovedFromWorld(MisCustomSimObj * rope){}

	virtual void OnRigidBodyBeRemovedFromWorld(GFPhysRigidBody * rigidbody){}

	//when start check collision with soft body
	virtual void BeginCheckSoftBodyCollision(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj) {}


	//when end check collision with soft body
	virtual void EndCheckSoftBodyCollision(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj) {}

	virtual void onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair){}

	virtual void onRSContactsBuildFinish(GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints){}//deprecated

	//when rigid and soft collision contact solved
	//note! this return a vector in all the world you should filter those contact not belong this tool manually
	virtual void RigidSoftCollisionsSolved(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints) {}//deprecated

	//new method
	virtual void  onRSFaceContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints) {}
	virtual void  onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints) {}
	//

	//when a physics update frame start
	virtual void PhysicsSimulationStart(int currStep , int TotalStep , float dt) {}
	
	//when a physics update frame end
	virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt) {}

	//when a ordinary frame update start(one ordinary frame may include zero-many physics frame)
	virtual void OneFrameUpdateStarted(float timeelapsed) {}
	
	//when a ordinary frame update end
	virtual void OneFrameUpdateEnded() {}

	virtual void CollideVeinConnectPair(VeinConnectObject * veinobject ,
										GFPhysCollideObject * convexobj,
										int cluster , 
										int pair,
										const GFPhysVector3 & collidepoint)
	{}

	virtual void OnSoftBodyNodesBeDeleted(GFPhysSoftBody *sb , const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes)
	{}

	virtual void OnSoftBodyFaceBeDeleted(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face)
	{}

	virtual void OnSoftBodyFaceBeAdded(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face)
	{}

	virtual GFPhysVector3 GetPluginForceFeedBack()
	{ 
		return GFPhysVector3(0, 0, 0); 
	}
	CTool * m_ToolObject;
};

#endif