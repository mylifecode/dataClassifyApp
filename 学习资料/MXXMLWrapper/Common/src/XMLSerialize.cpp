/**Author:zx**/
#include "stdafx.h"
#include "XMLSerialize.h"
#include "tinyxml.h"

extern std::string ws2s(const std::wstring& ws);
extern std::wstring s2ws(const std::string& s);

/*****************CXMLContentManager*****************/
map<Ogre::String, XML_OBJECT_LOADER> CXMLContentManager::m_objectMap;
map<Ogre::String, Ogre::String> CXMLContentManager::m_objectTagMap;

CXMLContentManager::CXMLContentManager()
{

}

CXMLContentManager::~CXMLContentManager()
{

}

MXXMLWRAPPER_API void RegisterXMLClass(Ogre::String strName, Ogre::String strInternalClassName, XML_OBJECT_LOADER Loader)
{
	CXMLContentManager::m_objectMap[strName] = Loader;
	CXMLContentManager::m_objectTagMap[strInternalClassName] = strName;
}

MXXMLWRAPPER_API CXMLSerialize* LoadXML(Ogre::String strXMLFileName)
{
	CXMLContentManager Manager;
	return Manager.Load(strXMLFileName);
}

CXMLSerialize* LoadFromXMLString(Ogre::String strXML)
{
	CXMLContentManager Manager;
	return Manager.LoadFromString(strXML);
}

CXMLSerialize * WalkTree(CXMLContentManager * pContentManager, Ogre::String & strFilePath, CXMLSerialize * pParent, TiXmlNode * pXmlNode, XML_ATTRIBUTE_MAP * pAttrMap, bool bLoadObject = true)
{
	CXMLSerialize * pMyself = NULL;
	// check itself
	Ogre::String strNodeName = Ogre::String(pXmlNode->Value());
	// create the root object
	XML_OBJECT_LOADER loader = NULL;

	map<Ogre::String, XML_OBJECT_LOADER>::iterator iter;

	iter = CXMLContentManager::m_objectMap.find(strNodeName);
	if (iter != CXMLContentManager::m_objectMap.end() && bLoadObject)
	{
		loader = iter->second;
		pMyself = (*loader)();
		pMyself->SetPath(strFilePath);
	}

	if (!bLoadObject && pParent)
	{
		pMyself = pParent;
	}

	if (!pMyself)
	{
		return NULL;	
	}

	// check attributes first
	TiXmlElement * pElement = pXmlNode->ToElement();
	TiXmlAttribute * pAttr = pElement->FirstAttribute();
	bool bXMLRedirect = false;
	Ogre::String strXML = Ogre::String("");
	while (pAttr)
	{
		Ogre::String strName = Ogre::String(pAttr->Name());
		Ogre::String strValue = Ogre::String(pAttr->Value());

		if (strName.length() > 0)
		{
			if (strName == Ogre::String("XML"))
			{
				strXML = strValue;
				bXMLRedirect = true;
			}

			XML_ATTRIBUTE_MAP map;
			bool bOK = pMyself->LookupAttribute(strName, map);
			if (bOK)
			{
				//if (map.Type == ATTRIBUTE_TYPE_ARRAY)
				//{
				//	CXMLSerialize * pChild = NULL;
				//	// look into child tree

				//}
				if (map.Type == ATTRIBUTE_TYPE_VALUE)
				{
					Variant value;
					value.strVal = strValue;
					value.vt = SYVT_STR;
					pContentManager->SetObjectAttributeValue(pMyself, map.Set, value);
				}
			}
		}
		pAttr = pAttr->Next();
	}

	// if it has XML redirection
	if (bXMLRedirect)
	{
		CXMLSerialize * pXMLRedirectRoot = pContentManager->Load(strXML, false, pMyself);
		//delete pMyself;
		//pMyself = pXMLRedirectRoot;
	}	

	// check all children node
	if (!pXmlNode->NoChildren())
	{
		TiXmlNode * pChildNode = pXmlNode->FirstChild();
		while (pChildNode)
		{
			Ogre::String strName = Ogre::String(pChildNode->Value());
			XML_ATTRIBUTE_MAP map;
			bool bOK = pMyself->LookupAttribute(strName, map);

			if (bOK)
			{
				//if (!pParent) pParent = pMyself;
				if (map.Type == ATTRIBUTE_TYPE_ARRAY)
				{
					CXMLSerialize * pChild = NULL;
					// look into child tree 
					TiXmlNode * pArrayChildNode = pChildNode->FirstChild();
					while (pArrayChildNode)
					{
						pChild = WalkTree(pContentManager, strFilePath, pMyself, pArrayChildNode, &map);
						Variant value;
						value.vt = SYVT_I4;
						value.lVal = *(long*)&pChild;
						pContentManager->SetObjectAttributeValue(pMyself, map.Set, value);
						pArrayChildNode = pArrayChildNode->NextSibling();
					}
				}
				else if (map.Type == ATTRIBUTE_TYPE_OBJECT)
				{
					// create sub object and traverse deeper
					//pContentManager->SetObjectAttributeValue(pParent, map.Set, 
					CXMLSerialize * pChild = NULL;
					pChild = WalkTree(pContentManager, strFilePath, pMyself, pChildNode, &map);
					Variant value;
					value.vt = SYVT_I4;
					value.lVal = *(long*)&pChild;
					pContentManager->SetObjectAttributeValue(pMyself, map.Set, value);

				}
			}


			pChildNode = pChildNode->NextSibling();
		}
	}

	return pMyself;
}

void CXMLContentManager::SetObjectAttributeValue(CXMLSerialize * pObject, ATTRIBUTE_SET funcSet, Variant value)
{
	typedef void (__stdcall * ATTRIBUTE_SET_PROXY)(void *, Variant);

	ATTRIBUTE_SET_PROXY pProc = *(ATTRIBUTE_SET_PROXY*)&funcSet;
	void * pBase = pObject->GetBasePtr();
	(*pProc)(pBase, value);
}

CXMLSerialize * CXMLContentManager::LoadFromString(Ogre::String strXML)
{
	return NULL;
}

CXMLSerialize * CXMLContentManager::Load(Ogre::String strXMLFileName, bool bLoadObject, CXMLSerialize * pSerialize)
{
	TiXmlDocument * pDoc = new TiXmlDocument(Ogre::String(strXMLFileName));
	if (!pDoc->LoadFile())
	{
		return NULL;
	}
	TiXmlNode * pRootNode = pDoc->RootElement();
	if (!pRootNode)
	{
		return NULL;
	}
	int nNodeType = pRootNode->Type();
	while (nNodeType != TiXmlNode::ELEMENT && !pRootNode)
	{
		pRootNode = pRootNode->NextSibling();
		nNodeType = pRootNode->Type();
	}

	CXMLSerialize * pRootObject = WalkTree(this, strXMLFileName, pSerialize, pRootNode, NULL, bLoadObject);
	return pRootObject;
}

/***************************CXMLSerialize***************************/
CXMLSerialize::CXMLSerialize(void)
{

}

CXMLSerialize::~CXMLSerialize(void)
{

}

Ogre::String CXMLSerialize::FormatF(float * pValues, int nNum)
{
	Ogre::String strRet;
	return strRet;
}

Ogre::String CXMLSerialize::FormatDouble(double * pValues, int nNum)
{
	Ogre::String strRet;
	return strRet;
}

