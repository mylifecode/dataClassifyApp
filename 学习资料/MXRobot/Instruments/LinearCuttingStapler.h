/*********************************************
FileName:    LinearCuttingStapler.h
FilePurpose: 实现直线切割吻合器相关功能
Author:      Supo
Email:       chiyou7410@163.com
LastData:    2012.3.15
*********************************************/
#pragma once
#include "Forceps.h"
#include "MisCTool_PluginClamp.h"
#include "MisCTool_PluginEndoGia.h"
#include "Painting.h"
#include "MisMedicNail.h"




class CXMLWrapperTool;
class CLinearCuttingStapler : public CForceps
{
public:
	CLinearCuttingStapler();
	
	CLinearCuttingStapler(CXMLWrapperTool * pToolConfig);
	
	virtual ~CLinearCuttingStapler(void);

	Ogre::TexturePtr GetToolBrandTexture();

	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	//设置钉子的状态，是否可见
	bool SetNailsState(bool IsShow);	
    void InternalSimulationStart(int currStep , int TotalStep , float dt);	
    void onFrameUpdateStarted(float timeelapsed);
	bool Update(float dt);
    
    GFPhysVector3 CalculateToolCustomForceFeedBack();
    float GetCutWidth();
    void SetCutterCenterlineDirection(const GFPhysVector3& dir);
private:
    bool CalcLocalCompressDir(const GFPhysVectorObj<GFPhysSoftBodyNode *>& nodesToBeCompressed,const GFPhysVector3 & Globaldir,GFPhysVector3 & Localdir);
    //bool CalcLocalCompressDir(const GFPhysVectorObj<GFPhysSoftBodyNode *>& nodesToBeCompressed,const GFPhysVector3 & Globaldir,GFPhysVector3 & Localdir);
    //bool CalcLocalCompressDir(const std::set<GFPhysSoftBodyFace*> & facesCreated,const GFPhysVector3 & Globaldir,const GFPhysVector3 & anthorchoice,GFPhysVector3 & Localdir);

   // void CutClampedOrgan(float dt);
	bool CutClampedOrgans();
    bool CreateNailOnOrgan(MisMedicOrgan_Ordinary * organ);
  
    MisCTool_PluginEndoGia * m_EndoGiaPlugin;
    //std::map<GFPhysSoftBodyFace *,float> m_burn_record;
    bool m_canCut;
	float m_ApplyClipTime;
    //float m_time_elapse;
    //CSoundManager * m_cuttingsound;
    PaintingTool painting;
    //Ogre::SceneNode* m_ScenenNode;
    //std::set<GFPhysSoftBodyFace*> m_FacesCreated;
    std::set<GFPhysSoftBodyFace*> m_FacesCreatedUpper;
    std::set<GFPhysSoftBodyFace*> m_FacesCreatedLower;
    GFPhysVectorObj<GFPhysSoftBodyNode *> m_NodesToBeCompressed;
    GFPhysVectorObj<GFPhysSoftBodyNode *> m_NodesUpper;
    GFPhysVectorObj<GFPhysSoftBodyNode *> m_NodesLower;  


    GFPhysVector3 m_CutterCenterlineDirection;
	Ogre::TexturePtr m_Tex;
	Ogre::SceneNode * m_NodeFaMen;
};

