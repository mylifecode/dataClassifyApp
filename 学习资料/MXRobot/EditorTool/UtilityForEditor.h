#pragma once
#include <Ogre.h>
#include "collision/GoPhysCollisionlib.h"
#include "dynamic/PhysicBody/GoPhysSoftTube.h"
#include "math/GoPhysTransformUtil.h"

class MisMedicOrgan_Ordinary;

namespace EditorTool
{
	class RayCastResult
	{
	public:
		RayCastResult():m_pFace(NULL){}
		float m_Weights[3];
		GFPhysSoftBodyFace *m_pFace;
		Ogre::Vector3 m_Position;
		Ogre::Vector2 m_Uv;
	};

	void ExtractPointWeights(const Ogre::Vector3 & p , Ogre::Vector3 facevert[3] , float weights[3]);

	RayCastResult PickPointOnOrgan(MisMedicOrgan_Ordinary* pOrgan ,const Ogre::Ray & ray);
}