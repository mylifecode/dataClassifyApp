#ifndef _MISMEDICORGANRENDER_
#define _MISMEDICORGANRENDER_
#include <Ogre.h>
#include "MisMedicOrganInterface.h"
class MisMedicOrganRender : public Ogre::MovableObject
{
public:
	class OrganRendSection: public Ogre::Renderable
	{
	public:
		OrganRendSection(MisMedicOrganRender * rendobj);

		~OrganRendSection();

		const Ogre::MaterialPtr& getMaterial(void) const;

		void getRenderOperation(Ogre::RenderOperation& op);

		void getWorldTransforms(Ogre::Matrix4* xform) const;

		Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;

		const Ogre::LightList& getLights(void) const;

		void setMaterialName( const Ogre::String& name, const Ogre::String& groupName  = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

		MisMedicOrganRender * m_RenderObj;

		/// Index data (to allow multiple unconnected chains)
		Ogre::IndexData  * m_SharedIndexData;

		size_t m_IndexCount;

		bool m_IndexNeedRecreating;

		/// Material 
		Ogre::String mMaterialName;

		Ogre::MaterialPtr mMaterial;

		bool m_Actived;

		bool m_Visible;

		//
		std::vector<int> m_BuildFacesIndex;
	};

	friend class OrganRendSection;

	MisMedicOrganRender(const Ogre::String & name);

	~MisMedicOrganRender();

	int  AddLayer(const Ogre::String & matName);

	void UpdateVertexBuffer(const std::vector<MMO_Node> & rendVertex ,
					        const Ogre::Vector3 & aabbMin,
					        const Ogre::Vector3 & aabbmax);

	void UpdateIndexBuffer(const GFPhysAlignedVectorObj<MMO_Face> & originFace,
		                   const GFPhysAlignedVectorObj<MMO_Face> & cuttedFace);

	void setupVertexDeclaration(void);

	void setupVertexBuffers(void);

	void setupIndexBuffers(void);

	// Overridden members follow
	void _notifyCurrentCamera(Ogre::Camera* cam);

	Ogre::Real getBoundingRadius(void) const;

	const Ogre::AxisAlignedBox& getBoundingBox(void) const;

	//const Ogre::MaterialPtr& getMaterial(void) const;
	const Ogre::String& getMovableType(void) const;

	void _updateRenderQueue(Ogre::RenderQueue * queue);

	/// @copydoc MovableObject::visitRenderables
	void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

	void setMaterialName(int layer , int rendsection,Ogre::String nameMaterial);

	Ogre::String GetMaterialName(int layer, int rendsection);
	
	int GetNumLayers()
	{
		return m_OriginlParts.size();
	}
	void SetOriginPartVisibility(bool visible);

	void SetCutPartVisibility(bool visible);

	void DirtyIndexBuffer();
	
	bool IsIndexBufferDirty();

protected:

	bool m_IndexBuffDirty;

	std::vector<OrganRendSection *> m_OriginlParts;

	std::vector<OrganRendSection *> m_CuttedParts;

	/// Vertex data
	Ogre::VertexData * m_SharedVertexData;//section share vertex buffer

	size_t m_VertexCount;

	bool m_VertexDeclDirty;

	bool m_BuffersNeedRecreating;

	/// Do the bounds need redefining?
	mutable bool mBoundsDirty;

	/// AABB
	mutable Ogre::AxisAlignedBox mAABB;

	/// Bounding radius
	mutable Ogre::Real mRadius;

	mutable Ogre::Vector3 m_Center;
};

class MisMedicOrganRenderFactory : public Ogre::MovableObjectFactory
{
protected:
	Ogre::MovableObject* createInstanceImpl( const Ogre::String& name, const Ogre::NameValuePairList* params);

public:
	static void Initalize();

	static void Terminate();

	MisMedicOrganRenderFactory() {}

	~MisMedicOrganRenderFactory();

	static MisMedicOrganRenderFactory* s_dynamicRendfactory;

	static Ogre::String FACTORY_TYPE_NAME;

	const Ogre::String& getType(void) const;

	void destroyInstance( Ogre::MovableObject* obj);
};
#endif