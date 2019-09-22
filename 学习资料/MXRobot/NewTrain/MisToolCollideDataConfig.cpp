#include "MisToolCollideDataConfig.h"
#include "tinyxml.h"
#include "ogre.h"
MisToolCollideDataConfig::MisToolCollideDataConfig()
{

}

MisToolCollideDataConfig::~MisToolCollideDataConfig()
{

}
MisToolCollideDataConfigMgr::MisToolCollideDataConfigMgr()
{
	m_Loaded = false;
}

MisToolCollideDataConfigMgr::~MisToolCollideDataConfigMgr()
{
	std::map<Ogre::String , MisToolCollideDataConfig*>::iterator itor = m_ToolConfigs.begin();
	while(itor != m_ToolConfigs.end())
	{
		delete itor->second;
		itor++;
	}
	m_ToolConfigs.clear();
}
//=========================================================================================
void MisToolCollideDataConfigMgr::ReadFromXML(char * filepath)
{
	if(m_Loaded == true)
	   return;

	TiXmlDocument document;
	
	document.LoadFile(filepath);

	TiXmlElement * rootElement = document.RootElement();

	TiXmlElement * toolelement = rootElement->FirstChildElement("Tool");
	while(toolelement != 0)
	{
		const char * toolname = toolelement->Attribute("Type");

		if(m_ToolConfigs.find(toolname) == m_ToolConfigs.end())
		{
			MisToolCollideDataConfig * toolconfig = new MisToolCollideDataConfig();

			TiXmlElement * leftpart = toolelement->FirstChildElement("LeftPart");
			if(leftpart)
			{	
				ReadToolPart(leftpart , toolconfig->m_left);
				toolconfig->m_left.m_Valid = true;
			}
			
			TiXmlElement * rightpart = toolelement->FirstChildElement("RightPart");
			if(rightpart)
			{
			   ReadToolPart(rightpart , toolconfig->m_right);
			   toolconfig->m_right.m_Valid = true;
			}
			

			TiXmlElement * centerpart = toolelement->FirstChildElement("CenterPart");
			if(centerpart)
			{
			   ReadToolPart(centerpart , toolconfig->m_center);
			   toolconfig->m_center.m_Valid = true;
			}
			m_ToolConfigs.insert(std::make_pair(toolname, toolconfig));
		}
		toolelement = toolelement->NextSiblingElement();
	}
	m_Loaded = true;
}

Ogre::Vector3 MisToolCollideDataConfigMgr::ParseVector3(Ogre::String strtemp)
{
	Ogre::vector<Ogre::String>::type strvec = Ogre::StringUtil::split(strtemp , ",");
	
	Ogre::Vector3 result;
	result.x = Ogre::StringConverter::parseReal(strvec[0]);
	result.y = Ogre::StringConverter::parseReal(strvec[1]);
	result.z = Ogre::StringConverter::parseReal(strvec[2]);

	return result;
}
void MisToolCollideDataConfigMgr::ReadToolPart(TiXmlElement * partelement , MisToolCollidePart & DstToolPart)
{
	bool ShowDebugNode = false;
	
	const char * strshowdebug = partelement->Attribute("ShowDebugNode");
	
	if(strcmp(strshowdebug , "true") == 0)
	   DstToolPart.m_ShowDebug = true;
	else
	   DstToolPart.m_ShowDebug = false;

	const char * strconvexHull = partelement->Attribute("IsConvexHull");

	if(strconvexHull == 0)
	   DstToolPart.m_IsConvexHull = true;
	else if(strcmp(strconvexHull , "false") == 0)
	   DstToolPart.m_IsConvexHull = false;
	else
       DstToolPart.m_IsConvexHull = true;

	TiXmlElement * collidebody = partelement->FirstChildElement("collidedshape");
	while(collidebody)
	{
		MisToolCollidePart::ToolCollideShapeData * collideshapeData = new MisToolCollidePart::ToolCollideShapeData();

		const char * shapetype = collidebody->Attribute("Type");

		Ogre::Quaternion shapeRote = Ogre::Quaternion::IDENTITY;
		
		const char * temp = collidebody->Attribute("Rotate");
		if (temp)
		{
			Ogre::String strRot(temp);
			Ogre::vector<Ogre::String>::type strRotvec = Ogre::StringUtil::split(strRot, ",");
			shapeRote.w = Ogre::StringConverter::parseReal(strRotvec[0]);
			shapeRote.x = Ogre::StringConverter::parseReal(strRotvec[1]);
			shapeRote.y = Ogre::StringConverter::parseReal(strRotvec[2]);
			shapeRote.z = Ogre::StringConverter::parseReal(strRotvec[3]);
		}
		if(strcmp(shapetype , "box") == 0)
		{
			Ogre::String strtemp = collidebody->Attribute("Center");
			Ogre::vector<Ogre::String>::type strvec = Ogre::StringUtil::split(strtemp , ",");
			Ogre::Vector3 center;
			center.x = Ogre::StringConverter::parseReal(strvec[0]);
			center.y = Ogre::StringConverter::parseReal(strvec[1]);
			center.z = Ogre::StringConverter::parseReal(strvec[2]);

			strtemp = collidebody->Attribute("Extend");
			strvec = Ogre::StringUtil::split(strtemp , ",");
			Ogre::Vector3 Extend;
			Extend.x = Ogre::StringConverter::parseReal(strvec[0]);
			Extend.y = Ogre::StringConverter::parseReal(strvec[1]);
			Extend.z = Ogre::StringConverter::parseReal(strvec[2]);

			
			collideshapeData->m_BoxCenter = center;
			collideshapeData->m_BoxExtend = Extend;
			collideshapeData->m_Rotate = shapeRote;
			collideshapeData->m_Type = 0;
		}
		else if(strcmp(shapetype , "cylinder") == 0)
		{
			Ogre::String strtemp = collidebody->Attribute("PointA");
			Ogre::vector<Ogre::String>::type strvec = Ogre::StringUtil::split(strtemp , ",");
			Ogre::Vector3 pointa;
			pointa.x = Ogre::StringConverter::parseReal(strvec[0]);
			pointa.y = Ogre::StringConverter::parseReal(strvec[1]);
			pointa.z = Ogre::StringConverter::parseReal(strvec[2]);

			strtemp = collidebody->Attribute("PointB");
			strvec = Ogre::StringUtil::split(strtemp , ",");
			Ogre::Vector3 pointb;
			pointb.x = Ogre::StringConverter::parseReal(strvec[0]);
			pointb.y = Ogre::StringConverter::parseReal(strvec[1]);
			pointb.z = Ogre::StringConverter::parseReal(strvec[2]);

			strtemp = collidebody->Attribute("Radius");
			float radius = Ogre::StringConverter::parseReal(strtemp);

			collideshapeData->m_CapPointA = pointa;
			collideshapeData->m_CapPointB = pointb;
			collideshapeData->m_Radius = radius;

            //
            collideshapeData->m_BoxCenter = (pointa + pointb) * 0.5f;
            float extendz = (pointa - pointb).length() * 0.5f;
            collideshapeData->m_BoxExtend = Ogre::Vector3(radius, radius, extendz);

            //
			collideshapeData->m_Type = 2;
		}
		else if(strcmp(shapetype , "convex") == 0)
		{
			TiXmlElement * vertelement = collidebody->FirstChildElement("vertex");
			
			TiXmlElement * poselement = vertelement->FirstChildElement("pos");
			
			while(poselement)
			{
				Ogre::String strtemp = poselement->Attribute("value");
				
				Ogre::Vector3 vertexpos = ParseVector3(strtemp);
				
				collideshapeData->m_VertexPos.push_back(vertexpos);

				poselement = poselement->NextSiblingElement();
			}

			TiXmlElement * faceselement = collidebody->FirstChildElement("faces");
			if(faceselement)
			{
				TiXmlElement * indexlement = faceselement->FirstChildElement("index");
				while(indexlement)
				{
					Ogre::String strtemp = indexlement->Attribute("value");
					Ogre::vector<Ogre::String>::type strvec = Ogre::StringUtil::split(strtemp , ",");
					for(size_t c = 0 ; c < strvec.size(); c++)
					{
						int Index = Ogre::StringConverter::parseInt(strvec[c]);
						collideshapeData->m_ConvexFaceVertIndex.push_back(Index);
					}
					
					collideshapeData->m_ConvexFace.push_back(strvec.size());
					indexlement = indexlement->NextSiblingElement();
				}
			}
			
			collideshapeData->m_Type = 1;
		}

		DstToolPart.m_CollideShapes.push_back(collideshapeData);

		collidebody = collidebody->NextSiblingElement();
	}
}
MisToolCollideDataConfig s_ErrorCollideConfig;
MisToolCollideDataConfig & MisToolCollideDataConfigMgr::GetToolCollideConfig(Ogre::String toolname)
{
	std::map<Ogre::String , MisToolCollideDataConfig*>::iterator itor = m_ToolConfigs.find(toolname);
	if(itor != m_ToolConfigs.end())
	{
		return (*itor->second);
	}
	else
	{
		return s_ErrorCollideConfig;
	}
}