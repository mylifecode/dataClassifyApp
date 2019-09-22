
#include "ScreenEffect.h"
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayContainer.h>
#include "MXOgreWrapper.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "Inception.h"
#include "EffectManager.h"

CScreenEffect::CScreenEffect()
{
	m_pOverlay = NULL;
	m_pLines = NULL;
	m_bWarnState = false;
	m_bQuit = false;
	m_WarnTick = 0;
	m_bGradual = false;
	m_bChangeScreenColor = false;
	m_WarnLastTime = 0;
	m_v4ScreenColor = m_WarnColor = Ogre::ColourValue(0,0,0,0);
	m_fStartChangeColorTick = 0;
	m_fColorChangeKeepTime = 0;
}

CScreenEffect::~CScreenEffect()
{
	//remove used overlay
	HideImage("PicOverlay/RedScreenWarn");
	HideImage("PicOverlay/SightCamera");
	HideImage("PicOverlay/SightCamera_Red");
	HideImage("FontOverlay/CameraNumber");
}

void CScreenEffect::HideAllOverLays()
{
	HideImage("PicOverlay/RedScreenWarn");
	HideImage("PicOverlay/SightCamera");
	HideImage("PicOverlay/SightCamera_Red");
	HideImage("FontOverlay/CameraNumber");
}

void CScreenEffect::ShowImage(Ogre::String name)
{
	m_pOverlay = Ogre::OverlayManager::getSingleton().getByName(name);
	if (m_pOverlay)
	{
		m_pOverlay->show();
	}
}

void CScreenEffect::HideImage(Ogre::String name)
{
	m_pOverlay = Ogre::OverlayManager::getSingleton().getByName(name);
	if (m_pOverlay)
	{
		m_pOverlay->hide();
	}
}

void CScreenEffect::ClearImage(Ogre::String name)
{
	m_pOverlay = Ogre::OverlayManager::getSingleton().getByName(name);
	if (m_pOverlay)
	{
		m_pOverlay->clear();
	}	
}

void CScreenEffect::RemoveLine()
{
	if (!m_pLines)
	{
		return;
	}	
	//m_pLines->

}

Ogre::Vector2 CScreenEffect::ScreenPosFrom3DPoints(const Ogre::Vector3 & v3TargetPos, const Ogre::Camera * const pCamera)
{
	Ogre::Matrix4 matView = pCamera->getViewMatrix();
	Ogre::Matrix4 matProj = pCamera->getProjectionMatrix();
	Ogre::Vector3 v3ScreenPos = matProj * matView * v3TargetPos;
	//[-1,1]
	//float fX = (v3ScreenPos.x + 1.0f) / 2.0f;
	//float fY = -1 * (v3ScreenPos.y - 1.0f) / 2.0f;
	return Ogre::Vector2(v3ScreenPos.x, v3ScreenPos.y);
}

void CScreenEffect::ShowNumber(Ogre::String strNumber)
{
	m_pOverlay= Ogre::OverlayManager::getSingleton().getByName("FontOverlay/Number");

	if (m_pOverlay)
	{
		m_pOverlay->show();
	}

	Ogre::OverlayElement* EleTime = Ogre::OverlayManager::getSingleton().getOverlayElement("Number");

	EleTime->setCaption(strNumber);

	//

	/*Ogre::MaterialPtr mat = EleTime->getMaterial();
	if(mat->getNumTechniques() > 0)
	{
		Ogre::Technique * FontTech = mat->getTechnique(0);
		
		Ogre::Pass * FontPass = FontTech->getPass(0);

		//this is a hack for font material since dx11 not support fix pipeline so we manually add shader
		//
		if(FontPass->hasVertexProgram() == false || FontPass->hasFragmentProgram() == false)
		{
			FontPass->setVertexProgram("FontShader_VS") ;

			FontPass->setFragmentProgram("FontShader_PS") ;
		}
	}*/
	//
}

void CScreenEffect::HideNumber()
{
	m_pOverlay= Ogre::OverlayManager::getSingleton().getByName("FontOverlay/Number");

	if (m_pOverlay)
	{
		m_pOverlay->hide();
	}
}

void CScreenEffect::ShowCameraNumber(Ogre::String strNumber)
{
	m_pOverlay= Ogre::OverlayManager::getSingleton().getByName("FontOverlay/CameraNumber");

	if (m_pOverlay)
	{
		m_pOverlay->show();
	}

	Ogre::OverlayElement* EleTime = Ogre::OverlayManager::getSingleton().getOverlayElement("CameraNumber");

	EleTime->setCaption(strNumber);
}

// v2ScreenPos: -1~1, may come from function ScreenPosFrom3DPoints
void CScreenEffect::ShowImage(Ogre::String strName, Ogre::Vector2 v2ScreenPos , Ogre::Angle angle)
{
	Ogre::String strOverlayName = "PicOverlay/" + strName;
	Ogre::String strContainerName = "PicOverlayElement/" + strName;
	Ogre::SceneManager * pManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();
	Ogre::SceneNode * pRoot = MXOgreWrapper::Instance()->GetDefaultSceneManger()->getRootSceneNode();
	Ogre::RenderWindow * pWindow = MXOgreWrapper::Instance()->GetRenderWindowByName(RENDER_WINDOW_LARGE);
	int nWidth = pWindow->getWidth();
	int nHeight = pWindow->getHeight();

	m_pOverlay= Ogre::OverlayManager::getSingleton().getByName(strOverlayName);
	m_pOverlay->hide();

	if (m_pOverlay)
	{
		m_pOverlay->setRotate(angle);
		m_pOverlay->setScroll(v2ScreenPos.x, v2ScreenPos.y);
		m_pOverlay->show();
	}
}

void CScreenEffect::CheckWarn()
{
	if (!m_bWarnState)  return;

	static bool bFirstEnter = true;
	static Ogre::String  materialName = "PicMaterial/RedScreenWarn";
	if (bFirstEnter)
	{
		bFirstEnter = false;
		m_WarnTick = GetTickCount();
		ShowImage("PicOverlay/RedScreenWarn");
		Ogre::Real alpha = m_bGradual ? 0 : m_WarnColor.a;
		EffectManager::Instance()->SetMaterialColour(materialName,alpha,Ogre::LBS_MANUAL,
			Ogre::LBS_MANUAL,Ogre::ColourValue(1,0,0),Ogre::ColourValue(1,0,0));
	}
	else
	{
		Ogre::Real tick = GetTickCount()-m_WarnTick;

		if ( tick > m_WarnLastTime )
		{
			bFirstEnter = true;
			m_bWarnState = false;
			if (!m_bQuit)
			{
				HideImage("PicOverlay/RedScreenWarn");
			}
			else
			{         
				Inception::Instance()->EmitQuitScene();
			}	
			m_bQuit = false;
		}
		else
		{
			Ogre::Real alpha = m_bGradual ? (tick/m_WarnLastTime) : m_WarnColor.a;
			EffectManager::Instance()->SetMaterialColour(materialName,alpha,Ogre::LBS_MANUAL,
				Ogre::LBS_MANUAL,m_WarnColor,m_WarnColor);
		}
	}
}

void CScreenEffect::Warn(const Ogre::ColourValue & color ,Ogre::Real lastTime, bool bGradual , bool bQuit)
{
	if (m_bQuit)   return;

	m_bQuit = bQuit;
	m_WarnTick = GetTickCount();
	m_bWarnState = true;
	m_WarnColor = color;
	m_bGradual = bGradual;
	m_WarnLastTime = lastTime;
}

void CScreenEffect::QuitScene()
{
	Warn(Ogre::ColourValue(0,0,0) ,QUIT_TIME,true ,true);
}

void CScreenEffect::RedScreen()
{
	Warn();
}

void CScreenEffect::SetMaterialTransparent(const Ogre::String strMaterialName,float fAlpha,const Ogre::ColourValue color/* =Ogre::ColourValue */)
{
	Ogre::MaterialPtr meterialPtr=Ogre::MaterialManager::getSingleton().getByName(strMaterialName);
	Ogre::Pass *  warnPass = meterialPtr->getTechnique(0)->getPass(0);
	warnPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
	warnPass->setDepthWriteEnabled(false);
	warnPass->getTextureUnitState(0)->setColourOperationEx(
		Ogre::LBX_MODULATE,
		Ogre::LBS_MANUAL,
		Ogre::LBS_MANUAL,
		color,
		color,
		1.0);
	warnPass->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE, 
		Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL,1.0, fAlpha);
}

void CScreenEffect::ShowRedScreen(float fAlpha,const Ogre::String strOverLayName/* ="PicOverlay/RedScreenWarn" */,const Ogre::String strMaterialName/* ="PicMaterial/RedScreenWarn" */)
{
	ShowImage(strOverLayName);
	SetMaterialTransparent(strMaterialName,fAlpha);
}

void CScreenEffect::HideRedScreen(const Ogre::String strOverLayName/* ="PicOverlay/RedScreenWarn" */,const Ogre::String strMaterialName/* ="PicMaterial/RedScreenWarn" */)
{
	HideImage(strOverLayName);
	float fAlpha=0.8f;
	SetMaterialTransparent(strMaterialName,0.8f);
}

void CScreenEffect::UpdateScreenColor()
{
	if (m_bChangeScreenColor)
	{
		static Ogre::String  materialName = "PicMaterial/RedScreenWarn";
		static Ogre::String  overlayName = "PicOverlay/RedScreenWarn";

		Ogre::Real tick = GetTickCount() - m_fStartChangeColorTick;

		if ( tick > m_fColorChangeKeepTime )
		{
			m_bChangeScreenColor = false;
			HideImage(overlayName);
		}
		else
		{
			ShowImage(overlayName);

			EffectManager::Instance()->SetMaterialColour(materialName,m_v4ScreenColor.a,Ogre::LBS_MANUAL,
				Ogre::LBS_MANUAL,m_v4ScreenColor,m_v4ScreenColor);
		}
	}
}

void CScreenEffect::SetScreenColor(const Ogre::ColourValue & color ,Ogre::Real lastTime)
{
	m_fStartChangeColorTick = GetTickCount();

	m_bChangeScreenColor = true;

	m_v4ScreenColor = color;

	m_fColorChangeKeepTime = lastTime;
}