#pragma once
#include "MXCommon.h"
#include <cstdlib>

class MXCOMMON_API MxVariant
{
public:
	enum VariantType
	{
		VT_Unkonw,
		VT_Int,
		SYVT_BOOL
	};

	MxVariant(void);

	MxVariant(const MxVariant& other);

	MxVariant(int value);

	MxVariant(bool value);

	~MxVariant(void);

	bool IsValid() {return m_type != VT_Unkonw;}

	bool CanTranslateType(VariantType type);

	int ToInt(bool * isOk = NULL);

	bool ToBool(bool * isOk = NULL);

private:
	int m_value;
	VariantType m_type;
};
