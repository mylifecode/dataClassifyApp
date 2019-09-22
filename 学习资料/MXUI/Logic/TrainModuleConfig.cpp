#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include "TrainModuleConfig.h"
TrainModuleConfig * TrainModuleConfig::Instance()
{
	static TrainModuleConfig globalPath;
	return &globalPath;
}

TrainModuleConfig::TrainModuleConfig(void)
{
	m_IsLoaded = false;
}
TrainModuleConfig::~TrainModuleConfig(void)
{

}
bool TrainModuleConfig::LoadFromXML(const QString &fileName)
{
	if (m_IsLoaded)
		return true;

	//const QString& fileName = MxGlobalConfig::Instance()->GetCourseTrainXmlConfigFileName();
	if (!QFile::exists(fileName))
		return false;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	
	m_document.setContent(&file);
	file.close();


	QDomElement element = m_document.documentElement();
	QDomNode node = element.firstChild();
	QString strMenuName;
	QString strMenuItemName;
	QString strObjectName;
	while (!node.isNull())
	{
		if (node.isElement())
		{
			QDomElement moduleElement = node.toElement();

			if (moduleElement.attribute("ModuleName") == QString("SkillTrain"))
			{
				QDomNode subModuleNode = moduleElement.firstChild();

				while (!subModuleNode.isNull())
				{
					QDomElement subModuleEle = subModuleNode.toElement();
					QString showName = subModuleEle.attribute("ShowName");

			
					QDomNode trainItemNode = subModuleEle.firstChild();

					while (!trainItemNode.isNull())
					{
						QDomElement trainItemEle = trainItemNode.toElement();
						QString showName = trainItemEle.attribute("ShowName");
						QString globalName = trainItemEle.attribute("GlobalName");
						QString strid = trainItemEle.attribute("id");
						QString casefile = trainItemEle.attribute("CaseFile");
						QString videofile = trainItemEle.attribute("AviFile");
						QString trainCode = trainItemEle.attribute("TrainCode");
						if (m_TrainItemMap.find(strid) != m_TrainItemMap.end())
						{
							QString message = QString("find same id in TrainModuleConfg.xml id = ") + strid + QString(" use the first one!!");
							QMessageBox::warning(0, QString("Redundunt Menu Id! "), message);
						}
						else
							m_TrainItemMap.insert(std::make_pair(strid, TrainModuleConfig::TrainItemAttribute(strid, globalName, showName, casefile, videofile, trainCode)));

						trainItemNode = trainItemNode.nextSibling();
					}
					subModuleNode = subModuleNode.nextSibling();
				}
				break;
			}

		}
		node = node.nextSibling();
	}
	m_IsLoaded = true;
}

bool TrainModuleConfig::GetMenuAttributeById(const QString & id, QString & globalName, QString & showName,QString& trainCode, QString & casefile, QString & videoFile)
{
	std::map<QString, TrainItemAttribute >::iterator itor = m_TrainItemMap.find(id);

	if (itor != m_TrainItemMap.end())
	{
		TrainModuleConfig::TrainItemAttribute & item = itor->second;
		globalName = item.m_GlobalName;
		showName   = item.m_ShowName;
		casefile   = item.m_strCaseFile;
		videoFile  = item.m_strAviFile;
		trainCode = item.m_strTrainCode;
		return true;
	}
	else
	{
		return false;
	}
}