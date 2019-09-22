#ifndef _MISCTOOLPLUGINCLAMPCONNECT_
#define _MISCTOOLPLUGINCLAMPCONNECT_
#include "ogre.h"
#include "MisMedicCToolPluginInterface.h"
class MisCTool_PluginClampConnectPair : public MisMedicCToolPluginInterface
{
public:

	class VeinConnectCotactMe
	{
	public:
		int m_ClusterId;
		int m_PairId;
		VeinConnectObject * m_Veinconnect;
		GFPhysCollideObject * m_ContactRigid;
		Ogre::Vector3 m_LocalPoint;
		float m_toolOpenAside;
	};

	MisCTool_PluginClampConnectPair(CTool * tool);

	~MisCTool_PluginClampConnectPair();

	virtual void OneFrameUpdateStarted(float timeelapsed);

	virtual void CollideVeinConnectPair(VeinConnectObject * veinobject ,
										GFPhysCollideObject * convexobj,
										int cluster , 
										int pair,
										const GFPhysVector3 & collidepoint);

	std::vector<VeinConnectCotactMe> m_contacts;
};
#endif