#include "stdafx.h"
#include "PhysicsWrapper.h"
#include "MXOgreWrapper.h"
#include "MXOgreGraphic.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include"UtilityForEditor.h"

namespace EditorTool
{
	void ExtractPointWeights(const Ogre::Vector3 & p , Ogre::Vector3 facevert[3] , float weights[3])
	{
		Ogre::Vector3 a = facevert[0];

		Ogre::Vector3 b = facevert[1];

		Ogre::Vector3 c = facevert[2];

		Ogre::Vector3 v0 = b-a;

		Ogre::Vector3 v1 = c-a;

		Ogre::Vector3 v2 = p-a;

		double d00 = v0.dotProduct(v0);
		double d01 = v0.dotProduct(v1);
		double d11 = v1.dotProduct(v1);
		double d20 = v2.dotProduct(v0);

		double d21 = v2.dotProduct(v1);
		double denom = d00*d11-d01*d01;

		float v = 0;
		float w = 0;
		if(fabs(denom) > 1e-10F)
		{
			v = (float)((d11 * d20 - d01 * d21) / denom);
			w =  (float)((d00 * d21 - d01 * d20) / denom);
		}

		float u = 1.0f-v-w;

		if(u < 0) u = 0;
		if(u > 1) u = 1;

		if(v < 0) v = 0;
		if(v > 1) v = 1;

		if(w < 0) w = 0;
		if(w > 1) w = 1;

		float sum = u+v+w;
		weights[0] = u / sum;
		weights[1] = v / sum;
		weights[2] = w / sum;
	}

	RayCastResult PickPointOnOrgan(MisMedicOrgan_Ordinary* pOrgan ,const Ogre::Ray & ray)
	{
		RayCastResult result;

		float closestDist = -1.0f;

		Ogre::Vector3 closestPoint;

		GFPhysSoftBodyFace * hittedface = 0;

		int faceIndex = -1;
		// 
		// 	GFPhysSoftBody * sb = pOrgan->m_physbody;
		// 
		// 	GFPhysSoftBodyFace * pFace = sb->GetFaceList();

		for (size_t f = 0 ; f < pOrgan->m_OriginFaces.size(); f++)
		{
			GFPhysSoftBodyFace * pFace = pOrgan->m_OriginFaces[f].m_physface;

			if(!pFace)
				continue;

			Ogre::Vector3 trianglevert[3];

			for(int v = 0 ; v < 3 ; v++)
			{
				GFPhysVector3 temp = pFace->m_Nodes[v]->m_CurrPosition;

				trianglevert[v] = Ogre::Vector3(temp.x(), temp.y(), temp.z());
			}

			std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, trianglevert[0], trianglevert[1], trianglevert[2], true, false);


			if (hit.first)
			{
				if ((closestDist < 0.0f) || (hit.second < closestDist))
				{
					closestDist = hit.second;
					hittedface = pFace;
					faceIndex = f;
				}
			}
		}

		if(hittedface)
		{
			Ogre::Vector3 facevert[3];

			for(int v = 0 ; v <3 ; v++)
			{
				GFPhysVector3 temp = hittedface->m_Nodes[v]->m_CurrPosition;
				facevert[v] = Ogre::Vector3(temp.x(), temp.y(), temp.z());
			}

			closestPoint = ray.getPoint(closestDist);   

			ExtractPointWeights(closestPoint , facevert , result.m_Weights);

			result.m_pFace = hittedface;
			result.m_Position = closestPoint;

			int vid[3];
			vid[0] = pOrgan->m_OriginFaces[faceIndex].vi[0];
			vid[1] = pOrgan->m_OriginFaces[faceIndex].vi[1];
			vid[2] = pOrgan->m_OriginFaces[faceIndex].vi[2];

			result.m_Uv = pOrgan->m_OrganRendNodes[vid[0]].m_TextureCoord  * result.m_Weights[0] + 
				pOrgan->m_OrganRendNodes[vid[1]].m_TextureCoord  * result.m_Weights[1] + 
				pOrgan->m_OrganRendNodes[vid[2]].m_TextureCoord  * result.m_Weights[2];
		}

		return result;
	}
}


