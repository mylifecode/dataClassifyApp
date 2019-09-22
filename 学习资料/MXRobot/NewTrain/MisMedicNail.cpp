#include "MisMedicNail.h"
#include "MXOgreGraphic.h"
#include "collision\NarrowPhase\GoPhysPrimitiveTest.h"

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

NailSegment::NailSegment()
{
    IsHead = false;
    IsTail = false;
    NeedRend = false;
}


NailSegment::NailSegment(NailPoint point[4],int t)
{
    m_Point[0] = point[0];
    m_Point[1] = point[1];
    m_Point[2] = point[2];
    m_Point[3] = point[3];

    if (t == -1)
    {
        IsHead = true;
        IsTail = false;
    }
    else if (t == 1)
    {
        IsHead = false;
        IsTail = true;
    }
    else
    {
        IsHead = false;
        IsTail = false;
    }            
    NeedRend = true;
}


MisMedicNail::MisMedicNail(Ogre::SceneManager * sceneMgr)
:m_NailedOrgan(0)
{
    m_Object = 0;
    m_SceneNode = 0;

    m_type = MOAType_Nail;
    static int s_NailId = 0;
    s_NailId++;

    m_NailID = s_NailId;
    Ogre::String strNailName = "NailObject" + Ogre::StringConverter::toString(s_NailId);

    CreateRendPart(strNailName , sceneMgr);
    m_SegmentVector.reserve(1000);
    m_NailPoints.reserve(200);
}
MisMedicNail::~MisMedicNail()
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

MisMedicOrgan_Ordinary* MisMedicNail::GetOrgan()
{
    return m_NailedOrgan;
}

void MisMedicNail::CreateRendPart(const Ogre::String & name  , Ogre::SceneManager * scenemgr)
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

	m_Object->setDynamic(true);
	m_Object->begin("ToolObjectNail" , Ogre::RenderOperation::OT_TRIANGLE_LIST);
	
	for(size_t v = 0 ; v < 3 ; v++)
	{
		m_Object->position(Ogre::Vector3::ZERO);
		m_Object->textureCoord(Ogre::Vector2::ZERO);
		m_Object->normal(Ogre::Vector3::UNIT_Z);	
	}
	
	m_Object->triangle(0 , 1 , 2);
	m_Object->end();
}

bool MisMedicNail::TryNail( MisMedicOrgan_Ordinary & organ ,
                            const GFPhysVector3 & cuttercenterlinedirection,
                            const std::set<GFPhysSoftBodyFace*> faces1,
                            const std::set<GFPhysSoftBodyFace*> faces2,
                            const GFPhysVectorObj<GFPhysSoftBodyNode *> nodes1,
                            const GFPhysVectorObj<GFPhysSoftBodyNode *> nodes2,
                            const int & M)
{
    m_NailedOrgan = &organ;

    if (false == TryNailInternal(cuttercenterlinedirection,faces1,nodes1,M))
    {
        return false;
    }
    if (false == TryNailInternal(cuttercenterlinedirection,faces2,nodes2,M))
    {
        return false;
    }
    return true;
}



bool MisMedicNail::TryNailInternal( const GFPhysVector3 & cuttercenterlinedirection,
                                    const std::set<GFPhysSoftBodyFace*> faces,
                                    const GFPhysVectorObj<GFPhysSoftBodyNode *> nodes,
                                    const int & M)
{    
    m_NailPoints.clear();
    if (nodes.empty()||faces.empty())
    {
        return false;
    }

    GFPhysAlignedVectorObj<GFPhysVector3> positions;

    GFPhysVectorObj<GFPhysSoftBodyNode*>::const_iterator itor = nodes.begin();
    while(itor != nodes.end())
    {
        GFPhysSoftBodyNode * node = (*itor);
        positions.push_back(node->m_UnDeformedPos);        
        itor++;
    }

    GFPhysVector3 localnormal , com;
    if (false == CalcPlaneNormalBySVD(positions,localnormal,com))
    {
        return false;   
    }
    //////////////////////////////////////////////////////////////////////////
    std::set<GFPhysSoftBodyFace*>::const_iterator iter = faces.begin();
    GFPhysSoftBodyFace * face = *iter;
    //GFPhysVector3 realnormal = face->m_FaceNormal;

    GFPhysVector3 realnormal = (face->m_Nodes[1]->m_UnDeformedPos - face->m_Nodes[0]->m_UnDeformedPos).Cross(face->m_Nodes[2]->m_UnDeformedPos - face->m_Nodes[0]->m_UnDeformedPos);
    if (localnormal.Dot(realnormal)<0)
    {
        localnormal *= -1.0f;
    }
    //////////////////////////////////////////////////////////////////////////
    /*Real max = 0.0f;
    int index1,index2;
    
    for (int i = 0;i < n-1; i++)
    { 
        for (int j = i+1; j< n;j++)
        {            
            Real templength = (nodes[i]->m_UnDeformedPos- nodes[j]->m_UnDeformedPos).Length2();
            if (max < templength)
            {
                max = templength;
                index1 = i;
                index2 = j;
            }
        }
    }*/
    //////////////////////////////////////////////////////////////////////////
    Real maxium = -GP_INFINITY;
    Real minium = GP_INFINITY;
    int n = (int) nodes.size();
    int index1,index2;
    GFPhysVectorObj<Real> projects;
    for (int i = 0;i < n; i++)
    {
        Real tempdis = nodes[i]->m_UnDeformedPos.Dot(cuttercenterlinedirection);
        projects.push_back(tempdis);
    }
    for (int i = 0;i < n; i++)
    {
        if (projects[i]>maxium)
        {
            maxium = projects[i];
            index1 = i;
        }
    }
    for (int i = 0;i < n; i++)
    {
        if (projects[i]<minium)
        {
            minium = projects[i];
            index2 = i;
        }
    }


    //////////////////////////////////////////////////////////////////////////

    //painting.PushBackPoint(CustomPoint(&(nodes[index1]->m_CurrPosition), Ogre::ColourValue::Red));
    //painting.PushBackPoint(CustomPoint(&(nodes[index2]->m_CurrPosition), Ogre::ColourValue::Blue));

    GFPhysVector3 major_axis; 
    major_axis = nodes[index1]->m_UnDeformedPos-nodes[index2]->m_UnDeformedPos;
    int K = 0;
    GFPhysVector3 temp;
    Real bignum = 1000.0f;
    Real Z1,Z2,Z3;
    GFPhysVector3 SplitSurfaceNormal = (localnormal.Cross(major_axis)).Normalized();

    for (Real beta = 0.01f; beta < 1.0f; beta += 1.0/M)
    {
        temp = beta * nodes[index1]->m_UnDeformedPos + (1-beta) * nodes[index2]->m_UnDeformedPos;    

        Z1 = calculateZ(0,bignum,nodes[index2]->m_UnDeformedPos-nodes[index1]->m_UnDeformedPos,temp);   
        Z2 = calculateZ(0.866f*bignum,-bignum/2.0,nodes[index2]->m_UnDeformedPos-nodes[index1]->m_UnDeformedPos,temp); 
        Z3 = calculateZ(-0.866f*bignum,-bignum/2.0,nodes[index2]->m_UnDeformedPos-nodes[index1]->m_UnDeformedPos,temp);  
        bool getnodelist = false; //if miss all the faces,

        GFPhysVector3 bigTriangle[3];
        bigTriangle[0] = GFPhysVector3(0,bignum,Z1);
        bigTriangle[1] = GFPhysVector3(0.866f*bignum,-bignum/2.0,Z2);
        bigTriangle[2] = GFPhysVector3(-0.866f*bignum,-bignum/2.0,Z3);



        std::map<Real, NailPoint> DistAndPoint;
        for(std::set<GFPhysSoftBodyFace*>::const_iterator iter = faces.begin();
            iter != faces.end();
            iter++)
        {
            GFPhysVector3 ResultPoint[2];

            GFPhysVector3 FaceVerts[3];
            GFPhysSoftBodyFace * face = *iter;
            FaceVerts[0] = face->m_Nodes[0]->m_UnDeformedPos;
            FaceVerts[1] = face->m_Nodes[1]->m_UnDeformedPos;
            FaceVerts[2] = face->m_Nodes[2]->m_UnDeformedPos;
            bool intersect = TriangleIntersect(bigTriangle , FaceVerts , ResultPoint);

            if(intersect)
            {
                getnodelist = true;

                for (int i = 0; i < 2; i++)
                {                    
                    Real dis =  SplitSurfaceNormal.m_x*(ResultPoint[i].m_x - temp.m_x)+
                                SplitSurfaceNormal.m_y*(ResultPoint[i].m_y - temp.m_y)+
                                SplitSurfaceNormal.m_z*(ResultPoint[i].m_z - temp.m_z);

                    float Weights[3] = {-1.0f,-1.0f,-1.0f};
                    CalcBaryCentric(FaceVerts[0],FaceVerts[1],FaceVerts[2],
                        ResultPoint[i],Weights[0],Weights[1],Weights[2]);

                    NailPoint P = NailPoint(face,Ogre::Vector2::ZERO,Weights);
                    DistAndPoint.insert(std::make_pair(dis, P));
                }            
            }
        }

        if (getnodelist)
        {
            //求出DistAndPoint 中两个端点

            if (DistAndPoint.empty())
            {
                return false;
            }
            float max_weight[3];
            float min_weight[3];
            Real maxdis = -GP_INFINITY;
            Real mindis = GP_INFINITY;
            GFPhysSoftBodyFace * FaceWithmaxpos;
            GFPhysSoftBodyFace * FaceWithminpos;

            std::map<Real, NailPoint>::iterator iter = DistAndPoint.begin();
            while(iter != DistAndPoint.end())
            {
                if (iter->first > maxdis)
                {
                    maxdis = iter->first;
                    max_weight[0] = iter->second.m_weight[0];
                    max_weight[1] = iter->second.m_weight[1];
                    max_weight[2] = iter->second.m_weight[2];
                    FaceWithmaxpos = iter->second.m_face;
                }

                if (iter->first < mindis)
                {
                    mindis = iter->first;
                    min_weight[0] = iter->second.m_weight[0];
                    min_weight[1] = iter->second.m_weight[1];
                    min_weight[2] = iter->second.m_weight[2];
                    FaceWithminpos = iter->second.m_face;
                }
                iter++;
            }
            Ogre::Vector2 maxtex(0.2f*(maxdis - mindis),K*2.0/M);
            Ogre::Vector2 mintex(0.8f*(maxdis - mindis),K*2.0/M);

            m_NailPoints.push_back(NailPoint(FaceWithmaxpos ,maxtex, max_weight));                       
            m_NailPoints.push_back(NailPoint(FaceWithminpos ,mintex, min_weight));     
            K++;                        
        }
        else
        {
            ////handle cut but still connect
            //float max_weight[3] = {0.0f,0.0f,0.0f};
            //float min_weight[3] = {0.0f,0.0f,0.0f};
            //GFPhysSoftBodyFace * FaceWithmaxpos = NULL;
            //GFPhysSoftBodyFace * FaceWithminpos = NULL;
            //m_NailPoints.push_back(NailPoint(FaceWithmaxpos ,Ogre::Vector2::ZERO, max_weight, true));                       
            //m_NailPoints.push_back(NailPoint(FaceWithminpos ,Ogre::Vector2::ZERO, min_weight, true));

            continue;
        }
        
    }
    assert(m_NailPoints != NULL); 
    if (m_NailPoints.size() < 4)
    {
        return false;
    }

    int s = m_SegmentVector.size();
    NailPoint point4[4];
    point4[0] = m_NailPoints[0];
    point4[1] = m_NailPoints[1];
    point4[2] = m_NailPoints[2];
    point4[3] = m_NailPoints[3]; 

    m_SegmentVector.push_back(NailSegment(point4,-1));//head

    int len = (int)m_NailPoints.size()/2;
    for (int i = 2; i < 2*len -4; i += 2)
    {
        point4[0] = m_NailPoints[i];
        point4[1] = m_NailPoints[i+1];
        point4[2] = m_NailPoints[i+2];
        point4[3] = m_NailPoints[i+3];
        m_SegmentVector.push_back(NailSegment(point4,0));//middle        
    }
   
    point4[0] = m_NailPoints[m_NailPoints.size()-4];
    point4[1] = m_NailPoints[m_NailPoints.size()-3];
    point4[2] = m_NailPoints[m_NailPoints.size()-2];
    point4[3] = m_NailPoints[m_NailPoints.size()-1];
    m_SegmentVector.push_back(NailSegment(point4,1));//tail   
    
    return true;
}


void MisMedicNail::Update(float deltatime)
{
    //painting.Update(deltatime);
    UpdateRendSegments(m_SegmentVector);
}

void MisMedicNail::UpdateRendSegments(GFPhysVectorObj<NailSegment>& segments)
{
    //update ogre mesh
    // m_Object->clear();
    //m_Object->setDynamic(true);
    float delta = 0.15f;
    //delta = m_NailedOrgan->GetNailRadius();    
	//m_Object->beginUpdate(0);
    int srcNumSect = m_Object->getNumSections();
	
	int dstNumSect = 0;
	
	for (int i = 0;i< (int)segments.size(); i++)
	{
		if (segments[i].IsHead && !segments[i].IsTail && segments[i].NeedRend)
		{
			dstNumSect++;
		}
	}

	if(dstNumSect != srcNumSect)
	{
       m_Object->clear();
	}
	
	int c = 0;
	for (int i = 0;i< (int)segments.size(); i++)
    {
        if (segments[i].IsHead && !segments[i].IsTail && segments[i].NeedRend)
        {
			if(dstNumSect != srcNumSect)
               m_Object->begin("ToolObjectNail" , Ogre::RenderOperation::OT_TRIANGLE_LIST);
			else
			{
			   m_Object->beginUpdate(c);
			   c++;
			}

            RendSegment(i,delta);
            m_Object->end();            
        }
    }
	
	
}


void MisMedicNail::RendSegment(const int & begin,const float & delta)
{
    m_NailRendSect.clear();
	int i = begin;
    m_NailRendSect.push_back(NailRendSection(m_SegmentVector[i].m_Point[0].GetPosition() , m_SegmentVector[i].m_Point[1].GetPosition(), delta));
	do
	{
	   m_NailRendSect.push_back(NailRendSection(m_SegmentVector[i].m_Point[2].GetPosition() , m_SegmentVector[i].m_Point[3].GetPosition(), delta));      
	}
	while(!m_SegmentVector[i++].IsTail);
	

	Real sum = 0.0f;
    for(int r = 0 ; r < m_NailRendSect.size()-1 ; r++)
    {
		sum += ((m_NailRendSect[r+1].m_center - m_NailRendSect[r].m_center).length());//ropeNodesDist[r];
		m_NailRendSect[r+1].m_AccumLength += sum;
    }

    Ogre::Vector3 RotateAxis = m_NailRendSect[1].m_center - m_NailRendSect[0].m_center;
    RotateAxis.normalise();
    Ogre::Vector3 TangentAxis;

    TangentAxis = (m_NailRendSect[0].m_center - m_NailRendSect[0].m_down).crossProduct(RotateAxis);
	TangentAxis.normalise();

    float invetubecapvnum = 1.0f / (float)NAILRENDSHAPECAPVERTNUM;

    //build first section
    for(int a = 0 ; a < NAILRENDSHAPECAPVERTNUM ; a++)
    {
        float rotateRadian = float(Ogre::Math::TWO_PI * a) * invetubecapvnum;

        Ogre::Vector3 VertNormal = Ogre::Quaternion(Ogre::Radian(rotateRadian) , RotateAxis)*TangentAxis;

        Ogre::Vector3 VertPos = m_NailRendSect[0].m_center + VertNormal * delta;

        m_NailRendSect[0].m_capVertex[a] = VertPos;

		m_Object->position(VertPos);
	
		m_Object->textureCoord(a * delta * 5.0f * invetubecapvnum , 0.0f);

		m_Object->normal(VertNormal.x, VertNormal.y, VertNormal.z);
    }
	
	//warped vertex
	Ogre::Vector3 SectionVertPos  = m_NailRendSect[0].m_capVertex[0];
	
	Ogre::Vector3 SectionVertNorm = (SectionVertPos - m_NailRendSect[0].m_center).normalisedCopy();

	m_Object->position(SectionVertPos.x, SectionVertPos.y, SectionVertPos.z);
	
	m_Object->textureCoord(delta * 5.0f , 0.0f);
	
	m_Object->normal(SectionVertNorm.x, SectionVertNorm.y, SectionVertNorm.z);


    for(int c = 1 ; c < (int)m_NailRendSect.size() ; c++)
    {
        NailRendSection & CurrSection = m_NailRendSect[c];

        NailRendSection & PrevSection = m_NailRendSect[c-1];

        Ogre::Vector3 planenorm;

        if(c < (int)m_NailRendSect.size()-1)
        {
            Ogre::Vector3 d1 = m_NailRendSect[c-1].m_center - m_NailRendSect[c].m_center;
            Ogre::Vector3 d2 = m_NailRendSect[c+1].m_center - m_NailRendSect[c].m_center;

            d1.normalise();
            d2.normalise();

            planenorm = d2-d1;
            float lengthvec = planenorm.length();
            if(lengthvec > FLT_EPSILON)
                planenorm /= lengthvec;
        }
        else
        {
            planenorm = m_NailRendSect[c].m_center - m_NailRendSect[c-1].m_center;
            float lengthvec = planenorm.length();
            if(lengthvec > FLT_EPSILON)
                planenorm /= lengthvec;
        }

        //get this cap vertex through previous one//planepoint = SectionsInSegment[c].m_center;
        Ogre::Vector3 raydir = m_NailRendSect[c].m_center - m_NailRendSect[c-1].m_center;
        raydir.normalise();

        Ogre::Vector3 planepoint = m_NailRendSect[c].m_center;

        Ogre::Vector3 radiusVec;// = PrevSection.m_capVertex[0] + raydir * t - planepoint;
            
        radiusVec = (planepoint - m_NailRendSect[c].m_down/*m_TempNailPoints[2*c+1].GetPosition()*/).crossProduct(raydir);

        radiusVec.normalise();

        for(int s = 0 ; s < NAILRENDSHAPECAPVERTNUM ; s++)
        {
            float rotateRadian = float(Ogre::Math::TWO_PI * s) * invetubecapvnum;

            Ogre::Vector3 VertNormal = Ogre::Quaternion(Ogre::Radian(rotateRadian) , planenorm)*radiusVec;

            Ogre::Vector3 VertPos = planepoint + VertNormal * delta;

            CurrSection.m_capVertex[s] = VertPos;

			m_Object->position(VertPos);
		         
			if (c == 0)
			{
				m_Object->textureCoord(s * delta * 5.0f * invetubecapvnum , 0.0f);
			}
			else
			{                
				m_Object->textureCoord(s * delta * 5.0f * invetubecapvnum , 2.0f * CurrSection.m_AccumLength);                
			}
			m_Object->normal(VertNormal.x, VertNormal.y, VertNormal.z);
        }
			
		//warped vertex
		Ogre::Vector3 SectionVertPos  = CurrSection.m_capVertex[0];
			
		Ogre::Vector3 SectionVertNorm = (SectionVertPos-planepoint).normalisedCopy();

		m_Object->position(SectionVertPos.x, SectionVertPos.y, SectionVertPos.z);
			
		m_Object->textureCoord(delta * 5.0f,2.0f * CurrSection.m_AccumLength);
	
		m_Object->normal(SectionVertNorm.x, SectionVertNorm.y, SectionVertNorm.z);
    }

    //build vertex
    int globalIndexStart = 0;

    int SectionVertNum = NAILRENDSHAPECAPVERTNUM+1;

    //rope cap vertex
    for(int c = 1 ; c < NAILRENDSHAPECAPVERTNUM-1 ; c++)
        m_Object->triangle(globalIndexStart , globalIndexStart+c+1 , globalIndexStart+c);

    for(int t = 0 ; t < (int)m_NailRendSect.size()-1 ; t++)
    {
        for(int c = 0 ; c < NAILRENDSHAPECAPVERTNUM ; c++)
        {
            int indexStart = globalIndexStart+c;
            m_Object->triangle(indexStart , indexStart+1 , indexStart+SectionVertNum);
            m_Object->triangle(indexStart+SectionVertNum , indexStart+1 , indexStart+SectionVertNum+1);
        }
        globalIndexStart += SectionVertNum;
    }

    for(int c = 1 ; c < NAILRENDSHAPECAPVERTNUM-1 ; c++)
        m_Object->triangle(globalIndexStart , globalIndexStart+c , globalIndexStart+c+1);    
}

void MisMedicNail::OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted ,GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ)
{
    std::set<GFPhysSoftBodyFace *> facestobedelete;    
    for (size_t i = 0;i<facesDeleted.size();i++)
    {
        facestobedelete.insert(facesDeleted[i]);
    }

    for (std::set<GFPhysSoftBodyFace *>::iterator iter = facestobedelete.begin();
        iter != facestobedelete.end();
        iter++)
    {
        deleteSegmentByFace(*iter);
    }
    
    int firsthead = -1;
    for (int i = 0; i < (int)m_SegmentVector.size(); i++)
    {
        if (m_SegmentVector[i].NeedRend)
        {
            firsthead = i;
            m_SegmentVector[i].IsHead = true;
            break;
        }
    }
    if (firsthead == -1)
    {
        return;
    }
    else
    {
        for (int i = firsthead+1; i< (int)m_SegmentVector.size()-1; i++)
        {
            if (!m_SegmentVector[i].NeedRend)
            {
                if (m_SegmentVector[i-1].NeedRend)
                {
                    m_SegmentVector[i-1].IsTail = true;                    
                }
                if (m_SegmentVector[i+1].NeedRend)
                {
                    m_SegmentVector[i+1].IsHead = true;                    
                }                        
            }
        }
    }    
}

void MisMedicNail::deleteSegmentByFace(GFPhysSoftBodyFace * face)
{    
    for (GFPhysVectorObj<NailSegment>::iterator iter = m_SegmentVector.begin();
        iter != m_SegmentVector.end();
        iter++)
    {
        NailSegment* seg = &(*iter);
        if (seg->NeedRend == true)
        {
            for (int i =0;i<4;i++)
            {
                if (seg->m_Point[i].m_face == face)
                {
                    seg->NeedRend = false;
                    break;                                        
                }
            }
        }        
    }
}