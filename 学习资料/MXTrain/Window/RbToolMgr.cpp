#include "MxDefine.h"
#include "RbToolMgr.h"
#include "MxGlobalConfig.h"

#define MERGERTOOLTYPE(type,subType) type + "-" + subType

RbToolMgr::RbToolMgr()
{
	readXML();
}

RbToolMgr::~RbToolMgr()
{

}

RbToolMgr& RbToolMgr::GetInstance()
{
	static RbToolMgr mgr;
	return mgr;
}

Tool_Unit RbToolMgr::INS( const QString & name )
{
	RbToolMgr& mgr = GetInstance();
	return mgr.m_toolMap[name];
}

const Tool_Unit& RbToolMgr::GetToolUnit(const QString& toolTypeName,const QString& toolSubTypeName)
{
	RbToolMgr& mgr = GetInstance();
	const static Tool_Unit emptyUnit;

	QMap<QString,Tool_Unit>::iterator itr = mgr.m_toolUnitMap.find(MERGERTOOLTYPE(toolTypeName,toolSubTypeName));
	if(itr != mgr.m_toolUnitMap.end())
	{
		return itr.value();
	}
	else
		return emptyUnit;
}

bool RbToolMgr::readXML()
{
	if ( !QFile::exists(MxGlobalConfig::Instance()->GetToolXmlConfigFileName()) ) return false;

	QDomDocument doc("mydocument");

	QFile file(MxGlobalConfig::Instance()->GetToolXmlConfigFileName());
	if ( !file.open(QIODevice::ReadOnly) ) return false;

	bool bSetSuccess = doc.setContent(&file);
	file.close();
	if (!bSetSuccess) return false;
	
	QDomElement element = doc.documentElement();
	
	QDomNode n = element.firstChild();
	//QString strCategoryName;
	//QString strCategoryFile;
	QString strToolType;
	QString strSubType;
	QString strDeviceName;
	QString strDeviceFile;

	Tool_Unit unit;
	m_toolMap.clear();
	while(!n.isNull())
	{
		if (n.isElement())
		{
			QDomElement e = n.toElement();
			//strCategoryName = e.attribute("CategoryName");
			//strCategoryFile = e.attribute("CategoryFile");
			//strToolType = e.attribute("ToolType");
			//strSubType = e.attribute("SubTypek");
			QDomNode dn = e.firstChild();
			while (!dn.isNull())
			{
				e = dn.toElement();
				unit.name = e.attribute("DeviceName");
				unit.picFile = e.attribute("DeviceFile");
				unit.type = e.attribute("ToolType");
				unit.subType = e.attribute("SubType");
				m_toolMap[unit.name] = unit;
				m_toolUnitMap.insert(MERGERTOOLTYPE(unit.type,unit.subType),unit);
				dn = dn.nextSibling();
			}
		}
		n = n.nextSibling();
	}
 return true;
}