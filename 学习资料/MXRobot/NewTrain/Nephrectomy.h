#ifndef _NEPHRECTOMY_
#define _NEPHRECTOMY_
#include "MisNewTraining.h"
#include "MisMedicObjectUnion.h"
#include "DeferredRendFrameWork.h"

class CNephrectomy :public MisNewTraining, public DeferredRendFrameWork::DeferreRendStageListener
{
public:
    CNephrectomy(const Ogre::String & strName);
    virtual ~CNephrectomy(void);
    bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
    bool BeginRendOneFrame(float timeelapsed);
    virtual MisMedicOrganInterface * LoadOrganism(MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain);
    void BeforeRendStage(DeferredRendSceneManager::DeferredRendStage stage);
    void AfterRendStage(DeferredRendSceneManager::DeferredRendStage stage);
    virtual bool Update(float dt);
protected:
    virtual void CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig);
    virtual void BuildOrgansVolumeTextureCoord();

public:
    MisMedicOrgan_Ordinary * m_MesoColon;
    MisMedicOrgan_Ordinary * m_Gerotas;
    MisMedicOrgan_Ordinary * m_ActiveDome;
    MisMedicOrgan_Ordinary * m_KidneyVessels;


public:
    Ogre::TexturePtr m_Area;
    int		m_AreaWidth;
    int		m_AreaHeight;
    std::string	m_AreaMarkTextureName;
    std::set<MisMedicOrganInterface*> m_DepthLowOrgans;

private:
    int m_ToolPlaceIndex;
    int  ChangeToolPlace(int id);
    void OnSimpleUIEvent(const SimpleUIEvent & event);	

};
#endif