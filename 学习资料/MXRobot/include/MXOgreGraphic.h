#pragma once

#include "Singleton.h"
#include "OgreRoot.h"
#include "ogrewidget.h"
#include <QWidget>
#include <map>
#include <OgreLog.h>
#include <QApplication>
#include "../amfc/include/Rbwindow.h"
#include "OgreRenderWindow.h"
#include "OgreFrameListener.h"
#include "Math/GoPhysVector3.h"
#include "Math/GoPhysQuaternion.h"
#include "Memory/GoPhysAlignedObjectArray.h"
using namespace GoPhys;

enum CG_PROGRAME_TYPE
{
	VERTEX_PROGRAME,
	FRAGMENT_PROGRAME,
};

bool CalcPlaneNormalByRegress(const GFPhysAlignedVectorObj<GFPhysVector3> & positions,GFPhysVector3 & normal);

bool CalcPlaneNormalBySVD(const GFPhysAlignedVectorObj<GFPhysVector3> & positions , GFPhysVector3 & normal , GFPhysVector3 & com);

void ApplyTextureToMaterial(Ogre::MaterialPtr mat , Ogre::TexturePtr tex , const Ogre::String & unitName);

void ApplyTextureToMaterial(Ogre::String materialname, Ogre::TexturePtr texturetoapp , Ogre::String textureunitname);

void ApplyTextureToMaterial(Ogre::String materialname, Ogre::TexturePtr effecttex , int texunit);

bool GetMaterialTextureName(Ogre::String materialname, const Ogre::String & NameAlias , Ogre::String & texName);

Real calculateZ( const Real & x,const Real & y, const GFPhysVector3 & n,/*normal vector */ const GFPhysVector3 & p/*point on plane */ );

Ogre::GpuProgramParametersSharedPtr GetShaderParamterPtr(Ogre::String materialName, CG_PROGRAME_TYPE type, int techindex, int passindex);


inline GFPhysVector3 OgreToGPVec3(const Ogre::Vector3 & ogrevec3)
{
	return GFPhysVector3(ogrevec3.x , ogrevec3.y , ogrevec3.z);
}

inline Ogre::Vector3 GPVec3ToOgre(const GFPhysVector3 & gpvec3)
{
	return Ogre::Vector3(gpvec3.m_x , gpvec3.m_y , gpvec3.m_z);
}

inline GFPhysQuaternion OgreToGPQuaternion(const Ogre::Quaternion & ogrequat)
{
	return GFPhysQuaternion(ogrequat.x , ogrequat.y , ogrequat.z , ogrequat.w);
}

inline Ogre::Quaternion GPQuaternionToOgre(const GFPhysQuaternion & gpquat)
{
	return Ogre::Quaternion(gpquat.w() , gpquat.x() , gpquat.y() , gpquat.z());
}