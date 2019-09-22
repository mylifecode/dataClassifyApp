#pragma once

#include "ITraining.h"
#include "Singleton.h"

class CTrainingMgr:public CSingleT<CTrainingMgr>
{
public:
	CTrainingMgr(void);
	virtual ~CTrainingMgr(void);

public:
	bool Initialize();

	bool Terminate();

	void AddTraining(Ogre::String strName, ITraining *pCurTraining);

	void RemoveTraining(Ogre::String strName);

	ITraining * GetTrainingByName(const Ogre::String& strName);

	void SetCurTraining(Ogre::String strName);

	inline ITraining * GetCurTraining() const { return m_curTraining; }

	void Update(float dt);

//	void ShowDebugFrame();

protected:
	void SetCurTraining(ITraining *curTraining);

private:
	typedef std::map<Ogre::String, ITraining*> MAP_NAME_TRAINING;
	MAP_NAME_TRAINING map_NameTraining;

	ITraining *m_curTraining;
};
