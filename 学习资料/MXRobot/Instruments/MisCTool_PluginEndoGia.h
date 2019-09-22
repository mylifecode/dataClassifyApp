#pragma once
#include "MisMedicCToolPluginInterface.h"
#include "MXOgreWrapper.h"

class MisCTool_PluginEndoGia : public MisMedicCToolPluginInterface
{
public:
	
	MisCTool_PluginEndoGia(CTool * tool);

	~MisCTool_PluginEndoGia();

	//when a physics update frame start
	virtual void PhysicsSimulationStart(int currStep , int TotalStep , float dt);

	//when a physics update frame end
	virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt);

    void TryApplyEndoGia(MisMedicOrgan_Ordinary * organ , 
                          GFPhysVectorObj<CompressNodesAndDir*> NodesAndDir,
                          Real compressrate);

	//when a ordinary frame update start(one ordinary frame may include zero-many physics frame)
	virtual void OneFrameUpdateStarted(float timeelapsed);

	void setState(int state);

	void createNail(MisMedicOrgan_Ordinary* organ, std::vector<GFPhysSoftBodyFace*>* faces, float len, float offset);

	Ogre::SceneNode* createSingleClip(int idx);

	void showCutArea();

	void tryApplyClips(MisMedicOrgan_Ordinary **organs);

	void getROIFaces(std::vector<GFPhysSoftBodyFace*>* faces, MisMedicOrgan_Ordinary* pOrgan, GFPhysVector3 &regMin, GFPhysVector3 &regMax);

public:
    bool m_canNail;
    bool m_AppAllowClamp;
    float m_TimeElapsedSinceLastNail;
    bool  m_hasNailOrganization;		//ÊÇ·ñ¼Ð×¡Æ÷¹Ù
	bool m_haseffectApplyed;
	int m_state; // 0-release, 1-ready, 2-cut, 3-cut over
	float m_prepareTime;

};

