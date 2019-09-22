#include "Line3D.h"

Line3D::Line3D():
mObjmanual(0),mSmg(0)
{
	mPointList.clear();
};

Line3D::Line3D(Ogre::SceneManager* smg, int index):mObjmanual(0),mSmg(0)
{
	mPointList.clear();
	InitLine3D(smg, index);
};
Line3D::~Line3D(void)
{

	mPointList.clear();
};

void Line3D::InitLine3D(Ogre::SceneManager* smg, int index)
{
	if (smg)
	{
		mSmg = smg;
		mLine3DlIndex = index;
		mPointList.clear();
	//	mObjmanual = mSmg->createManualObject("Menu"+Ogre::StringConverter::toString(mLine3DlIndex)); 
		mObjmanual = mSmg->createManualObject(); 
		mSmg->getRootSceneNode()->createChildSceneNode()->attachObject(mObjmanual);
	}
}

void Line3D::DrawLine(Ogre::Vector3 color)
{
	mObjmanual->clear();
	mObjmanual->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
	
	GPointListIt it;
	for (it=mPointList.begin(); it!= mPointList.end(); it++)
	{
		GPointListPair pl = (GPointListPair)(*it);
		mObjmanual->position(pl.second);
		mObjmanual->colour(color.x,color.y, color.z);
		mObjmanual->setMaterialName(0,"BaseWhiteNoLighting"/*"Examples/line"*/);
	}
	mObjmanual->end();
};

void Line3D::ClearPoint()
{
	mPointList.clear();
}

void Line3D::LinkPoint(Ogre::String name,Ogre::Vector3 v3)
{
	if (mPointList.find(name) != mPointList.end())
	{
		GPointListIt it = mPointList.find(name);
		it->second = v3;
	}else
	{
		mPointList.insert(GPointListPair(name, v3));
	}

};

void Line3D::DeletePoint(Ogre::String name)
{
	if (mPointList.find(name) != mPointList.end())
	{
		GPointListIt it = mPointList.find(name);
		mPointList.erase(it);
	}
};
