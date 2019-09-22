/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperParticles.h"
#include "XMLWrapperParticle.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperParticles)
REGISTER_CLASS(ParticlesConfig)
REGISTER_ATTRIBUTE(CXMLWrapperParticles,Particles,ARRAY)
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperParticles::CXMLWrapperParticles(void)
{
	m_flag_Particles = false;
}

CXMLWrapperParticles::~CXMLWrapperParticles(void)
{

}

void __stdcall CXMLWrapperParticles::Set_Particles(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}

	m_Particles.push_back((CXMLWrapperParticle *)value.lVal);
	m_flag_Particles = true;
}

void __stdcall CXMLWrapperParticles::Get_Particles(Variant * pValue)
{

}