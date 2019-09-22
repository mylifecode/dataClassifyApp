#include "StdAfx.h"
#include "Helper.h"

namespace MX
{
namespace Helper
{

void MXCOMMON_API RotateNodeByVector(Ogre::Node *pNode, Ogre::Vector3 vtBegin, Ogre::Vector3 vtEnd)
{
	Ogre::Vector3 axis = vtBegin.crossProduct(vtEnd);
	axis.normalise();
	pNode->rotate(axis, Ogre::Math::ACos(vtBegin.dotProduct(vtEnd)), Ogre::SceneNode::TS_WORLD);
}

Ogre::Quaternion MXCOMMON_API RotateByVector(Ogre::Vector3 vtBegin, Ogre::Vector3 vtEnd)
{
	Ogre::Vector3 axis = vtBegin.crossProduct(vtEnd);
	axis.normalise();

	Ogre::Quaternion q;
	q.FromAngleAxis(Ogre::Math::ACos(vtBegin.dotProduct(vtEnd)), axis);
	q.normalise();
	return q;
}

Ogre::Vector3 MXCOMMON_API ProjectToAxis(Ogre::Vector3 vt, Ogre::Vector3 axis)
{
	return vt.dotProduct(axis) * axis;
}

}
}
