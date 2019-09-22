/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperParticle : public CXMLSerialize
{
public:
	CXMLWrapperParticle(void);
	~CXMLWrapperParticle(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Type)
	IMPL_ATTRIBUTE_FLOAT3(InitialVelocity)
	IMPL_ATTRIBUTE_FLOAT(Radius)
	IMPL_ATTRIBUTE_FLOAT(Mass)
	IMPL_ATTRIBUTE_FLOAT(Life)
	IMPL_ATTRIBUTE_FLOAT3(Force)
	IMPL_ATTRIBUTE_FLOAT3(RandomForce)
	IMPL_ATTRIBUTE_FLOAT(EmitRate)
	IMPL_ATTRIBUTE_STRING(MaterialName)

	IMPL_ATTRIBUTE_FLOAT(LiveTimeAfterStop)
	IMPL_ATTRIBUTE_FLOAT(BillboardWidth)
	IMPL_ATTRIBUTE_FLOAT(BillboardHeight)


	DECLARE_SERIALIZATION_CLASS(CXMLWrapperParticle)
};

