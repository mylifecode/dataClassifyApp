#include "SimpleUI.h"
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayContainer.h>
#include "MXOgreWrapper.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "Inception.h"
#include "EffectManager.h"
#include <OgreOverlay.h>
#include "MisNewTraining.h"


CSimpleUIManger::CSimpleUIManger()
:	m_pCurrTraining(NULL),
	m_pOverlay(NULL)
{

}

CSimpleUIManger::~CSimpleUIManger()
{
	Clear();
}

void CSimpleUIManger::OnTrainingBeInitialized(ITraining *pTraining , const Ogre::String & SimpleUIName)
{
	m_pCurrTraining = pTraining;

	m_pOverlay = Ogre::OverlayManager::getSingleton().getByName(SimpleUIName);

}
void CSimpleUIManger::Clear()
{
	m_pCurrTraining = NULL;
	if(m_pOverlay)
		Ogre::OverlayManager::getSingleton().destroy(m_pOverlay);
	m_pOverlay = NULL;
}

void CSimpleUIManger::OnMousePressed(char button , int x , int y)
{
	if(m_pOverlay && m_pOverlay->isVisible())
	{
		Ogre::OverlayElement * pResult = FindElementWithPos(x,y);
		if(m_pCurrTraining && pResult)
		{
			SimpleUIEvent event(pResult , SimpleUIEvent::eEA_MousePressed);
			m_pCurrTraining->OnSimpleUIEvent(event);
		}
	}
}

void CSimpleUIManger::OnMouseReleased(char button , int x , int y)
{
	if(m_pOverlay && m_pOverlay->isVisible())
	{
		Ogre::OverlayElement * pResult = FindElementWithPos(x,y);
		if(m_pCurrTraining && pResult)
		{
			SimpleUIEvent event (pResult , SimpleUIEvent::eEA_MouseReleased);
			m_pCurrTraining->OnSimpleUIEvent(event);
		}
	}
}

Ogre::OverlayElement * CSimpleUIManger::GetElement(const Ogre::String &elementName)
{
	Ogre::OverlayElement *pResult = NULL;
	if(m_pOverlay)
	{
		Ogre::OverlayContainer * pContainer = m_pOverlay->getChild(elementName);
		if(pContainer)
			return pContainer;
		else
		{
			Ogre::Overlay::Overlay2DElementsIterator  ContainerItor = m_pOverlay->get2DElementsIterator();
			while(ContainerItor.hasMoreElements() && pResult == NULL)
			{
				Ogre::OverlayContainer* pContainer = ContainerItor.getNext();
				pResult = FindeElementWithName(pContainer , elementName);
			}
		}
	}
	return pResult;
}

Ogre::OverlayElement * CSimpleUIManger::FindElementWithPos(int x , int y)
{
	Ogre::OverlayManager& om = Ogre::OverlayManager::getSingleton();
	int width = om.getViewportWidth();
	float posx =  (float)x / (float)om.getViewportWidth();
	float posy =  (float)y / (float)om.getViewportHeight();

	Ogre::OverlayElement *pResult = NULL;
	if(m_pOverlay)
	{
		Ogre::Overlay::Overlay2DElementsIterator  ContainerItor = m_pOverlay->get2DElementsIterator();
		while(ContainerItor.hasMoreElements() && pResult == NULL)
		{
			Ogre::OverlayContainer* pContainer = ContainerItor.getNext();
			pResult = TestElementWithPos(pContainer , posx ,posy);
		}
	}
	return pResult;
}

Ogre::OverlayElement * CSimpleUIManger::FindeElementWithName(Ogre::OverlayElement * pElement ,const Ogre::String & name)
{
	Ogre::OverlayElement *pResult = NULL;
	if(pElement->getName() == name)
		pResult = pElement;
	else
	{
		Ogre::OverlayContainer* pContainer = dynamic_cast<Ogre::OverlayContainer*>(pElement);
		if (pContainer)
		{
			pResult = pContainer->getChild(name);

			if(!pResult)
			{
				std::vector<Ogre::OverlayElement*> toTest;

				Ogre::OverlayContainer::ChildIterator children = pContainer->getChildIterator();

				while (children.hasMoreElements())
				{
					toTest.push_back(children.getNext());
				}

				for (unsigned int i = 0; i < toTest.size(); i++)
				{
					pResult = FindeElementWithName(toTest[i] , name);
					if(pResult)
						return pResult;
				}
			}
		}
	}
	return pResult;
}

Ogre::OverlayElement * CSimpleUIManger::TestElementWithPos(Ogre::OverlayElement * pElement , float x , float y)
{
	Ogre::OverlayElement *pResult = NULL;
	Ogre::OverlayContainer* pContainer = dynamic_cast<Ogre::OverlayContainer*>(pElement);
	if (pContainer)
	{
		std::vector<Ogre::OverlayElement*> toTest;

		Ogre::OverlayContainer::ChildIterator children = pContainer->getChildIterator();

		while (children.hasMoreElements())
		{
			toTest.push_back(children.getNext());
		}

		for (unsigned int i = 0; i < toTest.size(); i++)
		{
			pResult = TestElementWithPos(toTest[i] , x , y);
			if(pResult)
				return pResult;
		}
	}
	if (pElement && pResult == NULL)
	{
		if(isCursorOver(pElement ,  x , y))
			pResult = pElement;
	}
	return pResult;
}

bool CSimpleUIManger::isCursorOver(Ogre::OverlayElement* element, float posX , float posY, Ogre::Real voidBorder /* = 0 */)
{
	Ogre::OverlayManager& om = Ogre::OverlayManager::getSingleton();

	Ogre::Real l = element->_getDerivedLeft();
	Ogre::Real t = element->_getDerivedTop();
	Ogre::Real r = l + element->getWidth();
	Ogre::Real b = t + element->getHeight();

	return (posX >= l + voidBorder && posX <= r - voidBorder &&
			posY >= t + voidBorder && posY <= b - voidBorder);
}