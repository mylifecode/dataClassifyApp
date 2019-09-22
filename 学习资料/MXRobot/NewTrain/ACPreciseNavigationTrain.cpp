#include "ACPreciseNavigationTrain.h"
#include "MisMedicOrganOrdinary.h"
#include "instruments\ElectricNeedle.h"
#include "SYScoreTableManager.h"
#include "Inception.h"
ACPreciseNavigationTrain::PBall::PBall(MisMedicRigidPrimtive * prim , int Color) : m_Primitive(prim)
{
	m_Color = Color;
	m_InitPos = m_Primitive->m_body->GetCenterOfMassPosition();
	GFPhysSphereShape * shape = dynamic_cast<GFPhysSphereShape*>(m_Primitive->m_body->GetCollisionShape());
	if (shape)
	{
		m_Radius = shape->GetRadius();
	}
	else
	{
		m_Radius = 0;
	}
	ExtractMesh();

	m_HasBeenTouched = false;
	m_HasTouchedByWrongTool = false;
	m_IsTouchOrderWrong = false;

	//m_HasTouchedInWrongPlace = false;
	//m_IsDeviateFromeRightPlace = false;
	//m_HasTouchedRightPlace = false;

	m_HasLocatedBeFailed = false;
	m_HasDeviateInProcess = false;
	m_HasBeenLocated  = false;

	m_LocatedKeepTime = 0;
	m_State = BS_NONE;
}

void ACPreciseNavigationTrain::PBall::ExtractMesh()
{
	m_Primitive->ExtractOgreMeshInfo(m_Primitive->m_entity->getMesh(), m_Vertex, m_TexCoords, m_Indices);
}

int ACPreciseNavigationTrain::PBall::CheckSegmentIntersect(const GFPhysVector3 & segStart, const GFPhysVector3 & segEnd)
{
	//approximate check first
	GFPhysVector3 pos = m_Primitive->m_body->GetCenterOfMassPosition();
	
	GFPhysVector3 ctPoint = CloasetPtToSegment(pos , segStart, segEnd);

	if ((ctPoint-pos).Length() > m_Radius)
		return 0;

    //
	Ogre::Matrix4 transMat = m_Primitive->m_entity->_getParentNodeFullTransform();
	transMat = transMat.inverse();
	
	Ogre::Vector4 t0 = transMat*Ogre::Vector4(segStart.m_x, segStart.m_y, segStart.m_z , 1);
	Ogre::Vector4 t1 = transMat*Ogre::Vector4(segEnd.m_x, segEnd.m_y, segEnd.m_z, 1);

	GFPhysVector3 localSegStart(t0.x , t0.y ,t0.z);
	GFPhysVector3 localSegEnd(t1.x, t1.y, t1.z);

	int minFaceIndex = -1;
	
	float minWeight = FLT_MAX;

	Real minTriWegihts[3];

	for (int c = 0; c < m_Indices.size(); c += 3)
	{
		int i0 = m_Indices[c];
		int i1 = m_Indices[c+1];
		int i2 = m_Indices[c+2];

		Real Rayweight;
		GFPhysVector3  intersectpt;
		Real triangleWeight[3];

		bool intersect = LineIntersectTriangle(OgreToGPVec3(m_Vertex[i0]),
			                                   OgreToGPVec3(m_Vertex[i1]), 
			                                   OgreToGPVec3(m_Vertex[i2]), 
											   localSegStart,
											   localSegEnd,
								               Rayweight, intersectpt, triangleWeight);

		if (intersect && Rayweight >= 0 && Rayweight <= 1)
		{
			if (Rayweight < minWeight)
			{
				minWeight = Rayweight;
				minFaceIndex = c;
				minTriWegihts[0] = triangleWeight[0];
				minTriWegihts[1] = triangleWeight[1];
				minTriWegihts[2] = triangleWeight[2];

			}
		}
	}

	if (minFaceIndex >= 0)
	{
		int i0 = m_Indices[minFaceIndex];
		int i1 = m_Indices[minFaceIndex + 1];
		int i2 = m_Indices[minFaceIndex + 2];

		Ogre::Vector2 texcoord = m_TexCoords[i0] * minTriWegihts[0] + m_TexCoords[i1] * minTriWegihts[1] + m_TexCoords[i2] * minTriWegihts[2];

		float delta = (texcoord - Ogre::Vector2(0.5f, 0.5f)).length();
		if (delta < 0.15f)
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}

	return 0;
}
//=============================================================================================
ACPreciseNavigationTrain::ACPreciseNavigationTrain(void)
{	
	m_CurrDstBallIndex = 0;
}
//=============================================================================================
ACPreciseNavigationTrain::~ACPreciseNavigationTrain(void)
{
	Ogre::MaterialManager::getSingleton().remove(m_RedCrePtr->getName());
	Ogre::MaterialManager::getSingleton().remove(m_BlueCrePtr->getName());
	m_RedCrePtr.setNull();
	m_BlueCrePtr.setNull();

}
//======================================================================================================================
bool ACPreciseNavigationTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig, pToolConfig);

	MisMedicOrgan_Ordinary * domeActive = dynamic_cast<MisMedicOrgan_Ordinary*>(GetOrgan(100));
	domeActive->m_physbody->SetCollisionCategory(1 << 17);

	//extract all ball
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while (itor != m_DynObjMap.end())
	{
		MisMedicRigidPrimtive * rigidBall = dynamic_cast<MisMedicRigidPrimtive*>(itor->second);
		
		if (rigidBall)
		{
			std::string name = rigidBall->getName();

			rigidBall->m_body->SetCollisionMask(~(1 << 17));
			rigidBall->m_body->SetMaxStepLinearDisplacment(0.1f);

			if (name.find("1") != std::string::npos || name.find("3") != std::string::npos || name.find("5") != std::string::npos)
			    m_pballs.push_back(ACPreciseNavigationTrain::PBall(rigidBall , 0));
			else
				m_pballs.push_back(ACPreciseNavigationTrain::PBall(rigidBall, 1));
		}
		itor++;
	}

	for (int c = 0; c < m_pballs.size(); c++)
	{
		ACPreciseNavigationTrain::PBall & pball = m_pballs[c];
		GFPhysVector3 center = pball.m_Primitive->m_body->GetWorldTransform().GetOrigin();

		float minDist = FLT_MAX;
		GFPhysSoftBodyFace * minFace = 0;
		//closet point need optimize brute method
		for (int f = 0; f < domeActive->m_physbody->GetNumFace(); f++)
		{
			 GFPhysSoftBodyFace * face =  domeActive->m_physbody->GetFaceAtIndex(f);
			 
			 GFPhysVector3 cpoint = ClosestPtPointTriangle(center,
				                                           face->m_Nodes[0]->m_UnDeformedPos,
														   face->m_Nodes[1]->m_UnDeformedPos,
														   face->m_Nodes[2]->m_UnDeformedPos);
			 float dist = (cpoint - center).Length();
			 if (dist < minDist)
			 {
				 minDist = dist;
				 minFace = face;
			 }
		}

		//build anchor;
		for (int n = 0; n < 3; n++)
		{
			 GFPhysSoftRigidAnchor * srAnchor = new GFPhysSoftRigidAnchor(minFace->m_Nodes[n], pball.m_Primitive->m_body);
			 srAnchor->SetStiffness(0.9f);
			 PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(srAnchor);
		}
	}

	//load need material
	Ogre::MaterialPtr creOrigin = Ogre::MaterialManager::getSingleton().load("cre", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).dynamicCast<Ogre::Material>();
	m_RedCrePtr  = creOrigin->clone("Cre_Red");
	m_BlueCrePtr = creOrigin->clone("Cre_Blue");
	
	Ogre::TexturePtr redTex  = Ogre::TextureManager::getSingleton().load("red.png",  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::TexturePtr blueTex = Ogre::TextureManager::getSingleton().load("blue.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	ApplyTextureToMaterial(m_RedCrePtr , redTex,  "BaseMap");
	ApplyTextureToMaterial(m_BlueCrePtr, blueTex, "BaseMap");

	Inception::Instance()->EmitShowTrainProgressBar();
	Inception::Instance()->EmitShowTrainCompletness(0, 6);

	return result;
}
//======================================================================================================================
bool ACPreciseNavigationTrain::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);

	CElectricNeedle * rNeedle = dynamic_cast<CElectricNeedle*>(GetRightTool());
	CElectricNeedle * lNeedle = dynamic_cast<CElectricNeedle*>(GetLeftTool());

	int lToolTouchedIndex = -1;
	
	int rToolTouchedIndex = -1;
	
	int lToolGoodIndex  = -1;

	int rToolGoodIndex = -1;

	//only check none succeed ball
	for (int c = m_CurrDstBallIndex ; c < m_pballs.size(); c++)
	{
		if (rNeedle)
		{
			GFPhysVector3 nRoot = rNeedle->m_NeedleRoot_WCS;
			GFPhysVector3 nHead = rNeedle->m_NeedleHead_WCS;// +(rNeedle->m_NeedleHead_WCS - rNeedle->m_NeedleRoot_WCS).Normalized() * 0.05f;
			int result = m_pballs[c].CheckSegmentIntersect(nRoot, nHead);
			if (result)
			{
				rToolTouchedIndex = c;
				if (result == 2)
				{
					rToolGoodIndex = c;
					break;
				}
			}
		}

		if (lNeedle)
		{
			GFPhysVector3 nRoot = lNeedle->m_NeedleRoot_WCS;
			GFPhysVector3 nHead = lNeedle->m_NeedleHead_WCS + (lNeedle->m_NeedleHead_WCS - lNeedle->m_NeedleRoot_WCS).Normalized() * 0.05f;
			int result = m_pballs[c].CheckSegmentIntersect(nRoot, nHead);
			if (result)
			{
				lToolTouchedIndex = c;
				if (result == 2)
				{
					lToolGoodIndex = c;
					break;
				}
			}
		}
	}
	int currBallColor = GetCurrentBallColor();

	bool targetOk = false;

	
	if (lToolTouchedIndex == m_CurrDstBallIndex || rToolTouchedIndex == m_CurrDstBallIndex)
	{
		m_pballs[m_CurrDstBallIndex].m_HasBeenTouched = true;
	}

	
	if (lToolTouchedIndex == m_CurrDstBallIndex && (GetLeftToolColor() != currBallColor))//器械选择错误
	{
		m_pballs[m_CurrDstBallIndex].m_HasTouchedByWrongTool = true;
		CTipMgr::Instance()->ShowTip("ToolError");
	}
	else if (rToolTouchedIndex == m_CurrDstBallIndex && (GetRightToolColor() != currBallColor))
	{
		m_pballs[m_CurrDstBallIndex].m_HasTouchedByWrongTool = true;
		CTipMgr::Instance()->ShowTip("ToolError");
	}
	else if (lToolTouchedIndex >= 0 && (lToolTouchedIndex > m_CurrDstBallIndex))//操作顺序错误
	{
		m_pballs[m_CurrDstBallIndex].m_IsTouchOrderWrong = true;
		CTipMgr::Instance()->ShowTip("OrderError");
	}
	else if (rToolTouchedIndex >= 0 && (rToolTouchedIndex > m_CurrDstBallIndex))
	{
		m_pballs[m_CurrDstBallIndex].m_IsTouchOrderWrong = true;
		CTipMgr::Instance()->ShowTip("OrderError");
	}
	else
	{
		//左手器械触碰到了当前操作小球
		bool located = false;
		if (lToolGoodIndex == m_CurrDstBallIndex && (GetLeftToolColor() == currBallColor))
		{
			located = true;
		}
		if (rToolGoodIndex == m_CurrDstBallIndex && (GetRightToolColor() == currBallColor))
		{
			located = true;
		}

		bool touched = (lToolTouchedIndex == m_CurrDstBallIndex || rToolTouchedIndex == m_CurrDstBallIndex);


		if (m_pballs[m_CurrDstBallIndex].m_State == BS_NONE)
		{
			if ((!located) && touched)
			{
				m_pballs[m_CurrDstBallIndex].m_HasLocatedBeFailed = true;
				CTipMgr::Instance()->ShowTip("TouchPlaceError");
			}
			else if (located)
			{
				m_pballs[m_CurrDstBallIndex].m_HasBeenLocated = true;
				m_pballs[m_CurrDstBallIndex].m_State = BS_LOCATED;
				CTipMgr::Instance()->ShowTip("KeepTouch");
			}
		}
		if (m_pballs[m_CurrDstBallIndex].m_State == BS_LOCATED)
		{
			if (located)
			{
				m_pballs[m_CurrDstBallIndex].m_LocatedKeepTime += dt;
			}
			else if (touched)
			{
				m_pballs[m_CurrDstBallIndex].m_LocatedKeepTime += dt;
				m_pballs[m_CurrDstBallIndex].m_HasDeviateInProcess = true;
			}
			else
			{//new around clear time
				m_pballs[m_CurrDstBallIndex].m_LocatedKeepTime = 0;
				m_pballs[m_CurrDstBallIndex].m_HasDeviateInProcess = false;
				m_pballs[m_CurrDstBallIndex].m_State = BS_NONE;
				CTipMgr::Instance()->ShowTip("TrainingIntro");
			}

			if (m_pballs[m_CurrDstBallIndex].m_LocatedKeepTime > 2.0f)
			{
				m_pballs[m_CurrDstBallIndex].m_State = BS_SUCCEED;
				m_pballs[m_CurrDstBallIndex].m_Primitive->m_entity->setMaterialName("playballfinish");
				m_CurrDstBallIndex++;


				Inception::Instance()->EmitShowTrainCompletness(m_CurrDstBallIndex, 6);

				if (m_CurrDstBallIndex < m_pballs.size())
					CTipMgr::Instance()->ShowTip("SuccedNext");
				else
					TrainingFinish();
			}
		}
	}

	return result;
}
//=======================================================================================================================================
void ACPreciseNavigationTrain::OnToolCreated(ITool * tool, int side)
{
	CElectricNeedle * rNeedle = dynamic_cast<CElectricNeedle*>(tool);
	if (rNeedle)
	{
		Ogre::Node::ChildNodeIterator iter = rNeedle->GetKernelNode()->getChildIterator();
		while (iter.hasMoreElements())
		{
			Ogre::SceneNode * dstNode = dynamic_cast<Ogre::SceneNode *>(iter.getNext());

			std::string name = dstNode->getName();
			if (name.find("T-Needle Electrode02") != string::npos)
			{
				Ogre::Entity * creEntity = dynamic_cast<Ogre::Entity *>(dstNode->getAttachedObject(0));
				if (side == 0)
				    creEntity->setMaterial(m_RedCrePtr);
				else
					creEntity->setMaterial(m_BlueCrePtr);
			}
		
		}
	}
}

//======================================================================================================================
SYScoreTable* ACPreciseNavigationTrain::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01100901");
}
QString GenSubCode(int index)
{
	if (index < 10)
	{
		return QString("0") + QString::number(index);
	}
	else
	{
		return QString::number(index);
	}
}
//=======================================================================================================================
void ACPreciseNavigationTrain::OnSaveTrainingReport()
{
	int perfectOperatBall = 0;

	for (int c = 0; c < m_CurrDstBallIndex; c++)
	{
		bool isperfect = true;

		//器械选择是否正确
		if (m_pballs[c].m_HasTouchedByWrongTool)
		{
			QString fullcode = "009" + GenSubCode(c * 2 + 1) + "02601";
			AddScoreItemDetail(fullcode, 0.0f);
			isperfect = false;
		}
		else
		{
			QString fullcode = "009" + GenSubCode(c * 2 + 1) + "02600";
			AddScoreItemDetail(fullcode, 0.0f);
		}

		//操作顺序
		if (m_pballs[c].m_IsTouchOrderWrong)
		{
			QString fullcode = "009" + GenSubCode(c * 2 + 1) + "02701";
			AddScoreItemDetail(fullcode, 0.0f);
			isperfect = false;
		}
		else
		{
			QString fullcode = "009" + GenSubCode(c * 2 + 1) + "02700";
			AddScoreItemDetail(fullcode, 0.0f);
		}

		//位置定位
		if (m_pballs[c].m_HasBeenLocated == false)
		{
			QString fullcode = "009" + GenSubCode((c + 1) * 2) + "02219";
			AddScoreItemDetail(fullcode, 0.0f);
			isperfect = false;
		}
		else if (m_pballs[c].m_HasLocatedBeFailed == false)
		{
			QString fullcode = "009" + GenSubCode((c + 1) * 2) + "02210";
			AddScoreItemDetail(fullcode, 0.0f);
		}
		else
		{
			QString fullcode = "009" + GenSubCode((c + 1) * 2) + "02211";
			AddScoreItemDetail(fullcode, 0.0f);
		}

		//器械保持
		if (m_pballs[c].m_State == BS_SUCCEED)
		{ 
			if (m_pballs[c].m_HasDeviateInProcess == false)
			{
				QString fullcode = "009" + GenSubCode((c + 1) * 2) + "02810";
				AddScoreItemDetail(fullcode, 0.0f);
			}
			else
			{
				QString fullcode = "009" + GenSubCode((c + 1) * 2) + "02811";
				AddScoreItemDetail(fullcode, 0.0f);
				isperfect = false;
			}
		}
		else
		{
			isperfect = false;
		}

		if (isperfect)
		{
			perfectOperatBall++;
		}
	}

	if (perfectOperatBall >= 5)
	{
		AddScoreItemDetail("0091300300", 0.0f);
	}
	else if (perfectOperatBall >= 3)
	{
		AddScoreItemDetail("0091300301", 0.0f);
	}

	

	float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
	float rightToolSpeed = m_pToolsMgr->GetRightToolMovedSpeed();

	if (m_pToolsMgr->GetLeftToolMovedDistance() > 10 || m_pToolsMgr->GetRightToolMovedDistance() > 10)
	{
		if (leftToolSpeed > 10 || rightToolSpeed > 10)
		{
			AddScoreItemDetail("0091500802", 0);//移动速度过快，有安全隐患
		}
		else if (leftToolSpeed < 5 && rightToolSpeed < 5)
		{
			AddScoreItemDetail("0091500800", 0);//移动平稳流畅
		}
		else
		{
			AddScoreItemDetail("0091500801", 0);//移动速度较快
		}
	}
	//
	if (m_CurrDstBallIndex >= m_pballs.size())
	{
		AddScoreItemDetail("0091402401", 0.0f);

		int TimeUsed = GetElapsedTime();
		if (TimeUsed < 60)
			AddScoreItemDetail("0091600500", 0);//1分钟内完成所有操作
		else if (TimeUsed < 90)
			AddScoreItemDetail("0091600501", 0);//在2分钟~3分钟内完成所有操作
		else
			AddScoreItemDetail("0091600502", 0);//完成所有规定操作时超过了3分钟
	}
	__super::OnSaveTrainingReport();
}