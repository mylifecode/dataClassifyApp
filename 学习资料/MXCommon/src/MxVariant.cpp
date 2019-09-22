#include "MxVariant.h"

MxVariant::MxVariant(void)
:m_value(0),
m_type(VT_Unkonw)
{

}

MxVariant::MxVariant(int value)
:m_value(value),
m_type(VT_Int)
{
	
}

MxVariant::MxVariant(const MxVariant& other)
:m_value(other.m_value),
m_type(other.m_type)
{
	
}

MxVariant::MxVariant(bool value)
:m_value(value),
m_type(SYVT_BOOL)
{
	
}

MxVariant::~MxVariant(void)
{

}

bool MxVariant::CanTranslateType(VariantType type)
{
	if(type == m_type)
		return true;
	else
		return false;
}

int MxVariant::ToInt(bool * isOk)
{
// 	bool canTranslate = CanTranslateType(VT_Int);
// 	if(isOk)
// 		*isOk = canTranslate;
	
	return m_value;
}

bool MxVariant::ToBool(bool * isOk)
{
// 	bool canTranslateType = CanTranslateType(SYVT_BOOL);
// 	if(isOk)
// 		*isOk = canTranslateType;
// 
// 	if(canTranslateType)
// 		return m_value;
// 	else
// 		return false;

	return m_value ? true : false;
}
