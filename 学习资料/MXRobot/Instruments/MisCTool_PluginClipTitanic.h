#ifndef _MISCTOOL_PLUGINCLIPTITANIC_
#define _MISCTOOL_PLUGINCLIPTITANIC_
#include "MisMedicCToolPluginInterface.h"

//检查正好剪断在钛夹位置时候的bug
class MisCTool_PluginClipTitanic : public MisMedicCToolPluginInterface
{
public:
	class ClipOperationData
	{
	public:
		ClipOperationData()
		{
			m_canCollectClampPoint = false;
			m_InClipState = false;
			m_sbcanclamp = 0;
			m_minClampShaftAngle = 5.0f;
			m_MaxPersistContactShaft = 0;
		}
		float     m_minClampShaftAngle;
		float     m_MaxPersistContactShaft;
		bool      m_canCollectClampPoint;
		bool      m_InClipState;

		//soft can be grasp
		GFPhysCollideObject * m_sbcanclamp;
		//std::map<GFPhysSoftBodyNode* , ClampedNode> m_NodesInClamp;
	};

	MisCTool_PluginClipTitanic(CTool * tool);

	~MisCTool_PluginClipTitanic();

	//when start check collision with soft body
// 	virtual void BeginCheckSoftBodyCollision(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj);

	//when detect a soft body's face collide with the tools convex
// 	virtual void SoftBodyFaceToolConvexCollided(GFPhysCollideObject * rigidobj , 
// 		GFPhysCollideObject * softobj ,
// 		GFPhysSoftBodyFace * facecollide,
// 		const GFPhysVector3 & CdpointOnFace,
// 		const GFPhysVector3 &  CdnormalOnFace,
// 		float depth,
// 		float weights[3],
// 		int   contactmode
// 		);

	//when end check collision with soft body
// 	virtual void EndCheckSoftBodyCollision(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj);

	//when rigid and soft collision contact solved
	//note! this return a vector in all the world you should filter those contact not belong this tool manually
// 	virtual void RigidSoftCollisionsSolved(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);

	//when a physics update frame start
	virtual void PhysicsSimulationStart(int currStep , int TotalStep , float dt);

	//when a physics update frame end
	virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt);

	void CustomClipOrgan(MisMedicOrgan_Ordinary * organ , 
						 std::vector<GFPhysSoftBodyFace*> & faceInRegL ,
						 std::vector<GFPhysSoftBodyFace*> & faceInRegR ,
						 const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & CompressTetras,
						 const GFPhysVector3 & direInMaterialSpace);
	//when a ordinary frame update start(one ordinary frame may include zero-many physics frame)
	virtual void OneFrameUpdateStarted(float timeelapsed);

	//when a ordinary frame update end
// 	virtual void OneFrameUpdateEnded();
	bool m_AppAllowClamp;
	ClipOperationData m_ClipData;

	float m_ClipBeginCheckShaft;

	bool  m_canClip;

	int		m_clipRemain;					//钛夹剩余的数量，初始状态下剩余量为6

	float m_TimeElapsedSinceLastClip;

	float m_LastClipShaft;//上一次成功释放钛夹的shaft值

	float m_MaxShaftSinceLastClip;//上一次成功释放钛夹后到目前为止的最大shaft

	bool  m_HasEmptyClipToRelease;
	bool  m_bHasNipObject;					//标记钛夹钳是否夹住物体

	bool m_bIsClampedVertical;				//夹住物体后是否与物体垂直
	bool m_bLowerHalfPartIsVisiable;		//是否下半部分可见
};

#endif