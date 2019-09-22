#include "MisMedicTubeShapeRendObject.h"

static bool IntersectsPlane(const Ogre::Vector3 pOrigin , 
							const Ogre::Vector3 dir ,
							const Ogre::Vector3 planenorm ,
							const Ogre::Vector3 planepoint , float & t)
{
	float d = -planenorm.dotProduct(planepoint);

	float denom = planenorm.dotProduct(dir);

	if (fabsf(denom) < FLT_EPSILON)
	{
		// Parallel
		t = 0;
		return false;
	}
	else
	{
		float nom = planenorm.dotProduct(pOrigin) + d;
		t = -(nom/denom);
		return true;
	}
}

MisMedicTubeShapeRendObject::MisMedicTubeShapeRendObject()
{
	m_Object = 0;
	m_SceneNode = 0;
}
//===============================================================================================
MisMedicTubeShapeRendObject::~MisMedicTubeShapeRendObject()
{
	if(m_Object)
	{
		((Ogre::SceneNode*)m_Object->getParentNode())->detachObject(m_Object);

		Ogre::SceneManager * sceneMgr = m_Object->_getManager();

		sceneMgr->destroyManualObject(m_Object);

		m_Object = 0;

		if(m_SceneNode)
		{
			sceneMgr->getRootSceneNode()->removeAndDestroyChild(m_SceneNode->getName());
			m_SceneNode = 0;
		}
	}
}
//==============================================================================================
void MisMedicTubeShapeRendObject::CreateRendPart(const Ogre::String & name  , Ogre::SceneManager * scenemgr)
{
	m_Object = scenemgr->createManualObject(name);
	m_Object->setDynamic(true);
	m_Object->setVisible(true);
	m_Object->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
	
	if(m_Object)
	{
		m_SceneNode = scenemgr->getRootSceneNode()->createChildSceneNode(name+"_Node");
		if(m_SceneNode)
		   m_SceneNode->attachObject(m_Object);
	}
}
void MisMedicTubeShapeRendObject::UpdateRendSegments(std::vector<std::vector<Ogre::Vector3>> & centerPosSec, float RendRadius, bool isLoop)
{
	//update ogre mesh
	m_Object->clear();
	m_Object->setDynamic(true);
	m_Object->estimateVertexCount(1000);
	m_Object->estimateIndexCount(3000);

	for(size_t s = 0 ; s < centerPosSec.size() ; s++)
	{
		m_Object->begin("RopeThreadDefaultMat" , Ogre::RenderOperation::OT_TRIANGLE_LIST);
		UpdateRendPart(centerPosSec[s] , RendRadius , 0 , isLoop ? 1 : 0);
		m_Object->end();
	}
}
void MisMedicTubeShapeRendObject::UpdateRendSegment(std::vector<Ogre::Vector3> & centerPosition, float RendRadius, std::string materialName, bool isLoop)
{
	//update ogre mesh
	m_Object->clear();
	m_Object->setDynamic(true);
	m_Object->estimateVertexCount(1000);
	m_Object->estimateIndexCount(3000);
	if (materialName != "")
	{
		m_Object->begin(materialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);
	}
	else
	{
		m_Object->begin("RopeThreadDefaultMat", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	}
	UpdateRendPart(centerPosition, RendRadius, 0 ,isLoop ? 1 : 0);
	m_Object->end();

	
}
void MisMedicTubeShapeRendObject::UpdateSegmentTagColor(std::vector<Ogre::Vector3> & ropeNodes , 
													    std::vector<bool> & bTagged,
													    std::vector<Ogre::ColourValue> & TagColor,
													    float RendRadius)
{
	m_Object->begin("RopeThreadWithTag" , Ogre::RenderOperation::OT_TRIANGLE_LIST);
	if(ropeNodes.size() == 0)
		return;

	Ogre::Vector3 RotateAxis = ropeNodes[1] - ropeNodes[0];
	RotateAxis.normalise();
	Ogre::Vector3 TangentAxis = RotateAxis.perpendicular();

	m_RopeRendSect.clear();

	for(size_t r = 0 ; r < ropeNodes.size() ; r++)
	{
		m_RopeRendSect.push_back(RopeRendSection(ropeNodes[r] , RendRadius));
	}

	float invetubecapvnum = 1.0f / (float)TUBERENDSHAPECAPVERTNUM;

	//build first section
	for(int a = 0 ; a < TUBERENDSHAPECAPVERTNUM ; a++)
	{
		float rotateRadian = float(Ogre::Math::TWO_PI * a) * invetubecapvnum;

		Ogre::Vector3 capvertex = Ogre::Quaternion(Ogre::Radian(rotateRadian) , RotateAxis)*TangentAxis;

		capvertex = ropeNodes[0] + capvertex * RendRadius;

		m_RopeRendSect[0].m_capVertex[a] = capvertex;
	}

	for(int c = 1 ; c < (int)ropeNodes.size() ; c++)
	{
		RopeRendSection & CurrSection = m_RopeRendSect[c];

		RopeRendSection & PrevSection = m_RopeRendSect[c-1];

		Ogre::Vector3 planenorm;

		if(c < (int)ropeNodes.size()-1)
		{
			Ogre::Vector3 d1 = ropeNodes[c-1] - ropeNodes[c];
			Ogre::Vector3 d2 = ropeNodes[c+1] - ropeNodes[c];

			d1.normalise();
			d2.normalise();

			planenorm = d2-d1;
			float lengthvec = planenorm.length();
			if(lengthvec > FLT_EPSILON)
				planenorm /= lengthvec;
		}
		else
		{
			planenorm = ropeNodes[c]-ropeNodes[c-1];
			float lengthvec = planenorm.length();
			if(lengthvec > FLT_EPSILON)
				planenorm /= lengthvec;
		}

		//get this cap vertex through previous one//planepoint = SectionsInSegment[c].m_center;
		Ogre::Vector3 raydir = ropeNodes[c] - ropeNodes[c-1];
		raydir.normalise();

		Ogre::Vector3 planepoint = ropeNodes[c];
		float t;

		bool intersected = IntersectsPlane(PrevSection.m_capVertex[0], raydir , planenorm ,planepoint, t);

		if(intersected == true)
		{
			Ogre::Vector3 radiusVec = PrevSection.m_capVertex[0] + raydir * t -planepoint;

			radiusVec.normalise();

			for(int s = 0 ; s < TUBERENDSHAPECAPVERTNUM ; s++)
			{
				float rotateRadian = float(Ogre::Math::TWO_PI * s) / ((float)TUBERENDSHAPECAPVERTNUM);

				Ogre::Vector3 capvertex = Ogre::Quaternion(Ogre::Radian(rotateRadian) , planenorm)*radiusVec;

				capvertex = planepoint + capvertex * RendRadius;

				CurrSection.m_capVertex[s] = capvertex;
			}
		}
		else
		{
			for(int v = 0 ; v < TUBERENDSHAPECAPVERTNUM ; v++)
			{
				CurrSection.m_capVertex[v] = PrevSection.m_capVertex[v];
			}
			assert(0 && "error construct tube!");
		}
	}


	//build vertex
	int globalIndexStart = 0;

	int SectionVertNum = TUBERENDSHAPECAPVERTNUM+1;

	for(size_t t = 0 ; t < bTagged.size() ; t++)
	{
		if(bTagged[t])
		{
			Ogre::ColourValue vertColor = TagColor[t];

			for(int c = 0 ; c < 2 ; c++)
			{
				const RopeRendSection & CurrCapVert = m_RopeRendSect[t+c];

				Ogre::Vector3 SectonCenter = ropeNodes[t+c] ;

				for(int d = 0 ; d < TUBERENDSHAPECAPVERTNUM ; d++)
				{
					Ogre::Vector3 SectionVertPos   = CurrCapVert.m_capVertex[d];
					Ogre::Vector3 SectionVertNorm  = (SectionVertPos-SectonCenter).normalisedCopy();

					m_Object->position(SectionVertPos.x , SectionVertPos.y , SectionVertPos.z);
					m_Object->normal(SectionVertNorm.x, SectionVertNorm.y, SectionVertNorm.z);
					m_Object->colour(vertColor);
				}

				//warped vertex
				Ogre::Vector3 SectionVertPos  = CurrCapVert.m_capVertex[0];
				Ogre::Vector3 SectionVertNorm = (SectionVertPos-SectonCenter).normalisedCopy();

				m_Object->position(SectionVertPos.x, SectionVertPos.y, SectionVertPos.z);
				m_Object->normal(SectionVertNorm.x, SectionVertNorm.y, SectionVertNorm.z);
				m_Object->colour(vertColor);
			}
			
			for(int c = 0 ; c < TUBERENDSHAPECAPVERTNUM ; c++)
			{
				int indexStart = globalIndexStart+c;
				m_Object->triangle(indexStart , indexStart+1 , indexStart+SectionVertNum);
				m_Object->triangle(indexStart+SectionVertNum , indexStart+1 , indexStart+SectionVertNum+1);
			}
			globalIndexStart += 2*SectionVertNum;
		}
	}
	
	//for(int c = 1 ; c < TUBERENDSHAPECAPVERTNUM-1 ; c++)
	//	m_Object->triangle(globalIndexStart , globalIndexStart+c , globalIndexStart+c+1);
	//
	m_Object->end();
}
//==============================================================================================
void MisMedicTubeShapeRendObject::UpdateRendPart(std::vector<Ogre::Vector3> & ropeNodes, float RendRadius, int type, int subtype)
{
	if(ropeNodes.size() == 0)
       return;

	Ogre::Vector3 RotateAxis = ropeNodes[1] - ropeNodes[0];
	RotateAxis.normalise();
	
	if (subtype != 0)
	{
		RotateAxis = (ropeNodes[1] - ropeNodes[ropeNodes.size() - 1]).normalisedCopy();
	}
	Ogre::Vector3 TangentAxis = RotateAxis.perpendicular();

	m_RopeRendSect.clear();

	for(size_t r = 0 ; r < ropeNodes.size() ; r++)
	{
		m_RopeRendSect.push_back(RopeRendSection(ropeNodes[r] , RendRadius));
	}

	float invetubecapvnum = 1.0f / (float)TUBERENDSHAPECAPVERTNUM;

	//build first section
	for(int a = 0 ; a < TUBERENDSHAPECAPVERTNUM ; a++)
	{
		float rotateRadian = float(Ogre::Math::TWO_PI * a) * invetubecapvnum;

		Ogre::Vector3 capvertex = Ogre::Quaternion(Ogre::Radian(rotateRadian) , RotateAxis)*TangentAxis;

		capvertex = ropeNodes[0] + capvertex * RendRadius;

		m_RopeRendSect[0].m_capVertex[a] = capvertex;
	}

	for(int c = 1 ; c < (int)ropeNodes.size() ; c++)
	{
		RopeRendSection & CurrSection = m_RopeRendSect[c];

		RopeRendSection & PrevSection = m_RopeRendSect[c-1];

		Ogre::Vector3 planenorm;

		if(c < (int)ropeNodes.size()-1)
		{
			Ogre::Vector3 d1 = ropeNodes[c-1] - ropeNodes[c];
			Ogre::Vector3 d2 = ropeNodes[c+1] - ropeNodes[c];

			d1.normalise();
			d2.normalise();

			planenorm = d2-d1;
			float lengthvec = planenorm.length();
			if(lengthvec > FLT_EPSILON)
				planenorm /= lengthvec;
		}
		else if (subtype == 0)
		{
			planenorm = ropeNodes[c]-ropeNodes[c-1];
			float lengthvec = planenorm.length();
			if(lengthvec > FLT_EPSILON)
				planenorm /= lengthvec;
		}
		else
		{
			Ogre::Vector3 d1 = ropeNodes[c - 1] - ropeNodes[c];
			Ogre::Vector3 d2 = ropeNodes[0] - ropeNodes[c];

			d1.normalise();
			d2.normalise();

			planenorm = d2 - d1;
			float lengthvec = planenorm.length();
			if (lengthvec > FLT_EPSILON)
				planenorm /= lengthvec;
		}

		//get this cap vertex through previous one//planepoint = SectionsInSegment[c].m_center;
		Ogre::Vector3 raydir = ropeNodes[c] - ropeNodes[c-1];
		raydir.normalise();

		Ogre::Vector3 planepoint = ropeNodes[c];
		float t;

		bool intersected = IntersectsPlane(PrevSection.m_capVertex[0], raydir , planenorm ,planepoint, t);

		if(intersected == true)
		{
			Ogre::Vector3 radiusVec = PrevSection.m_capVertex[0] + raydir * t -planepoint;

			radiusVec.normalise();

			for(int s = 0 ; s < TUBERENDSHAPECAPVERTNUM ; s++)
			{
				float rotateRadian = float(Ogre::Math::TWO_PI * s) / ((float)TUBERENDSHAPECAPVERTNUM);

				Ogre::Vector3 capvertex = Ogre::Quaternion(Ogre::Radian(rotateRadian) , planenorm)*radiusVec;

				capvertex = planepoint + capvertex * RendRadius;

				CurrSection.m_capVertex[s] = capvertex;
			}
		}
		else
		{
			for(int v = 0 ; v < TUBERENDSHAPECAPVERTNUM ; v++)
			{
				CurrSection.m_capVertex[v] = PrevSection.m_capVertex[v];
			}
			assert(0 && "error construct tube!");
		}
	}

	
	//build vertex
	for(size_t t = 0 ; t < m_RopeRendSect.size() ; t++)
	{
		const RopeRendSection & CurrCapVert = m_RopeRendSect[t];

		Ogre::Vector3 SectonCenter = ropeNodes[t] ;

		Ogre::Vector3 centerVec;

		if (t < m_RopeRendSect.size() - 1)
		{
			centerVec = (ropeNodes[t + 1] - ropeNodes[t]).normalisedCopy();
		}
		else
		{
			centerVec = (ropeNodes[t] - ropeNodes[t - 1]).normalisedCopy();
		}

		float lenpercent = (float)t / (float)m_RopeRendSect.size();

		for(int c = 0 ; c < TUBERENDSHAPECAPVERTNUM ; c++)
		{
			Ogre::Vector3 SectionVertPos   = CurrCapVert.m_capVertex[c];
			
			Ogre::Vector3 SectionVertNorm  = (SectionVertPos-SectonCenter).normalisedCopy();
			
			Ogre::Vector3 tangent = SectionVertNorm.crossProduct(centerVec).normalisedCopy();

			m_Object->position(SectionVertPos.x , SectionVertPos.y , SectionVertPos.z);
			m_Object->normal(SectionVertNorm.x, SectionVertNorm.y, SectionVertNorm.z);
			m_Object->tangent(tangent);
			m_Object->textureCoord(float(c) / float(TUBERENDSHAPECAPVERTNUM), lenpercent);
			m_Object->colour(1, 1, 1, 1);

		}

		//warped vertex
		Ogre::Vector3 SectionVertPos  = CurrCapVert.m_capVertex[0];
		Ogre::Vector3 SectionVertNorm = (SectionVertPos-SectonCenter).normalisedCopy();
		Ogre::Vector3 tangent = SectionVertNorm.crossProduct(centerVec).normalisedCopy();

		m_Object->position(SectionVertPos.x, SectionVertPos.y, SectionVertPos.z);
		m_Object->normal(SectionVertNorm.x, SectionVertNorm.y, SectionVertNorm.z);
		m_Object->tangent(tangent);
		m_Object->textureCoord(1.0f, lenpercent);
		m_Object->colour(1, 1, 1, 1);
	}

	int globalIndexStart = 0;

	int SectionVertNum = TUBERENDSHAPECAPVERTNUM+1;

	//rope cap vertex
	if (subtype == 0)//not loop to the first section
	{
		for (int c = 1; c < TUBERENDSHAPECAPVERTNUM - 1; c++)
			m_Object->triangle(globalIndexStart, globalIndexStart + c + 1, globalIndexStart + c);
	}

	for(int t = 0 ; t < (int)m_RopeRendSect.size()-1 ; t++)
	{
		for(int c = 0 ; c < TUBERENDSHAPECAPVERTNUM ; c++)
		{
			int indexStart = globalIndexStart+c;
			if(type == 0)
			{
				m_Object->triangle(indexStart , indexStart+1 , indexStart+SectionVertNum);
				m_Object->triangle(indexStart+SectionVertNum , indexStart+1 , indexStart+SectionVertNum+1);
			}
			else
			{
				if(t >= 4 && t <=7)
				{
					m_Object->triangle(indexStart , indexStart+1 , indexStart+SectionVertNum);
					m_Object->triangle(indexStart+SectionVertNum , indexStart+1 , indexStart+SectionVertNum+1);
				}
			}

		}
		globalIndexStart += SectionVertNum;
	}

	if (subtype != 0)//loop to the first section
	{
		for (int c = 0; c < TUBERENDSHAPECAPVERTNUM; c++)
		{
			int indexStart = globalIndexStart + c;
			m_Object->triangle(indexStart, indexStart + 1, c);
			m_Object->triangle(c, indexStart + 1, c + 1);
		}
	}
	if (subtype == 0)//not loop to the first section
	{
		for (int c = 1; c < TUBERENDSHAPECAPVERTNUM - 1; c++)
			m_Object->triangle(globalIndexStart, globalIndexStart + c, globalIndexStart + c + 1);
	}
	//


}
