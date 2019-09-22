#pragma once
#include "IMXDefine.h"
#include <vector>
#include <Ogre.h>
#include <OgreFontManager.h>
#include <OgreBorderPanelOverlayElement.h>
#include <OgreTextAreaOverlayElement.h>
#include <math.h>
#include <OgreOverlay.h>
#include "Singleton.h"


class MisNewTraining;
class ITraining;

class SimpleUIEvent
{
public:
	enum EventAction
	{
		eEA_Empty,
		eEA_MousePressed,
		eEA_MouseReleased,
		//todo
	};
	SimpleUIEvent(Ogre::OverlayElement *pElement = NULL , EventAction action = eEA_Empty) : m_pEventElement(pElement) , m_EventAction(action) {}
	Ogre::OverlayElement * m_pEventElement;
	EventAction m_EventAction;
};


class CSimpleUIManger  : public CSingleT<CSimpleUIManger>
{
public:
	CSimpleUIManger();

	~CSimpleUIManger();

	void OnTrainingBeInitialized(ITraining *pTraining , const Ogre::String & SimpleUIName);

	void ShowUI() { if(m_pOverlay) m_pOverlay->show(); }
	
	void Clear();

	void OnMousePressed(char button , int x , int y);

	void OnMouseReleased(char button , int x , int y);

	Ogre::OverlayElement * GetElement(const Ogre::String &elementName);

private:
	Ogre::OverlayElement * FindElementWithPos(int x , int y);

	Ogre::OverlayElement * FindeElementWithName(Ogre::OverlayElement *  pElement  ,const Ogre::String & name);

	Ogre::OverlayElement * TestElementWithPos(Ogre::OverlayElement *  pElement , float  x, float y);

	bool isCursorOver(Ogre::OverlayElement* element, float x , float y, Ogre::Real voidBorder = 0);


	ITraining *m_pCurrTraining;
	Ogre::Overlay * m_pOverlay;
};