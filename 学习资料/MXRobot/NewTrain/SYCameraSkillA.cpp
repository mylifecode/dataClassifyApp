#pragma once
#include "Inception.h"
#include "SYCameraSkillA.h"
#include "XMLWrapperTraining.h"
#include "ScreenEffect.h"
#include "MXOgreWrapper.h"
#include "EffectManager.h"
#include "InputSystem.h"
#include "OgreMaxScene.hpp"
#include "LightMgr.h"
#include "MisMedicRigidPrimtive.h"
#include "SYScoreTableManager.h"
#define LETTERSCALE 0.15f

static class CollisionMeshDetect : public GFPhysTriangleProcessor
{
public:
	CollisionMeshDetect(const GFPhysVector3 & source, const GFPhysVector3 & target)
	{
		m_raySource = source;
		m_rayTarget = target;
		m_closetCastWeightInRay = FLT_MAX;
		m_CastSucced = false;
	}

	void ProcessTriangle(GFPhysVector3* triangleVerts, int partId, int triangleIndex, void * UserData)
	{
		Real  Rayweight;
		Real  triangleWeight[3];
		GFPhysVector3  intersectpt;

		bool intersect = LineIntersectTriangle(triangleVerts[0],
			                                   triangleVerts[1],
			                                   triangleVerts[2],
			                                   m_raySource,
			                                   m_rayTarget,
			                                   Rayweight,
			                                   intersectpt,
			                                   triangleWeight);

		if (intersect && Rayweight > 0 && Rayweight < m_closetCastWeightInRay)
		{
			m_closetCastWeightInRay = Rayweight;
			m_ResultPoint = m_raySource + (m_rayTarget - m_raySource) * Rayweight;
			m_CastSucced = true;
		}
	}

	GFPhysVector3 m_raySource;
	GFPhysVector3 m_rayTarget;
	GFPhysVector3 m_ResultPoint;
	float m_closetCastWeightInRay;
	bool  m_CastSucced;
};

SYCameraSkillA::SYCameraSkillA(MXOgreWrapper::CameraState eCS)
{
	m_CamLetterNode = 0;
	m_state = TRAINSTATE::TT_NOTFOCUS;
	//m_focuseTime = 0;
	m_ScaleTime  = 0;
	m_DetectCameraIntersect = true;
}
//=========================================================================================================
SYCameraSkillA::~SYCameraSkillA(void)
{
	std::map<std::string, SphereMeshData*>::iterator itor = m_SphereMeshMap.begin();
	while (itor != m_SphereMeshMap.end())
	{
		delete itor->second;
		itor++;
	}
	m_SphereMeshMap.clear();
}
//=========================================================================================================
bool SYCameraSkillA::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	MisNewTraining::Initialize(pTrainingConfig, pToolConfig);

	m_NumSphere = 7;

	m_SphereMeshMap.insert(std::make_pair("Red",  new SYCameraSkillA::SphereMeshData("sphere.mesh", "CameraSkillA_SphereRed")));
	m_SphereMeshMap.insert(std::make_pair("Blue", new SYCameraSkillA::SphereMeshData("sphere.mesh", "CameraSkillA_SphereBlue")));

	CreateAllSpheresRandomly();
	CreateCameraRect("sight_rect.mesh");

	//active the first one
	m_SceneSpheres[0].m_SceneNode->setVisible(true);
	//

	Inception::Instance()->EmitShowTrainProgressBar();
	Inception::Instance()->EmitShowTrainCompletness(0, 7);
	return true;
}
//==============================================================================================
void SYCameraSkillA::CreateAllSpheresRandomly()
{
	srand(GetTickCount());

	float interAngle = 3.1415f * 2.0f / (float)(m_NumSphere); //3.1415f * 2.0f / 5.0f;
	
	for (int c = 0; c < m_NumSphere; c++)
	{
		int colortype = rand() % 2;

		CreateSphereInRandomPos(colortype == 0 ? "Red" : "Blue",
			                    interAngle*c , interAngle * (c + 0.8f),
								3.1415f / 6.0f, 3.1415f / 4.0f);
	}
}
//==============================================================================================
void SYCameraSkillA::CreateSphereInRandomPos(const std::string & letter,
	                                       float minTheta, float maxTheta,
	                                       float minAlpha, float maxAlpha)
{
	//
	//srand(GetTickCount());
	float tt = float(rand() % 100) / 100.0f;
	float ta = float(rand() % 100) / 100.0f;

	float theta = minTheta * (1 - tt) + maxTheta * tt;
	float alpha = minAlpha * (1 - ta) + maxAlpha * ta;
	CreateSphereInHemiSphere(letter, theta, alpha);
}
//=======================================================================================
void SYCameraSkillA::CreateSphereInHemiSphere(const std::string & letter, float theta, float alpha)//theta range (0 ~ 360) alpha(0-90)
{
	GFPhysVector3 halfSphereCenter(-1.758f, 15.5f, -2.322f);
	GFPhysVector3 halfSphereZ(0, 0, -1);
	GFPhysVector3 halfSphereX(1, 0, 0);
	GFPhysVector3 halfSphereY(0, 1, 0);

	//float theta = 45.0f * 3.1415f / 180.0f;//range (0 ~ 360)
	//float alpha = 45.0f * 3.1415f / 180.0f;//range (0 ~ 90)

	GFPhysVector3 rayDir = halfSphereZ * cosf(alpha) + (halfSphereX*cosf(theta) + halfSphereY*sinf(theta))*sinf(alpha);
	rayDir = rayDir.Normalized();

	MisMedicRigidPrimtive * rigidMesh = dynamic_cast<MisMedicRigidPrimtive*>(m_DynObjMap.begin()->second);
	GFPhysBvhTriMeshShape * triMesh = dynamic_cast<GFPhysBvhTriMeshShape*>(rigidMesh->m_body->GetCollisionShape());

	GFPhysVector3 raySource = halfSphereCenter;
	GFPhysVector3 rayTarget = raySource + rayDir*30.0f;

	CollisionMeshDetect detector(raySource, rayTarget);

	triMesh->CastRay(&detector, raySource, rayTarget);

	if (detector.m_CastSucced)
	{
		GFPhysVector3 letterPos = detector.m_ResultPoint +(raySource - rayTarget).Normalized() * 1.0f;
		CreateWorldSphere(letter, GPVec3ToOgre(letterPos));
	}
}
//=======================================================================================
Ogre::Vector3  SYCameraSkillA::GetCameraPivot()
{
	return InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetPivotPosition();
}
//=======================================================================================
Ogre::SceneNode * SYCameraSkillA::CreateCameraRect(const std::string & rectMesh)
{
	const std::string nodeName = "CamRect";
	
	if (m_CamLetterNode == 0)
	{
		m_CamLetterNode = MXOgre_SCENEMANAGER->getRootSceneNode()->createChildSceneNode(nodeName);
	}

	Ogre::MeshPtr letterMesh    = Ogre::MeshManager::getSingleton().load(rectMesh , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	m_CamLetterEntity = MXOgre_SCENEMANAGER->createEntity(rectMesh);

	m_CamLetterNode->attachObject(m_CamLetterEntity);

	m_CamLetterEntity->setMaterialName("Camera_Sight_NotFocused");
	
	m_CamLetterEntity->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
	
	return m_CamLetterNode;
}
//=======================================================================================
Ogre::SceneNode * SYCameraSkillA::CreateWorldSphere(const std::string & letter, const Ogre::Vector3 & letterPos)
{
	static int sphereNodeID = 0;
	sphereNodeID++;

	const SphereMeshData & meshData = (*m_SphereMeshMap[letter]);
	
	Ogre::Entity * entityLetter = MXOgre_SCENEMANAGER->createEntity(meshData.m_WorldMesh);

	std::string sceneNodeName = "SphereEntity" + Ogre::StringConverter::toString(sphereNodeID);

	Ogre::SceneNode * seneNode = MXOgre_SCENEMANAGER->getRootSceneNode()->createChildSceneNode(sceneNodeName);
	
	seneNode->attachObject(entityLetter);

	entityLetter->setMaterialName(meshData.m_MeshMaterial);

	Ogre::Vector3 VecLetterToEye = (GetCameraPivot() - letterPos).normalisedCopy();

	//get rotation make z- axis align camera pos
	Ogre::Quaternion rotQuat = Ogre::Vector3::UNIT_Z.getRotationTo(VecLetterToEye);
	
	seneNode->setOrientation(rotQuat);

	seneNode->setPosition(letterPos);

	seneNode->setScale(LETTERSCALE, LETTERSCALE, LETTERSCALE);

	seneNode->setVisible(false);

	m_SceneSpheres.push_back(SYCameraSkillA::SceneSphereInstance(seneNode, entityLetter, letter));

	return seneNode;
}
//=======================================================================================
void SYCameraSkillA::FinishTraining()
{
	TrainingFinish();
}
//=======================================================================================
bool SYCameraSkillA::Update(float dt)
{
	MisNewTraining::Update(dt);

	//
	Ogre::Vector3 AxisZ = -m_pLargeCamera->getDerivedDirection();
	Ogre::Vector3 AxisX = m_pLargeCamera->getDerivedRight();
	Ogre::Vector3 AxisY = m_pLargeCamera->getDerivedUp();

	Ogre::Quaternion rotQuat;
	rotQuat.FromAxes(AxisX, AxisY, AxisZ);
	if (m_CamLetterNode)
	{
		m_CamLetterNode->setOrientation(rotQuat);
		m_CamLetterNode->setPosition(m_pLargeCamera->getDerivedPosition() - AxisZ * 10.0f);
		m_CamLetterNode->setScale(LETTERSCALE, LETTERSCALE, LETTERSCALE);
	}
	//
	UpdateState(dt);
	return true;
}
//=================================================================================================
void SYCameraSkillA::UpdateState(float dt)
{
	if (m_SceneSpheres.size() <= 0)
	{
		return;
	}

	//calculate 
	Ogre::Matrix4 matViewPrj = m_pLargeCamera->getProjectionMatrix() * m_pLargeCamera->getViewMatrix();
	Ogre::Vector3 spherPos = m_SceneSpheres[0].m_SceneNode->_getDerivedPosition();

	Ogre::Vector4 projPos = matViewPrj * Ogre::Vector4(spherPos);
	projPos.x /= projPos.w;
	projPos.y /= projPos.w;

	int numWidPixel  = m_pLargeCamera->getViewport()->getActualWidth();
	int numWidHeight = m_pLargeCamera->getViewport()->getActualHeight();

	int delatPixelX = projPos.x * numWidPixel;
	int delatPixelY = projPos.y * numWidHeight;
	int thredholdFocus = 18;
	int thresholdLoss  = 30;

	bool getFocus = (fabsf(delatPixelX) < thredholdFocus && fabsf(delatPixelY) < thredholdFocus);
	bool getLoss = (fabsf(delatPixelX) > thresholdLoss || fabsf(delatPixelY) > thresholdLoss);

#if(1)
	{
		GFPhysVector3 raySource = OgreToGPVec3(m_pLargeCamera->getDerivedPosition() - spherPos);//transform to sphere coorindate
		GFPhysVector3 rayTagert = raySource + OgreToGPVec3(m_pLargeCamera->getDerivedDirection())*50.0f;
		std::map<std::string, SphereMeshData*>::iterator itor = m_SphereMeshMap.find(m_SceneSpheres[0].m_name);
		if (itor != m_SphereMeshMap.end())
		{
			float stoneScale = m_SceneSpheres[0].m_SceneNode->getScale().x;

			bool intersected = itor->second->isRayIntersect(raySource / stoneScale, rayTagert / stoneScale);

			if (intersected == true)
			{
				getFocus = true;
				getLoss  = false;
			}
			else
			{
				getFocus = false;
				getLoss  = true;
			}
		}
	}
#endif
	if (m_state == TRAINSTATE::TT_NOTFOCUS)
	{
		if (getFocus)
		{
			m_state = TRAINSTATE::TT_INFOCUS;
			m_CamLetterEntity->setMaterialName("Camera_Sight_Focused");
			CTipMgr::Instance()->ShowTip("TipKeepFocus");
		}
	}
	else if (m_state == TRAINSTATE::TT_INFOCUS)
	{
		if (getLoss)
		{
			m_state = TRAINSTATE::TT_NOTFOCUS;
			m_CamLetterEntity->setMaterialName("Camera_Sight_NotFocused");
			m_SceneSpheres[0].m_FocusTime = 0;
			m_SceneSpheres[0].m_FailedNum ++;
			CTipMgr::Instance()->ShowTip("TipFocusLoss");
		}
		else
		{
			m_SceneSpheres[0].m_FocusTime += dt;
			if (m_SceneSpheres[0].m_FocusTime > 2.0f)
			{
				m_state = TRAINSTATE::TT_INDISAPPEAR;
				CTipMgr::Instance()->ShowTip("TipFocusSuccNext");
			}
		}
	}
	else if (m_state == TRAINSTATE::TT_INDISAPPEAR)
	{
		m_ScaleTime += dt;
		if (m_ScaleTime < 1.0f)
		{
			float scale = 1.0f - m_ScaleTime;
			float minS = 0.2f;
			float maxs = 1.0f;
			scale = (maxs *scale + (1 - scale)*minS)*LETTERSCALE;
			m_SceneSpheres[0].m_SceneNode->setScale(Ogre::Vector3(scale , scale , scale));
		}
		else
		{
			
			static int finishnum = 0;

			finishnum++;

			Inception::Instance()->EmitShowTrainCompletness(finishnum , 7);// (float(finishnum) / 7.0f);

			NextSphere();
		}
	}
}
void SYCameraSkillA::NextSphere()
{
	if (m_SceneSpheres.size() > 0)
	{
		m_state = TRAINSTATE::TT_NOTFOCUS;
		m_CamLetterEntity->setMaterialName("Camera_Sight_NotFocused");
//		m_focuseTime = 0;
		m_ScaleTime = 0;
		//
		m_SceneSpheres[0].m_SceneNode->detachAllObjects();
		MXOgre_SCENEMANAGER->destroyEntity(m_SceneSpheres[0].m_Entity);
		MXOgre_SCENEMANAGER->destroySceneNode(m_SceneSpheres[0].m_SceneNode);
		
		m_ElimitedSpheres.push_back(m_SceneSpheres[0]);
		m_SceneSpheres.erase(m_SceneSpheres.begin());
		
		//
		if (m_SceneSpheres.size() > 0)
		{
			m_SceneSpheres[0].m_SceneNode->setVisible(true);
			CTipMgr::Instance()->ShowTip("TrainingIntro");
		}
		else
		{
			//finish
			CTipMgr::Instance()->ShowTip("TipFocusSuccFinish");
			TrainingFinish();
		}
	}
}
void SYCameraSkillA::OnSaveTrainingReport()
{
	for (int c = 0; c < m_ElimitedSpheres.size(); c++)
	{
		int FailedNum = m_ElimitedSpheres[c].m_FailedNum;
		if (FailedNum == 0)
		{
			QString fullcode = "0010" + QString::number(c + 1) + "00110";
			AddScoreItemDetail(fullcode , 0);// ("0010100110", 0);
		}
		else
		{
			QString fullcode = "0010" + QString::number(c + 1) + "00111";
			AddScoreItemDetail(fullcode, 0);
		}

		if (FailedNum <= 1)
		{
			QString fullcode = "0010" + QString::number(c + 1) + "00210";
			AddScoreItemDetail(fullcode, 0);
		}
		else
		{
			QString fullcode = "0010" + QString::number(c + 1) + "00211";
			AddScoreItemDetail(fullcode, 0);
		}
	}

	if (m_ElimitedSpheres.size() > 0)
	{
		int TimeUsed = GetElapsedTime();

		if (m_SceneSpheres.size() == 0)//all eliminated
		{
			
			if (TimeUsed < 60)
				AddScoreItemDetail("0010800300", 0);//60秒内完成所有操作
			else if (TimeUsed < 90)
				AddScoreItemDetail("0010800301", 0);//在2分钟~3分钟内完成所有操作
		}
		if (m_CameraSpeed < 2)
		{
			AddScoreItemDetail("0010900400", 0);
		}
		else if (m_CameraSpeed < 5)
		{
			AddScoreItemDetail("0010900401", 0);
		}
		else
		{
			AddScoreItemDetail("0010900402", 0);
		}

		if (m_SceneSpheres.size() == 0)
		{
			if (TimeUsed < 60)
				AddScoreItemDetail("0011000500", 0);//60秒内完成所有操作
			else if (TimeUsed < 90)
				AddScoreItemDetail("0011000501", 0);//在2分钟~3分钟内完成所有操作
			else
				AddScoreItemDetail("0011000502", 0);
		}
	}
	__super::OnSaveTrainingReport();
}

//======================================================================================================================
SYScoreTable* SYCameraSkillA::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01100101");
}