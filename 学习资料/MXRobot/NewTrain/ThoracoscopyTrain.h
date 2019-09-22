#ifndef _THORACOSCOPYTRAINING_
#define _THORACOSCOPYTRAINING_
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"

class MisMedicOrgan_Ordinary;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class VeinConnectPair;

class CThoracoscopyTrain :public MisNewTraining
{
public:
	CThoracoscopyTrain(void);

	virtual ~CThoracoscopyTrain(void);

public:
	virtual void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ);

	virtual bool Update(float dt);

	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);	

	virtual void OnOrganCutByTool(MisMedicOrganInterface * organ , bool iselectriccut);

protected:
	virtual void CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig);
		
	/**
		for debug !!!
	*/
	void onDebugMessage(const std::string&);

private:

	/**
		计算定位后的组织连接所占的屏幕比例
	*/
	void CaculateVeinArea();

private:
	
	Ogre::String m_TrainName;

	GFPhysRigidBody * m_GroundSphere[2];

};

#endif