/**Author:zx**/
#pragma once
#include "CommonDataStruct.h"
#include "MXXMLWrapper.h"
using namespace::cds;

class CXMLSerialize;
struct XML_ATTRIBUTE_MAP;
struct XML_ATTRIBUTE_FUNC;

typedef CXMLSerialize * (*XML_OBJECT_LOADER)();
typedef void (__stdcall *ATTRIBUTE_SET)(Variant val);
typedef void (__stdcall *ATTRIBUTE_GET)(Variant * pVal);
typedef map<Ogre::String, XML_ATTRIBUTE_MAP> AttributeMap;

MXXMLWRAPPER_API void RegisterXMLClass(Ogre::String strName, Ogre::String strInternalClassName, XML_OBJECT_LOADER Loader);
MXXMLWRAPPER_API CXMLSerialize * LoadXML(Ogre::String strXMLFileName);

extern wstring s2ws(const string & s);
extern string ws2s(const wstring & ws);

enum ATTRIBUTE_TYPE
{
	ATTRIBUTE_TYPE_VALUE,		// XML attribute
	ATTRIBUTE_TYPE_TEXT,		// long text string
	ATTRIBUTE_TYPE_SUBTREE,     // sub tree
	ATTRIBUTE_TYPE_ARRAY,		// array
	ATTRIBUTE_TYPE_OBJECT,		// object
};

struct XML_ATTRIBUTE_MAP
{
	Ogre::String strName;
	ATTRIBUTE_SET Set;
	ATTRIBUTE_GET Get;
	ATTRIBUTE_TYPE Type;
};

struct XML_OBJECT_MAP
{
	Ogre::String strObjectName;
	XML_OBJECT_LOADER Loader;
};

#define DECLARE_SERIALIZATION_CLASS(class_name)\
	public:\
	static void RegisterClass();\
	bool LookupAttribute(Ogre::String strName, XML_ATTRIBUTE_MAP & map, int * pLevel = NULL);\
	static CXMLSerialize * Creator()\
	{\
		return (CXMLSerialize *) new class_name();\
	}\
	Ogre::String GetClassName() { return Ogre::String(#class_name);}\
	void GetAttributeMap(vector<AttributeMap *> & vector)\
	{\
	__super::GetAttributeMap(vector);\
	vector.push_back(&m_attributesMap);\
	}\
	private:\
	static AttributeMap m_attributesMap;

#define BEGIN_IMPL_SERIALIZATION_CLASS(class_name)\
	AttributeMap class_name::m_attributesMap;\
	bool class_name::LookupAttribute(Ogre::String strName, XML_ATTRIBUTE_MAP & map, int * pLevel)\
	{\
		XML_ATTRIBUTE_MAP mapItem;\
		AttributeMap::iterator iterFind;\
		iterFind = m_attributesMap.find(strName);\
		bool bOK = false;\
		if (iterFind == m_attributesMap.end()) {\
			if (pLevel) (*pLevel)++;\
			bOK = __super::LookupAttribute(strName, map, pLevel);\
		}\
		else {\
			bOK = true;\
			mapItem = iterFind->second;\
			map = mapItem;\
		}\
		return bOK;\
	}\
	void class_name::RegisterClass()\
	{\
		XML_ATTRIBUTE_MAP mapItem;\
		Ogre::String strAttrKey;\
		void (__stdcall class_name::*pProc_Set)(Variant);\
		void (__stdcall class_name::*pProc_Get)(Variant *);\
		pProc_Set = NULL;\
		pProc_Get = NULL;\
		Ogre::String strXMLClass;\
		Ogre::String strInternalClassName = Ogre::String(#class_name);

#define REGISTER_CLASS(xml_class)\
	strXMLClass = Ogre::String(#xml_class);

#define REGISTER_ATTRIBUTE(class_name, name, type)\
	mapItem.strName = Ogre::String(#name);\
	strAttrKey = mapItem.strName;\
	pProc_Set = &class_name::Set_##name;\
	pProc_Get = &class_name::Get_##name;\
	mapItem.Set = *(ATTRIBUTE_SET*)&pProc_Set;\
	mapItem.Get = *(ATTRIBUTE_GET*)&pProc_Get;\
	mapItem.Type = ATTRIBUTE_TYPE_##type;\
	m_attributesMap[strAttrKey] = mapItem;

#define INIT_ATTRIBUTE_FLOAT(name)\
	m_flag_##name = false;\
	m_##name = 0.0f;

#define INIT_ATTRIBUTE_FLOAT_VALUE(name, value)\
	m_flag_##name = false;\
	m_##name = value;

#define IMPL_ATTRIBUTE_FLOAT(name)\
	float m_##name;\
	bool m_flag_##name;\
	void __stdcall Set_##name(Variant value)\
	{\
		if (value.vt != SYVT_STR) {\
			m_##name = 0.0f;\
		}\
		else {\
			Ogre::String str = Ogre::String(value.strVal);\
			float f = (float)atof(str.c_str());\
			m_##name = f;\
		}\
		m_flag_##name = true;\
	}\
	void __stdcall Get_##name(Variant * pValue)\
	{\
		if (m_flag_##name)\
		{\
			(*pValue).vt = SYVT_R4;\
			(*pValue).fltVal = m_##name;\
		}\
		else\
		{\
			(*pValue).vt = SYVT_EMPTY;\
		}\
	}

#define INIT_ATTRIBUTE_LONG(name)\
	m_flag_##name = false;\
	m_##name = 0;

#define INIT_ATTRIBUTE_LONG_VALUE(name, value)\
	m_flag_##name = false;\
	m_##name = value;

#define IMPL_ATTRIBUTE_LONG(name)\
	long m_##name;\
	bool m_flag_##name;\
	void __stdcall Set_##name(Variant value)\
	{\
		if (value.vt != SYVT_STR) {\
			m_##name = 0;\
		}\
		else {\
			Ogre::String str = Ogre::String(value.strVal);\
			long l = atol(str.c_str());\
			m_##name = l;\
		}\
		m_flag_##name = true;\
	}\
	void __stdcall Get_##name(Variant * pValue)\
	{\
		if (m_flag_##name)\
		{\
			(*pValue).vt = SYVT_I4;\
			(*pValue).lVal = m_##name;\
		}\
		else\
		{\
			(*pValue).vt = SYVT_EMPTY;\
		}\
	}

#define INIT_ATTRIBUTE_STRING(name)\
	m_flag_##name = false;\
	m_##name = Ogre::String("");

#define INIT_ATTRIBUTE_STRING_VALUE(name, value)\
	m_flag_##name = false;\
	m_##name = value;

#define IMPL_ATTRIBUTE_STRING(name)\
	Ogre::String m_##name;\
	bool m_flag_##name;\
	void __stdcall Set_##name(Variant value)\
	{\
		if (value.vt != SYVT_STR) {\
			m_##name = Ogre::String("");\
		}\
		else {\
			m_##name = value.strVal;\
		}\
		m_flag_##name = true;\
	}\
	void __stdcall Get_##name(Variant * pValue)\
	{\
		if (m_flag_##name)\
		{\
			(*pValue).vt = SYVT_STR;\
			(*pValue).strVal = m_##name;\
		}\
		else\
		{\
			(*pValue).vt = SYVT_EMPTY;\
		}\
	}
	
#define INIT_ATTRIBUTE_BOOL(name)\
	m_flag_##name =false;\
	m_##name = false;

#define INIT_ATTRIBUTE_BOOL_VALUE(name,value)\
	m_flag_##name =false;\
	m_##name = value;

#define IMPL_ATTRIBUTE_BOOL(name)\
	bool m_##name;\
	bool m_flag_##name;\
	void __stdcall Set_##name(Variant value)\
	{\
		m_flag_##name = true;\
		if (value.vt != SYVT_STR) {\
			m_##name = false;\
			m_flag_##name = false;\
		}\
		else {\
			if (value.strVal == Ogre::String("true"))\
			{\
				m_##name = true;\
			}\
			else {\
				m_##name = false;\
			}\
			m_flag_##name = true;\
		}\
	}\
	void __stdcall Get_##name(Variant* pValue)\
	{\
		if(m_flag_##name)\
		{\
			(*pValue).bVal = m_##name;\
			(*pValue).vt = SYVT_BOOL;\
		}\
		else\
		{\
			(*pValue).vt = SYVT_EMPTY;\
		}\
	}

#define INIT_ATTRIBUTE_FLOAT3(name)\
	m_flag_##name =false;\
	m_##name.x = m_##name.y = m_##name.z = 0;

#define INIT_ATTRIBUTE_FLOAT3_VALUE(name,fx,fy,fz)\
	m_flag_##name =false;\
	m_##name.x = fx;\
    m_##name.y = fy;\
    m_##name.z = fz;

#define IMPL_ATTRIBUTE_FLOAT3(name)\
	Ogre::Vector3 m_##name;\
	bool m_flag_##name;\
	void __stdcall Set_##name(Variant value)\
	{\
		if (value.vt != SYVT_STR) {\
			m_##name.x = 0;\
			m_##name.y = 0;\
			m_##name.z = 0;\
			m_flag_##name = false;\
		}\
		else {\
			m_flag_##name = true;\
			Ogre::String strMatrix = value.strVal;\
			sscanf_s(strMatrix.c_str(), "%f,%f,%f", &m_##name.x, &m_##name.y, &m_##name.z);\
		}\
	}\
	void __stdcall Get_##name(Variant* pValue)\
	{\
	}\

#define INIT_ATTRIBUTE_QUATERNION(name)\
	m_flag_##name =false;\
	m_##name.w = m_##name.x = m_##name.y = m_##name.z = 0;

#define INIT_ATTRIBUTE_QUATERNION_VALUE(name,w,x,y,z)\
	m_flag_##name =false;\
	m_##name.w = w; m_##name.x = x; m_##name.y = y; m_##name.z = z;

#define IMPL_ATTRIBUTE_QUATERNION(name)\
	Ogre::Quaternion m_##name;\
	bool m_flag_##name;\
	void __stdcall Set_##name(Variant value)\
	{\
		if (value.vt != SYVT_STR) {\
			m_##name.w = 0;\
			m_##name.x = 0;\
			m_##name.y = 0;\
			m_##name.z = 0;\
		m_flag_##name = false;\
		}\
		else {\
			m_flag_##name = true;\
			Ogre::String strMatrix = value.strVal;\
			sscanf_s(strMatrix.c_str(), "%f,%f,%f,%f", &m_##name.w, &m_##name.x, &m_##name.y, &m_##name.z);\
		}\
	}\
	void __stdcall Get_##name(Variant* pValue)\
	{\
	}\

#define INIT_ATTRIBUTE_COLOR(name)\
	m_flag_##name =false;\
	m_##name = Ogre::ColourValue::White;

#define INIT_ATTRIBUTE_COLOR_VALUE(name,r,g,b,a)\
	m_flag_##name =false;\
	m_##name.r = r; m_##name.g = g; m_##name.b = b; m_##name.a = a;

#define IMPL_ATTRIBUTE_COLOR(name)\
	Ogre::ColourValue m_##name;\
	bool m_flag_##name;\
	void __stdcall Set_##name(Variant value)\
	{\
	if (value.vt != SYVT_STR) {\
	m_##name = Ogre::ColourValue::White;\
	m_flag_##name = false;\
	}\
		else {\
		m_flag_##name = true;\
		Ogre::String strMatrix = value.strVal;\
		sscanf_s(strMatrix.c_str(), "%f,%f,%f,%f", &m_##name.r, &m_##name.g, &m_##name.b, &m_##name.a);\
		}\
	}\
	void __stdcall Get_##name(Variant* pValue)\
	{\
	}\

#define END_IMPL_SERIALIZATION_CLASS\
		::RegisterXMLClass(strXMLClass, strInternalClassName, Creator);\
	}

class MXXMLWRAPPER_API CXMLContentManager
{
public:
	CXMLContentManager();
	~CXMLContentManager();

	CXMLSerialize * Load(Ogre::String strXMLFileName, bool bLoadObject = true, CXMLSerialize * pSerialize = NULL);
	CXMLSerialize * LoadFromString(Ogre::String strXML);
	
	static map<Ogre::String, XML_OBJECT_LOADER> m_objectMap;
	static map<Ogre::String, Ogre::String> m_objectTagMap;

	void SetObjectAttributeValue(CXMLSerialize * pObject, ATTRIBUTE_SET funcSet, Variant value);
};

class CXMLSerialize
{
public:
	CXMLSerialize(void);
	virtual ~CXMLSerialize(void);

	static Ogre::String FormatF(float * pValue, int nNum);
	static Ogre::String FormatDouble(double * pValues, int nNum);

	virtual void Init(){}
	virtual void SetParent(void * pParent){}
	virtual bool LookupAttribute(Ogre::String strName, XML_ATTRIBUTE_MAP & map, int * pLevel = 0) { return false; }
	virtual void GetAttributeMap(vector<AttributeMap*> & vector) { }
	virtual void * GetBasePtr() { return this; }
	virtual Ogre::String GetClassName() { return Ogre::String(" "); }
	virtual bool IsEmpty() { return false; }
	virtual void SetPath(Ogre::String & strPath){}
};