#pragma once

#include "MXCommon.h"
#include <OgreSceneNode.h>
#include <OgreVector3.h>

namespace MX
{
namespace Helper
{

void MXCOMMON_API RotateNodeByVector(Ogre::Node *pNode, Ogre::Vector3 vtSrc, Ogre::Vector3 vtDest);
Ogre::Quaternion MXCOMMON_API RotateByVector(Ogre::Vector3 vtBegin, Ogre::Vector3 vtEnd);
Ogre::Vector3 MXCOMMON_API ProjectToAxis(Ogre::Vector3 vt, Ogre::Vector3 axis);

}
}
