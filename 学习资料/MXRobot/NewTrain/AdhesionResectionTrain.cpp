#include "AdhesionResectionTrain.h"
#include "CustomConstraint.h"
#include <basetsd.h>


CAdhesionResectionTrain::CAdhesionResectionTrain(const Ogre::String & strName)
:	CAcessoriesCutTraining(strName) ,
m_ExploreResult(false) , 
m_pUterus(NULL) 
{

}


bool CAdhesionResectionTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = CAcessoriesCutTraining::Initialize(pTrainingConfig , pToolConfig);



	return true;
}

bool CAdhesionResectionTrain::Update(float dt)
{
	CAcessoriesCutTraining::Update(dt);

	
	return true;
}

void CAdhesionResectionTrain::receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif)
{

}

void CAdhesionResectionTrain::ElecCutStart()
{


}

void CAdhesionResectionTrain::ElecCutEnd()
{
	
}

void CAdhesionResectionTrain::ProcessElecCut(MisMedicOrganInterface * oif , uint32 areaValue)
{
	
}
