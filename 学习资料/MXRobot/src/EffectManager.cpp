#include "EffectManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreMaterialManager.h"
#include "ScreenEffect.h"
#include "MXOgreWrapper.h"
#include "InputSystem.h"
#include "TrainingMgr.h"

#include <qstring.h> 


#include "OgreParticleSystemManager.h"
#include "OgreMaterial.h"
#include "OgreParticleEmitter.h"

#include "stdafx.h"
#define RANDNUM 6
#include "VesselBleedEffect.h"
#include "SmokeManager.h"
#include "WaterManager.h"
#include "BubbleManager.h"
#include "MXOgreGraphic.h"

EffectManager::EffectManager()
{
	//m_bSmokeOn = false;
	m_bArrowOn = false;
	m_enmCameraType = CT_NORMAL;
	m_bUpdateFlashCamera = false;

	for (int i = 0; i < EDOT_ORGAN_MAX; i++)
	{
		m_arybCanvas[i] = false;
	}
	m_nArrowOnTime = 0;

	//m_bSmokeEffectOn=false;
	//m_dwSmokeEffectStartTime=0;

	//m_smokeNode=NULL;
	//m_smokeParicle=NULL;

	// 渗血效果
	//m_pBloodNode=NULL;
	//m_pBloodParticle=NULL;
	//m_bBloodEffectOn=false;
	//m_dwBloodEffectStartTime=0;
	m_nCushion = 0;

	m_vecCurrentPosition=Ogre::Vector3::ZERO;

	m_bOperationPositionError = false;

	Ogre::SceneManager * pSceneMgr = MXOgreWrapper::Get()->GetDefaultSceneManger();
	if (pSceneMgr)
	{
		//烟效
		//Ogre::SceneNode *smokeNode=pSceneMgr->getRootSceneNode()->createChildSceneNode("smokeNode");
		//Ogre::ParticleSystem *smokeParticle=pSceneMgr->createParticleSystem("smoke",PT_SMOKE_00);
		//smokeNode->attachObject(smokeParticle);
		//smokeParticle->setEmitting(false);

		//流血
		//Ogre::SceneNode *bloodNode=pSceneMgr->getRootSceneNode()->createChildSceneNode("bloodNode");
		//Ogre::ParticleSystem *bloodParticle=pSceneMgr->createParticleSystem("blood",PT_BLOOD_00);
		//Ogre::ParticleSystem *bloodParticle1=pSceneMgr->createParticleSystem("blood1",PT_BLOOD_01);
		//Ogre::ParticleSystem *bloodParticle2=pSceneMgr->createParticleSystem("blood2",PT_BLOOD_02);

		//bloodNode->attachObject(bloodParticle);
		//bloodNode->attachObject(bloodParticle1);
		//bloodNode->attachObject(bloodParticle2);

		//bloodParticle->setEmitting(false);
		//bloodParticle1->setEmitting(false);
		//bloodParticle2->setEmitting(false);

		//火花
		Ogre::SceneNode *sparkNode=pSceneMgr->getRootSceneNode()->createChildSceneNode("sparkNode");

		Ogre::BillboardSet * sparkBBS = pSceneMgr->createBillboardSet("star",1);
	   
		sparkBBS->setBillboardType(Ogre::BBT_POINT);

		sparkBBS->setBillboardOrigin(Ogre::BBO_CENTER);

		sparkBBS->setBillboardRotationType(Ogre::BBR_VERTEX);

		sparkBBS->setCommonUpVector(Ogre::Vector3(0,0,1));

		sparkBBS->setDefaultDimensions(0.5f , 0.5f);

		sparkBBS->setMaterialName("MatEffect/Spark");
		
		Ogre::Billboard * billboard = sparkBBS->createBillboard(Ogre::Vector3(0 , 0 , 0));

		billboard->setColour(Ogre::ColourValue(0.5,0.5,0.5,0.1));

		sparkNode->attachObject(sparkBBS);

		sparkNode->setVisible(false);

		Ogre::ParticleSystem *sparkParticle=pSceneMgr->createParticleSystem("spark",PT_SPARK_00);
		sparkNode->attachObject(sparkParticle);
		sparkParticle->setEmitting(false);
	}
	m_pSparkParticle=NULL;
	m_pSparkNode=NULL;
	m_bSparkEffectOn=false;
	m_dwSparkEffectStartTime=0;

	m_pOgreMaxScene=NULL;
	m_SmokeMgr = 0;
	m_WaterMgr = NULL;
	m_BubbleManager = 0;
}


EffectManager::~EffectManager()
{

}

void EffectManager::BeginRendOneFrame(float dt)
{
	DWORD dwCurrentTime = GetTickCount();
	
	// update particle

	// check arrow tick count
	dwCurrentTime = GetTickCount();
	if (m_nArrowOnTime != -1 && dwCurrentTime - m_dwArrowStartTime > m_nArrowOnTime * 1000) // 0.8 sec
	{
		HideArrow();
	}

	if(m_SmokeMgr)
	{
	   m_SmokeMgr->update(dt);
	}

	if(m_WaterMgr)
	{
	   m_WaterMgr->update(dt);
	}
	
	if (m_BubbleManager)
	{
		m_BubbleManager->BeginRendOneFrame(dt);
	}
	if(m_bSparkEffectOn && dwCurrentTime - m_dwSparkEffectStartTime > 500)
		SparkEffectOff();

}

void EffectManager::CreateSmokeManager()
{
	if(!m_SmokeMgr)
	{
	   m_SmokeMgr = new SmokeManager();
	}
}

void EffectManager::CreateWaterManager()
{
	if(!m_WaterMgr)
	{
	   m_WaterMgr = new WaterManager();
	}
}

void EffectManager::CreateBubbleManager()
{
	if (!m_BubbleManager)
	{
		m_BubbleManager = new BubbleManager();
	}
}
void EffectManager::DestoryBubbleManager()
{
	if (m_BubbleManager)
	{
		delete m_BubbleManager;
		m_BubbleManager = 0;
	}
}


void EffectManager::DestorySmokeManager()
{
	if(m_SmokeMgr)
	{
		delete m_SmokeMgr;
		m_SmokeMgr = 0;
	}
}

void EffectManager::DestoryWaterManager()
{
	if(m_WaterMgr)
	{
		delete m_WaterMgr;
		m_WaterMgr = NULL;
	}
}
BubbleManager * EffectManager::GetBubbleManager()
{
	return m_BubbleManager;
}
SmokeManager * EffectManager::GetSmokeManager()
{
	return m_SmokeMgr;
}

WaterManager * EffectManager::GetWaterManager()
{
	return m_WaterMgr;
}

void EffectManager::ShowArrow(Ogre::Vector2 v2ScreenPos, int nShowSeconds, Ogre::String strTransitionName/* = ""*/, bool bForceCreateNew/* = false*/)
{
	m_nArrowOnTime = nShowSeconds;
	if (m_bArrowOn && !bForceCreateNew) 
	{
		m_dwArrowStartTime = GetTickCount();
		return;
	}

	m_bArrowOn = true;
	m_dwArrowStartTime = GetTickCount();

	CScreenEffect::Instance()->ShowImage("Arrow", v2ScreenPos, Ogre::Angle(0));	
	m_strArrowTransitionName = strTransitionName;
}

void EffectManager::ShowArrow(Ogre::Vector3 v3TargetPos, Ogre::Vector2 v2ScreenOffset, Ogre::Camera * pCamera, int nShowSeconds,  Ogre::String strTransitionName/* = ""*/, bool bForceCreateNew/* = false*/)
{
	m_nArrowOnTime = nShowSeconds;
	if (m_bArrowOn && !bForceCreateNew) 
	{
		m_dwArrowStartTime = GetTickCount();
		return;
	}

	m_bArrowOn = true;
	m_dwArrowStartTime = GetTickCount();

	Ogre::Vector2 v2TargetPos = CScreenEffect::Instance()->ScreenPosFrom3DPoints(v3TargetPos, pCamera);
	Ogre::Vector2 v2ScreenPos = v2TargetPos + v2ScreenOffset;

	Ogre::Vector2 v2DirScreenPos2TargetPos = v2TargetPos - v2ScreenPos;
	Ogre::Vector2 v2DirLeft(-1,0);

	Ogre::Radian fRadian = Ogre::Math::ACos(v2DirScreenPos2TargetPos.dotProduct(v2DirLeft) / (v2DirScreenPos2TargetPos.length() * v2DirLeft.length()));
	float fAngle = fRadian.valueDegrees();

	if (v2TargetPos.y > v2ScreenPos.y)
	{
		fAngle = -fAngle;
	}

	CScreenEffect::Instance()->ShowImage("Arrow", v2ScreenPos, Ogre::Angle(fAngle));

	m_strArrowTransitionName = strTransitionName;
}

void EffectManager::ShowFixedArrow(Ogre::Vector3 v3TargetPos, Ogre::Vector2 v2ScreenPos, Ogre::Camera * pCamera, int nShowSeconds, Ogre::String strTransitionName /*= ""*/, bool bForceCreateNew /*= false*/)
{
	m_nArrowOnTime = nShowSeconds;
	if (m_bArrowOn && !bForceCreateNew) 
	{
		m_dwArrowStartTime = GetTickCount();
		return;
	}

	m_bArrowOn = true;
	m_dwArrowStartTime = GetTickCount();

	Ogre::Vector2 v2TargetPos = CScreenEffect::Instance()->ScreenPosFrom3DPoints(v3TargetPos, pCamera);

	Ogre::Vector2 v2DirScreenPos2TargetPos = v2TargetPos - v2ScreenPos;
	Ogre::Vector2 v2DirLeft(-1,0);

	Ogre::Radian fRadian = Ogre::Math::ACos(v2DirScreenPos2TargetPos.dotProduct(v2DirLeft) / (v2DirScreenPos2TargetPos.length() * v2DirLeft.length()));
	float fAngle = fRadian.valueDegrees();

	if (v2TargetPos.y > v2ScreenPos.y)
	{
		fAngle = -fAngle;
	}

	CScreenEffect::Instance()->ShowImage("Arrow", v2ScreenPos, Ogre::Angle(fAngle));

	m_strArrowTransitionName = strTransitionName;
}

void EffectManager::HideArrow()
{
	if (m_bArrowOn)
	{
		m_bArrowOn = false;
	}
	else
	{
		return;
	}
	CScreenEffect::Instance()->HideImage("PicOverlay/Arrow");	
}

void EffectManager::ClearArrow()
{
	if (m_bArrowOn)
	{
		m_bArrowOn = false;
	}
	else
	{
		return;
	}
	CScreenEffect::Instance()->ClearImage("PicOverlay/Arrow");	
}

void EffectManager::ShowFloatWindowText(const std::wstring & strText, TipInfo::TipPosition enmTipPositioin, int nLastSeconds /*= -1*/, int nDelaySeconds /*= 0*/, TipInfo::TipIconType eIconType /*= TIT_INFO*/, int nCustomPosX /*= 0*/, int nCustomPosY /*= 0*/)
{
	TipInfo tipInfo;
	tipInfo.str = QString::fromWCharArray(strText.c_str());
	tipInfo.pos = enmTipPositioin;
	tipInfo.msDelay = nDelaySeconds;
	tipInfo.msLast = 1000*nLastSeconds/*nLastSeconds*/;
	tipInfo.eIconType = eIconType;
	tipInfo.nCustomPosX = nCustomPosX;
	tipInfo.nCustomPosY = nCustomPosY;

	Inception::Instance()->EmitShowTip(tipInfo);		
}

// 粒子烟效 [3/22/2012 yl]
/*
void EffectManager::SmokeEffectOn(const Ogre::Vector3 & vec)
{
		if (m_bSmokeEffectOn)
			return;

		m_bSmokeEffectOn = true;
		m_dwSmokeEffectStartTime = GetTickCount();

		Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
		m_smokeNode = pSceneManager->getSceneNode("smokeNode");
		m_smokeParicle = pSceneManager->getParticleSystem("smoke");

		//修改粒子方向跟随相机角度
		Ogre::ParticleEmitter *particleEmit = m_smokeParicle->getEmitter(0);
		
		Ogre::Camera *pCamera = pSceneManager->getCamera("Camera001$1");
		
		Ogre::Vector3 direction = pCamera->getDirection();
		
		Ogre::Vector3 tmpDir(-direction.x,-direction.y,-direction.z);
		particleEmit->setDirection(direction);

		if (m_smokeNode&&m_smokeParicle)
		{
			m_smokeNode->setPosition(vec);
			m_smokeParicle->setEmitting(true);	
		}
}

void EffectManager::SmokeEffectOff()
{
		if (m_bSmokeEffectOn)
			m_bSmokeEffectOn = false;
		else
			return;

		m_dwSmokeEffectStartTime=0;

		if (m_smokeParicle)
			m_smokeParicle->setEmitting(false);
}
*/
Ogre::Pass * EffectManager::GetMaterialPass(Ogre::MaterialPtr  mat , int techid , int passid)
{
       if(mat.isNull())
		   return 0;
	  
	   if(mat->getNumTechniques() <= techid) 
		   return 0;
	   
	   Ogre::Technique * tech = mat->getTechnique(techid);
	  
	   if(tech && passid < tech->getNumPasses())
			return tech->getPass(passid);
	   else
		   return 0;
}
Ogre::TextureUnitState * GetMaterialTexUnit(Ogre::MaterialPtr  mat , int techid , int passid,int texid)
{
		Ogre::Pass * pass = EffectManager::GetMaterialPass(mat,  techid ,  passid);
		if (pass)
		{
			if( texid < pass->getNumTextureUnitStates())
			{
				return pass->getTextureUnitState(texid);
			}
		}
		return 0;
}
void EffectManager::SetNodeTransparent(const std::string &meterialName,int nAlpha)
{
		Ogre::MaterialPtr meterialPtr = Ogre::MaterialManager::getSingleton().getByName(meterialName);
		
		Ogre::Pass *pass = EffectManager::GetMaterialPass(meterialPtr , 0 , 0);//meterialPtr->getTechnique(0)->getPass(0);
		
		if (pass)
		{
			pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
			pass->setDepthWriteEnabled(false);
			pass->setDiffuse(1.0,1.0,1.0,(float)nAlpha/255);	
		}
}


bool EffectManager::MoveCamera( Ogre::String cameraName, Ogre::int32 nCushion )
{
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
	if (!pSceneManager->getSceneNode( cameraName ))
	{
		return false;
	}

	Inception::Instance()->m_strTargetCameraName = cameraName;
	m_nCushion = nCushion;

	if(m_enmCameraType == CT_FLASH)
	{
		m_bUpdateFlashCamera = false;
	}

	return true;
}

void EffectManager::UpdateCamera( OgreMax::OgreMaxScene *pOms )
{
	if (m_enmCameraType == CT_SMOOTH || m_enmCameraType == CT_NORMAL)
	{
		if (m_nCushion)
		{
			m_nCushion--;

			Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();		
			Ogre::Node *pCamera001 = pSceneManager->getSceneNode("Camera001$1");
			Ogre::Node *pCameraTarget = pSceneManager->getSceneNode(Inception::Instance()->m_strTargetCameraName);

			if (pCamera001 && pCameraTarget)
			{
				Ogre::Quaternion quat = pCameraTarget->getOrientation() - pCamera001->getOrientation();
				quat.w /= 10;
				quat.x /= 10;
				quat.y /= 10;
				quat.z /= 10;
				pCamera001->setOrientation(pCamera001->getOrientation() + quat);
				pCamera001->setPosition( pCamera001->getPosition() + (pCameraTarget->getPosition() - pCamera001->getPosition()) / 10 );
			}
		}
	}
	else if (m_enmCameraType == CT_FLASH)
	{
		if (m_bUpdateFlashCamera == false)
		{
			Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();		
			Ogre::Node *pCamera001 = pSceneManager->getSceneNode("Camera001$1");
			Ogre::Node *pCameraTarget = pSceneManager->getSceneNode(Inception::Instance()->m_strTargetCameraName);

			pCamera001->setOrientation(pCameraTarget->getOrientation());
			pCamera001->setPosition( pCameraTarget->getPosition());

			m_bUpdateFlashCamera = true;
		}		
	}
}



void EffectManager::DrawOgreAxis(float mlength,float mTipOffset)
{
	Ogre::SceneManager * mSceneMgr = MXOgreWrapper::Get()->GetDefaultSceneManger();
	Ogre::ManualObject* pManualObject=mSceneMgr->createManualObject("OgreAxisManualObject"); 
	if(NULL==pManualObject) 
		return; 
	pManualObject-> begin("BaseWhiteNoLighting",   Ogre::RenderOperation::OT_LINE_LIST); 
	{ 
		//x
		pManualObject-> position(0,0,0); 
		pManualObject-> colour(Ogre::ColourValue::Red);
		pManualObject-> position(mlength,0,0); 
		pManualObject-> colour(Ogre::ColourValue::Red); 
		pManualObject-> position(mlength,0,0); 
		pManualObject-> colour(Ogre::ColourValue::Red); 
		pManualObject-> position(mlength-mTipOffset,-mTipOffset,0); 
		pManualObject-> colour(Ogre::ColourValue::Red); 
		pManualObject-> position(mlength,0,0); 
		pManualObject-> colour(Ogre::ColourValue::Red); 
		pManualObject-> position(mlength-mTipOffset,-mTipOffset,0); 
		pManualObject-> colour(Ogre::ColourValue::Red); 

		//y
		pManualObject-> position(0,0,0); 
		pManualObject-> colour(Ogre::ColourValue::Green); 
		pManualObject-> position(0,mlength,0); 
		pManualObject-> colour(Ogre::ColourValue::Green); 
		pManualObject-> position(0,mlength,0); 
		pManualObject-> colour(Ogre::ColourValue::Green); 
		pManualObject-> position(mTipOffset,mlength-mTipOffset,0); 
		pManualObject-> colour(Ogre::ColourValue::Green); 
		pManualObject-> position(0,mlength,0); 
		pManualObject-> colour(Ogre::ColourValue::Green); 
		pManualObject-> position(-mTipOffset,mlength-mTipOffset,0); 
		pManualObject-> colour(Ogre::ColourValue::Green); 

		//z
		pManualObject-> position(0,0,0); 
		pManualObject-> colour(Ogre::ColourValue::Blue); 
		pManualObject-> position(0,0,mlength); 
		pManualObject-> colour(Ogre::ColourValue::Blue); 
		pManualObject-> position(0,0,mlength); 
		pManualObject-> colour(Ogre::ColourValue::Blue); 
		pManualObject-> position(0,mTipOffset,mlength-mTipOffset); 
		pManualObject-> colour(Ogre::ColourValue::Blue); 
		pManualObject-> position(0,0,mlength); 
		pManualObject-> colour(Ogre::ColourValue::Blue); 
		pManualObject-> position(0,-mTipOffset,mlength-mTipOffset); 
		pManualObject-> colour(Ogre::ColourValue::Blue); 
	} 
	pManualObject-> end(); 
	Ogre::SceneNode* pSceneNode=mSceneMgr->getRootSceneNode()->createChildSceneNode("OgreAxisSceneNode"); 
	if(NULL == pSceneNode) return ; 
	pSceneNode->attachObject(pManualObject);
}


void EffectManager::SparkEffectOn(const Ogre::Vector3 & vec  , bool needRand )
{
// 	if (m_bSparkEffectOn)
// 	{
// 		m_dwSparkEffectStartTime=GetTickCount();
// 		return;
// 	}

	if (!m_bSparkEffectOn)
	{
		m_dwSparkEffectStartTime=GetTickCount();
		m_bSparkEffectOn=true;
	}

	if(needRand)
	{
		int num=rand()%RANDNUM;
		if (num !=RANDNUM-1)
		{
			return;
		}
	}
	
// 	m_bSparkEffectOn=true;
// 	m_dwSparkEffectStartTime=GetTickCount();

	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
	if (NULL==pSceneManager)
	{
		return;
	}

	m_pSparkNode=pSceneManager->getSceneNode("sparkNode");
	m_pSparkParticle=pSceneManager->getParticleSystem("spark");

	if (m_pSparkNode)
	{
		m_pSparkNode->setVisible(true);
		m_pSparkNode->setPosition(vec);
		//m_pSparkParticle->setMaterialName("MatEffect/Spark");
		//m_pSparkParticle->setEmitting(true);
		//m_pSparkParticle->setVisible(true);
		//m_pSparkParticle->fastForward(0.4f);
		//m_pSparkParticle->getParentNode()->setPosition(vec);

		//if(m_pSparkParticle->getNumParticles() > 0)
		//{
			//Ogre::Particle * par = m_pSparkParticle->getParticle(0);
			//par->position = vec;
		//}
	
		//m_pSparkParticle->getParentNode()->setPosition(vec);
		//m_pSparkParticle->fastForward(0.4f);
		//m_pSparkNode->setPosition(vec);
	}
}

void EffectManager::SparkEffectOff()
{
	if (m_bSparkEffectOn)
	{
		m_bSparkEffectOn=false;
	}
	else
	{
		return;
	}

	m_dwSparkEffectStartTime=0;


	if(m_pSparkNode)
		m_pSparkNode->setVisible(false);

}


void EffectManager::SetMaterialColour( const Ogre::String & MaterialName,Ogre::Real alpha,Ogre::LayerBlendSource source1, Ogre::LayerBlendSource source2, const Ogre::ColourValue& arg1, const Ogre::ColourValue& arg2)
{
    if(!Ogre::MaterialManager::getSingleton().resourceExists(MaterialName))  return;
	
	//Ogre::MaterialPtr meterialPtr=Ogre::MaterialManager::getSingleton().getByName(MaterialName);
	
	Ogre::GpuProgramParametersSharedPtr ShaderPtr = GetShaderParamterPtr(MaterialName, FRAGMENT_PROGRAME, 0 , 0);

	if(ShaderPtr.isNull() == false)
	{
		if (ShaderPtr->_findNamedConstantDefinition("customColor", false))
		{
			Ogre::ColourValue color = arg1;
			color.a = alpha;
			Ogre::Vector4 colorfloat4;
			colorfloat4[0] = color.r;
			colorfloat4[1] = color.g;
			colorfloat4[2] = color.b;
			colorfloat4[3] = color.a;

			ShaderPtr->setNamedConstant("customColor", colorfloat4);
		}
	}
	//Ogre::Pass * warnPass = meterialPtr->getTechnique(0)->getPass(0);
	//warnPass->setDepthWriteEnabled(false);
	//warnPass->getTextureUnitState(0)->setColourOperationEx(Ogre::LBX_MODULATE,source1,source2,arg1,arg2,1.0);
	//warnPass->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE, 
	//	Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL,1.0, alpha);
}

void EffectManager::SetMaterialAmbient( const Ogre::String & MaterialName,const Ogre::ColourValue& newColor)
{
	if(!Ogre::MaterialManager::getSingleton().resourceExists(MaterialName))  return;
	Ogre::MaterialPtr meterialPtr=Ogre::MaterialManager::getSingleton().getByName(MaterialName);
	Ogre::Pass * warnPass = meterialPtr->getTechnique(0)->getPass(0);
	warnPass->setAmbient(newColor);
}

void EffectManager::SetMaterialDiffuse( const Ogre::String & MaterialName,const Ogre::ColourValue& newColor)
{
	if(!Ogre::MaterialManager::getSingleton().resourceExists(MaterialName))  return;
	Ogre::MaterialPtr meterialPtr=Ogre::MaterialManager::getSingleton().getByName(MaterialName);
	Ogre::Pass * warnPass = meterialPtr->getTechnique(0)->getPass(0);
	warnPass->setDiffuse(newColor);
}

void EffectManager::DrawOgreAxisByMesh(Ogre::Vector3 v3Pos)
{
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
	Ogre::Entity* axisEnt=pSceneManager->createEntity("axes.mesh");
	axisEnt->setMaterialName("axes");
	Ogre::SceneNode* MeshNode=pSceneManager->getRootSceneNode()->createChildSceneNode("axes");
	MeshNode->attachObject(axisEnt);
	MeshNode->setPosition(v3Pos);
}

void EffectManager::DrawLightPosByMesh(Ogre::Vector3 v3Pos,Ogre::Quaternion quatDir)
{
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
	bool flag=pSceneManager->hasEntity("LightArrow");
	Ogre::Entity* LightEnt=NULL;
	if (flag)
	{
		LightEnt=pSceneManager->getEntity("LightArrow");
	}
	else
	{
		LightEnt=pSceneManager->createEntity("LightArrow","LightArrow.mesh");
	}
	
	LightEnt->setMaterialName("LightArrow");
	Ogre::SceneNode* lightMeshNode=pSceneManager->getRootSceneNode()->createChildSceneNode();
	lightMeshNode->attachObject(LightEnt);
	lightMeshNode->setPosition(v3Pos);
	lightMeshNode->setOrientation(quatDir);
	lightMeshNode->setScale(0.5,0.5,0.5);
}

OgreMax::OgreMaxScene* EffectManager::GetOgreMaxScene()
{
	return m_pOgreMaxScene;
}

void EffectManager::SetOgreMaxScene(OgreMax::OgreMaxScene* pOgreMaxScene)
{
	m_pOgreMaxScene=pOgreMaxScene;
}

CVesselBleedEffect* EffectManager::createVesselBleedEffect(MisMedicOrganInterface * organif)
{
 	CVesselBleedEffect* pNewEffect = new CVesselBleedEffect(organif);
	m_EffectList.push_back(pNewEffect);

	return pNewEffect;

}

void EffectManager::removeVesselBleedEffect(CVesselBleedEffect* eff)
{
	m_EffectList.remove(eff);
	SAFE_DELETE(eff);
}

void EffectManager::createVesselBleedEffect(Ogre::Vector3 bleedpos)
{
// 	CVesselBleedEffect* pNewEffect = new CVesselBleedEffect(organif);


}