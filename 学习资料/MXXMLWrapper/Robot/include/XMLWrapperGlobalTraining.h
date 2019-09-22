/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperTraining;
class CXMLWrapperGlobalTraining : public CXMLSerialize
{
public:
	CXMLWrapperGlobalTraining(void);
	~CXMLWrapperGlobalTraining(void);

	vector<CXMLWrapperTraining *> m_Trainings;
	bool m_flag_Trainings;

	void __stdcall Set_Trainings(Variant value);
	void __stdcall Get_Trainings(Variant * pValue);

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperGlobalTraining)
};

