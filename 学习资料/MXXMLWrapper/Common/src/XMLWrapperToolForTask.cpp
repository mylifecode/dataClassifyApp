/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperToolForTask.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperToolForTask)
	REGISTER_CLASS(ToolForTask)
	REGISTER_ATTRIBUTE(CXMLWrapperToolForTask,DeviceName,VALUE)
	
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperToolForTask::CXMLWrapperToolForTask(void)
{
	INIT_ATTRIBUTE_STRING(DeviceName)
}

CXMLWrapperToolForTask::~CXMLWrapperToolForTask(void)
{

}