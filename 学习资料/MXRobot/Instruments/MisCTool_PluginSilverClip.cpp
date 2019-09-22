#include "MisCTool_PluginSilverClip.h"
#include "Instruments/Tool.h"
#include "Instruments/ClipApplier.h"
#include "MisMedicOrganOrdinary.h"
#include "Math/GoPhysAabbUtil2.h"
#include "collision/NarrowPhase/GoPhysPrimitiveTest.h"
#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "MisMedicOrganAttachment.h"
#include "PhysicsWrapper.h"
#include "IObjDefine.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "AcessoriesCutTrain.h"
#include "TrainingMgr.h"
class ClipFaceInterval
{
public:
	ClipFaceInterval(GFPhysSoftBodyFace * face , 
		float t0 , 
		float t1 , 
		const GFPhysVector3 & minPos,
		const GFPhysVector3 & maxPos) : 
	m_face(face) , m_tMin(t0) , m_tMax(t1) , m_PosMin(minPos) ,m_PosMax(maxPos)
	{}
	GFPhysSoftBodyFace * m_face;
	GFPhysVector3 m_PosMin;
	GFPhysVector3 m_PosMax;
	float m_tMin;
	float m_tMax;
};

//========================================================================================================
MisCTool_PluginSilverClip::MisCTool_PluginSilverClip(CTool * tool) : MisMedicCToolPluginInterface(tool),m_hasClipOrganization(false)
{
	m_ClipBeginCheckShaft = 4;
	m_canClip = false;
	m_TimeElapsedSinceLastClip = 0;
	m_LastClipShaft = 0;
	m_HasEmptyClipToRelease = false;

}
//========================================================================================================
MisCTool_PluginSilverClip::~MisCTool_PluginSilverClip()
{
	
}
//==================================================================================================
static int SelectFaceToApplyClip(const std::vector<GFPhysSoftBodyFace*> & LeftClampedFace, 
								 const std::vector<GFPhysSoftBodyFace*> & RightClampedFace, 
								 GFPhysVector3 leftbladepoint[2] , 
								 GFPhysVector3 rightbladepoint[2] ,
								 std::vector<GFPhysSoftBodyFace*> & ResultFaces,
								 GFPhysSoftBodyFace * FaceForOrient[2])
{
	float LefBladeLen  = (leftbladepoint[1]-leftbladepoint[0]).Length();
	float RigBladeLen  = (rightbladepoint[1]-rightbladepoint[0]).Length();

	GFPhysVector3 DirL = (leftbladepoint[1]-leftbladepoint[0]) / LefBladeLen;
	GFPhysVector3 DirR = (rightbladepoint[1]-rightbladepoint[0]) / RigBladeLen;

	GFPhysVector3 temp = rightbladepoint[0]-leftbladepoint[0];
	temp = temp - DirL*temp.Dot(DirL);
	GFPhysVector3 CutL = temp.Normalize();

	temp = leftbladepoint[0]-rightbladepoint[0];
	temp = temp - DirR*temp.Dot(DirR);
	GFPhysVector3 CutR = temp.Normalize();

	//check left clamp
	float leftmint = FLT_MAX;
	float leftmaxt = -FLT_MAX;

	GFPhysVector3 CliptriVerts[2][3];
	CliptriVerts[0][0] = leftbladepoint[0]-CutL*0.5f;
	CliptriVerts[0][1] = leftbladepoint[1]-CutL*0.5f;
	CliptriVerts[0][2] = leftbladepoint[0]+CutL*0.5f;

	CliptriVerts[1][0] = leftbladepoint[1]-CutL*0.5f;
	CliptriVerts[1][1] = leftbladepoint[0]+CutL*0.5f;
	CliptriVerts[1][2] = leftbladepoint[1]+CutL*0.5f;

	GFPhysAlignedVectorObj<ClipFaceInterval> FClipIntervalLeft;
	int lminIndex = -1;
	int lmaxIndex = -1;
	for(size_t f = 0 ; f < LeftClampedFace.size() ; f++)
	{
		GFPhysVector3 ResultPoint[2];

		GFPhysVector3 FaceVerts[3];
		GFPhysSoftBodyFace * face = LeftClampedFace[f];
		FaceVerts[0] = face->m_Nodes[0]->m_CurrPosition;
		FaceVerts[1] = face->m_Nodes[1]->m_CurrPosition;
		FaceVerts[2] = face->m_Nodes[2]->m_CurrPosition;

		float fMin = FLT_MAX;
		float fMax = -FLT_MAX;
		GFPhysVector3 fmaxPos;
		GFPhysVector3 fminPos;
		for(int t = 0 ; t < 2 ; t++)//test intersect with quad
		{
			bool intersect = TriangleIntersect(CliptriVerts[t] , FaceVerts , ResultPoint);

			if(intersect)
			{
				float tMin = (ResultPoint[0]-leftbladepoint[0]).Dot(DirL);
				float tMax = (ResultPoint[1]-leftbladepoint[0]).Dot(DirL);
				if(tMin > tMax)
				{
					std::swap(tMin , tMax);
					std::swap(ResultPoint[0] , ResultPoint[1]);
				}

				if(tMin < fMin)
				{
					fMin = tMin;
					fminPos = ResultPoint[0];
				}

				if(tMax > fMax)
				{
					fMax = tMax;
					fmaxPos = ResultPoint[1];
				}
			}
		}
		if(fMax >= fMin)//this face intersect quad
		{
			//update global interval
			if(fMin < leftmint)
			{
				lminIndex = (int)FClipIntervalLeft.size(); 
				leftmint = fMin;
			}

			if(fMax > leftmaxt)
			{
				lmaxIndex = (int)FClipIntervalLeft.size();
				leftmaxt = fMax;
			}

			FClipIntervalLeft.push_back(ClipFaceInterval(face , fMin , fMax , fminPos , fmaxPos));
		}
	}

	//check right
	float rightmint = FLT_MAX;
	float rightmaxt = -FLT_MAX;

	int rminIndex = -1;
	int rmaxIndex = -1;
	CliptriVerts[2][3];
	CliptriVerts[0][0] = rightbladepoint[0]-CutR*0.5f;
	CliptriVerts[0][1] = rightbladepoint[1]-CutR*0.5f;
	CliptriVerts[0][2] = rightbladepoint[0]+CutR*0.5f;

	CliptriVerts[1][0] = rightbladepoint[1]-CutR*0.5f;
	CliptriVerts[1][1] = rightbladepoint[0]+CutR*0.5f;
	CliptriVerts[1][2] = rightbladepoint[1]+CutR*0.5f;

	GFPhysAlignedVectorObj<ClipFaceInterval> FClipIntervalRight;
	for(size_t f = 0 ; f < RightClampedFace.size() ; f++)
	{
		GFPhysVector3 ResultPoint[2];

		GFPhysVector3 FaceVerts[3];
		GFPhysSoftBodyFace * face = RightClampedFace[f];
		FaceVerts[0] = face->m_Nodes[0]->m_CurrPosition;
		FaceVerts[1] = face->m_Nodes[1]->m_CurrPosition;
		FaceVerts[2] = face->m_Nodes[2]->m_CurrPosition;

		float fMin = FLT_MAX;
		float fMax = -FLT_MAX;
		GFPhysVector3 fmaxPos;
		GFPhysVector3 fminPos;

		for(int t = 0 ; t < 2 ; t++)//test intersect with quad
		{
			bool intersect = TriangleIntersect(CliptriVerts[t] , FaceVerts , ResultPoint);

			if(intersect)
			{
				float tMin = (ResultPoint[0]-rightbladepoint[0]).Dot(DirR);
				float tMax = (ResultPoint[1]-rightbladepoint[0]).Dot(DirR);
				if(tMin > tMax)
				{
					std::swap(tMin , tMax);
					std::swap(ResultPoint[0] , ResultPoint[1]);
				}

				if(tMin < fMin)
				{
					fMin = tMin;
					fminPos = ResultPoint[0];
				}

				if(tMax > fMax)
				{
					fMax = tMax;
					fmaxPos = ResultPoint[1];
				}
			}
		}
		if(fMax >= fMin)//this face intersect quad
		{
			//update global interval
			if(fMin < rightmint)
			{
				rminIndex = (int)FClipIntervalRight.size();
				rightmint = fMin;
			}

			if(fMax > rightmaxt)
			{
				rmaxIndex = (int)FClipIntervalRight.size();
				rightmaxt = fMax;
			}

			FClipIntervalRight.push_back(ClipFaceInterval(face , fMin , fMax , fminPos , fmaxPos));
		}
	}

	if(FClipIntervalRight.size() == 0 || FClipIntervalLeft.size() == 0)
		return -1;

	
	//select left interval if intersect interval greater than right
	int SelSide = -1;

	if((leftmaxt-leftmint) > (rightmaxt-rightmint))//choose left side as compress direction
	{
		float center = (leftmaxt+leftmint)*0.5f;
		float MinDistC = FLT_MAX;
		int   AttachFaceIndex = -1;
		for(size_t f = 0 ; f < FClipIntervalLeft.size() ; f++)
		{
			float t0 = FClipIntervalLeft[f].m_tMin;
			float t1 = FClipIntervalLeft[f].m_tMax;
			if(t0 <= center && t1 >= center)
			{
				MinDistC = 0;
				AttachFaceIndex = f;
			}
			else if(t1 < center)
			{
				if(fabsf(t1-center) < MinDistC)
				{
					MinDistC = fabsf(t1-center);
					AttachFaceIndex = f;
				}
			}
			else if(t0 > center)
			{
				if(fabsf(t0-center) < MinDistC)
				{
					MinDistC = fabsf(t0-center);
					AttachFaceIndex = f;
				}
			}
		}
		//swap to zero
		if(AttachFaceIndex >= 0)
			std::swap(FClipIntervalLeft[AttachFaceIndex] , FClipIntervalLeft[0]);

		for(size_t f = 0 ; f < FClipIntervalLeft.size() ; f++)
		{
			ResultFaces.push_back(FClipIntervalLeft[f].m_face);
		}
		SelSide = 0;
		FaceForOrient[0] = FClipIntervalLeft[lminIndex].m_face;
		FaceForOrient[1] = FClipIntervalLeft[lmaxIndex].m_face;
	}
	else
	{	
		float center = (rightmaxt+rightmint)*0.5f;
		float MinDistC = FLT_MAX;
		int   AttachFaceIndex = -1;
		for(size_t f = 0 ; f < FClipIntervalRight.size() ; f++)
		{
			float t0 = FClipIntervalRight[f].m_tMin;
			float t1 = FClipIntervalRight[f].m_tMax;
			if(t0 <= center && t1 >= center)
			{
				MinDistC = 0;
				AttachFaceIndex = f;
			}
			else if(t1 < center)
			{
				if(center-t1 < MinDistC)
				{
					MinDistC = center-t1;
					AttachFaceIndex = f;
				}
			}
			else if(t0 > center)
			{
				if(t0-center < MinDistC)
				{
					MinDistC = t0-center;
					AttachFaceIndex = f;
				}
			}
		}

		//swap to zero
		if(AttachFaceIndex >= 0)
			std::swap(FClipIntervalRight[AttachFaceIndex] , FClipIntervalRight[0]);

		for(size_t f = 0 ; f < FClipIntervalRight.size() ; f++)
		{
			ResultFaces.push_back(FClipIntervalRight[f].m_face);
		}
		SelSide = 1;

		FaceForOrient[0] = FClipIntervalRight[rminIndex].m_face;
		FaceForOrient[1] = FClipIntervalRight[rmaxIndex].m_face;
	}

	if(leftmaxt > 0.1f &&  rightmaxt > 0.1f)
		return SelSide;
	else
		return -1;
}


void MisCTool_PluginSilverClip::TryApplySilverClip(MisMedicOrgan_Ordinary * organclip , 
												   std::vector<GFPhysSoftBodyFace*> & faceInRegL ,
												   std::vector<GFPhysSoftBodyFace*> & faceInRegR ,
												   const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & CompressTetras,
												   const GFPhysVector3 & direInMaterialSpace)
{
	if(m_canClip == false || m_hasClipOrganization == true)
	   return;
	
	float toolshaft = m_ToolObject->GetShaftAside();
	int leftNum  = faceInRegL.size();
	int rightNum = faceInRegR.size();

	if(rightNum > 0 && leftNum > 0)
	{
		std::vector<GFPhysSoftBodyFace*> ResultFaces;
		GFPhysSoftBodyFace * FaceForOrient[2];
		int selSide = SelectFaceToApplyClip(faceInRegL , 
											faceInRegR ,
											m_ToolObject->m_CutBladeLeft.m_LinePointsWorld ,
											m_ToolObject->m_CutBladeRight.m_LinePointsWorld ,
											ResultFaces,
											FaceForOrient
											);
		if(selSide >= 0)
		{
		   //compress for deformed effect
		   std::set<GFPhysSoftBodyNode*> NodesBePressed;
		   for(size_t c = 0 ; c < ResultFaces.size() ; c++)
		   {
			   NodesBePressed.insert(ResultFaces[c]->m_Nodes[0]);
			   NodesBePressed.insert(ResultFaces[c]->m_Nodes[1]);
			   NodesBePressed.insert(ResultFaces[c]->m_Nodes[2]);
		   }

		   GFPhysVectorObj<GFPhysSoftBodyNode*> CompressNodes;
		   std::set<GFPhysSoftBodyNode*>::iterator nitor = NodesBePressed.begin();
		   while(nitor != NodesBePressed.end())
		   {
				CompressNodes.push_back(*nitor);
				nitor++;
		   }

		   GoPhysSoftBodyRestShapeModify restShapeModify;

		   /*
		   if(selSide == 0)//left
		   {
			  restShapeModify.CompressInOneDirection( PhysicsWrapper::GetSingleTon().m_dynamicsWorld , organclip->m_physbody , LeftClampNormal , CompressNodes , 0.5f , 0.05f ,0.2f , true);
		   }
		   else
		   {
			  restShapeModify.CompressInOneDirection( PhysicsWrapper::GetSingleTon().m_dynamicsWorld , organclip->m_physbody , RightClampNormal , CompressNodes , 0.5f , 0.05f , 0.2f , true);
		   }*/
		   
		  // restShapeModify.HardCompressInMaterialSpace(PhysicsWrapper::GetSingleTon().m_dynamicsWorld , 
													//   organclip->m_physbody , 
													//   direInMaterialSpace ,
													//   CompressTetras , 0.2f , 0.1f);

	
		   Ogre::SceneNode * ToolKernelNode = m_ToolObject->GetKernelNode();
		   Ogre::Vector3 ToolWorldPos = ToolKernelNode->_getDerivedPosition();		
		   Ogre::Quaternion ToolWorldOrient = ToolKernelNode->_getDerivedOrientation();
		   Ogre::Vector3 ToolWorlScale = ToolKernelNode->_getDerivedScale();

		   //create silver clip
		   MisMedicSilverClamp * silverClip = new MisMedicSilverClamp( ResultFaces[0] ,
																	   &ResultFaces[0] ,
																	   ResultFaces.size() ,
																	   FaceForOrient,
																	   organclip,
																	   ToolWorldPos ,
																	   ToolWorldOrient,
																	   Ogre::Vector3(1,1,1),
																	   0);
		   organclip->AddOrganAttachment(silverClip);

		   m_ToolObject->SetMinShaftAside(1.0f);
		   m_canClip = false;
		   m_TimeElapsedSinceLastClip = 0;
		   m_MaxShaftSinceLastClip = m_LastClipShaft = toolshaft;

		   //当银夹夹住器官时，隐藏银夹钳上的银夹
		   Ogre::Node::ChildNodeIterator iter = m_ToolObject->GetKernelNode()->getChildIterator();
		   while(iter.hasMoreElements())
		   {
			   std::string name = iter.getNext()->getName();
			   if (name.find("applier01-1")!= string::npos)
			   {
				   dynamic_cast<Ogre::SceneNode*>(m_ToolObject->GetKernelNode()->getChild(name))->setVisible(false);
			   }
		   }
		   iter = m_ToolObject->GetRightNode()->getChildIterator();
		   while (iter.hasMoreElements())
		   {
			   std::string name = iter.getNext()->getName();
			   if (name.find("applier03")!= string::npos)
			   {
				   dynamic_cast<Ogre::SceneNode*>(m_ToolObject->GetRightNode()->getChild(name))->removeAndDestroyAllChildren();
			   }
			   else if (name.find("applier01-2") != string::npos)
			   {
				   dynamic_cast<Ogre::SceneNode*>(m_ToolObject->GetRightNode()->getChild(name))->setVisible(false);
			   }
		   }

		   MxEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew( MxEvent::MXET_AddSilverClip, m_ToolObject , organclip );
		   if ( pEvent != NULL )
		   {
			   CMXEventsDump::Instance()->PushEvent(pEvent , true);
			   m_hasClipOrganization = true;
		   }


		   //通知训练--需要重构 改为在收到事件通知后处理
		   std::vector<Ogre::Vector2> TextureCoords;
		   std::vector<Ogre::Vector2> TFUV;
		   CAcessoriesCutTraining* cutTrain = dynamic_cast<CAcessoriesCutTraining*>(CTrainingMgr::Get()->GetCurTraining());
		   if (cutTrain)
		   {						
			   for(size_t cU = 0 ; cU < ResultFaces.size() ; cU++)
			   {
				   GFPhysSoftBodyFace * face  = ResultFaces[cU];
				   
				   float centerWeights[3] = {0.3333f , 0.3333f , 0.3333f};
				   
				   Ogre::Vector2 textureCoord = organclip->GetTextureCoord(face , centerWeights);
				   TextureCoords.push_back(textureCoord);

			   }
			   cutTrain->receiveCheckPointList(MisNewTraining::OCPT_Clip, TextureCoords, TFUV , NULL , organclip );
		   }

		}
	}

}
//==================================================================================================
void MisCTool_PluginSilverClip::PhysicsSimulationStart(int currStep , int TotalStep , float dt)
{

}
//================================================================================================
void MisCTool_PluginSilverClip::PhysicsSimulationEnd(int currStep , int TotalStep , float dt)
{  
	
}
//==================================================================================================
void MisCTool_PluginSilverClip::OneFrameUpdateStarted(float timeelapsed)
{
	float toolshaft = m_ToolObject->GetShaftAside();

	if(m_canClip == false)
	{
		m_TimeElapsedSinceLastClip += timeelapsed;

		if(toolshaft > m_MaxShaftSinceLastClip)
		   m_MaxShaftSinceLastClip = toolshaft;

		if(m_TimeElapsedSinceLastClip > 1.0f && (m_MaxShaftSinceLastClip > m_LastClipShaft+2))
		{
		   m_canClip = true;
		   m_ToolObject->SetMinShaftAside(0);
		}
	}
	else if(toolshaft <= 0.001f)
	{
		m_HasEmptyClipToRelease = true;
	}
	else if(toolshaft > 1.0f && m_HasEmptyClipToRelease)
	{
		//((CClipApplier*)m_ToolObject)->CreateEmptyClip();

		m_HasEmptyClipToRelease = false;
	}

}
//==================================================================================================
// void MisCTool_PluginClipTitanic::OneFrameUpdateEnded()
// {
// 
// }
