#ifndef _MISCTOOL_PLUGINSILVERCLIP_
#define _MISCTOOL_PLUGINSILVERCLIP_
#include "Ogre.h"
#include "MisMedicCToolPluginInterface.h"
#include "Instruments/Tool.h"
//检查正好剪断在钛夹位置时候的bug
class MisCTool_PluginSilverClip : public MisMedicCToolPluginInterface
{
public:
	
	MisCTool_PluginSilverClip(CTool * tool);

	~MisCTool_PluginSilverClip();

	//const NewTrainToolConvexData & GetToolClampUpperPart();

	//const NewTrainToolConvexData & GetToolClampDownPart();

	//const NewTrainToolConvexData & GetClampRegionBelongPart();

	//GFPhysVector3 GetToolUpperPartWorldNormal();
	
	//GFPhysVector3 GetToolDownPartWorldNormal();

	//predict the orientation of silver clip in undeformed frame
	//void  PredictClipOrientInUndeformedSpace(GFPhysVectorObj<GFPhysSoftBodyNode*> & PressedNodes);

	/*void  CollectPressedFaces(GFPhysSoftBody* sb,
							  std::vector<GFPhysSoftBodyFace*> & FacesClampedUpper,
						 	  std::vector<GFPhysSoftBodyFace*> & FacesClampedDown);*/

	//bool  IsPressedFacesEnough(const std::vector<GFPhysSoftBodyFace*> & FacesClampedUpper,
	//						  const std::vector<GFPhysSoftBodyFace*> & FacesClampedDown);

	void  TryApplySilverClip(MisMedicOrgan_Ordinary * organclip , 
							 std::vector<GFPhysSoftBodyFace*> & faceInRegL ,
							 std::vector<GFPhysSoftBodyFace*> & faceInRegR ,
							 const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & CompressTetras,
							 const GFPhysVector3 & direInMaterialSpace);

	//when a physics update frame start
	virtual void PhysicsSimulationStart(int currStep , int TotalStep , float dt);

	//when a physics update frame end
	virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt);

	//when a ordinary frame update start(one ordinary frame may include zero-many physics frame)
	virtual void OneFrameUpdateStarted(float timeelapsed);

	//when a ordinary frame update end
// 	virtual void OneFrameUpdateEnded();
	//SliverClipData m_ClipRegion;

	//ClipOperationData m_ClipData;

	float m_ClipBeginCheckShaft;

	bool  m_canClip;

	float m_TimeElapsedSinceLastClip;

	float m_LastClipShaft;//上一次成功释放钛夹的shaft值

	float m_MaxShaftSinceLastClip;//上一次成功释放钛夹后到目前为止的最大shaft

	bool  m_HasEmptyClipToRelease;

	bool  m_hasClipOrganization;		//是否夹住器官
	//bool  testNode(GFPhysVector3& nodePos_0, GFPhysVector3& nodePos_1, float& wiget_0, float& wiget_1, float& wiget_2);
};

#endif