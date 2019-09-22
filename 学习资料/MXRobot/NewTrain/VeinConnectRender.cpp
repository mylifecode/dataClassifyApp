#include "VeinConnectRender.h"
#include "stdafx.h"
//============================================================================================================
class  RendQuadAlphaSort  
{  
public:  
	RendQuadAlphaSort(Ogre::Vector3 campos , Ogre::Vector3 camdir)
	{
		m_campos = campos;
		m_camdir = camdir;
	}
	// Return whether first element is less than the second  
	bool operator () (const VeinConnRendQuad &a , const VeinConnRendQuad & b) const  
	{  
		if(a.m_AlphaSortValue > b.m_AlphaSortValue )
			return true;  
		else
			return false;
	}

	Ogre::Vector3 m_campos;
	Ogre::Vector3 m_camdir;
};

int QSortCmp ( const void *a , const void *b )  
{  
	float alphaDeviate = ((VeinConnRendQuad*)a)->m_AlphaSortValue - ((VeinConnRendQuad *)b)->m_AlphaSortValue;
	if (alphaDeviate < 0)
		return 1;
	else
		return -1;
	//return -(();
	//if()
		//return true;
	//else
		//return false;
}  

void AlphaSort(VeinConnRendQuad rendQuad[], int low, int high)
{  
	if(low >= high)
		return;   

	int i = low;  

	int j = high+1;  

	float pivot = rendQuad[i].m_AlphaSortValue; 

	VeinConnRendQuad temp;   

	while(i<j) 
	{    
		for(i = i+1 ; i < high; i++)   
			if(rendQuad[i].m_AlphaSortValue >= pivot)      
				break;    
		for(j = j-1; j > low; j--)   
			if(rendQuad[j].m_AlphaSortValue <= pivot)        
				break; 
		if(i < j)   
		{   
			temp = rendQuad[i];    
			rendQuad[i] =  rendQuad[j];    
			rendQuad[j] = temp;     
		}  
	}    
	
	temp = rendQuad[j];  
	rendQuad[j] = rendQuad[low]; 
	rendQuad[low] = temp;   
	
	AlphaSort(rendQuad, low, j-1);  
	
	AlphaSort(rendQuad, i, high); 
}
//============================================================================================================
VeinConnectRendable::VeinConnectRendable(const Ogre::String& name)
{
	mVertexData = OGRE_NEW Ogre::VertexData();
	mIndexData = OGRE_NEW Ogre::IndexData();
	mVertexData->vertexCount = 0;
	mVertexData->vertexStart = 0;
	mVertexDeclDirty = true;
}
//============================================================================================================
VeinConnectRendable::~VeinConnectRendable()
{
	if(mVertexData)
	{
		OGRE_DELETE mVertexData;
		mVertexData = 0;
	}

	if(mIndexData)
	{
		OGRE_DELETE mIndexData;
		mIndexData = 0;
	}
}
//============================================================================================================
void VeinConnectRendable::sortQuadUnit(Ogre::Camera * camera , std::vector<VeinConnRendQuad> & quadvec)
{
	Ogre::Vector3 camdir = camera->getDerivedDirection();

	Ogre::Vector3 campos = camera->getDerivedPosition();

	for(size_t q = 0 ; q < quadvec.size() ; q++)
	{
		VeinConnRendQuad & quad = quadvec[q];
		
		Ogre::Vector3 dist = quad.m_center - campos;

		quad.m_AlphaSortValue = dist.dotProduct(camdir);
	}
#if(0)//stl 优化没打开时qsort快于sort
	sort( quadvec.begin() , quadvec.end() , RendQuadAlphaSort(campos , camdir)); 
#else
	qsort(&quadvec[0] , (int)quadvec.size() , sizeof(VeinConnRendQuad) , QSortCmp);  
	//AlphaSort(&quadvec[0] , 0 , (int)quadvec.size()-1);
#endif
}
void VeinConnectRendable::sortQuadUnit(Ogre::Camera * camera , VeinConnRendQuad * quadArray , int QuadNum)
{
	Ogre::Vector3 camdir = camera->getDerivedDirection();

	Ogre::Vector3 campos = camera->getDerivedPosition();

	for(int q = 0 ; q < QuadNum ; q++)
	{
		VeinConnRendQuad & quad = quadArray[q];

		Ogre::Vector3 dist = quad.m_center - campos;

		quad.m_AlphaSortValue = dist.dotProduct(camdir);
	}
	qsort(quadArray , QuadNum , sizeof(VeinConnRendQuad) , QSortCmp);  
}
//============================================================================================================
void VeinConnectRendable::setupVertexDeclaration(void)
{
	if (mVertexDeclDirty)
	{
		Ogre::VertexDeclaration* decl = mVertexData->vertexDeclaration;
		decl->removeAllElements();

		size_t offset = 0;
		// Add a description for the buffer of the positions of the vertices
		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES , 0);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

		decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES , 1);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);

		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_TANGENT);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

		decl->addElement(0, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);


		mVertexDeclDirty = false;
	}
}
//============================================================================================================
void VeinConnectRendable::setupBuffers(void)
{
		setupVertexDeclaration();

		if(m_VertexCount > mVertexData->vertexCount)
		{
				mVertexData->vertexCount = m_VertexCount;
				
				// Create the vertex buffer (always dynamic due to the camera adjust)
				Ogre::HardwareVertexBufferSharedPtr pBuffer =
					Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
					mVertexData->vertexDeclaration->getVertexSize(0),
					mVertexData->vertexCount,
					Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

				// (re)Bind the buffer
				// Any existing buffer will lose its reference count and be destroyed
				mVertexData->vertexBufferBinding->setBinding(0, pBuffer);
		}

		int oldindexNum = (mIndexData->indexBuffer.isNull() ? 0 : mIndexData->indexBuffer->getNumIndexes());
		if (m_IndexCount > oldindexNum)
		{
			mIndexData->indexBuffer =
				Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
				Ogre::HardwareIndexBuffer::IT_16BIT,
				m_IndexCount, // max we can use
				Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		}
}
//========================================================================================================================
void VeinConnectRendable::updateRenderBuffer(Ogre::Camera * camera , VeinConnStrip * stripsArray , int stripsNum)
{
		int maxquadcount = 20;

		m_VertexCount = 4*maxquadcount*stripsNum;//strips.size();

		m_IndexCount  = 6*maxquadcount*stripsNum;//strips.size();

		setupBuffers();

		Ogre::Vector3 camdir = camera->getDerivedDirection();

		Ogre::Vector3 campos = camera->getDerivedPosition();

		//static std::vector<VeinConnRendQuad> rendquads;
		//rendquads.clear();
		//rendquads.reserve(MAXVEINCONBUILDQUADNUM);
		VeinConnRendQuad rendquads[MAXVEINCONBUILDQUADNUM];
		int rendQuadNum = 0;

		//build rend buffer
		mIndexData->indexCount = 0;

		if(m_VertexCount && m_IndexCount)
		{
			Ogre::uint16 * pShort = static_cast<Ogre::uint16*>(mIndexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

			Ogre::HardwareVertexBufferSharedPtr pBuffer = mVertexData->vertexBufferBinding->getBuffer(0);

			void  * pBufferStart = pBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);

			float * pFloat = (float*)pBufferStart;

			//int startidx = 0;

			int vertexStart = 0;

			for(size_t i = 0 ; i < stripsNum/*strips.size()*/ ; i++)
			{
				VeinConnStrip & stripinfo = stripsArray[i];

				pFloat = BuildOneConnect(camera->getDerivedDirection() , stripinfo, rendquads, rendQuadNum, pFloat, vertexStart);
			}
			sortQuadUnit(camera , rendquads , rendQuadNum);

			for(int  i =  0 ; i  <  rendQuadNum ; i++)
			{
				VeinConnRendQuad & quadunit = rendquads[i];

				*pShort++ = quadunit.m_QuadVertIndex[0];
				*pShort++ = quadunit.m_QuadVertIndex[1];
				*pShort++ = quadunit.m_QuadVertIndex[2];

				*pShort++ = quadunit.m_QuadVertIndex[2];
				*pShort++ = quadunit.m_QuadVertIndex[1];
				*pShort++ = quadunit.m_QuadVertIndex[3];//startidx += 4;

				mIndexData->indexCount += 6;
			}

			pBuffer->unlock();
			mIndexData->indexBuffer->unlock();
		}
}
//============================================================================================================
float * VeinConnectRendable::BuildOneConnect(const Ogre::Vector3 & cameraDir, VeinConnStrip & connstrip, VeinConnRendQuad * quadArray, int & quadNum, float * vertexbuffer, int & vertexStart)
{
	Ogre::Vector3 CenterPoints[MAXVEINCONBUILDSEGNUM];
	
	float PointsPathLength[MAXVEINCONBUILDSEGNUM];
	
	int   CenterPointCount = 0;

	Ogre::Vector3 StickACenter = (connstrip.m_adhereA[0] + connstrip.m_adhereA[1])*0.5f;
	
	Ogre::Vector3 StickBCenter = (connstrip.m_adhereB[0] + connstrip.m_adhereB[1])*0.5f;

	//push first build point
	CenterPoints[CenterPointCount] = StickACenter;
	
	PointsPathLength[CenterPointCount] = 0;
	
	CenterPointCount++;

	//build hook part is needed
	if(connstrip.m_InHookState == true)
	{
	   Ogre::Vector3 horizDir = (StickBCenter - StickACenter).normalisedCopy();

	   Ogre::Vector3 HookPt[5];
	   HookPt[1] = connstrip.m_HookPosition - horizDir * 0.06f;
	   HookPt[2] = connstrip.m_HookPosition;
	   HookPt[3] = connstrip.m_HookPosition + horizDir * 0.06f;

	   Ogre::Vector3 t0 = (HookPt[1] - HookPt[2]).normalisedCopy();
	   Ogre::Vector3 t1 = (StickACenter - HookPt[1]).normalisedCopy();
	   Ogre::Vector3 t3 = (t0 + t1).normalisedCopy();
	   HookPt[0] = HookPt[1] + t3 * 0.05f;

	   t1 = (StickBCenter - HookPt[2]).normalisedCopy();
	   t3 = (-t0 + t1).normalisedCopy();

	   HookPt[4] = HookPt[3] + t3 * 0.05f;

	   for (int s = 0; s < 5; s++)
	   {
		   Ogre::Vector3 SegPoint = HookPt[s];

		   Ogre::Vector3 prevPoint = CenterPoints[CenterPointCount - 1];

		   float prevLen = PointsPathLength[CenterPointCount - 1];

		   float curSeglen = (prevPoint - SegPoint).length();

		   CenterPoints[CenterPointCount] = SegPoint;
			 
		   PointsPathLength[CenterPointCount] = curSeglen + prevLen;
			 
		   CenterPointCount++;
	   }
	}

	//final point
	Ogre::Vector3 prevPoint = CenterPoints[CenterPointCount - 1];

	Ogre::Vector3 currPoint = StickBCenter;

	float prevLen = PointsPathLength[CenterPointCount - 1];

	float curSeglen = (prevPoint - currPoint).length();

	CenterPoints[CenterPointCount] = currPoint;

	PointsPathLength[CenterPointCount] = curSeglen + prevLen;
		
	CenterPointCount++;

	float TotalLength = PointsPathLength[CenterPointCount - 1];

	//now construct strip Quad via Reference Point
	int oldquadcount = (int)quadNum;
		
	for (int e = 0; e < CenterPointCount - 1; e++)
	{
		int offset = e*2;

		VeinConnRendQuad rendquad;

		rendquad.m_QuadVertIndex[0] = offset+vertexStart;
		rendquad.m_QuadVertIndex[1] = offset+1+vertexStart;
		rendquad.m_QuadVertIndex[2] = offset+2+vertexStart;
		rendquad.m_QuadVertIndex[3] = offset+3+vertexStart;
			//rendquad.m_transparent = 1.0f;
			//quadvec.push_back(rendquad);
		quadArray[quadNum++] = rendquad;//
	}

	Ogre::Vector3  spanA = connstrip.m_adhereA[1]-connstrip.m_adhereA[0];

	Ogre::Vector3  spanB = connstrip.m_adhereB[1]-connstrip.m_adhereB[0];
		
	Ogre::Vector3  normalVec = (connstrip.m_adhereA[1] - connstrip.m_adhereA[0]).crossProduct(connstrip.m_adhereB[0] - connstrip.m_adhereA[0]).normalisedCopy();

	if (cameraDir.dotProduct(normalVec) > 0)
		normalVec *= -1.0f;

	float * pFloat = (float*)vertexbuffer;

	//Ogre::ColourValue colorStrip = Ogre::ColourValue(1.0f , 1.0f , 1.0f , 1.0f);

	Ogre::Vector3 prevpos0 , prevpos1;

	float spanScale = connstrip.m_SpanScale;

	float InvTotalLen = 1.0f / TotalLength;

	Ogre::ARGB StripColor = connstrip.m_stripValue.getAsARGB();

	for (int e = 0; e < CenterPointCount; e++)
	{
		 float percent = PointsPathLength[e] * InvTotalLen;
			
		 Ogre::Vector3 span = spanA*(1-percent) + spanB*percent;
			
		 float expandScale = 1.6f;

		 Ogre::Vector3 widthhalf = span * 0.5f * expandScale * spanScale;

		 Ogre::Vector3 position0 = CenterPoints[e] - widthhalf;

		 Ogre::Vector3 position1 = CenterPoints[e] + widthhalf;

		 Ogre::Vector2 textcoord0 = connstrip.m_adherAtext[0]*(1-percent) + connstrip.m_adherBtext[0]*percent;
		 Ogre::Vector2 alphtextcoord0 = Ogre::Vector2(0 , percent);

		 Ogre::Vector2 textcoord1 = connstrip.m_adherAtext[1]*(1-percent) + connstrip.m_adherBtext[1]*percent;
		 Ogre::Vector2 alphtextcoord1 = Ogre::Vector2(1 , percent);

		 //position
		 *pFloat++ = position0.x;
		 *pFloat++ = position0.y;
		 *pFloat++ = position0.z;

		 //normal
		 *pFloat++ = normalVec.x;
		 *pFloat++ = normalVec.y;
		 *pFloat++ = normalVec.z;

		 //texture coord
		 *pFloat++ = textcoord0.x;
		 *pFloat++ = textcoord0.y;
			
		 //alpha texture coord
		 *pFloat++ = alphtextcoord0.x;
		 *pFloat++ = alphtextcoord0.y;

		 //tangent
		 *pFloat++ = 1;
		 *pFloat++ = 0;
		 *pFloat++ = 0;

		 //color
		 *((Ogre::ARGB*)pFloat) = StripColor;//colorStrip.getAsARGB();
		 pFloat = (float*)((Ogre::uint8*)pFloat+sizeof(Ogre::ARGB));

		
		 //position
		 *pFloat++ = position1.x;
		 *pFloat++ = position1.y;
		 *pFloat++ = position1.z;

		 //normal
		 *pFloat++ = normalVec.x;
		 *pFloat++ = normalVec.y;
		 *pFloat++ = normalVec.z;

		 //texture coord
		 *pFloat++ = textcoord1.x;
		 *pFloat++ = textcoord1.y;

		 //alpha texture coord
		 *pFloat++ = alphtextcoord1.x;
		 *pFloat++ = alphtextcoord1.y;

		 //tangent
		 *pFloat++ = 1;
		 *pFloat++ = 0;
		 *pFloat++ = 0;

		 //color
		 *((Ogre::ARGB*)pFloat) = StripColor;
		 pFloat = (float*)((Ogre::uint8*)pFloat+sizeof(Ogre::ARGB));

		 if(e >= 1)
		 {
		    VeinConnRendQuad & rendQuad = quadArray[oldquadcount+e-1];
			rendQuad.m_center = 0.25f*(prevpos0+prevpos1+position0+position1);
		 }
		 prevpos0 = position0;
		 prevpos1 = position1;
	}

	vertexStart += (int)CenterPointCount * 2;
		
	return pFloat;
}

//-----------------------------------------------------------------------
void VeinConnectRendable::setMaterialName( const Ogre::String& name, const Ogre::String& groupName)
{
	mMaterialName = name;
	mMaterial = Ogre::MaterialManager::getSingleton().getByName(mMaterialName, groupName);

	if (mMaterial.isNull())
	{
		Ogre::LogManager::getSingleton().logMessage("Can't2 assign material  to BillboardChain  because this "
			"Material does not exist. Have you forgotten to define it in a "
			".material script?");
		mMaterial = Ogre::MaterialManager::getSingleton().getByName("BaseWhiteNoLighting");
		if (mMaterial.isNull())
		{
			OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Can't assign default material "
				"to BillboardChain of . Did "
				"you forget to call MaterialManager::initialise()?",
				"BillboardChain.setMaterialName");
		}
	}
	// Ensure new material loaded (will not load again if already loaded)
	mMaterial->load();
}

//================================================================================================
void VeinConnectRendable::_updateRenderQueue(Ogre::RenderQueue * queue , bool queuepriorityset , bool queueIDSet, Ogre::uint8 queueid)
{
	if (mIndexData && mIndexData->indexCount > 0)
	{
		if (queuepriorityset)
			queue->addRenderable(this, queueid, queue->getDefaultRenderablePriority()+1);
		else if (queueIDSet)
			queue->addRenderable(this, queueid);
		else
			queue->addRenderable(this);
	}
}
//================================================================================================
void VeinConnectRendable::getRenderOperation(Ogre::RenderOperation& op)
{
	op.indexData = mIndexData;
	op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
	op.srcRenderable = this;
	op.useIndexes = true;
	op.vertexData = mVertexData;
}
//================================================================================================
void VeinConnectRendable::getWorldTransforms(Ogre::Matrix4* xform) const
{
	SY_ASSERT(m_moveable);
	*xform = m_moveable->_getParentNodeFullTransform();//_getParentNodeFullTransform();
}
//================================================================================================
const Ogre::LightList& VeinConnectRendable::getLights(void) const
{
	SY_ASSERT(m_moveable);
	return m_moveable->queryLights();
}
//================================================================================================
Ogre::Real VeinConnectRendable::getSquaredViewDepth(const Ogre::Camera* cam) const
{
	Ogre::Vector3 min, max, mid, dist;
	min = mAABB.getMinimum();
	max = mAABB.getMaximum();
	mid = ((max - min) * 0.5) + min;
	dist = cam->getDerivedPosition() - mid;

	return dist.squaredLength();
}
//================================================================================================
const Ogre::MaterialPtr& VeinConnectRendable::getMaterial(void) const
{
	return mMaterial;
}
//================================================================================================
void VeinConnectRendable::updateBoundingBox(void) const
{
	if (mBoundsDirty)
	{
		mAABB.setNull();

		mAABB.setMinimum(-1000, -1000 , -1000);

		mAABB.setMaximum(1000, 1000 , 1000);

		// Set the current radius
		if (mAABB.isNull())
		{
			mRadius = 0.0f;
		}
		else
		{
			mRadius = Ogre::Math::Sqrt(
				std::max(mAABB.getMinimum().squaredLength(),
				mAABB.getMaximum().squaredLength()));
		}

		mBoundsDirty = false;
	}
}

//================================================================================================
VeinConnectStripsObject::VeinConnectStripsObject()
{
	m_Renderable = OGRE_NEW_T(VeinConnectRendable, Ogre::MEMCATEGORY_GENERAL)("");
	m_Renderable->setMaterialName("MisMedical/NewStrip");
	m_Renderable->m_moveable = this;

}
//================================================================================================
VeinConnectStripsObject::VeinConnectStripsObject( Ogre::String nameMaterial )
{
	m_Renderable = OGRE_NEW_T(VeinConnectRendable, Ogre::MEMCATEGORY_GENERAL)("veinConRenderable");
	m_Renderable->setMaterialName( nameMaterial );
	m_Renderable->m_moveable = this;
}

//================================================================================================
VeinConnectStripsObject::~VeinConnectStripsObject()
{
	OGRE_DELETE_T(m_Renderable, VeinConnectRendable, Ogre::MEMCATEGORY_GENERAL);
}
//================================================================================================
void VeinConnectStripsObject::setMaterialName(Ogre::String nameMaterial)
{
	if(m_Renderable)
	   m_Renderable->setMaterialName( nameMaterial );
}

Ogre::String VeinConnectStripsObject::GetMaterialName()
{
	if (m_Renderable)
	{
		return m_Renderable->getMaterial()->getName();
	}
	else
	{
		return Ogre::String("");
	}
	
}
//-------------------------------------------------------------------------------------------------------
void VeinConnectStripsObject::updateBoundingBox(void) const
{
	mAABB.setNull();

	mAABB.setMinimum(-1000, -1000 , -1000);

	mAABB.setMaximum(1000, 1000 , 1000);

	// Set the current radius
	if (mAABB.isNull())
	{
		mRadius = 0.0f;
	}
	else
	{
		mRadius = Ogre::Math::Sqrt(
			std::max(mAABB.getMinimum().squaredLength(),
			mAABB.getMaximum().squaredLength()));
	}
}
//-------------------------------------------------------------------------------------------------------
void VeinConnectStripsObject::_updateRenderQueue(Ogre::RenderQueue * queue)
{
	if(m_Renderable)
	   m_Renderable->_updateRenderQueue(queue , mRenderQueuePrioritySet , mRenderQueueIDSet, mRenderQueueID);

}
//-------------------------------------------------------------------------------------------------------
void VeinConnectStripsObject::visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables)
{
	// only one renderable
	visitor->visit(m_Renderable, 0, false);
}
//================================================================================================
void VeinConnectStripsObject::BuildStrips(Ogre::Camera * rightcam , VeinConnStrip * stripsArray , int stripsNum)
{
	//Ogre::Vector3 rightvec = rightcam->getDerivedDirection();
	m_Renderable->updateRenderBuffer(rightcam , stripsArray , stripsNum);
}
//-------------------------------------------------------------------------------------------------------
const Ogre::String& VeinConnectStripsObject::getMovableType(void) const
{
	return VeinConnectStripsObjectFactory::FACTORY_TYPE_NAME;
}
//---------------------------------------------------------------------
void  VeinConnectStripsObject::_notifyCurrentCamera(Ogre::Camera* cam)
{
	Ogre::MovableObject::_notifyCurrentCamera(cam);
}
//-----------------------------------------------------------------------
Ogre::Real VeinConnectStripsObject::getBoundingRadius(void) const
{
	return mRadius;
}
//-----------------------------------------------------------------------
const Ogre::AxisAlignedBox& VeinConnectStripsObject::getBoundingBox(void) const
{
	updateBoundingBox();
	return mAABB;
}
//-----------------------------------------------------------------------
Ogre::String VeinConnectStripsObjectFactory::FACTORY_TYPE_NAME = "VeinConnStripObj";

VeinConnectStripsObjectFactory::~VeinConnectStripsObjectFactory()
{

}
//-----------------------------------------------------------------------
const Ogre::String& VeinConnectStripsObjectFactory::getType(void) const
{
	return FACTORY_TYPE_NAME;
}
//-----------------------------------------------------------------------
Ogre::MovableObject* VeinConnectStripsObjectFactory::createInstanceImpl( const Ogre::String& name,
																	const Ogre::NameValuePairList* params)
{

	return OGRE_NEW VeinConnectStripsObject();

}
//-----------------------------------------------------------------------
void VeinConnectStripsObjectFactory::destroyInstance( Ogre::MovableObject* obj)
{
	OGRE_DELETE  obj;
}

static VeinConnectStripsObjectFactory * s_dynamicRendfactory = NULL;

void VeinConnectStripsObjectFactory::Initalize()
{
	if(s_dynamicRendfactory == NULL)
	{
		s_dynamicRendfactory = OGRE_NEW VeinConnectStripsObjectFactory();
		Ogre::Root::getSingleton().addMovableObjectFactory(s_dynamicRendfactory);
	}

}

void VeinConnectStripsObjectFactory::Terminate()
{
	if (s_dynamicRendfactory)
	{
		Ogre::Root::getSingleton().removeMovableObjectFactory(s_dynamicRendfactory);
		s_dynamicRendfactory = NULL;
	}
}