#pragma once
#include "Inception.h"
#include "SYCameraSkillB.h"
#include "XMLWrapperTraining.h"
#include "ScreenEffect.h"
#include "MXOgreWrapper.h"
#include "EffectManager.h"
#include "InputSystem.h"
#include "OgreMaxScene.hpp"
#include "LightMgr.h"
#include "MisMedicRigidPrimtive.h"
#include "SYScoreTableManager.h"
#define LETTERSCALE 0.25f

static class CollisionMeshDetectB : public GFPhysTriangleProcessor
{
public:
	CollisionMeshDetectB(const GFPhysVector3 & source, const GFPhysVector3 & target)
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

		if (intersect && Rayweight >= 0 && Rayweight <=1 && Rayweight < m_closetCastWeightInRay)
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

std::wstring s2ws(const std::string& s)
{
	const char* str = s.c_str();
	size_t len = s.size() + 1;
	wchar_t *wstr = new wchar_t[len];
	std::mbstowcs(wstr, str, len);
	std::wstring ret(wstr);
	delete[] wstr;
	return ret;
}

SYCameraSkillB::SYCameraSkillB(MXOgreWrapper::CameraState eCS)
{
	srand(GetTickCount());
	m_CheckLetter[0] = 'A';
	m_CheckLetter[1] = 'B';
	m_CheckLetter[2] = 'C';
	m_CheckLetter[3] = 'D';
	m_CheckLetter[4] = 'E';
	std::random_shuffle(&(m_CheckLetter[0]) , &(m_CheckLetter[4]));
}
//=========================================================================================================
SYCameraSkillB::~SYCameraSkillB(void)
{

}
//=========================================================================================================
bool SYCameraSkillB::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	MisNewTraining::Initialize(pTrainingConfig, pToolConfig);
	m_LetterMeshMap.insert(std::make_pair("A", SYCameraSkillB::LetterMeshData("LetterAUnfocus.mesh", "LetterAWorld.mesh", "LetterACam.mesh")));
	m_LetterMeshMap.insert(std::make_pair("B", SYCameraSkillB::LetterMeshData("LetterBUnfocus.mesh", "LetterBWorld.mesh", "LetterBCam.mesh")));
	m_LetterMeshMap.insert(std::make_pair("C", SYCameraSkillB::LetterMeshData("LetterCUnfocus.mesh", "LetterCWorld.mesh", "LetterCCam.mesh")));
	m_LetterMeshMap.insert(std::make_pair("D", SYCameraSkillB::LetterMeshData("LetterDUnfocus.mesh", "LetterDWorld.mesh", "LetterDCam.mesh")));
	m_LetterMeshMap.insert(std::make_pair("E", SYCameraSkillB::LetterMeshData("LetterEUnfocus.mesh", "LetterEWorld.mesh", "LetterECam.mesh")));

	//CreateLetterInHemiSphere("A", 45.0f * 3.1415f / 180.0f, 45.0f * 3.1415f / 180.0f);

	CreateAllLettersRandomly();
	CreateNextCameraLetter();

	Inception::Instance()->EmitShowTrainProgressBar();
	Inception::Instance()->EmitShowTrainCompletness(0, 5);
	return true;
}
//==================================================================================================
void SYCameraSkillB::OnTrainingIlluminated()
{
	MisNewTraining::OnTrainingIlluminated();
	std::string letter;
	letter.append({ m_CheckLetter[0] });

	wstring wletter = s2ws(letter);

	CTipMgr::Instance()->ShowTip("TrainingFirstTip", wletter.c_str());

}
//==============================================================================================
void SYCameraSkillB::CreateAllLettersRandomly()
{
	srand(GetTickCount());

	//first remove exist letters
	for (int c = 0; c < (int)m_WorldLettersInstance.size(); c++)
	{
		 Ogre::SceneNode * sceneNode = m_WorldLettersInstance[c].m_SceneNode;
		 sceneNode->detachObject(m_WorldLettersInstance[c].m_LetterEntityUnFocuse);
		 sceneNode->detachObject(m_WorldLettersInstance[c].m_LetterEntityFocuse);

		 MXOgre_SCENEMANAGER->destroyEntity(m_WorldLettersInstance[c].m_LetterEntityUnFocuse);
		 MXOgre_SCENEMANAGER->destroyEntity(m_WorldLettersInstance[c].m_LetterEntityFocuse);

		 MXOgre_SCENEMANAGER->getRootSceneNode()->removeChild(sceneNode);
		 MXOgre_SCENEMANAGER->destroySceneNode(sceneNode);
	}
	m_WorldLettersInstance.clear();

	//
	float minAlphaAngle = 3.1415f / 6.0f;
	float maxAlphaAngle = 3.1415f / 4.0f;

	float interAngle = 3.1415f * 2.0f / 5.0f;
	CreateLetterInRandomPos("A", 
		                    0.0f, interAngle*0.8f,
							minAlphaAngle, maxAlphaAngle);
	CreateLetterInRandomPos("B",
		                    interAngle, interAngle*1.8f,
							minAlphaAngle, maxAlphaAngle);
	CreateLetterInRandomPos("C",
		                    interAngle * 2, interAngle * 2.8f,
							minAlphaAngle, maxAlphaAngle);
	CreateLetterInRandomPos("D",
		                    interAngle * 3, interAngle * 3.8,
							minAlphaAngle, maxAlphaAngle);
	CreateLetterInRandomPos("E",
		                    interAngle * 4, interAngle * 4.8,
							minAlphaAngle, maxAlphaAngle);

}
//==============================================================================================
void SYCameraSkillB::CreateLetterInRandomPos(const std::string & letter,
	                                       float minTheta, float maxTheta,
	                                       float minAlpha, float maxAlpha)
{
	//
	//srand(GetTickCount());
	float tt = float(rand() % 100) / 100.0f;
	float ta = float(rand() % 100) / 100.0f;

	float theta = minTheta * (1 - tt) + maxTheta * tt;
	float alpha = minAlpha * (1 - ta) + maxAlpha * ta;
	CreateLetterInHemiSphere(letter, theta, alpha);
}
//=======================================================================================
void SYCameraSkillB::CreateLetterInHemiSphere(const std::string & letter, float theta, float alpha)//theta range (0 ~ 360) alpha(0-90)
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

	CollisionMeshDetectB detector(raySource, rayTarget);

	triMesh->CastRay(&detector, raySource, rayTarget);

	if (detector.m_CastSucced)
	{
		GFPhysVector3 letterPos = detector.m_ResultPoint +(raySource - rayTarget).Normalized() * 1.0f;
		CreateWorldLetter(letter, GPVec3ToOgre(letterPos));
	}
}
//=======================================================================================
Ogre::Vector3  SYCameraSkillB::GetCameraPivot()
{
	return InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetPivotPosition();
}
//=======================================================================================
void SYCameraSkillB::CreateNextCameraLetter()
{
	std::string letter;
	letter.append({ m_CheckLetter[m_CamLetter.m_index] });

	const LetterMeshData & meshData = m_LetterMeshMap[letter];

	const std::string nodeName = "CamLetter";
	
	//ok first remove old one 
	if (m_CamLetter.m_CamLetterEnt)
	{
		m_CamLetter.m_CamLetterNode->detachObject(m_CamLetter.m_CamLetterEnt);
		MXOgre_SCENEMANAGER->destroyEntity(m_CamLetter.m_CamLetterEnt);
	}

	if (m_CamLetter.m_CamLetterNode == 0)//reuse scene node
	{
		m_CamLetter.m_CamLetterNode = MXOgre_SCENEMANAGER->getRootSceneNode()->createChildSceneNode(nodeName);
	}

	Ogre::MeshPtr letterMesh = Ogre::MeshManager::getSingleton().load(meshData.m_CameraMesh,
		                                                              Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	m_CamLetter.m_CamLetterEnt = MXOgre_SCENEMANAGER->createEntity(meshData.m_CameraMesh);

	m_CamLetter.m_CamLetterNode->attachObject(m_CamLetter.m_CamLetterEnt);

	m_CamLetter.m_CamLetterEnt->setMaterialName("Camera_Letter_Dashed");
	
	m_CamLetter.m_CamLetterEnt->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
	
	m_CamLetter.m_name = letter;

	m_CamLetter.m_AimSucced = false;
	m_CamLetter.m_AimUsedTime = 0;

	m_CamLetter.m_index++;
}
//=======================================================================================
Ogre::SceneNode * SYCameraSkillB::CreateWorldLetter(const std::string & letter, const Ogre::Vector3 & letterPos)
{
	const LetterMeshData & meshData = m_LetterMeshMap[letter];
	
	//Ogre::MeshPtr letterMesh = Ogre::MeshManager::getSingleton().load(meshData.m_WorldMeshUnFocused,
		                                                              //Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	std::string sceneNodeName = "LetterEntity" + letter;

	Ogre::SceneNode * seneNode = MXOgre_SCENEMANAGER->getRootSceneNode()->createChildSceneNode(sceneNodeName);
	
	Ogre::Entity * entityUnfoc = MXOgre_SCENEMANAGER->createEntity(meshData.m_WorldMeshUnFocused);
	entityUnfoc->setMaterialName("orange");

	Ogre::Entity * entityfoc   = MXOgre_SCENEMANAGER->createEntity(meshData.m_WorldMeshFocused);
	entityfoc->setMaterialName("orange");

	seneNode->attachObject(entityUnfoc);
	seneNode->attachObject(entityfoc);

	

	Ogre::Vector3 VecLetterToEye = (GetCameraPivot() - letterPos).normalisedCopy();

	//get rotation make z- axis align camera pos
	Ogre::Quaternion rotQuat = Ogre::Vector3::UNIT_Z.getRotationTo(VecLetterToEye);
	
	Ogre::Quaternion gimablRot;
	float t = float(rand() % 100) / 100.0f;

	gimablRot.FromAngleAxis(Ogre::Radian(t*3.1415f*2.0f) , VecLetterToEye);

	seneNode->setOrientation(gimablRot * rotQuat);

	seneNode->setPosition(letterPos);

	seneNode->setScale(LETTERSCALE, LETTERSCALE, LETTERSCALE);

	entityUnfoc->setVisible(true);
	entityfoc->setVisible(false);

	m_WorldLettersInstance.push_back(SYCameraSkillB::SceneLetterInstance(seneNode, entityUnfoc, entityfoc , letter, LETTERSCALE));

	return seneNode;
}
//=======================================================================================
void SYCameraSkillB::FinishTraining()
{
	TrainingFinish();
}
//=======================================================================================
bool SYCameraSkillB::Update(float dt)
{
	MisNewTraining::Update(dt);

	UpdateCameraLetter();

	bool hasSomeLetterDisappered = UpdateSceneLetters(1.0f, dt);

	if (hasSomeLetterDisappered)
	{
		if (m_CamLetter.m_index <= 4)
			CreateNextCameraLetter();
	}

	if (m_CamLetter.m_AimSucced == false)
	{
		bool focused = CheckCoincide(m_CamLetter.m_name , dt);
		//if (focused)
		//{
			
		//}
	}
		
	
	return true;
}
//=
bool SYCameraSkillB::BeginRendOneFrame(float timeelapsed)
{
	return MisNewTraining::BeginRendOneFrame(timeelapsed);
}
#define OVERLAPDISTTHRES  0.5f
#define OVERLAPANGLTHRES  5.0f
#define OVERLAPNEEDTIME   3.0f
//=================================================================================================
bool SYCameraSkillB::CheckCoincide(const std::string & checkname , float dt)
{
	if (m_CamLetter.m_CamLetterNode == 0)
		return false;

	m_CamLetter.m_AimUsedTime += dt;

	Ogre::Matrix4 matViewPrj = m_pLargeCamera->getProjectionMatrix() * m_pLargeCamera->getViewMatrix();
	
	int ScreenWid = m_pLargeCamera->getViewport()->getActualWidth();
	
	int ScreenHei = m_pLargeCamera->getViewport()->getActualHeight();

	for (int c = 0; c < (int)m_WorldLettersInstance.size(); c++)
	{
		if (checkname == m_WorldLettersInstance[c].m_name && (!m_WorldLettersInstance[c].m_OverLapSucceed))
		{
			Ogre::SceneNode * sceneNode = m_WorldLettersInstance[c].m_SceneNode;

			Ogre::Vector3  deltaPos = sceneNode->_getDerivedPosition() - m_CamLetter.m_CamLetterNode->_getDerivedPosition();

			Ogre::Quaternion deltaRot = sceneNode->_getDerivedOrientation().Inverse() * m_CamLetter.m_CamLetterNode->_getDerivedOrientation();

			Ogre::Degree  angle;
			Ogre::Vector3 localAixs;
			deltaRot.ToAngleAxis(angle, localAixs);

			float degree = angle.valueDegrees();

			//镜头字母和场景字母基本重合
			float OPDistThres = OVERLAPDISTTHRES;
			float OPAnglThres = OVERLAPANGLTHRES;
			bool  isLastOverLap = (m_WorldLettersInstance[c].m_OverLapTime > FLT_EPSILON);

			//如果当前处于重合状态则增大误差允许范围
			if (isLastOverLap)
			{
				OPDistThres *= 1.2f;
				OPAnglThres *= 1.5f;
			}

			if (deltaPos.length() < OPDistThres && (fabsf(degree) < OPAnglThres || fabsf(degree - 360) < OPAnglThres))
			{
				m_WorldLettersInstance[c].m_OverLapTime += dt;

				//重合后显示重合字母模型
				m_WorldLettersInstance[c].m_LetterEntityUnFocuse->setVisible(false);
				m_WorldLettersInstance[c].m_LetterEntityFocuse->setVisible(true);

				//重合时间超过2秒，则判断为消除成功
				if (m_WorldLettersInstance[c].m_OverLapTime > OVERLAPNEEDTIME)
				{
					m_CamLetter.m_AimSucced = true;
					m_WorldLettersInstance[c].m_OverLapSucceed = true;
					m_WorldLettersInstance[c].m_UsedTime = m_CamLetter.m_AimUsedTime;
					Inception::Instance()->EmitShowTrainCompletness(m_CamLetter.m_index, 5);
					if (m_CamLetter.m_index <= 4)
					{
						std::string letter;
						letter.append({ m_CheckLetter[m_CamLetter.m_index] });

						wstring wletter = s2ws(letter);

						CTipMgr::Instance()->ShowTip("OverLapSucceed", wletter.c_str());
						m_WorldLettersInstance[c].m_IsTipShowed = true;
					}
					else
					{
						CTipMgr::Instance()->ShowTip("TrainingFinish");
						TrainingFinish();
					}
				}
				else
				{
					CTipMgr::Instance()->ShowTip("KeepCamera");
				}
				return true;
			}
			else
			{
				m_WorldLettersInstance[c].m_LetterEntityUnFocuse->setVisible(true);
				m_WorldLettersInstance[c].m_LetterEntityFocuse->setVisible(false);
				m_WorldLettersInstance[c].m_OverLapTime = 0;
				if (isLastOverLap)
				{
					m_WorldLettersInstance[c].m_LossOverLapTime = 1.0f;
					m_WorldLettersInstance[c].m_FailedNum++;
					CTipMgr::Instance()->ShowTip("CameraNotStable");
				}
				else
				{
					if (m_WorldLettersInstance[c].m_LossOverLapTime < 0.0f)
					{ //检查摄像头是否对准字母中心并给予提示
						Ogre::Vector4 projPos = matViewPrj * Ogre::Vector4(sceneNode->_getDerivedPosition());
						projPos.x /= projPos.w;
						projPos.y /= projPos.w;

						int delatPixelX = projPos.x * ScreenWid;
						int delatPixelY = projPos.y * ScreenHei;

						int thredholdFocus = 20;
						//int thresholdLoss = 50;

						bool beLocated = (fabsf(delatPixelX) < thredholdFocus && fabsf(delatPixelY) < thredholdFocus);
						//bool getLoss = (fabsf(delatPixelX) > thresholdLoss || fabsf(delatPixelY) > thresholdLoss);

						/*if (getFocus && (!m_WorldLettersInstance[c].m_BeLocated))
						{
							m_WorldLettersInstance[c].m_BeLocated = true;
							
							CTipMgr::Instance()->ShowTip("FocuseSucceed");
						}
						else if (getLoss && m_WorldLettersInstance[c].m_BeLocated)
						{
							m_WorldLettersInstance[c].m_BeLocated = false;
							wstring wletter = s2ws(checkname);
							CTipMgr::Instance()->ShowTip("TrainingFirstTip", wletter.c_str());
						}*/
						if (beLocated)
						{
							CTipMgr::Instance()->ShowTip("FocuseSucceed");
						}
					}
					else
					{
						m_WorldLettersInstance[c].m_LossOverLapTime -= dt;//显示完扶镜不稳后，防止定位tip刷走
					}
				}
				//

			}
		}
	}
	return false;
}
//=================================================================================================
void SYCameraSkillB::ChangeCamLetterMaterial(int mode)
{
	if (m_CamLetter.m_CamLetterEnt)
	{
		if (mode == 0)
			m_CamLetter.m_CamLetterEnt->setMaterialName("Camera_Letter_Dashed");
		else
			m_CamLetter.m_CamLetterEnt->setMaterialName("Camera_Letter_Solid");
	}
}
//===========================================================================================================
void SYCameraSkillB::UpdateCameraLetter()
{
	//
	Ogre::Vector3 AxisZ = -m_pLargeCamera->getDerivedDirection();
	Ogre::Vector3 AxisX = m_pLargeCamera->getDerivedRight();
	Ogre::Vector3 AxisY = m_pLargeCamera->getDerivedUp();

	Ogre::Quaternion rotQuat;
	rotQuat.FromAxes(AxisX, AxisY, AxisZ);
	if (m_CamLetter.m_CamLetterNode)
	{
		m_CamLetter.m_CamLetterNode->setOrientation(rotQuat);
		m_CamLetter.m_CamLetterNode->setPosition(m_pLargeCamera->getDerivedPosition() - AxisZ * 5.0f);
		m_CamLetter.m_CamLetterNode->setScale(LETTERSCALE, LETTERSCALE, LETTERSCALE);
		
	}
	//
}
//======================================================================================================
bool SYCameraSkillB::UpdateSceneLetters(float shrinkspeed, float dt)
{
	bool hasSomeLetterDisappered = false;

	for (int c = 0; c < (int)m_WorldLettersInstance.size(); c++)
	{
		if (m_WorldLettersInstance[c].m_name == m_CamLetter.m_name && m_WorldLettersInstance[c].m_OverLapSucceed)
		{
			Ogre::SceneNode * sceneNode = m_WorldLettersInstance[c].m_SceneNode;

			if (m_WorldLettersInstance[c].m_EnlargeStage)
			{
				m_WorldLettersInstance[c].m_Scale += shrinkspeed*dt;
				if (m_WorldLettersInstance[c].m_Scale > 1.5f)
				{
					m_WorldLettersInstance[c].m_Scale = 1.5f;
					m_WorldLettersInstance[c].m_EnlargeStage = false;
				}
			}
			else
			{
				m_WorldLettersInstance[c].m_Scale -= shrinkspeed*dt;
			}
			
			if (m_WorldLettersInstance[c].m_Scale < 0.1)
			{
				m_WorldLettersInstance[c].m_Scale = 0.1;
				sceneNode->setVisible(false);
				hasSomeLetterDisappered = true;
			}

			float finalscale = m_WorldLettersInstance[c].m_InitScale * m_WorldLettersInstance[c].m_Scale;
			sceneNode->setScale(Ogre::Vector3(finalscale, finalscale, finalscale));	
		}
	}

	return hasSomeLetterDisappered;
}

void SYCameraSkillB::OnSaveTrainingReport()
{
	int FinishCount = 0;
	for (int c = 0; c < m_WorldLettersInstance.size(); c++)
	{
		bool overLapped = m_WorldLettersInstance[c].m_OverLapSucceed;
		int FailedNum   = m_WorldLettersInstance[c].m_FailedNum;
		float usedtime  = m_WorldLettersInstance[c].m_UsedTime;

		if (overLapped)
		{
			if (usedtime < 10)//成功对准字母
			{
				QString fullcode = "0020" + QString::number(c + 1) + "00110";
				AddScoreItemDetail(fullcode, 0);
			}
			else//成功对准，但是超时完成
			{
				QString fullcode = "0020" + QString::number(c + 1) + "00111";
				AddScoreItemDetail(fullcode, 0);
			}

			if (FailedNum <= 1)
			{
				QString fullcode = "0020" + QString::number(c + 1) + "00210";
				AddScoreItemDetail(fullcode, 0);
			}
			else
			{
				QString fullcode = "0020" + QString::number(c + 1) + "00211";
				AddScoreItemDetail(fullcode, 0);
			}
			FinishCount++;
		}
		else
		{
			QString fullcode = "0020" + QString::number(c + 1) + "00119";
			AddScoreItemDetail(fullcode, 0);
		}


		
	}


	if (FinishCount > 0)
	{
		int TimeUsed = GetElapsedTime();

		if (FinishCount == m_WorldLettersInstance.size())
		{
			if (TimeUsed < 60)
				AddScoreItemDetail("0020600300", 0);//60秒内完成所有操作
			else if (TimeUsed < 90)
				AddScoreItemDetail("0020600301", 0);//在2分钟~3分钟内完成所有操作
		}
		if (m_CameraSpeed < 2)
		{
			AddScoreItemDetail("0020700400", 0);
		}
		else if (m_CameraSpeed < 5)
		{
			AddScoreItemDetail("0020700401", 0);
		}
		else
		{
			AddScoreItemDetail("0020700402", 0);
		}

		if (FinishCount == m_WorldLettersInstance.size())
		{
			if (TimeUsed < 60)
				AddScoreItemDetail("0020804710", 0);//60秒内完成所有操作
			else if (TimeUsed < 90)
				AddScoreItemDetail("0020804711", 0);//在2分钟~3分钟内完成所有操作

			if (TimeUsed < 60)
				AddScoreItemDetail("0020900500", 0);//60秒内完成所有操作
			else if (TimeUsed < 90)
				AddScoreItemDetail("0020900501", 0);//在2分钟~3分钟内完成所有操作
			else
				AddScoreItemDetail("0020900502", 0);//在2分钟~3分钟内完成所有操作
		}
	}
	__super::OnSaveTrainingReport();
}

SYScoreTable* SYCameraSkillB::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01100201");
}