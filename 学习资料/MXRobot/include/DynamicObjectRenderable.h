#ifndef _DYNAMICOBJECTRENDERABLE_
#define _DYNAMICOBJECTRENDERABLE_
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include <ogre.h>

using namespace GoPhys;
//class CDynamicObject;
class DynamicObjectRenderable : public Ogre::MovableObject
{
public:

	class DynamicObjRenerSection: public Ogre::Renderable
	{
	public:
		bool m_Actived;

		DynamicObjRenerSection(DynamicObjectRenderable*movobj);

		~DynamicObjRenerSection();

		void setupVertexDeclaration(void);

		void setupBuffers(void);

		const Ogre::MaterialPtr& getMaterial(void) const;

		void getRenderOperation(Ogre::RenderOperation& op);

		void getWorldTransforms(Ogre::Matrix4* xform) const;

		Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;

		const Ogre::LightList& getLights(void) const;

		void setMaterialName( const Ogre::String& name, const Ogre::String& groupName  = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

		DynamicObjectRenderable * m_moveableobj;

		/// Vertex data
		Ogre::VertexData* mVertexData;

		/// Index data (to allow multiple unconnected chains)
		Ogre::IndexData* mIndexData;

		size_t m_VertexCount;

		size_t m_IndexCount;

		bool mVertexDeclDirty;

		bool mBuffersNeedRecreating;
		/// Material 
		Ogre::String mMaterialName;

		Ogre::MaterialPtr mMaterial;
	};

	friend class DynamicObjRenerSection;

	DynamicObjectRenderable(const Ogre::String & name);

	~DynamicObjectRenderable();

	//void updateRenderBuffer(CDynamicObject * dynobj);

	//virtual void updateS3m(CDynamicObject * dynobj);

	//virtual void updateS4m(CDynamicObject * dynobj);

	//virtual void updateS1m(CDynamicObject * dynobj);

	//void updateS3mMapping(CDynamicObject * dynobj);

	// Overridden members follow
	void _notifyCurrentCamera(Ogre::Camera* cam);

	Ogre::Real getBoundingRadius(void) const;

	const Ogre::AxisAlignedBox& getBoundingBox(void) const;

	//const Ogre::MaterialPtr& getMaterial(void) const;
	const Ogre::String& getMovableType(void) const;

	void _updateRenderQueue(Ogre::RenderQueue *);

	/// @copydoc MovableObject::visitRenderables
	void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

	int m_maxbloodparticle;

protected:
	DynamicObjRenerSection * m_MainBody;

	DynamicObjRenerSection * m_FaceCutted;

	DynamicObjRenerSection * m_subdivdface;
	//std::vector<DynamicObjRenerSection*> m_Renderable;
	//virtual void updateBoundingBox(void) const;

	/// Vertex data
	Ogre::VertexData* mVertexData;

	/// Index data (to allow multiple unconnected chains)
	Ogre::IndexData* mIndexData;

	int m_VertexCount;

	int m_IndexCount;
	/// Is the vertex declaration dirty?
	bool mVertexDeclDirty;
	/// Do the buffers need recreating?

	bool mBuffersNeedRecreating;

	/// Do the bounds need redefining?
	mutable bool mBoundsDirty;

	/// Is the index buffer dirty?
	bool mIndexContentDirty;

	/// AABB
	mutable Ogre::AxisAlignedBox mAABB;

	/// Bounding radius
	mutable Ogre::Real mRadius;

	/// Material 
	Ogre::String mMaterialName;

	Ogre::MaterialPtr mMaterial;

};

class DynamicSurfaceFreeDeformObject: public DynamicObjectRenderable
{
public:
	struct ElementCtrlData
	{
		int m_ctrlelefaceid;
		//int m_ctrlelevid[3];
		Ogre::Vector3 m_closetlocalcoord;//closet point in element 's local coordinate
		Ogre::Vector3 m_p2clocalcoord;//project to closet local coordinate
		Ogre::Vector3 m_w2plocalcoord;//warp to project must be something like (0 , 0, z);
		float m_p2cdist;
		float m_w2pdist;
		float m_w2cdist;
		float m_weight;//precalculated 1/weight ^n
	};

	struct ManipulatedVertex
	{
		Ogre::Vector3 m_UnDeformedPos;
		Ogre::Vector3 m_Currposition;
		std::vector<ElementCtrlData> m_ctrldatavec;
	};

	struct RendVertexData
	{
		int manivertid;
		int texcoorid;
		//pre-cached
		Ogre::Vector2 texcoord;
	};
	struct RendFaceData
	{
		int rendvertexindex[3];

		//members pre-loaded for performance
		int texcoordindex[3];
		int manivertindex[3];
		Ogre::Vector2 texcoord[3];
	};

	struct CtrlFace
	{
		int vid[3];
		Ogre::Vector3 m_e0;
		Ogre::Vector3 m_e1;
		Ogre::Vector3 m_e2;
		Ogre::Vector3 m_origin;
	};
	DynamicSurfaceFreeDeformObject(const Ogre::String & name);

	void LoadFFDFile(const char * filename);

	//void updateS4m(CDynamicObject * dynobj);

	//void updateS4mFFD(CDynamicObject * dynobj);

	void updateS4mFFD(GFPhysSoftBody * sb , Ogre::String matName);

	void InitalizeUndeformedPos(GFPhysSoftBody * sb);

	std::vector<Ogre::Vector2>			m_texcoords;
	std::vector<ManipulatedVertex>		m_manivertex;

	std::vector<RendVertexData>		m_rendvertexs;
	std::vector<RendFaceData>			m_rendfaces;

	std::vector<CtrlFace>			m_ctrlfaces;//control face element

	bool m_useffd;
};
/** Factory object for creating BillboardChain instances */
class DynamicObjectRenderableFactory : public Ogre::MovableObjectFactory
{
protected:
	Ogre::MovableObject* createInstanceImpl( const Ogre::String& name, const Ogre::NameValuePairList* params);

public:
	static void Initalize();

	static void Terminate();

	DynamicObjectRenderableFactory() {}

	~DynamicObjectRenderableFactory();

	static Ogre::String FACTORY_TYPE_NAME;

	const Ogre::String& getType(void) const;

	void destroyInstance( Ogre::MovableObject* obj);
};

#endif