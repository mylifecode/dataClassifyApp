
#pragma once
#include "IMXDefine.h"
#include <vector>
using namespace::std;
#include <OgreOverlay.h>
#include "Singleton.h"

enum W_TYPE{ RED_T=0,QUIT_T};

#define  WARN_TIME 500
#define  QUIT_TIME 2000

class DynamicLines;
class CScreenEffect  : public CSingleT<CScreenEffect>
{
public:
	CScreenEffect();
	~CScreenEffect();
	void HideAllOverLays();
	// screen image routine, zx
	void ShowImage(Ogre::String strName, Ogre::Vector2 v2ScreenPos, Ogre::Angle angle);

	void ShowImage(Ogre::String name = "PicOverlay/FalsePic");
	void HideImage(Ogre::String name);
	void ClearImage(Ogre::String name);
	//void ShowText(int nX, int nY, Ogre::String strText);  // not need temporarily
	//void ShowLine(vector<Ogre::Vector2> vecPoints);
	void RemoveLine();

	void ShowNumber(Ogre::String strNumber);
	void HideNumber();
	void ShowCameraNumber(Ogre::String strNumber);

	// get screen position from 3d points
	Ogre::Vector2 ScreenPosFrom3DPoints(const Ogre::Vector3 & v3TargetPos, const Ogre::Camera * const pCamera);

	void CheckWarn();
	void Warn(const Ogre::ColourValue & color=Ogre::ColourValue(1,0,0,0.8f) ,Ogre::Real lastTime=WARN_TIME,bool bGradual=false,bool bQuit=false);
	void QuitScene();
	void RedScreen();

	void SetMaterialTransparent(const Ogre::String strMaterialName,float fAlpha,const Ogre::ColourValue color=Ogre::ColourValue(1,0,0,0.8f) );
	void ShowRedScreen(float fAlpha,const Ogre::String strOverLayName="PicOverlay/RedScreenWarn",const Ogre::String strMaterialName="PicMaterial/RedScreenWarn");
	void HideRedScreen(const Ogre::String strOverLayName="PicOverlay/RedScreenWarn",const Ogre::String strMaterialName="PicMaterial/RedScreenWarn");

	void UpdateScreenColor();
	void SetScreenColor(const Ogre::ColourValue & color=Ogre::ColourValue(1,0,0,0.8f) ,Ogre::Real lastTime=WARN_TIME);

	
private:
	Ogre::Overlay * m_pOverlay;
	DynamicLines * m_pLines;

	bool m_bWarnState;
	Ogre::Real m_WarnTick;
	Ogre::ColourValue m_WarnColor;
	bool m_bGradual;
	bool m_bQuit;
	Ogre::Real m_WarnLastTime;
	bool m_bChangeScreenColor;
	Ogre::Real m_fStartChangeColorTick;
	Ogre::ColourValue m_v4ScreenColor;
	Ogre::Real m_fColorChangeKeepTime;
};