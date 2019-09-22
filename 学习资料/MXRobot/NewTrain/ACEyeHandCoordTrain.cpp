#include "ACEyeHandCoordTrain.h"
#include "MisMedicOrganOrdinary.h"


//=============================================================================================
ACEyeHandCoordTrain::ACEyeHandCoordTrain(void)
{

}
//=============================================================================================
ACEyeHandCoordTrain::~ACEyeHandCoordTrain(void)
{

}
//======================================================================================================================
bool ACEyeHandCoordTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig, pToolConfig);

	MisMedicOrgan_Ordinary * organBag0 = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap.find(10)->second);
	MisMedicOrgan_Ordinary * organBag1 = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap.find(11)->second);

	organBag0->m_physbody->EnableDoubleFaceSoftSoftCollision();
	organBag1->m_physbody->EnableDoubleFaceSoftSoftCollision();

	return result;
}
//======================================================================================================================
bool ACEyeHandCoordTrain::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);

	return result;
}