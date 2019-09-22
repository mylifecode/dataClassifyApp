
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperMovie : public CXMLSerialize
{
public:
	CXMLWrapperMovie(void);
	~CXMLWrapperMovie(void);

	IMPL_ATTRIBUTE_STRING(MovieName)
	IMPL_ATTRIBUTE_STRING(Title)
	IMPL_ATTRIBUTE_STRING(Path)

	IMPL_ATTRIBUTE_BOOL(AutoClose)

	IMPL_ATTRIBUTE_LONG(DialogPosX)
	IMPL_ATTRIBUTE_LONG(DialogPosY)
	IMPL_ATTRIBUTE_LONG(DialogWidth)
	IMPL_ATTRIBUTE_LONG(DialogHeight)

	IMPL_ATTRIBUTE_LONG(TextureWidth)
	IMPL_ATTRIBUTE_LONG(TextureHeight)

	IMPL_ATTRIBUTE_LONG(OverlayWidth)
	IMPL_ATTRIBUTE_LONG(OverlayHeight)

	IMPL_ATTRIBUTE_BOOL(Active)

	IMPL_ATTRIBUTE_FLOAT(OverlayPosX)
	IMPL_ATTRIBUTE_FLOAT(OverlayPosY)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperMovie)
};
