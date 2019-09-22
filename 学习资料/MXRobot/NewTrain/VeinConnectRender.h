#ifndef _VEINCONNECTRENDER_
#define _VEINCONNECTRENDER_
#include <Ogre.h>
#define  MAXVEINCONNECTNUM 500
#define  MAXVEINCONBUILDQUADNUM 8000
#define  MAXVEINCONBUILDSEGNUM  12
struct VeinConnRendQuad
{
	Ogre::Vector3  m_center;//for alpha sort
	unsigned short m_QuadVertIndex[4];
	float m_AlphaSortValue;
};

struct VeinConnStrip
{
	VeinConnStrip()
	{
		m_InHookState = false;
		m_SpanScale = 1.0f;
		m_stripValue = Ogre::ColourValue::White;
	}
	Ogre::Vector3 m_adhereA[2];
	Ogre::Vector3 m_adhereB[2];
	Ogre::Vector2 m_adherAtext[2];
	Ogre::Vector2 m_adherBtext[2];

	Ogre::Vector3 m_HookPosition;
	Ogre::Vector3 m_HookVerticleDir;
	bool m_InHookState;
	float m_SpanScale;
	Ogre::ColourValue m_stripValue;
};

class VeinConnectRendable : public Ogre::Renderable
{
public:
	VeinConnectRendable(const Ogre::String& name);

	~VeinConnectRendable();

	void setupVertexDeclaration(void);

	void setupBuffers(void);

	void updateRenderBuffer(Ogre::Camera * camera , VeinConnStrip * stripsArray , int stripsNum);

	float * BuildOneConnect(const Ogre::Vector3 & cameraDir , VeinConnStrip & stripinfo , VeinConnRendQuad * quadArray , int & quadNum , float * vertexbuffer, int & vertexStart);

	void setMaterialName( const Ogre::String& name, const Ogre::String& groupName  = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

	void sortQuadUnit(Ogre::Camera * camera , std::vector<VeinConnRendQuad> & quadvec);

	void sortQuadUnit(Ogre::Camera * camera , VeinConnRendQuad * quadArray , int QuadNum);
	// Overridden members follow
	Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;

	//************************************
	// Method:    getMaterial
	// FullName:  VeinConnectRendable::getMaterial
	// Access:    public 
	// Returns:   const Ogre::MaterialPtr&
	// Qualifier: const
	// Parameter: void
	//************************************
	const Ogre::MaterialPtr& getMaterial(void) const;

	void _updateRenderQueue(Ogre::RenderQueue * queue , bool queuepriorityset , bool queueIDSet, Ogre::uint8 queueid);

	void getRenderOperation(Ogre::RenderOperation &);

	void getWorldTransforms(Ogre::Matrix4 *) const;

	const Ogre::LightList& getLights(void) const;

	Ogre::MovableObject * m_moveable;

protected:

	virtual void updateBoundingBox(void) const;

	/// Vertex data
	Ogre::VertexData* mVertexData;
	
	/// Index data (to allow multiple unconnected chains)
	Ogre::IndexData* mIndexData;

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

	size_t m_VertexCount;

	size_t m_IndexCount;

};


class VeinConnectStripsObject : public Ogre::MovableObject 
{
public:
	VeinConnectStripsObject();

	VeinConnectStripsObject( Ogre::String nameMaterial );

	~VeinConnectStripsObject();

	void setMaterialName(Ogre::String nameMaterial);
	
	Ogre::String GetMaterialName();

	void BuildStrips(Ogre::Camera * rightcam , VeinConnStrip * stripsArray , int stripsNum);

	void destoryStrips();

	// Overridden members follow
	void _notifyCurrentCamera(Ogre::Camera* cam);

	Ogre::Real getBoundingRadius(void) const;

	const Ogre::AxisAlignedBox& getBoundingBox(void) const;

	//const Ogre::MaterialPtr& getMaterial(void) const;
	const Ogre::String& getMovableType(void) const;

	void _updateRenderQueue(Ogre::RenderQueue *);

	/// @copydoc MovableObject::visitRenderables
	void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

	void updateBoundingBox(void) const;

	void setToolKernel2Vector( Ogre::Vector3 vec, int index );
	bool checkCollisionWithTool();

	Ogre::String& get_setName()
	{
		return m_szName; 
	}
	/// AABB
	mutable Ogre::AxisAlignedBox mAABB;

	/// Bounding radius
	mutable Ogre::Real mRadius;

	VeinConnectRendable * m_Renderable;

	Ogre::String m_szName;

};

/** Factory object for creating BillboardChain instances */
class VeinConnectStripsObjectFactory : public Ogre::MovableObjectFactory
{
protected:
	Ogre::MovableObject* createInstanceImpl( const Ogre::String& name, const Ogre::NameValuePairList* params);

public:
	static void Initalize();

	static void Terminate();

	VeinConnectStripsObjectFactory() {}

	~VeinConnectStripsObjectFactory();

	static Ogre::String FACTORY_TYPE_NAME;

	const Ogre::String& getType(void) const;

	void destroyInstance( Ogre::MovableObject* obj);
};
#endif