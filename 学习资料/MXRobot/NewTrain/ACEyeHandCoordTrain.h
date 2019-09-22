#ifndef _ACEYEHANDCOORDTRAIN_
#define _ACEYEHANDCOORDTRAIN_
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"

#include "TrainScoreSystem.h"
#include "SmokeManager.h"
#include "VesselBleedEffect.h"

class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;


class ACEyeHandCoordTrain :public MisNewTraining
{
public:

	ACEyeHandCoordTrain(void);
	
	virtual ~ACEyeHandCoordTrain(void);

public:
	
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	
	virtual bool Update(float dt);

private:
	
};

#endif