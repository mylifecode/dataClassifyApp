/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperMeshNode;
class CXMLWrapperSceneNode;
class CXMLWrapperStaticScene : public CXMLSerialize
{
public:
	CXMLWrapperStaticScene(void);
	~CXMLWrapperStaticScene(void);

	vector<CXMLWrapperMeshNode *> m_Mesh;
	bool m_flag_Mesh;

	void __stdcall Set_Mesh(Variant value);
	void __stdcall Get_Mesh(Variant * pValue);

	vector<CXMLWrapperSceneNode *> m_Scene;
	bool m_flag_Scene;

	void __stdcall Set_Scene(Variant value);
	void __stdcall Get_Scene(Variant * pValue);

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperStaticScene)
};

