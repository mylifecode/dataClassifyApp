#ifndef _SIGMOIDECTOMY_
#define _SIGMOIDECTOMY_
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
//#include "MisMedicOrganTube.h"
#include "TrainScoreSystem.h"
#include "MisMedicObjectUnion.h"
#include "OgreMesh.h"
#include "DeferredRendFrameWork.h"
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class VeinConnectPair;


class CSigmoidectomy :public MisNewTraining, public DeferredRendFrameWork::DeferreRendStageListener
{
public:
	void BeforeRendStage(DeferredRendSceneManager::DeferredRendStage);
	void AfterRendStage(DeferredRendSceneManager::DeferredRendStage);

    CSigmoidectomy(const Ogre::String & strName);

    virtual ~CSigmoidectomy(void);
    void KeyPress(QKeyEvent * event);

	virtual void OnSimpleUIEvent(const SimpleUIEvent & event);
    void OnOrganCutByTool(MisMedicOrganInterface * pOrgan, bool iselectriccut);

public:    
    virtual bool Update(float dt);
	bool BeginRendOneFrame(float timeelapsed);

	virtual void InternalSimulateEnd(int currStep , int TotalStep , Real dt);

    bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual void BuildOrgansVolumeTextureCoord();
    virtual void SerializerReadFinish(MisMedicOrgan_Ordinary * organ , MisMedicObjetSerializer & serialize);
    MisMedicOrganInterface * LoadOrganism(/*MisMedicObjetSerializer& serializer,*/ MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain);
    
    MisMedicOrgan_Ordinary * m_SigmoidCutpart;
	MisMedicOrgan_Ordinary * m_DynamicDomPart;

	std::set<MisMedicOrganInterface*> m_DepthLowOrgans;
private:
	virtual bool OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight);

	//MisMedicObjectUnion m_StaDynDomeUnion;
	//Ogre::MeshPtr m_StaticDomeMeshPtr;

    int m_CameraPlaceIndex;
    int m_ToolPlaceIndex;
    bool Checkfinish();
    int ChangeCamera(int id);
    int ChangeToolPlace(int id);



};

#endif