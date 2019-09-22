#include <OgreMovablePlane.h>
#include "AppendectomyTraining.h"
#include "MisMedicOrganOrdinary.h"
#include "VeinConnectObject.h"
#include "TextureBloodEffect.h"
#include "MisMedicEffectRender.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "XMLWrapperTraining.h"
#include "Dynamic/Constraint/GoPhysSoftFixpointConstraint.h"
#include "CustomConstraint.h"
#include "InputSystem.h"
#include "MXToolEvent.h"
#include "MisMedicBindedRope.h"
#include "TrainingCommon.h"
#include "MxOrganBindedEvent.h"
#include "Inception.h"
#include "DeferredRendFrameWork.h"


UINT32 AppendixAreaMark[] = 
{
	0x00001,		//	阑尾剪切区域
	0x00004,		//	阑尾取出区域
	0x00008,		//	阑尾烧白区域
	0x00010,		//	筋膜剪切区域
	0x00020,		//	筋膜取出区域
	0x00040,		//	筋膜烧白区域
	0x00080,		//不可取出部分
};

static bool GradForCutAppendProcess = false;

//=============================================================================================
CAppendectomyTraining::CAppendectomyTraining(const Ogre::String & strName)
:m_pDrawObject(NULL),
m_AppendixEndPoint(Ogre::Vector3::ZERO),
m_AppendixBottomSplitePlanePos(Ogre::Vector3::ZERO),
m_AppendixBottomSplitePlaneNormal(Ogre::Vector3::ZERO),
m_AppendixRightSideSplitePlanePos(Ogre::Vector3::ZERO),
m_AppendixRightSideSplitePlaneNormal(Ogre::Vector3::ZERO),
m_AppendixKnoteLeftPlanePos(Ogre::Vector3::ZERO),
m_AppendixKonteLeftPlaneNormal(Ogre::Vector3::ZERO),
m_AppendixKnoteRightPlanePos(Ogre::Vector3::ZERO),
m_AppendixKnoteRightPlaneNormal(Ogre::Vector3::ZERO),
m_MinDamageInterval(1.f),
m_DamageAppendixTimes(0),
m_LastDamgeAppendixTime(0.f),
m_DamageLargeIntestineTimes(0),
m_LastDamgeLargeIntestineTime(0.f),
m_DamageOtherOrganTimes(0),
m_LastDamageOtherOrganTime(0.f),
m_BleedTimes(0),
m_KnotAppendixAtErrorAreaTimes(0),
m_BurnFaceTotalTime(0.f)
{
	m_trainName = strName;
	m_AppendIntestineSepPlane[0] = GFPhysVector3(-8.215  , -5.606  , 7.462);
	m_AppendIntestineSepPlane[1] = GFPhysVector3(-19.668 , -12.129 , 8.055);
	m_AppendIntestineSepPlane[2] = GFPhysVector3(-18.412 , -15.579 , -5.648);
	
	//GFPhysVector3 sepNormal = (m_AppendIntestineSepPlane[1]-m_AppendIntestineSepPlane[0]).Cross(m_AppendIntestineSepPlane[2]-m_AppendIntestineSepPlane[0]).Normalized();
	//m_AppendOperatePlane[0] = m_AppendIntestineSepPlane[0]+sepNormal*2.4;
	//m_AppendOperatePlane[1] = m_AppendIntestineSepPlane[1]+sepNormal*2.4;
	//m_AppendOperatePlane[2] = m_AppendIntestineSepPlane[2]+sepNormal*2.4;

	m_KnotValidRegionPoints[0] = GFPhysVector3(-9.599f , -11.442f , 2.797f);//qianbu
	m_KnotValidRegionPoints[1] = GFPhysVector3(-11.219f , -9.484f , 2.71f);//接近阑尾根部的点
    m_KnotRadius = 5.0f;

	m_AppendixDirection = (m_KnotValidRegionPoints[0]-m_KnotValidRegionPoints[1]).Normalized();//从根部指向前部

	//m_KnotValidRegionTetras[0] = m_KnotValidRegionTetras[1] = 0;

	m_Vessels.m_PointsUndeformed.push_back(GFPhysVector3(-7.532,-12.138,1.716));
	m_Vessels.m_PointsUndeformed.push_back(GFPhysVector3(-7.948,-11.925,1.841));
	m_Vessels.m_PointsUndeformed.push_back(GFPhysVector3(-8.582,-11.541,1.989));
	m_Vessels.m_PointsUndeformed.push_back(GFPhysVector3(-9.474,-10.677,2.167));
	m_Vessels.m_PointsUndeformed.push_back(GFPhysVector3(-9.763,-10.372,2.166));
	m_Vessels.m_PointsUndeformed.push_back(GFPhysVector3(-10.257,-9.816,2.059));
	m_Vessels.m_PointsUndeformed.push_back(GFPhysVector3(-10.371,-9.596,1.955));

	m_AreaBuffer = 0;
	m_Appendix = 0;
	m_bToDebugDisplayMode = false;

	m_CameraCanCheck = false;
	m_CameraCheckFinish = false;
	m_InitCheckValue = FLT_MAX;

	m_NumSeparatedNode = 0;
	m_SeparaeRate = 0.15f;
	m_ValidNumSectionOfAppendix = 1;

	m_OrganBindedTimes = 0;
	m_ValidKnotTimes = 0;
	m_OrganBindedFinish = false;

	m_OrganCutFinish = false;

	m_NumNeedBurnFace = 0;
	m_CanDealWithAppendixRoot = false;
	m_DealWithAppendixRootFinish = false;
	m_TakeOffAppendixFinish = false;

	m_pDrawObject = 0;
}
//=============================================================================================
CAppendectomyTraining::~CAppendectomyTraining(void)
{
	if(m_AreaBuffer)
	{
	   delete []m_AreaBuffer;
	   m_AreaBuffer = 0;
	}

	MisNewTrainingDebugObject::GetInstance()->SetTraining(NULL);
}
//=============================================================================================
void CAppendectomyTraining::OnToolCreated(ITool * tool, int side)
{
	CKnotter * knotter = dynamic_cast<CKnotter*>(tool);
	if(knotter)
	{
	   knotter->SetLoopControlRegion(this);
	}
}
//=============================================================================================
void CAppendectomyTraining::OnToolRemoved(ITool * tool)
{
	CKnotter * knotter = dynamic_cast<CKnotter*>(tool);
	if(knotter)
	{
		knotter->SetLoopControlRegion(0);
	}
}
//=============================================================================================
bool CAppendectomyTraining::CanControlLoop(CKnotter * knotter)
{
	return true;
#if(0)
	if(m_KnotValidRegionTetras[0] == 0 || m_KnotValidRegionTetras[1] == 0)
	   return false;

	GFPhysVector3 ValidLoopRangePt0 =  m_KnotValidRegionTetras[0]->m_TetraNodes[0]->m_CurrPosition*m_KnotTetrasWeights[0][0]
									  +m_KnotValidRegionTetras[0]->m_TetraNodes[1]->m_CurrPosition*m_KnotTetrasWeights[0][1]
									  +m_KnotValidRegionTetras[0]->m_TetraNodes[2]->m_CurrPosition*m_KnotTetrasWeights[0][2]
									  +m_KnotValidRegionTetras[0]->m_TetraNodes[3]->m_CurrPosition*m_KnotTetrasWeights[0][3];

	GFPhysVector3 ValidLoopRangePt1 =  m_KnotValidRegionTetras[1]->m_TetraNodes[0]->m_CurrPosition*m_KnotTetrasWeights[1][0]
									  +m_KnotValidRegionTetras[1]->m_TetraNodes[1]->m_CurrPosition*m_KnotTetrasWeights[1][1]
									  +m_KnotValidRegionTetras[1]->m_TetraNodes[2]->m_CurrPosition*m_KnotTetrasWeights[1][2]
									  +m_KnotValidRegionTetras[1]->m_TetraNodes[3]->m_CurrPosition*m_KnotTetrasWeights[1][3];

	Ogre::Vector3 temp = knotter->GetKernelNode()->_getFullTransform().getTrans();
	
	GFPhysVector3 headPos(temp.x , temp.y , temp.z);
	
	//calculate head to line short pos
	
	float t1 = (headPos-ValidLoopRangePt0).Dot(ValidLoopRangePt1-ValidLoopRangePt0);
	
	float denom = (ValidLoopRangePt1-ValidLoopRangePt0).Length2();
	
	if(denom > FLT_EPSILON)
	{
		float t = t1 / denom;
		if(t >= 0 && t < 1)
		{
			GFPhysVector3 p2 = ValidLoopRangePt0 + (ValidLoopRangePt1-ValidLoopRangePt0)*t;
			float dist = (p2-headPos).Length();
			if(dist  < m_KnotRadius)
			{
				return true;
			}
		}
	}
	return false;
#endif
}
//==========================================================================================
MisMedicOrganInterface * CAppendectomyTraining::LoadOrganism( MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain)
{
	if (cs.m_objTopologyType == DOT_UNDEF)	// this should be 
	{
		m_pOms->s_nLoadCount++;
	
		Ogre::String rigidType = cs.m_RigidType;
		
		Ogre::String rigidName = cs.m_name;
		
		transform(rigidType.begin(), rigidType.end(), rigidType.begin(), ::tolower);
		
		transform(rigidName.begin(), rigidName.end(), rigidName.begin(), ::tolower);

		if ((int)rigidType.find("areamark") >= 0)
		{
			Ogre::String m_RigidType;
			cs.m_objTopologyType = DOT_AreaMark;
			
			m_AreaMarkTextureName = cs.m_s3mfilename;

			m_AreaTexPtr = Ogre::TextureManager::getSingleton().load(cs.m_s3mfilename ,
																	 Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																	 Ogre::TEX_TYPE_2D,
																	 0,
																	 1.0f,
																	 false,
																	 Ogre::PF_A8R8G8B8);
			m_AreaWidth  = m_AreaTexPtr->getWidth();
			
			m_AreaHeight = m_AreaTexPtr->getHeight();
			
			m_AreaBuffer = new Ogre::uint32[m_AreaWidth*m_AreaHeight];
			
			Ogre::HardwarePixelBufferSharedPtr pixelBuffer = m_AreaTexPtr->getBuffer();
			
			// lock the pixel buffer and get a pixel box
			pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
			
			const Ogre::PixelBox & pixelBox = pixelBuffer->getCurrentLock();

			Ogre::uint32 * pDest = static_cast<Ogre::uint32*>(pixelBox.data);
			
			int RowSkip = pixelBox.rowPitch;
			
			for(int h = 0 ; h < m_AreaHeight ; h++)
			{
				for(int w = 0 ; w < m_AreaWidth ; w++)
				{
					m_AreaBuffer[m_AreaWidth*h+w] = pDest[w];
				}
				pDest += RowSkip;
			}

			pixelBuffer->unlock();
			
			return NULL;
		}
	}

	return MisNewTraining::LoadOrganism(cs, ptrain);
}

//========================================================================================================
uint32 CAppendectomyTraining::GetPixelFromAreaBuffer(float u, float v)
{
	int tcx = u * (m_AreaWidth-1);

	int tcy = v * (m_AreaHeight-1);

	uint32 pix = m_AreaBuffer[tcy * m_AreaWidth + tcx];

	return pix;
}

void CAppendectomyTraining::SerializerReadFinish(MisMedicOrgan_Ordinary * organ , MisMedicObjetSerializer & serialize)
{
	return;

	GFPhysVector3 sepNormal = (m_AppendIntestineSepPlane[1]-m_AppendIntestineSepPlane[0]).Cross(m_AppendIntestineSepPlane[2]-m_AppendIntestineSepPlane[0]).Normalized();
	GFPhysVector3 sepPoint  = m_AppendIntestineSepPlane[0];

	for(int t = 0 ;t < serialize.m_InitTetras.size() ; t++)
	{
		MisMedicObjetSerializer::MisSerialTetra & teraelement = serialize.m_InitTetras[t];

		if(teraelement.m_unionObjectID == EDOT_APPENDIX)
		{
			bool isappendpart = false;

			for(int n = 0 ; n < 4 ; n++)
			{
				int nodeIndex = teraelement.m_Index[n];

				GFPhysVector3 nodePos = serialize.m_NodeInitPositions[nodeIndex];

				if((nodePos-sepPoint).Dot(sepNormal) > 0)
				{
					isappendpart = true;
					break;
				}
			}

			if(isappendpart == false)
			{
				teraelement.m_unionObjectID = EDOT_LARGEINTESTINE;
				//teraelement.massScale = 0.06f;
			}
		}
		else if(teraelement.m_unionObjectID == EDOT_APPENDMENSTORY)
		{
			//teraelement.massScale = 0.6f;
		}
	}
	//serialize.m_MergedObjectStiffness.insert(std::make_pair(EDOT_LARGEINTESTINE , 0.82f));
}
//============================================================================================
bool CAppendectomyTraining::ReadTrainParam(const std::string& strFileName)
{
	std::ifstream stream;
	
	stream.open(strFileName.c_str());
	
	if(stream.is_open())
	{
		char buffer[1000];
		while(!stream.eof())//while(stream.getline(buffer,99))
		{
			stream.getline(buffer,999);
			std::string str = buffer;
			int keyEnd = str.find('(');
			std::string key = str.substr(0,keyEnd);
			if (key == "AppendSeperatePlane")
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				Ogre::vector<Ogre::String>::type stringVec = Ogre::StringUtil::split(val , " ");
				for(int n = 0 ; n < 3 ; n++)
				{
					m_AppendIntestineSepPlane[n].m_x = atof(stringVec[3*n].c_str());
					m_AppendIntestineSepPlane[n].m_y = atof(stringVec[3*n+1].c_str());
					m_AppendIntestineSepPlane[n].m_z = atof(stringVec[3*n+2].c_str());
				}
			}
			else if (key == "LooperRange")
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				Ogre::vector<Ogre::String>::type stringVec = Ogre::StringUtil::split(val , " ");
				for(int n = 0 ; n < 2 ; n++)
				{
					m_KnotValidRegionPoints[n].m_x = atof(stringVec[3*n].c_str());
					m_KnotValidRegionPoints[n].m_y = atof(stringVec[3*n+1].c_str());
					m_KnotValidRegionPoints[n].m_z = atof(stringVec[3*n+2].c_str());
				}
			}
			else if (key == "Vessel")
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				Ogre::vector<Ogre::String>::type stringVec = Ogre::StringUtil::split(val , " ");
				m_Vessels.m_PointsUndeformed.clear();
				for(size_t n = 0 ; n < stringVec.size() / 3 ; n++)
				{
					GFPhysVector3 vesselPoint;
					vesselPoint.m_x = atof(stringVec[3*n].c_str());
					vesselPoint.m_y = atof(stringVec[3*n+1].c_str());
					vesselPoint.m_z = atof(stringVec[3*n+2].c_str());
					m_Vessels.m_PointsUndeformed.push_back(vesselPoint);
				}
			}
			else if(key == "CheckPoint")
			{
				int valEnd = str.find(')');
				std::string val = str.substr(keyEnd+1, valEnd-(keyEnd+1));
				Ogre::vector<Ogre::String>::type stringVec = Ogre::StringUtil::split(val , ",");
				m_CameraCheckPoints.clear();
				Ogre::Vector3 pt;
				for(size_t n = 0 ; n < stringVec.size() / 3 ; n++)
				{
					pt.x = atof(stringVec[3*n].c_str());
					pt.y = atof(stringVec[3*n+1].c_str());
					pt.z = atof(stringVec[3*n+2].c_str());
					m_CameraCheckPoints.push_back(pt);

					//m_pDrawObject->DrawPoint(pt);
				}
			}
		}
		return true;
	}
	return false;
}

void CAppendectomyTraining::ReadCustomDataFile(const Ogre::String & customDataFile)
{
	ReadTrainParam(customDataFile);//("../Config/MultiPortConfig/Appendectomy/sep_adult.txt");
}
class MarkVesselTetraCB : public GFPhysNodeOverlapCallback
{
public:
	MarkVesselTetraCB(OrganAppendixVessel & vessel) : m_vessel(vessel)
	{
		
	}
	virtual void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{

	}
	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
	{
		/*GFPhysSoftBodyTetrahedron * tetra = (GFPhysSoftBodyTetrahedron * )dynNodeA->m_UserData;
		
		int segIndex = (int)dynNodeB->m_UserData;
		
		GFPhysVector3 p0 = m_vessel.m_PointsUndeformed[segIndex];
		
		GFPhysVector3 p1 = m_vessel.m_PointsUndeformed[segIndex+1];

		GFPhysVector3 TetraVerts[4];
		TetraVerts[0] = tetra->m_TetraNodes[0]->m_UnDeformedPos;
		TetraVerts[1] = tetra->m_TetraNodes[1]->m_UnDeformedPos;
		TetraVerts[2] = tetra->m_TetraNodes[2]->m_UnDeformedPos;
		TetraVerts[3] = tetra->m_TetraNodes[3]->m_UnDeformedPos;

		bool interset = LineSegmentTetraIntersect(p0 , p1 , TetraVerts);
		if(interset)
		{
		   m_IntersectTetras.push_back(tetra);
		}*/
	}
	std::vector<GFPhysSoftBodyTetrahedron*> m_IntersectTetras;

	OrganAppendixVessel & m_vessel;
};

void CAppendectomyTraining::MarkTetraInVessel()
{
	const GFPhysDBVTree & undeformTetraTree = m_Appendix->m_physbody->GetSoftBodyShape().GetTetrahedronBVTree(false);

	GFPhysDBVTree vesselSegTrees;
	for(int c = 0 ; c < m_Vessels.m_PointsUndeformed.size()-1 ; c++)
	{
		GFPhysVector3 point0 = m_Vessels.m_PointsUndeformed[c];
		GFPhysVector3 point1 = m_Vessels.m_PointsUndeformed[c+1];
		
		GFPhysVector3 aabbmin = point0;
		GFPhysVector3 aabbmax = point0;

		aabbmin.SetMin(point1);
		aabbmax.SetMax(point1);
		
		aabbmin -= GFPhysVector3(0.001f , 0.001f , 0.001f);
		aabbmax += GFPhysVector3(0.001f , 0.001f , 0.001f);

		GFPhysDBVNode * bvNode = vesselSegTrees.InsertAABBNode(aabbmin , aabbmax);
		bvNode->m_UserData = (void*)c;

	}
	MarkVesselTetraCB callback(m_Vessels);
	undeformTetraTree.CollideWithDBVTree(vesselSegTrees , &callback);

	for(size_t c = 0 ; c < callback.m_IntersectTetras.size() ; c++)
	{
		GFPhysSoftBodyTetrahedron * tetras = callback.m_IntersectTetras[c];
		PhysTetra_Data & tetraAppData = m_Appendix->GetPhysTetraAppData(tetras);
		if(tetraAppData.m_IsMenstary == false)
		{
			int i = 0;
			int j = i+1;
		}
		if(!tetraAppData.m_HasError)
		{
			tetraAppData.m_ContainsVessel = true;
		}
	}
}
//======================================================================================================================
void CAppendectomyTraining::CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig)
{
	MisNewTraining::CreateTrainingScene(pTrainingConfig);
	
	DynObjMap::iterator itor = m_DynObjMap.find(EDOT_APPENDIX);

	m_Appendix = (MisMedicOrgan_Ordinary *)itor->second;
	std::vector<float> AppendoriginMass;
	GFPhysSoftBodyNode * node = m_Appendix->m_physbody->GetSoftBodyShape().GetNodeList();
	while (node)
	{
		AppendoriginMass.push_back(node->m_Mass);
		node = node->m_Next;
	}
	std::string morphedMeshName = "App_PreIlian_Morph.mesh";
	RemapAppendixToMorphPos(morphedMeshName);

	
	VeinConnectObject * connectCalot = 0;
	itor = m_DynObjMap.find(EODT_VEINCONNECT);

	//MisMedicOrganInterface * oif = m_DynObjMap.find(EODT_VEINCONNECT)->second;
	if (itor != m_DynObjMap.end())//oif)
	{
		MisMedicOrganInterface * oif = itor->second;
		connectCalot = dynamic_cast<VeinConnectObject*>(oif);
		connectCalot->m_Actived = false;
	}

	float simInterval = 1.0f / PhysicsWrapper::GetSingleTon().m_SimulateFrequency;
	for (int c = 0; c < (int)(1.0f / simInterval); c++)
	{
		PhysicsWrapper::GetSingleTon().UpdateWorld(simInterval);
	}

	int nc = 0;
	node = m_Appendix->m_physbody->GetSoftBodyShape().GetNodeList();
	while (node)
	{
		node->SetMass(AppendoriginMass[nc]);
		nc++;
		node = node->m_Next;
	}

	if (connectCalot)
	{
		connectCalot->m_Actived = true;
		connectCalot->BuildConnectConstraint(m_DynObjMap);//rebuild connection reset value
	}

	
}
//======================================================================================================================
bool CAppendectomyTraining::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{  
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SoftCollisionHashGridSize(1.0f);///----------

	//DeferredRendFrameWork::Get()->SetBloomBrightPassThreshold(0.92f);

	DynObjMap::iterator itor = m_DynObjMap.find(EDOT_APPENDIX);
	
	if(itor == m_DynObjMap.end())
	   return true;

	m_Appendix = (MisMedicOrgan_Ordinary *)itor->second;
		
	m_Appendix->SetTimeNeedToEletricCut(1.5f);
	
	m_Appendix->SetEletricCutParameter(0.3f, 5);
	
	m_Appendix->m_MaxCutCount = 5;

	m_Appendix->m_ElecCutThreshold = 0.4f;
	
	m_Appendix->m_CutWidthScale = 1.2f;

	m_Appendix->SetIsolatedPartGravity(30.0f);
	
	MarkTetraInVessel();


	for(size_t f = 0 ; f < m_Appendix->m_OriginFaces.size() ; f++)
	{
		MMO_Face & organface = m_Appendix->m_OriginFaces[f];

		GFPhysSoftBodyFace * physface = organface.m_physface;

		if(physface == 0)
		   continue;
	
		for(int v = 0 ; v < 3 ; v++)
		{
			GFPhysSoftBodyNode * FaceNode = physface->m_Nodes[v];

			Ogre::Vector2 vertTexCoord = organface.GetTextureCoord(v);

			uint32 areaValue = GetPixelFromAreaBuffer(vertTexCoord.x, vertTexCoord.y);

			//标记取出区域和剪切区域 用来判定是否取出
			if((areaValue & AppendixAreaMark[AT_AppendTakeOutPart]) != 0)
			{
				FaceNode->m_Flag |= AppendixAreaMark[AT_AppendTakeOutPart];
			}
			else if((areaValue & AppendixAreaMark[AT_AppendCutPart]) != 0)
			{
				FaceNode->m_Flag |= AppendixAreaMark[AT_AppendCutPart];
			}
			else if((areaValue & AppendixAreaMark[AT_MENSTakeOutPart]) != 0)
			{
				FaceNode->m_Flag |= AppendixAreaMark[AT_MENSTakeOutPart];
			}
			else if((areaValue & AppendixAreaMark[AT_MENSCutPart]) != 0)
			{
				FaceNode->m_Flag |= AppendixAreaMark[AT_MENSCutPart];
			}
			else
			{
				FaceNode->m_Flag |= AppendixAreaMark[AT_NoneTakeablePart];
			}
		}
	}

	int k = 0;
	for(std::size_t n = 1;n < m_Appendix->m_OrganRendNodes.size();++n)
	{
		MMO_Node& node = m_Appendix->m_OrganRendNodes[n];
		if(node.m_PhysNode)
		{
			if(IsMarkedArea(AT_SeparatedMentary,node.m_TextureCoord))
			{
				m_SeparatedNodes.insert(node.m_PhysNode);
				//m_pDrawObject->AddDynamicPoint(node.m_PhysNode,0.01f,Ogre::ColourValue::White);
			}
			else if(IsMarkedArea(AT_AppendixEndArea,node.m_TextureCoord))
			{
				m_AppendixEndPoint += GPVec3ToOgre(node.m_PhysNode->m_UnDeformedPos);
				++k;
			}
		}
	}
	m_NumSeparatedNode = m_SeparatedNodes.size();
	if(k)
		m_AppendixEndPoint /= k;
	//m_pDrawObject->DrawPoint(m_AppendixEndPoint,0.04,Ogre::ColourValue::Red);
	
	GradForCutAppendProcess = false;

	InitializePlane();

	return result;
}
bool CAppendectomyTraining::BeginRendOneFrame(float timeelapsed)
{
	bool result = MisNewTraining::BeginRendOneFrame(timeelapsed);

	//if (m_StaticDomeMeshPtr.isNull())
	//	return result;

	////attach
	//m_StaticDynamicUnion.UpdateStaticVertexByDynamic(m_StaticDomeMeshPtr);

	return result;
}
void CAppendectomyTraining::InitializePlane()
{
	Ogre::SceneManager* sceneMgr = MXOgre_SCENEMANAGER;
	Ogre::Matrix3 matrix1,matrix2;
	Ogre::Vector3 dir(0,1,0);

	//1 set knote plane
	Ogre::SceneNode* leftNode = sceneMgr->getSceneNode("KnotLeftPlane$1");
	Ogre::SceneNode* rightNode = sceneMgr->getSceneNode("KnotRightPlane$1");

	//1.1 set visible
	leftNode->setVisible(false);
	rightNode->setVisible(false);

	//1.2 set pos
	m_AppendixKnoteLeftPlanePos = leftNode->_getDerivedPosition();
	m_AppendixKnoteRightPlanePos = rightNode->_getDerivedPosition();

	//1.3 get direction
	leftNode->_getDerivedOrientation().ToRotationMatrix(matrix1);
	rightNode->_getDerivedOrientation().ToRotationMatrix(matrix2);

	//1.4 set normal
	Ogre::Vector3 leftDir = m_AppendixEndPoint - m_AppendixKnoteLeftPlanePos;
	Ogre::Vector3 rightDir = m_AppendixEndPoint - m_AppendixKnoteRightPlanePos;

	m_AppendixKonteLeftPlaneNormal = matrix1 * dir;
	m_AppendixKnoteRightPlaneNormal = matrix2 * dir;

	if(m_AppendixKonteLeftPlaneNormal.dotProduct(leftDir) < 0.f)
		m_AppendixKonteLeftPlaneNormal *= -1;
	if(m_AppendixKnoteRightPlaneNormal.dotProduct(rightDir) < 0.f)
		m_AppendixKnoteRightPlaneNormal *= -1;

	//test
// 	m_pDrawObject->DrawPoint(leftNode->_getDerivedPosition());		//
// 	m_pDrawObject->DrawPoint(rightNode->_getDerivedPosition());		//
// 	m_pDrawObject->DrawLine(leftNode->_getDerivedPosition(),leftNode->_getDerivedPosition() + m_AppendixKonteLeftPlaneNormal);		//
// 	m_pDrawObject->DrawLine(rightNode->_getDerivedPosition(),rightNode->_getDerivedPosition() + m_AppendixKnoteRightPlaneNormal);	//

	//2 set appendix splite plane
	Ogre::SceneNode* rightSideSpliteNode = sceneMgr->getSceneNode("appendixSplitePlane$1");
	Ogre::SceneNode* bottomSpliteNode = sceneMgr->getSceneNode("appendixBottomSplitePlane$1");

	//2.1 set visible
	rightSideSpliteNode->setVisible(false);
	bottomSpliteNode->setVisible(false);

	//2.2 set pos
	m_AppendixRightSideSplitePlanePos = rightSideSpliteNode->_getDerivedPosition();
	m_AppendixBottomSplitePlanePos = bottomSpliteNode->_getDerivedPosition();

	//2.3 get direction
	rightSideSpliteNode->_getDerivedOrientation().ToRotationMatrix(matrix1);
	bottomSpliteNode->_getDerivedOrientation().ToRotationMatrix(matrix2);

	//2.4 set normal
	m_AppendixRightSideSplitePlaneNormal = matrix1 * dir;
	m_AppendixBottomSplitePlaneNormal = matrix2 * dir;

	if(m_AppendixRightSideSplitePlaneNormal.dotProduct(m_AppendixKonteLeftPlaneNormal) > 0)
		m_AppendixRightSideSplitePlaneNormal *= -1;
	m_AppendixBottomSplitePlaneNormal *= -1;

	//test
// 	m_pDrawObject->DrawPoint(rightSideSpliteNode->_getDerivedPosition());		//
// 	m_pDrawObject->DrawPoint(bottomSpliteNode->_getDerivedPosition());			//
// 	m_pDrawObject->DrawLine(rightSideSpliteNode->_getDerivedPosition(),rightSideSpliteNode->_getDerivedPosition() + m_AppendixRightSideSplitePlaneNormal);		//
// 	m_pDrawObject->DrawLine(bottomSpliteNode->_getDerivedPosition(),bottomSpliteNode->_getDerivedPosition() + m_AppendixBottomSplitePlaneNormal);	//
}

void CAppendectomyTraining::SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ)
{
	if(organ->m_OrganID == EDOT_APPENDIX)
	{
		organ->GetCreateInfo().m_distributemass = true;
	}
	if(organ->m_OrganID == EDOT_LARGEINTESTINE)
	{
		organ->GetCreateInfo().m_distributemass = false;
	}
}

bool CAppendectomyTraining::Update( float dt )
{
	bool result = MisNewTraining::Update(dt);

	if(m_CameraCheckFinish == false && m_CameraCanCheck && (m_CameraCheckPoints.size() == m_CameraCheckValues.size()))
	{
		Ogre::Ray ray;
		const Ogre::Vector3& cameraPos = m_pLargeCamera->getDerivedPosition();
		float d = 0.f;
		float newValue = 0.f;

		m_pLargeCamera->getCameraToViewportRay(0.5,0.5,&ray);

		for(std::size_t p = 0;p < m_CameraCheckPoints.size();++p)
		{
			d = (m_CameraCheckPoints[p] - cameraPos).dotProduct(ray.getDirection());
			d /= (m_CameraCheckPoints[p] - cameraPos).length() * ray.getDirection().length();
			if(d > m_CameraCheckValues[p])
				m_CameraCheckValues[p] = d;

			newValue += m_CameraCheckValues[p];
		}
		
		d = newValue - m_InitCheckValue;
		if(d > 0.08)
		{
			m_CameraCheckFinish = true;
			AddOperateItem("LocateSuccess",1.f,true);
			//ShowTip("CheckFinish");			//
		}
	}

	if(m_DealWithAppendixRootFinish && m_TakeOffAppendixFinish)
	{
		TrainingFinish();
	}


	bool bPlus =  InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetShowModel_Plus();
	
	if(m_Appendix)
	{
		if (m_bToDebugDisplayMode != bPlus && m_AreaMarkTextureName.length() > 0)
		{
			m_bToDebugDisplayMode = bPlus;

			if (m_bToDebugDisplayMode)
			{
				m_AppendMaterialName = m_Appendix->getMaterialName();

				m_Appendix->setMaterialName("Appendix_Debug");

				Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("Appendix_Debug");

				Ogre::TexturePtr  srctex = Ogre::TextureManager::getSingleton().load(m_AreaMarkTextureName , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

				ApplyTextureToMaterial(material , srctex , "AreaMarkMap");
			}
			else
			{
				m_Appendix->setMaterialName(m_AppendMaterialName);
			}
		}
	}
	
	if(m_pDrawObject)
		m_pDrawObject->Update();

	return result;
}

//=====================================================================================================
void CAppendectomyTraining::CheckAppendixSeperated()
{	
#if(0)
	int MaxAppendTakeOutNum = 0;
	int MaxAppendTakeOutNum_Mens = 0;
	int MaxAppendTakeOutNum_NoTakePart = 0;

	GFPhysSoftBody * physbody = m_Appendix->m_physbody;

	std::vector<GFPhysSoftBodyTetrahedron *> SelectedTetras;
	SelectedTetras.reserve(10000);

	for(size_t th = 0 ; th < sb->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * tetra = sb->GetTetrahedronAtIndex(th);

		if(tetra->getUserIndex(0xff) == EDOT_APPENDIX || tetra->getUserIndex(0xff) == EDOT_APPENDMENSTORY)
		{
			tetra->m_TempData = (void*)0;
			SelectedTetras.push_back(tetra);
		}
		else
		{
			tetra->m_TempData = (void*)(~0);
		}
		
	}

	
	std::set<GFPhysSoftBodyNode*> ClusterNodesSet;
	
	std::deque<GFPhysSoftBodyTetrahedron*> QueueTetras;

	int GlobalClusterID = 1;

	for(size_t t = 0 ; t < SelectedTetras.size(); t++)
	{
		tetra = SelectedTetras[t];

		if(tetra->m_TempData == 0)//this tetra not belong any cluster
		{
			QueueTetras.clear();
			
			ClusterNodesSet.clear();
					
			tetra->m_TempData = (void*)GlobalClusterID;
			
			QueueTetras.push_back(tetra);

			while(QueueTetras.size() > 0)
			{	
				GFPhysSoftBodyTetrahedron * QTetra = QueueTetras.front();
				QueueTetras.pop_front();

				//add surface node to set
				if(QTetra->m_TetraNodes[0]->m_insurface)
				   ClusterNodesSet.insert(QTetra->m_TetraNodes[0]);

				if(QTetra->m_TetraNodes[1]->m_insurface)
				   ClusterNodesSet.insert(QTetra->m_TetraNodes[1]);

				if(QTetra->m_TetraNodes[2]->m_insurface)
				   ClusterNodesSet.insert(QTetra->m_TetraNodes[2]);

				if(QTetra->m_TetraNodes[3]->m_insurface)
				   ClusterNodesSet.insert(QTetra->m_TetraNodes[3]);

				//add 4 neighbor tetrahedron in cluster if not added yet
				for(int nb = 0 ; nb < 4 ; nb++)
				{
					GFPhysGeneralizedFace * genFace = QTetra->m_TetraFaces[nb];

					if(genFace && genFace->m_ShareTetrahedrons.size() > 1)
					{
						GFPhysSoftBodyTetrahedron * t0 = genFace->m_ShareTetrahedrons[0].m_Hosttetra;

						GFPhysSoftBodyTetrahedron * t1 = genFace->m_ShareTetrahedrons[1].m_Hosttetra;

						GFPhysSoftBodyTetrahedron * NBTetra = (t0 == QTetra ? t1 : t0);

						if(NBTetra->m_TempData == 0)
						{
							NBTetra->m_TempData = (void*)GlobalClusterID;
							QueueTetras.push_back(NBTetra);
						}
					}
				}
			}

			int AppendTakeOutNum = 0;//take out part and cut part all belong to take out
			
			int MenstoryTakeOutNum = 0;//take out part and cut part all belong to take out

			int NoTakeableNum = 0;

			GFPhysSoftBodyNode * ns;
			
			std::set<GFPhysSoftBodyNode *>::iterator snitor = ClusterNodesSet.begin();
			
			while(snitor != ClusterNodesSet.end())
			{
				ns = (*snitor);
				
				if (ns->m_Flag & AppendixAreaMark[AT_AppendTakeOutPart])
					AppendTakeOutNum++;
				
				else if (ns->m_Flag & AppendixAreaMark[AT_AppendCutPart])
					AppendTakeOutNum++;
				
				else if (ns->m_Flag & AppendixAreaMark[AT_MENSTakeOutPart])
					MenstoryTakeOutNum++;
				
				else if (ns->m_Flag & AppendixAreaMark[AT_MENSCutPart])
					MenstoryTakeOutNum++;

				else if(ns->m_Flag & AppendixAreaMark[AT_NoneTakeablePart])
					NoTakeableNum++;

				snitor++;
			}

			if(AppendTakeOutNum > MaxAppendTakeOutNum)
			{
				MaxAppendTakeOutNum = AppendTakeOutNum;
				MaxAppendTakeOutNum_Mens = MenstoryTakeOutNum;
				MaxAppendTakeOutNum_NoTakePart = NoTakeableNum;
			}
			
			GlobalClusterID++;//advance
		}
	}
	
	if(MaxAppendTakeOutNum > 0 && MaxAppendTakeOutNum_NoTakePart <= 10)
	{
		if(GradForCutAppendProcess == false)
		{
			CScoreMgr::Instance()->Grade("SeperateAppendix");
			CTipMgr::Instance()->ShowTip("CutAppendixSucced");
			GradForCutAppendProcess = true;
		}
	}
#endif
}

void CAppendectomyTraining::OnTrainingIlluminated()
{
	__super::OnTrainingIlluminated();

	const Ogre::Vector3& cameraPos = m_pLargeCamera->getDerivedPosition();
	Ogre::Ray ray;

	m_pLargeCamera->getCameraToViewportRay(0.5,0.5,&ray);
	m_CameraCheckValues.clear();
	m_InitCheckValue = 0.f;

	for(std::size_t p = 0;p < m_CameraCheckPoints.size();++p)
	{
		float value = (m_CameraCheckPoints[p] - cameraPos).dotProduct(ray.getDirection());
		value /= (m_CameraCheckPoints[p] - cameraPos).length() * ray.getDirection().length();
		m_CameraCheckValues.push_back(value);
		m_InitCheckValue += value;
	}

	//debug object
	MisNewTrainingDebugObject::GetInstance()->SetTraining(this);
	m_pDrawObject = MisNewTrainingDebugObject::GetInstance()->GetDrawObject();
}

void CAppendectomyTraining::OnCameraStateChanged(bool bFixed)
{
	m_CameraCanCheck = !bFixed;
}

void CAppendectomyTraining::FacesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces, GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * organ)
{
    __super::FacesBeRemoved(faces, createdfaces, organ);

	for(std::size_t f = 0;f < faces.size();++f)
	{
		RemoveMentaryNode(faces[f]->m_Nodes[0],organ);
		RemoveMentaryNode(faces[f]->m_Nodes[1],organ);
		RemoveMentaryNode(faces[f]->m_Nodes[2],organ);
	}
}

void CAppendectomyTraining::RemoveMentaryNode(GFPhysSoftBodyNode * node , MisMedicOrgan_Ordinary * organ)
{
	int curSize = m_SeparatedNodes.size();

	if(curSize && organ == m_Appendix)
	{
		if(m_SeparatedNodes.erase(node))
		{
			float rate = (float)(curSize - 1)/ (float)m_NumSeparatedNode;
			if(rate < m_SeparaeRate)
			{
				m_SeparatedNodes.clear();
				AddOperateItem("SeparateMentary",1.f,true);
				ShowTip("SeparateMentaryFinish");
				SetNextTip("PleaseKnotApendix",2.5f);
			}
			//showDebugInfo("curSize	= ",curSize -1);
			//showDebugInfo("rate		= ",rate);
		}
	}
}

bool CAppendectomyTraining::IsMarkedArea(AreaType type,const Ogre::Vector2& texCoord)
{
	uint32 colorValue = GetPixelFromAreaBuffer(texCoord.x,texCoord.y);

	if(colorValue & 0xff000000)
	{
		if(colorValue & type)
			return true;
	}

	return false;
}

void CAppendectomyTraining::OnHandleEvent(MxEvent* pEvent)
{
	MxToolEvent* pToolEvent = static_cast<MxToolEvent*>(pEvent);
	MisMedicOrgan_Ordinary* pOrgan = NULL;
	TextureBloodTrackEffect *track = NULL;
	CTool* pTool = NULL;
	bool damageAppendix = false;
	bool damageLargeIntestine = false;
	bool damageOtherOrgan = false;
	
	switch(pEvent->m_enmEventType)
	{
	case MxEvent::MXET_OrganBinded:
		{
			MxOrganBindedEvent* pOrganBindedEvent = (MxOrganBindedEvent*)pEvent;
			pOrgan = (MisMedicOrgan_Ordinary*)pOrganBindedEvent->GetOrgan();
			DealWithOrganBinded(pOrgan,pOrganBindedEvent->GetRope());
		}
		break;
	case MxEvent::MXET_Cut:
		pOrgan = (MisMedicOrgan_Ordinary*)pToolEvent->GetOrgan();
		pTool = (CTool*)pToolEvent->m_pTool;
		DealWithCutOrgan(pOrgan,pTool);
		break;
	case MxEvent::MXET_ElecCutKeep:
	case MxEvent::MXET_ElecCoagKeep:
		{
			if(m_CanDealWithAppendixRoot)
			{
				std::set<GFPhysSoftBodyFace*>::iterator itr;
				for(std::size_t f = 0; f < pToolEvent->m_OperatorFaces.size();++f)
				{
					GFPhysSoftBodyFace * pFace = pToolEvent->m_OperatorFaces[f];
					itr = m_NeedBurnedFaceSet.find(pFace);

					if(itr != m_NeedBurnedFaceSet.end())
					{
						m_AlreadyBurnedFaceSet.insert(*itr);
						m_NeedBurnedFaceSet.erase(itr);
						m_BurnFaceTotalTime += pToolEvent->m_DurationTime;
					}
					else
					{
						itr = m_AlreadyBurnedFaceSet.find(pFace);
						if(itr != m_AlreadyBurnedFaceSet.end())
							m_BurnFaceTotalTime += pToolEvent->m_DurationTime;
					}
						
				}

				bool canAddOperateItem = false;
				int curNum = m_NeedBurnedFaceSet.size();

				float ratio = (float)curNum / m_NumNeedBurnFace;
				showDebugInfo("curNum",curNum);
				showDebugInfo("NumNeedBurnFace",m_NumNeedBurnFace);
				if(ratio < 0.75f && m_BurnFaceTotalTime > 5.f || m_BurnFaceTotalTime > 10.f)
				{
					if(!HasOperateItem("DealWithAppendixRoot"))
						AddOperateItem("DealWithAppendixRoot",1.f,true);
					m_CanDealWithAppendixRoot = false;
					m_DealWithAppendixRootFinish = true;

					ShowTip("DealWithAppendixRootFinish");
				}
			}
		}
		break;
	case MxEvent::MXET_ElecCoagStart:
	case MxEvent::MXET_ElecCutStart:
		pOrgan = (MisMedicOrgan_Ordinary*)pToolEvent->GetOrgan();
		if(pOrgan == m_Appendix)
		{
			if(pToolEvent->m_OperatorFaces.size())
			{
				GFPhysSoftBodyFace* pFace = pToolEvent->m_OperatorFaces[0];
				//排除操作的为切割面
				if(pOrgan->GetCrossFaceIndexFromUsrData(pFace) != -1)
					break;

				GFPhysVector3 pt = pFace->m_Nodes[0]->m_UnDeformedPos;
				//排除非正常的套扎区域
				if(PositionInAppendixArea(pt))
				{
					if(PositionBetweenInKnotPlane(pt) == false)
						damageAppendix = true;
				}
				else
					damageLargeIntestine = true;
			}
		}
		else if(pOrgan->GetOrganType() == EDOT_LARGEINTESTINE)
		{
			damageLargeIntestine = true;
		}
		else
		{
			damageOtherOrgan = true;
		}
		break;
	case MxEvent::MXET_PunctureSurface:
		pOrgan = (MisMedicOrgan_Ordinary*)pToolEvent->GetOrgan();
		if(pOrgan == m_Appendix)
		{
			if(pToolEvent->m_OperatorFaces.size())
			{
				GFPhysVector3 pt = pToolEvent->m_OperatorFaces[0]->GetUndeformedMassCenter(pToolEvent->m_weights);
				
				//阑尾区域
				if(PositionInAppendixArea(pt))
				{
					//排除非正常的套扎区域
					if(PositionBetweenInKnotPlane(pt) == false)
						damageAppendix = true;
				}
				else
					damageLargeIntestine = true;
			}
		}
		else if(pOrgan->GetOrganType() == EDOT_LARGEINTESTINE)
		{
			//损伤大肠
			damageLargeIntestine = true;
		}
		else
		{
			damageOtherOrgan = true;
		}
		break;
	case MxEvent::MXET_BleedStart:
		{
			bool canAdd = true;
			track = (TextureBloodTrackEffect*)pToolEvent->m_pUserPoint;
			pOrgan = (MisMedicOrgan_Ordinary*)track->m_OrganIf;

			//对于阑尾需要排除在非正常套扎区域的出血
			if(pOrgan == m_Appendix)
			{
				int faceId = pToolEvent->m_UserData;
				MMO_Face& originFace = pOrgan->GetMMOFace_OriginPart(faceId);
				if(originFace.m_physface)
				{
					GFPhysVector3 pt = originFace.m_physface->GetUndeformedMassCenter(pToolEvent->m_weights);
					if(PositionBetweenInKnotPlane(pt))
						canAdd = false;
				}
			}
			
			if(canAdd)
			{
				++m_BleedTimes;
				ShowTip("DamageOrgan");
			}
		}
		break;
	case MxEvent::MXET_TakeOutOrganWithSpecimenBag:
		{
			int organId = (int)pToolEvent->m_pUserPoint;
			pOrgan = (MisMedicOrgan_Ordinary*)GetOrgan(organId);

			if(pOrgan == m_Appendix && m_TakeOffAppendixFinish == false)
			{
				m_TakeOffAppendixFinish = true;
				AddOperateItem("TakeOffAppendix",1.f,true);
				//RemoveAppendixCutPart();

				Inception::Instance()->EmitShowMovie("TakeOutAppendix");
				ShowTip("TakeOffAppendix");
				SetNextTip("DealWithAppendixRoot",5);
			}
		}
		break;
	}


	//器官被损伤
	float curTime = GetElapsedTime();

	//阑尾
	if(damageAppendix)
	{
		if(curTime - m_LastDamgeAppendixTime > m_MinDamageInterval)
		{
			m_LastDamgeAppendixTime = curTime;
			++m_DamageAppendixTimes;
			AddOperateItem("DamageAppendix",1.f,true,AM_MergeValue);
			ShowTip("DamageOrgan");
		}
	}

	//盲肠
	if(damageLargeIntestine)
	{
		if(curTime - m_LastDamgeLargeIntestineTime > m_MinDamageInterval)
		{
			m_LastDamgeLargeIntestineTime = curTime;
			++m_DamageLargeIntestineTimes;
			AddOperateItem("DamageLargeIntestine",1.f,true,AM_MergeValue);
			ShowTip("DamageOrgan");
		}
	}

	//其他器官
	if(damageOtherOrgan)
	{
		if(curTime - m_LastDamageOtherOrganTime > m_MinDamageInterval)
		{
			m_LastDamageOtherOrganTime = curTime;
			++m_DamageOtherOrganTimes;
			AddOperateItem("DamageOtherOrgan",1.f,false,AM_MergeAll);
			ShowTip("DamageOrgan");
		}
	}
}

void CAppendectomyTraining::DealWithOrganBinded(MisMedicOrgan_Ordinary* pOrgan,MisMedicBindedRope* pRope)
{
	if(m_OrganBindedFinish == false && pOrgan == m_Appendix)	
	{
		std::vector<MisMedicBindedRope::ThreadBindPoint> bindPoints;
		pRope->GetBindPoints(bindPoints);

		//绑到合适的区域
		bool bindFitArea = false;
		for(std::size_t p = 0;p < bindPoints.size();++p)
		{
			MisMedicBindedRope::ThreadBindPoint& bindPoint = bindPoints[p];
			if(bindPoint.m_AttachFace)
			{
				Ogre::Vector2 textureCoord = pOrgan->GetTextureCoord(bindPoint.m_AttachFace,bindPoint.m_Weights);
				if(textureCoord != Ogre::Vector2::ZERO)
				{
					if(IsMarkedArea(AT_KnotArea,textureCoord))
					{
						pRope->SetUserData(1);
						bindFitArea = true;
						++m_ValidKnotTimes;

						if(m_ValidKnotTimes == 1)
						{
							AddOperateItem("KnotVessel",1.f,true);
							SetNextTip("KnotNextPlace",3.f);
						}
						else if(m_ValidKnotTimes == 2)
						{
							AddOperateItem("KnotAppendix",1.f,true);
						}
						ShowTip("KnotSucceed");
						break;
					}
					else
					{
						++m_KnotAppendixAtErrorAreaTimes;
						if(m_KnotAppendixAtErrorAreaTimes < 3)
							AddOperateItem("KnotAppendixAtErrorArea",1.f,false,AM_MergeAll);
					}
				}
			}
		}

		if(m_OrganBindedTimes >= 1)
		{
			std::vector<MisMedicBindedRope::ThreadBindPoint> bindPoints1;
			std::vector<MisMedicBindedRope::ThreadBindPoint> bindPoints2;
			std::vector<MisMedicOrganAttachment*> attachments;

			pOrgan->GetAttachment(MOAType_BindedRope,attachments);

			//判断绳子之间的距离是否合理
			for(int a1 = 0;a1 < attachments.size() - 1;++a1)
			{
				for(std::size_t a2 = a1 + 1;a2 < attachments.size();++a2)
				{
					MisMedicBindedRope* rope1 = (MisMedicBindedRope*)attachments[a1];
					MisMedicBindedRope* rope2 = (MisMedicBindedRope*)attachments[a2];
					//绑到正确区域
					if(rope1->GetUserData() == 1 || rope2->GetUserData() == 1)
					{
						bindPoints1.clear();
						bindPoints2.clear();
						rope1->GetBindPoints(bindPoints1);
						rope2->GetBindPoints(bindPoints2);

						float minDis = 0.5f;
						bool isTooShort = false;
						for(std::size_t p1 = 0;p1 < bindPoints1.size();++p1)
						{
							for(std::size_t p2 = 0;p2 < bindPoints2.size();++p2)
							{
								float dis = bindPoints1[p1].GetPassPointUndeformedPosition().distance(bindPoints2[p2].GetPassPointUndeformedPosition());
								if(dis < minDis)
								{
									isTooShort = true;
									break;
								}
							}

							if(isTooShort)
								break;
						}

						if(!isTooShort)
						{
							//套扎完成
							m_OrganBindedFinish = true;
							//AddOperateItem("KnotFinish",1.f,true);
							//存在合适的套扎距离
							AddOperateItem("CorrectKnotSpacing",1.f,true);
							ShowTip("KnotFinish");
							SetNextTip("PleaseCutAppendix",3.5f);
						}
					}
					else
						continue;
				}
			}
		}

		++m_OrganBindedTimes;
	}
}

//========================================================================================================
void CAppendectomyTraining::DealWithCutOrgan(MisMedicOrgan_Ordinary * pOrgan , CTool * pTool)
{
	if(pOrgan == m_Appendix)
	{
		std::vector<GFPhysSoftBodyFace*> cutCrossFace;
		bool hasError = false;
		bool cutNearEnd = false;
		bool cutFarEnd = false;

		pOrgan->GetLastTimeCutCrossFaces(cutCrossFace);

		//如果存在交叉面，则说明阑尾被剪切，否则剪切的为系模
		if(cutCrossFace.size())
		{
			MisMedicBindedRope* farRope = NULL;
			MisMedicBindedRope* nearRope = NULL;
			std::vector<MisMedicOrganAttachment*> attachments;
			int nAttachment = 0;

			pOrgan->GetAttachment(MOAType_BindedRope,attachments);
			nAttachment = attachments.size();

			if(nAttachment)
			{
				float minRopeDis = 1000;
				float maxRopeDis = 1000;
				float minCfDis,maxCfDis;
				GFPhysVector3 minCfPos,maxCfPos;

				GetDisOfCutCrossFaceToEndPoint(cutCrossFace,minCfDis,maxCfDis,minCfPos,maxCfPos);
				
				if(nAttachment == 1)
				{
					nearRope = static_cast<MisMedicBindedRope*>(attachments[0]);
					GetDisOfRopeToEndPoint(nearRope,minRopeDis,maxRopeDis);

					if(minCfDis <= maxRopeDis && PosAtRopeRight(nearRope,minCfPos))
					{
						hasError = true;
						cutNearEnd = true;
					}
					else
					{
						//剪断远端
						cutFarEnd = true;
					}
				}
				else
				{
					GetEndpointRope(attachments,farRope,nearRope);
					assert(farRope && nearRope);

					//绳子重叠在一起的情况
					if(farRope == nearRope)
					{
						GetDisOfRopeToEndPoint(nearRope,minRopeDis,maxRopeDis);

						if(minCfDis <= maxRopeDis && PosAtRopeRight(nearRope,minCfPos))
						{
							hasError = true;
							cutNearEnd = true;
						}
						else
						{
							//剪断远端
							cutFarEnd = true;
						}
					}
					else
					{
						float minNearRopeDis,maxNearRopeDis;
						float minFarRopeDis,maxFarRopeDis;

						GetDisOfRopeToEndPoint(nearRope,minNearRopeDis,maxNearRopeDis);

						//剪断近端
						if(maxNearRopeDis > minCfDis && PosAtRopeRight(nearRope,minCfPos))
						{
							hasError = true;
							cutNearEnd = true;
						}
						else
						{
							GetDisOfRopeToEndPoint(farRope,minFarRopeDis,maxFarRopeDis);

							//在绳子间剪断
							if(minCfDis < maxFarRopeDis && PosAtRopeRight(farRope,minCfPos))
							{
								//ShowTip("PleaseCutAppendix");
							}
							else	
							{//在远端剪断
								cutFarEnd = true;
							}
						}
					}
				}
			}
			else
			{
				hasError = true;
				//未套扎直接剪阑尾
				if(!m_OrganCutFinish && !HasOperateItem("UnknotButCutAppendix"))
					AddOperateItem("UnknotButCutAppendix",1.f,true);
			}

			//剪切阑尾远端
			if(cutFarEnd)
			{
				MxOperateItem * pOperateItem = NULL;
				if(nAttachment == 1)
				{
					if(!HasOperateItem("CutAppendixAtFarEnd1"))
					{
						AddOperateItem("CutAppendixAtFarEnd1",1.f,false);
					}
				}
				else
				{
					if(!HasOperateItem("CutAppendixAtFarEnd2"))
					{
						AddOperateItem("CutAppendixAtFarEnd2",1.f,false);
					}
				}
			}
				
			if(cutNearEnd)
			{
				hasError = true;
				if(!HasOperateItem("CutAppendixAtNearEnd"))
					AddOperateItem("CutAppendixAtNearEnd",1.f,true);
			}

			if(hasError)
			{
				m_OrganCutFinish = true;
				//TrainingFatalError("CutError");
			}
			else
			{
				if(!m_OrganCutFinish)
				{
					int numOfAllNode = 0;
					int validNumSection = 0;

					for(std::size_t s = 0 ; s < m_Appendix->GetNumSubParts();++s)
					{
						numOfAllNode += m_Appendix->GetSubPart(s)->m_Nodes.size();
					}

					for (std::size_t s = 0; s < m_Appendix->GetNumSubParts(); ++s)
					{
						int n = m_Appendix->GetSubPart(s)->m_Nodes.size();
						float ratio = (float)n / numOfAllNode;

						if(ratio > 0.05f)
							validNumSection++;

						showDebugInfo("subPart : " + Ogre::StringConverter::toString(s) + " ",n);
					}

					if(validNumSection > m_ValidNumSectionOfAppendix)
					{
						//在两个套扎点中间剪断阑尾
						if(!cutFarEnd)
						{
							//m_OrganCutFinish = true;
							AddOperateItem("CutAppendixFinish",1.f,true);
							ShowTip("PleaseTakeOffAppendix");
						}
						else
						{//在远端剪断阑尾
							ShowTip("PleaseTakeOffAppendix");
						}

						m_ValidNumSectionOfAppendix = validNumSection;
						m_OrganCutFinish = true;
						m_CanDealWithAppendixRoot = true;

						SetRootPartNodes(nearRope);
						ClearInvalidateCutFace();

						//test
// 						for(set<GFPhysSoftBodyFace*>::iterator itr = m_NeedBurnedFaceSet.begin();itr != m_NeedBurnedFaceSet.end();++itr)
// 						{
// 							GFPhysSoftBodyFace* pFace = *itr;
// 							m_pDrawObject->AddDynamicPoint(pFace->m_Nodes[0]);
// 							m_pDrawObject->AddDynamicPoint(pFace->m_Nodes[1]);
// 							m_pDrawObject->AddDynamicPoint(pFace->m_Nodes[2]);
// 						}
					}

					UpdateNeedBurnFaces(nearRope,cutCrossFace);
				}
			}
		}
	}
}

void CAppendectomyTraining::GetEndpointRope(std::vector<MisMedicOrganAttachment*>& attachments,MisMedicBindedRope* & farRope,MisMedicBindedRope* & nearRope)
{
	int nRope = attachments.size();

	if(nRope == 0)
	{
		farRope = NULL;
		nearRope = NULL;
	}
	else if(nRope == 1)
	{
		nearRope = static_cast<MisMedicBindedRope*>(attachments[0]);
		farRope = NULL;
	}
	else
	{
		//maxDis & minDis
		std::vector<std::pair<int,int>> disOfMaxAndMin;
		
		disOfMaxAndMin.resize(attachments.size());
		
		for(int r = 0;r < nRope;++r)
		{
			MisMedicBindedRope* pRope = static_cast<MisMedicBindedRope*>(attachments[r]);
			float minDis = 1000;
			float maxDis = 1000;

			GetDisOfRopeToEndPoint(pRope,minDis,maxDis);

			disOfMaxAndMin[r].first = maxDis;
			disOfMaxAndMin[r].second = minDis;
		}

		int minDisIndex = 0;
		int maxDisIndex = 0;
		float minDis =disOfMaxAndMin[0].first;
		float maxDis = disOfMaxAndMin[0].second;

		for(std::size_t r = 1;r < disOfMaxAndMin.size();++r)
		{
			if(maxDis < disOfMaxAndMin[r].first)
			{
				maxDis = disOfMaxAndMin[r].first;
				maxDisIndex = r;
			}

			if(minDis > disOfMaxAndMin[r].second)
			{
				minDis = disOfMaxAndMin[r].second;
				minDisIndex = r;
			}
		}

		farRope = static_cast<MisMedicBindedRope*>(attachments[maxDisIndex]);
		nearRope = static_cast<MisMedicBindedRope*>(attachments[minDisIndex]);
	}
}

void CAppendectomyTraining::GetDisOfRopeToEndPoint(MisMedicBindedRope* pRope,float & minDis,float & maxDis)
{
	static std::vector<MisMedicBindedRope::ThreadBindPoint> bindPoints;
	bindPoints.clear();

	pRope->GetBindPoints(bindPoints);
	if(bindPoints.size())
	{
		minDis = bindPoints[0].GetPassPointUndeformedPosition().distance(m_AppendixEndPoint);
		maxDis = minDis;

		for(std::size_t p = 1;p < bindPoints.size();++p)
		{
			float dis = bindPoints[p].GetPassPointUndeformedPosition().distance(m_AppendixEndPoint);
			if(dis < minDis)
				minDis = dis;
			else if(dis > maxDis)
				maxDis = dis;
		}
	}
	else
	{
		minDis = pRope->GetKnotPoint().GetPassPointUndeformedPosition().distance(m_AppendixEndPoint);
		maxDis = minDis;
	}
}

void CAppendectomyTraining::GetDisOfCutCrossFaceToEndPoint(const std::vector<GFPhysSoftBodyFace*>& cutCrossFace,float& minDis,float& maxDis,GFPhysVector3& minPos,GFPhysVector3& maxPos)
{
	if(cutCrossFace.size())
	{
		minDis = GPVec3ToOgre(cutCrossFace[0]->m_Nodes[0]->m_CurrPosition).distance(m_AppendixEndPoint);
		maxDis = minDis;

		for(std::size_t f = 0;f < cutCrossFace.size();++f)
		{
			for(int n = 0;n < 3;++n)
			{
				float dis = GPVec3ToOgre(cutCrossFace[f]->m_Nodes[n]->m_UnDeformedPos).distance(m_AppendixEndPoint);
				if(dis < minDis)
				{
					minDis = dis;
					minPos = cutCrossFace[f]->m_Nodes[n]->m_UnDeformedPos;
				}
				else if(dis > maxDis)
				{
					maxDis = dis;
					maxPos = cutCrossFace[f]->m_Nodes[n]->m_UnDeformedPos;
				}
			}
		}
	}
}

bool CAppendectomyTraining::PosAtRopeRight(MisMedicBindedRope* pRope,const GFPhysVector3& curPosition)
{
	if(pRope)
	{
		GFPhysVector3 planNormal;
		GFPhysVector3 planPoint;
		
		if(pRope->GetApproximateLoopPlane_MaterialSpace(planNormal,planPoint))
		{
			std::vector<MisMedicBindedRope::ThreadBindPoint> bindPoints;
			pRope->GetBindPoints(bindPoints);
			if(bindPoints.size())
			{
				Ogre::Vector3 newPlanPoint(0,0,0);

				for(std::size_t p = 0;p < bindPoints.size();++p)
					newPlanPoint += bindPoints[p].GetPassPointUndeformedPosition();

				if(bindPoints.size() > 1)
					newPlanPoint /= bindPoints.size();
				planPoint = OgreToGPVec3(newPlanPoint);
			}

			GFPhysVector3 appendixEndPoint(m_AppendixEndPoint.x,m_AppendixEndPoint.y,m_AppendixEndPoint.z);
			GFPhysVector3 rightDir = appendixEndPoint - planPoint;
			GFPhysVector3 curDir = curPosition - planPoint;

			if(planNormal.Dot(rightDir) < 0)
				planNormal *= -1;

			if(curDir.Dot(planNormal) > 0)
			{
				return true;
			}
		}
	}

	return false;
}

void CAppendectomyTraining::UpdateNeedBurnFaces(MisMedicBindedRope* pNearRope,const std::vector<GFPhysSoftBodyFace*>& cutCrossFace)
{
	for(std::size_t f = 0;f < cutCrossFace.size();++f)
	{
		GFPhysSoftBodyFace* pFace = (GFPhysSoftBodyFace*)cutCrossFace[f];

		for(int n = 0;n < 3;++n)
		{
			//前面已经排除了系模
			if(PosAtRopeRight(pNearRope,pFace->m_Nodes[n]->m_UnDeformedPos))
				break;

			if(n == 2)
				m_NeedBurnedFaceSet.insert(pFace);
		}
	}

	m_NumNeedBurnFace = m_NeedBurnedFaceSet.size();
}

void CAppendectomyTraining::SetRootPartNodes(MisMedicBindedRope* pNearRope)
{
	const MisMedicBindedRope::ThreadBindPoint& bindPoint = pNearRope->GetBindPoint(0);

	for(std::size_t s = 0;s < m_Appendix->GetNumSubParts();++s)
	{
		m_RootPartNodes.clear();

		GFPhysSubConnectPart* pSubPart = m_Appendix->GetSubPart(s);
		for(std::size_t n = 0; n < pSubPart->m_Nodes.size();++n)
			m_RootPartNodes.insert(pSubPart->m_Nodes[n]);

		for(int n = 0;n < 3;++n)
		{
			GFPhysSoftBodyNode* pNode0 = bindPoint.m_AttachFace->m_Nodes[0];

			if(m_RootPartNodes.find(pNode0) != m_RootPartNodes.end())
				return;
		}
	}
}

void CAppendectomyTraining::ClearInvalidateCutFace()
{
	for(set<GFPhysSoftBodyFace*>::iterator itr = m_NeedBurnedFaceSet.begin();itr != m_NeedBurnedFaceSet.end();)
	{
		GFPhysSoftBodyNode* pNode = (*itr)->m_Nodes[0];

		if(m_RootPartNodes.find(pNode) == m_RootPartNodes.end())
			itr = m_NeedBurnedFaceSet.erase(itr);
		else
			++itr;
	}
}

int CAppendectomyTraining::GetNumberOfActiveBleedPoint()
{
	int n = 0;
	std::vector<MisMedicOrganInterface*> organs;

	GetAllOrgan(organs);
	//获取未处理的流血点
	for(std::size_t i = 0;i < organs.size();++i)
		n += organs[i]->GetNumberOfActiveBleedPoint();

	return n;
}

void CAppendectomyTraining::OnSaveTrainingReport()
{
	float time = m_pToolsMgr->GetToolSuctionTime() + m_pToolsMgr->GetToolIrrigationTime();
	if(time > FLT_EPSILON)
		AddOperateItem("SuctionAndIrrigateOrgan",1.f,false);
	
	//未处理阑尾末端
	if(m_DealWithAppendixRootFinish == false)
		AddOperateItem("UnhandleAppendixRootFinish",1.f,false);

	AddOperateItem("BleedTimes",m_BleedTimes,false);

	int n = GetNumberOfActiveBleedPoint();
	for(int i = 0 ;i < n;++i)
		AddOperateItem("UnhandleBleedPointTimes",1,false,MisNewTraining::AM_MergeAll);

	
	__super::OnSaveTrainingReport();
}

bool CAppendectomyTraining::PositionBetweenInKnotPlane(const GFPhysVector3& pos)
{
	Ogre::Vector3 curPos = GPVec3ToOgre(pos);
	float d1 = m_AppendixKonteLeftPlaneNormal.dotProduct(curPos - m_AppendixKnoteLeftPlanePos);
	float d2 = m_AppendixKnoteRightPlaneNormal.dotProduct(curPos - m_AppendixKnoteRightPlanePos);

	if(d1 > 0 && d2 < 0)
		return true;

	return false;
}

bool CAppendectomyTraining::PositionInAppendixArea(const GFPhysVector3& pos)
{
	Ogre::Vector3 curPos = GPVec3ToOgre(pos);

	float d1 = m_AppendixRightSideSplitePlaneNormal.dotProduct(curPos - m_AppendixRightSideSplitePlanePos);
	float d2 = m_AppendixBottomSplitePlaneNormal.dotProduct(curPos - m_AppendixBottomSplitePlanePos);

	if(d1 > 0 && d2 > 0)
		return true;

	return false;
}
void CAppendectomyTraining::RemapAppendixToMorphPos(std::string & name)
{
	try
	{
		Ogre::MeshPtr meshptr = Ogre::MeshManager::getSingleton().load(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		m_Appendix->MapSurfaceToMorphedMesh(meshptr);
	}
	catch (Ogre::Exception &ex)
	{
		
	}

}