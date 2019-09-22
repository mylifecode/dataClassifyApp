#pragma once

#include "TrainingMgr.h"
#include <vector>


class MisMedicRigidPrimtive;
class CTool;


class ToolSpenetrateMgr
{
	//correct info
	/*struct CorrectInfo
	{
		float collideTime;
		bool  collideFaceInner;
		GFPhysVector3 CollidePtOnTri;	
		GFPhysVector3 collideNormal;
		GFPhysVector3 CollidePtTool;
		GFPhysVector3 KNodePosWhenCollide;
		GFPhysVector3 slipDir;
	};*/

	struct EdgeInfo
	{
		EdgeInfo()
		{
			numTriangle = 0;
		}

		int GetNumTriangle()
		{
			return numTriangle;
		}
		void AddTriangle(const GFPhysVector3 & p1,
			const GFPhysVector3 & p2,
			const GFPhysVector3 & p3,
			const GFPhysVector3 & collideNormal, int faceIndex)
		{
			point1 = p1; 
			point2 = p2;
			AddTriangle(p3, collideNormal, faceIndex);
		}

		void AddTriangle(const GFPhysVector3 & p3,
			const GFPhysVector3 & collideNormal, int faceIndex)
		{
			if (numTriangle < 19)
			{
				faceIndices[numTriangle] = faceIndex;
				thirdPoints[numTriangle] = p3;
				collideNormals[numTriangle] = collideNormal;
				numTriangle++;
			}
		}
		GFPhysVector3 point1;
		GFPhysVector3 point2;
		
		//std::vector<int> faceIndices[];
		int faceIndices[20];
		
		GFPhysVector3 thirdPoints[20];
		
		GFPhysVector3 collideNormals[20];

		int numTriangle;
		//std::vector<GFPhysVector3> thirdPoints;
		//std::vector<GFPhysVector3> collideNormals;
	};
private:
	ToolSpenetrateMgr(double coefficient);
	ToolSpenetrateMgr();
	~ToolSpenetrateMgr();

public:
	static ToolSpenetrateMgr* GetInstance();

	static void Destroy();

	//两条直线上最近的点
	void GetNearestPoints(Ogre::Vector3 P0, Ogre::Vector3 L0, Ogre::Vector3 P1, Ogre::Vector3 L1, double &t0, Ogre::Vector3 &Q0, double &t1, Ogre::Vector3 &Q1);
	void CorrectToolTransform(CTrainingMgr * m_pTrainingMgr,float dt);

	void AddStaticObject(MisMedicRigidPrimtive* object)
	{
		m_staticRigidObjects.push_back(object);
	}

	void RemoveStaticObject(MisMedicRigidPrimtive* object)
	{
		for (int c = 0; c < m_staticRigidObjects.size(); c++)
		{
			if (m_staticRigidObjects[c] == object)
			{
				m_staticRigidObjects.erase(m_staticRigidObjects.begin() + c);
				break;
			}
		}
	}

private:
	void CorrectToolPositionByStaticRigidObjects(CTool* leftTool,CTool* rightTool,float dt);

private:
	Ogre::Vector3 lastPosH0, lastPosS0;
	Ogre::Quaternion lastOriH0, lastOriS0;

	Ogre::Vector3 lastPosH1, lastPosS1;
	Ogre::Quaternion lastOriH1, lastOriS1;

	double k;
		
private:
	std::vector<MisMedicRigidPrimtive*> m_staticRigidObjects;
	std::vector<EdgeInfo> m_CollideEdgeList;


	static ToolSpenetrateMgr* m_toolSpenetrateMgr;
};