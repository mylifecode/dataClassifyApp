#pragma once
#include "Ogre.h"
#include <map>
#include "MXCommon.h"

typedef std::pair<Ogre::String,Ogre::Vector3> GPointListPair;
typedef std::map<Ogre::String,Ogre::Vector3>::iterator GPointListIt;

class MXCOMMON_API Line3D
{
public:
	Line3D();
	Line3D(Ogre::SceneManager* smg, int index);
	~Line3D(void);

public:
	void InitLine3D(Ogre::SceneManager* smg, int index);
	void DrawLine(Ogre::Vector3 color = Ogre::Vector3::UNIT_SCALE);
	void LinkPoint(Ogre::String name ="point",Ogre::Vector3 v3=Ogre::Vector3::ZERO);
	void DeletePoint(Ogre::String name ="point");
	void ClearPoint();
private:
	std::map<Ogre::String,Ogre::Vector3> mPointList;
	Ogre::ManualObject* mObjmanual;
	Ogre::SceneManager* mSmg;

public:
	int mLine3DlIndex;
};
