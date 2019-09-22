/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperParticle;

class CXMLWrapperParticles : public CXMLSerialize
{
public:
	CXMLWrapperParticles(void);
	~CXMLWrapperParticles(void);

	std::vector<CXMLWrapperParticle *> m_Particles;
	bool m_flag_Particles;

	void __stdcall Set_Particles(Variant value);
	void __stdcall Get_Particles(Variant * pValue);

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperParticles)
};

