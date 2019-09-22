/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperMark : public CXMLSerialize
{
public:
	CXMLWrapperMark(void);
	~CXMLWrapperMark(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_LONG(SMFileType)
	IMPL_ATTRIBUTE_STRING(Indexs) // comma separated
	IMPL_ATTRIBUTE_COLOR(Color)
	IMPL_ATTRIBUTE_LONG(ShowTime)
	IMPL_ATTRIBUTE_STRING(MarkHideType)

	IMPL_ATTRIBUTE_BOOL(NeedShow) // if show this mark

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperMark)
};
