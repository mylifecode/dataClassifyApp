#ifndef _MISTOOLCOLLIDEDATACONFIG_
#define _MISTOOLCOLLIDEDATACONFIG_
#include <vector>
#include "Ogre.h"
#include "Singleton.h"
class TiXmlElement;
class MisToolCollidePart
{
public:
	MisToolCollidePart()
	{
		m_Valid = false;
		m_ShowDebug = false;
		m_IsConvexHull = true;
	}
	~MisToolCollidePart()
	{
		for(size_t c = 0 ; c < m_CollideShapes.size() ; c++)
		{
            delete m_CollideShapes[c];
		}
		m_CollideShapes.clear();
	}
	class ToolCollideShapeData
	{
	public:
		ToolCollideShapeData()
		{
			m_Type = 0;
			m_Rotate = Ogre::Quaternion::IDENTITY;
		}

		int m_Type;//0- box 1- convex 2-cyliner
		
		Ogre::Vector3 m_BoxCenter;
		Ogre::Vector3 m_BoxExtend;
		Ogre::Quaternion m_Rotate;//

		float m_Radius;//for cylinder and capsule
		Ogre::Vector3 m_CapPointA;
		Ogre::Vector3 m_CapPointB;

		std::vector<Ogre::Vector3> m_VertexPos;
		std::vector<int> m_ConvexFaceVertIndex;
		std::vector<int> m_ConvexFace;
	};

	std::vector<ToolCollideShapeData*> m_CollideShapes;
	
	bool m_Valid;
	bool m_ShowDebug;
	bool m_IsConvexHull;
	
};
class MisToolCollideDataConfig
{
public:

	MisToolCollideDataConfig();
	~MisToolCollideDataConfig();

	MisToolCollidePart m_left;
	MisToolCollidePart m_right;
	MisToolCollidePart m_center;
};

class MisToolCollideDataConfigMgr : public CSingleT<MisToolCollideDataConfigMgr>
{
public:
	MisToolCollideDataConfigMgr();
	
	~MisToolCollideDataConfigMgr();

	void ReadFromXML(char * filepath);
	
	Ogre::Vector3 ParseVector3(Ogre::String strvec);

	void ReadToolPart(TiXmlElement * partelement , MisToolCollidePart & DstToolPart);

	MisToolCollideDataConfig & GetToolCollideConfig(Ogre::String toolname);

	std::map<Ogre::String , MisToolCollideDataConfig*> m_ToolConfigs;

	bool m_Loaded;
};

#endif