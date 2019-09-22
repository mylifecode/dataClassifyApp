#include "TrainingMgr.h"
#include "ResourceManager.h"
#include "Inception.h"
#include "BasicTraining.h"
#include "XMLWrapperTraining.h"
#include "MisRobotInput.h"
#include <QFile>
CTrainingMgr::CTrainingMgr(void):m_curTraining(NULL)
{
	map_NameTraining.clear();
}

CTrainingMgr::~CTrainingMgr(void)
{
	map_NameTraining.clear();
}

void CTrainingMgr::AddTraining( Ogre::String strName, ITraining *pCurTraining )
{
	map_NameTraining[strName] = pCurTraining;
}

void CTrainingMgr::RemoveTraining( Ogre::String strName )
{
	MAP_NAME_TRAINING::iterator it = map_NameTraining.find(strName);
	if (it != map_NameTraining.end())
	{
		map_NameTraining.erase(strName);
	}
}

void CTrainingMgr::SetCurTraining( ITraining *curTraining )
{
	if(curTraining !=  NULL)
	{
		m_curTraining = curTraining;
	}
}

void CTrainingMgr::SetCurTraining( Ogre::String strName )
{
	MAP_NAME_TRAINING::iterator it = map_NameTraining.find(strName);
	if (it != map_NameTraining.end())
	{
		if (it->second != NULL)
		{
			SetCurTraining(it->second);
		}
	}
}

void CTrainingMgr::Update(float dt)
{
	if (m_curTraining != NULL)
	{
		m_curTraining->Update(dt);
	}
}

bool CTrainingMgr::Initialize()
{
	//MAP_NAME_TRAINING::iterator it = map_NameTraining.begin();
	//while(it != map_NameTraining.end())
	//{
	//	it->second->Initialize();
	//	++it;
	//}

	//load default training
	CResourceManager::Instance()->LoadTraining(this);
	Ogre::LogManager::getSingletonPtr()->logMessage("*** CurTraining***" + Inception::Instance()->m_strTrainingName);
	this->SetCurTraining(Inception::Instance()->m_strTrainingName);
	return true;
}

ITraining * CTrainingMgr::GetTrainingByName( const Ogre::String& strName )
{
	if (!map_NameTraining.empty())
	{
		MAP_NAME_TRAINING::iterator it = map_NameTraining.find(strName);
		if (it != map_NameTraining.end())
		{
			return it->second;
		}
		return NULL;
	}
	return NULL;
}

bool CTrainingMgr::Terminate()
{
	MAP_NAME_TRAINING::iterator it = map_NameTraining.begin();
	while (it != map_NameTraining.end())
	{
		it->second->OnSaveTrainingReport();
		delete it->second;
		it = map_NameTraining.erase(it);
	}
	m_curTraining = NULL;
	return true;
}