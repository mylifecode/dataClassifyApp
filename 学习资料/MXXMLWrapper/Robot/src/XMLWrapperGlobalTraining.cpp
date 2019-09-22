/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperGlobalTraining.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperGlobalTraining)
	REGISTER_CLASS(GlobalTraining)
	REGISTER_ATTRIBUTE(CXMLWrapperGlobalTraining,Trainings,ARRAY)
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperGlobalTraining::CXMLWrapperGlobalTraining(void)
{
	m_flag_Trainings = false;
}

CXMLWrapperGlobalTraining::~CXMLWrapperGlobalTraining(void)
{
}

void __stdcall CXMLWrapperGlobalTraining::Set_Trainings(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	
	m_Trainings.push_back((CXMLWrapperTraining *)value.lVal);

	m_flag_Trainings = true;
}

void __stdcall CXMLWrapperGlobalTraining::Get_Trainings(Variant * pValue)
{

}