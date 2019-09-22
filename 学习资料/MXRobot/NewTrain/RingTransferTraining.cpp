#include "RingTransferTraining.h"
#include "MisMedicOrganOrdinary.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "OgreAxisAlignedBox.h"
#include "Instruments/Tool.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "Instruments/GraspingForceps.h"
#include "MisMedicObjectUnion.h"
#include "ACTubeShapeObject.h"
#include "SYScoreTableManager.h"
#include <fstream>

CRingTransferTrain::TubeInScene::TubeInScene(ACTubeShapeObject * tube, Ogre::Entity * pillar, int side, int index)
{
	m_CouplePillar = pillar;
	m_TubeObj = tube;
	m_isPassedByTwoTool = false;
	m_Side = side;
	m_Index = index;
	m_TubeOriginColor[0] = tube->GetOwnerMaterialPtr()->getTechnique(0)->getPass(0)->getAmbient();
	m_TubeOriginColor[1]   = tube->GetOwnerMaterialPtr()->getTechnique(0)->getPass(0)->getDiffuse();
	
	m_PillarOriginColor[0] = m_CouplePillar->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getAmbient();
	m_PillarOriginColor[1] = m_CouplePillar->getSubEntity(0)->getMaterial()->getTechnique(0)->getPass(0)->getDiffuse();
	
	m_TubeCurrColor[0] = m_TubeOriginColor[0];
	m_TubeCurrColor[1] = m_TubeOriginColor[1];

	m_PillarCurrColor[0] = m_PillarOriginColor[0];
	m_PillarCurrColor[1] = m_PillarOriginColor[1];

	m_IsPutSucceed  = false;
	//m_HasBeenPassed = false;

	m_State = RS_WAITBECLAMP;
	m_TouchGroundWhenPass = false;
	m_HasPassSucced = false;
	m_PutFaileNum = 0;
	m_FirstClampToolSide = -1;
}
void CRingTransferTrain::TubeInScene::ResetToOriginColor()
{
	m_TubeCurrColor[0] = m_TubeOriginColor[0];
	m_TubeCurrColor[1] = m_TubeOriginColor[1];

	m_PillarCurrColor[0] = m_PillarOriginColor[0];
	m_PillarCurrColor[1] = m_PillarOriginColor[1];
}
//=============================================================================================
CRingTransferTrain::CRingTransferTrain(void)
{
	
}
//=============================================================================================
CRingTransferTrain::~CRingTransferTrain(void)
{
	
}
//======================================================================================================================
bool CRingTransferTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);
	
	GFPhysVector3 tubeCenters[4];
	tubeCenters[0] = GFPhysVector3(-5.954f, 0.151f, 1.389f);
	tubeCenters[1] = GFPhysVector3(-3.033f, 0.151f, -1.389f);
	
	tubeCenters[2] = GFPhysVector3(5.954f, 0.151f,  1.389f);
	tubeCenters[3] = GFPhysVector3(3.033f, 0.151f, -1.389f);

	int side[4]  = { 0, 0, 1, 1 };
	int index[4] = {0 , 3 , 0 , 3};
#if(1)
	for (int t = 0; t < 4; t++)
	{
		ACTubeShapeObject * tubeShape = CreateSoftTube(10000+t , tubeCenters[t], 0.8f, 0.15f);
		Ogre::String pillarName;
		int pillarIndex = (index[t] == 0 ? 2 : 1);
		if (side[t] == 0)
		{
			pillarName = "zhuB" + Ogre::StringConverter::toString(pillarIndex) + "$";//to do use more good find method
	    }
		else
		{
			pillarName = "zhuA" + Ogre::StringConverter::toString(pillarIndex) + "$";

		}
		Ogre::Entity * PillarEntity = 0;

		Ogre::Node::ChildNodeIterator iter = m_pOms->GetRootNode()->getChildIterator();
		while (iter.hasMoreElements())
		{
			Ogre::Node * node = iter.getNext();
			std::string name = node->getName();
			if (name.find(pillarName) != string::npos)
			{
				PillarEntity = MXOgre_SCENEMANAGER->getEntity(name);
				break;
			}
		}
		Ogre::MaterialPtr mat = PillarEntity->getSubEntity(0)->getMaterial();
		Ogre::MaterialPtr clonedmat = mat->clone("OwnMat" + pillarName);
		PillarEntity->getSubEntity(0)->setMaterial(clonedmat);

		CRingTransferTrain::TubeInScene tubeins(tubeShape, PillarEntity, side[t], index[t]);
		
		m_Tubes.push_back(tubeins);
	}
#else
	m_TestTube = new ACTubeShapeObject(0, this);
	m_TestTube->CreateToturs(MXOgre_SCENEMANAGER,center, GFPhysVector3(0, 1, 0), 0.8f);
#endif
	
	
	return result; 
}
//======================================================================================================================
bool CRingTransferTrain::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);

	int putSuccedCount = 0;

	//int lastSuccedCount = 0;

	for (int c = 0; c < m_Tubes.size(); c++)
	{
		float touchGroundPerct = TubeTouchGroundPercent(m_Tubes[c].m_TubeObj);
		bool  isClampByLTool = m_Tubes[c].m_TubeObj->IsClampedByLTool();
		bool  isClampByRTool = m_Tubes[c].m_TubeObj->IsClampedByRTool();

		if (m_Tubes[c].m_State == RS_FINISHPUT)
		{
			continue;
		}
		else if (m_Tubes[c].m_State == RS_WAITBECLAMP)
		{
			if (/*m_Tubes[c].m_Side == 0 && */isClampByLTool)//left
			{
				m_Tubes[c].m_State = RS_WAITFORPASS;
				m_Tubes[c].m_FirstClampToolSide = 0;
				CTipMgr::Instance()->ShowTip("PassLeftRightNeedded");
			}
			else if (/*m_Tubes[c].m_Side == 1 &&*/ isClampByRTool)//right
			{
				m_Tubes[c].m_State = RS_WAITFORPASS;
				m_Tubes[c].m_FirstClampToolSide = 1;
				CTipMgr::Instance()->ShowTip("PassLeftRightNeedded");
			}
		}
		else if (m_Tubes[c].m_State == RS_WAITFORPASS)
		{
			if (isClampByLTool && isClampByRTool)
			{
				m_Tubes[c].m_State = RS_INPASS;
			}
			else if (isClampByLTool || isClampByRTool)
			{
				CTipMgr::Instance()->ShowTip("PassLeftRightNeedded");
			}
		}
		else if (m_Tubes[c].m_State == RS_INPASS)
		{
			if (isClampByRTool == false && isClampByLTool == false)//没有器械夹着属于掉落
			{
				m_Tubes[c].m_State = RS_WAITBECLAMP;
				m_Tubes[c].m_FirstClampToolSide = -1;

				m_Tubes[c].ResetToOriginColor();
				m_Tubes[c].m_PutFaileNum++;
			}
			else if ((isClampByLTool == false && m_Tubes[c].m_FirstClampToolSide == 0)
			      || (isClampByRTool == false && m_Tubes[c].m_FirstClampToolSide == 1))
			{
				m_Tubes[c].m_State = RS_FINISHPASS;

				m_Tubes[c].m_HasPassSucced = true;

				m_Tubes[c].m_TubeCurrColor[0] = m_Tubes[c].m_TubeCurrColor[1] = Ogre::ColourValue::Blue;

				m_Tubes[c].m_PillarCurrColor[0] = m_Tubes[c].m_PillarCurrColor[1] = Ogre::ColourValue::Blue;

				if (touchGroundPerct > 0)//传递过程中碰到地面记录下来
				{
					m_Tubes[c].m_TouchGroundWhenPass = true;
				}

				CTipMgr::Instance()->ShowTip("TransferRequired");
			}
		}
		else if (m_Tubes[c].m_State == RS_FINISHPASS)
		{
			if (touchGroundPerct > 0)
			{
				bool succeed = CheckTubeSurroundPillar(m_Tubes[c].m_TubeObj, m_Tubes[c].m_CouplePillar);

				if (succeed)
				{
					m_Tubes[c].m_State = RS_FINISHPUT;
					putSuccedCount++;
				}
				else
				{
					if (isClampByRTool == false && isClampByLTool == false)//没有器械夹着属于掉落
					{
						m_Tubes[c].m_State = RS_WAITBECLAMP;
						m_Tubes[c].m_FirstClampToolSide = -1;

						m_Tubes[c].ResetToOriginColor();
						m_Tubes[c].m_PutFaileNum++;
					}
					/*else if (touchGroundPerct > 0.5f)
					{
						m_Tubes[c].m_State = RS_WAITFORPASS;
						m_Tubes[c].ResetToOriginColor();
						m_Tubes[c].m_PutFaileNum++;
					}*/
					else
					{

					}
				}
			}
		}
	}

	int numFinishCount = 0;
	for (int c = 0; c < (int)m_Tubes.size(); c++)
	{
		if (m_Tubes[c].m_State == RS_FINISHPUT)
			numFinishCount++;
	}

	if (numFinishCount == m_Tubes.size())
	{
		CTipMgr::Instance()->ShowTip("TrainingFinish");
		TrainingFinish();
	}
	else if (putSuccedCount > 0)//something new put succeed
	{
		CTipMgr::Instance()->ShowTip("TransferSucceed");
	}
	return result;
}
//=============================================================================================
bool CRingTransferTrain::BeginRendOneFrame(float timeelpsed)
{
	MisNewTraining::BeginRendOneFrame(timeelpsed);

	for (int c = 0; c < m_Tubes.size(); c++)
	{
		UpdateTubeColor(m_Tubes[c].m_TubeObj, 
			            m_Tubes[c].m_TubeCurrColor[0],
						m_Tubes[c].m_TubeCurrColor[1]);

		UpdatePillarColor(m_Tubes[c].m_CouplePillar , 
			              m_Tubes[c].m_PillarCurrColor[0],
						  m_Tubes[c].m_PillarCurrColor[1]);

		m_Tubes[c].m_TubeObj->UpdateScene(timeelpsed, m_pLargeCamera);
	}
	return true;
}
//=============================================================================================================================
void CRingTransferTrain::UpdateTubeColor(ACTubeShapeObject * tubeObj,
	                                     const Ogre::ColourValue & ambient,
	                                     const Ogre::ColourValue & diffuse)
{
	 Ogre::MaterialPtr matPtr = tubeObj->GetOwnerMaterialPtr();
	 Ogre::Pass * pass = matPtr->getTechnique(0)->getPass(0);
	 pass->setDiffuse(diffuse);
	 pass->setAmbient(ambient);
}
//========================================================================================================
void CRingTransferTrain::UpdatePillarColor(Ogre::Entity * pillar,
	                                       const Ogre::ColourValue & ambient,
	                                       const Ogre::ColourValue & diffuse)
{
	Ogre::MaterialPtr matPtr = pillar->getSubEntity(0)->getMaterial();
	Ogre::Pass * pass = matPtr->getTechnique(0)->getPass(0);
	pass->setDiffuse(diffuse);
	pass->setAmbient(ambient);
}
bool CRingTransferTrain::CheckTubeSurroundPillar(ACTubeShapeObject * tubeObj, Ogre::Entity * pillar)
{
	GFPhysVector3 pillarPoints[2];//point 0 near bottom

	bool PutSucced = false;

	Ogre::AxisAlignedBox aabb = pillar->getWorldBoundingBox(true);

	Ogre::Vector3 center   = aabb.getCenter();
	Ogre::Vector3 halfSize = aabb.getHalfSize();
	pillarPoints[0] = OgreToGPVec3(center) - GFPhysVector3(0, halfSize.y, 0);
	pillarPoints[1] = OgreToGPVec3(center) + GFPhysVector3(0, halfSize.y, 0);

	GFPhysVector3 pillarDir = (pillarPoints[1] - pillarPoints[0]).Normalized();

	GFPhysSoftTube * physTube = tubeObj->GetPhysicsBody();

	GFPhysVector3 com(0, 0, 0);

	for (int c = 0; c < physTube->GetNumNodes(); c++)
	{
		com += physTube->GetNode(c)->m_CurrPosition;
	}

	com /= physTube->GetNumNodes();

	for (int c = 0; c < physTube->GetNumNodes(); c++)
	{
		int e0 = c;
		int e1 = (c + 1) % physTube->GetNumNodes();
		GFPhysVector3 p0 = physTube->GetNode(e0)->m_CurrPosition;
		GFPhysVector3 p1 = physTube->GetNode(e1)->m_CurrPosition;

		GFPhysVector3 n = (p0 - com).Cross(p1 - com).Normalized();
		//if (fabsf(n.Dot(pillarDir)) < 0.96f)
		//{
			//PutSucced = false;
			//break;
		//}
		//else
		{
			Real Rayweight;
			GFPhysVector3 intersectpt;
			Real triangleWeight[3];
			bool intersect = LineIntersectTriangle(com, p0, p1, pillarPoints[0], pillarPoints[1], Rayweight, intersectpt, triangleWeight);
			if (intersect && Rayweight >= 0 && Rayweight <= 0.75f)//near bottom
			{
				PutSucced = true;
			}
		}
	}
	return PutSucced;
}
void CRingTransferTrain::onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{
	MisNewTraining::onRSFaceContactsSolveEnded(RSContactConstraints);
}

float CRingTransferTrain::TubeTouchGroundPercent(ACTubeShapeObject * tubeObj)
{
	float groundY = 0.0f;
	int numNodeTouched = 0;
	GFPhysSoftTube * softTube = tubeObj->GetPhysicsBody();
	for (int c = 0; c < softTube->GetNumNodes(); c++)
	{
		float y = softTube->GetNode(c)->m_CurrPosition.m_y;
		if (y < groundY + softTube->GetCollisionRadius()*1.2f)
		{
			numNodeTouched++;
		}
	}
	return (float)numNodeTouched / (float)softTube->GetNumNodes();
}
//======================================================================================================================
void CRingTransferTrain::OnSaveTrainingReport()
{
	int numTubeFallGroundWhenPass = 0;
	int numSucced = 0;
	for (int c = 0; c < m_Tubes.size(); c++)
	{
		{//传递是否成功，以及传递的时候是否接触地面
			QString passEntryCode;
			if (m_Tubes[c].m_HasPassSucced)
			{
				if (m_Tubes[c].m_TouchGroundWhenPass == false)
				{
					passEntryCode = "0250" + QString::number(c * 2 + 1) + "03310";
				}
				else
				{
					passEntryCode = "0250" + QString::number(c * 2 + 1) + "03311";
					numTubeFallGroundWhenPass++;
				}
			}
			else
			{
				passEntryCode = "0250" + QString::number(c * 2 + 1) + "03319";

			}
			AddScoreItemDetail(passEntryCode, 0);
		}

		{//是否摆放成功
			QString putEntryCode;
			if (m_Tubes[c].m_State == RS_FINISHPUT)
			{
				putEntryCode = "0250" + QString::number((c + 1) * 2) + "02210";
			}
			else if (m_Tubes[c].m_State != RS_WAITBECLAMP)
			{
				putEntryCode = "0250" + QString::number((c + 1) * 2) + "02211";
			}
			else
			{
				putEntryCode = "0250" + QString::number((c + 1) * 2) + "02219";
			}
			AddScoreItemDetail(putEntryCode, 0);
		}

		{
			QString putQualityCode;
			if (m_Tubes[c].m_State == RS_FINISHPUT)//摆放正确,看是否一次性摆放成功
			{
				if (m_Tubes[c].m_PutFaileNum == 0)
					putQualityCode = "0250" + QString::number((c + 1) * 2) + "04810";
				else
					putQualityCode = "0250" + QString::number((c + 1) * 2) + "04811";
				
				AddScoreItemDetail(putQualityCode, 0);

				numSucced++;
			}
			
		}
	}

	
	if (numSucced == m_Tubes.size())
	{
		if (numTubeFallGroundWhenPass == 0)
		{
			AddScoreItemDetail("0250900710", 0);
		}
		else if (numTubeFallGroundWhenPass > 3)
		{
			AddScoreItemDetail("0250900718", 0);//每次双手配合传递较好，一次完成
		}
		else
		{
			AddScoreItemDetail("0250900711", 0);//双手传递过程中有掉落
		}
	}

	if (numSucced > 0)
	{
		AddScoreItemDetail("0251003500", 0);//动作稳定，操作对象始终在训练区域
		AddScoreItemDetail("0251104511", 0);//配合尚可，较熟练
	}
	if (numSucced == m_Tubes.size())
	{
		if (numTubeFallGroundWhenPass == 0)
		    AddScoreItemDetail("0251200300", 0);
		else
		    AddScoreItemDetail("0251200301", 0);
	}


	float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
	float rightToolSpeed = m_pToolsMgr->GetRightToolMovedSpeed();

	if (m_pToolsMgr->GetLeftToolMovedDistance() > 10 || m_pToolsMgr->GetRightToolMovedDistance() > 10)
	{
		if (leftToolSpeed > 10 || rightToolSpeed > 10)
		{
			AddScoreItemDetail("0251300802", 0);//移动速度过快，有安全隐患
		}
		else if (leftToolSpeed < 5 && rightToolSpeed < 5)
		{
			AddScoreItemDetail("0251300800", 0);//移动平稳流畅
		}
		else
		{
			AddScoreItemDetail("0251300801", 0);//移动速度较快
		}
	}
	//
	if (numSucced == m_Tubes.size())
	{
		int TimeUsed = GetElapsedTime();
		if (TimeUsed < 60)
			AddScoreItemDetail("0251400500", 0);//1分钟内完成所有操作
		else if (TimeUsed < 120)
			AddScoreItemDetail("0251400501", 0);//在2分钟~3分钟内完成所有操作
		else
			AddScoreItemDetail("0251400502", 0);//完成所有规定操作时超过了3分钟
	}
	__super::OnSaveTrainingReport();
}
//======================================================================================================================
SYScoreTable * CRingTransferTrain::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01102501");

}