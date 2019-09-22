#pragma once
#include "MisMedicOrganAttachment.h"
#include "MisMedicOrganOrdinary.h"
#include <Ogre.h>

//#include "Painting.h"
#define NAILRENDSHAPECAPVERTNUM 8

class NailRendSection
{
public:
	NailRendSection(Ogre::Vector3 upperPos , Ogre::Vector3 downPos , float radius)
    {
		m_upper  = upperPos;
        m_down   = downPos;
        m_center = (upperPos+downPos)*0.5f;
        m_radius = radius;
		m_AccumLength = 0.0f;
    }
    Ogre::Vector3 m_capVertex[NAILRENDSHAPECAPVERTNUM];
    float m_radius;
    Ogre::Vector3 m_center;
	Ogre::Vector3 m_upper;
	Ogre::Vector3 m_down;
	float m_AccumLength;
};
struct NailPoint
{   
    NailPoint()
    {
        m_face = NULL;
        m_textureCoord = Ogre::Vector2::ZERO;
        m_weight[0] = 0.0f;
        m_weight[1] = 0.0f;
        m_weight[2] = 0.0f;
        m_placeholder = false;
    }

    NailPoint(GFPhysSoftBodyFace * face,Ogre::Vector2 textureCoord, float weight[3], bool placeholder = false) :  m_face(face) 
    {
        m_weight[0] = weight[0];
        m_weight[1] = weight[1];
        m_weight[2] = weight[2];
        m_textureCoord = textureCoord; 
        m_placeholder = placeholder;
    }
    Ogre::Vector3 NailPoint::GetPosition() const
    {
        GFPhysVector3 temp = m_face->m_Nodes[0]->m_CurrPosition * m_weight[0]+
            m_face->m_Nodes[1]->m_CurrPosition * m_weight[1]+
            m_face->m_Nodes[2]->m_CurrPosition * m_weight[2];

        return Ogre::Vector3(temp.x() , temp.y() , temp.z());
    }
    float m_weight[3];
    GFPhysSoftBodyFace * m_face;
    Ogre::Vector2 m_textureCoord;
    bool m_placeholder;
};

struct NailSegment
{
    NailSegment();
    NailSegment(NailPoint point[4],int t);    

    NailPoint m_Point[4];
    bool IsHead;
    bool IsTail;
    bool NeedRend;
};

class MisMedicNail : public MisMedicOrganAttachment
{
    public:
        MisMedicNail();
        MisMedicNail(Ogre::SceneManager * sceneMgr);//copy from MisMedicBindedRope
        ~MisMedicNail();

        bool TryNail( MisMedicOrgan_Ordinary & organ ,  
                      const GFPhysVector3 & cuttercenterlinedirection,
                      const std::set<GFPhysSoftBodyFace*> faces1,
                      const std::set<GFPhysSoftBodyFace*> faces2,
                      const GFPhysVectorObj<GFPhysSoftBodyNode *> nodes1,
                      const GFPhysVectorObj<GFPhysSoftBodyNode *> nodes2,
                      const int & M);
        
        void OnFaceSplittedByCut(std::vector<FaceSplitByCut> & facesSplitted ,GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & facesDeleted, MisMedicOrgan_Ordinary * organ);

        void Update(float deltatime);
        void CreateRendPart(const Ogre::String & name  , Ogre::SceneManager * scenemgr);
        MisMedicOrgan_Ordinary* GetOrgan();

    private:
        bool TryNailInternal( const GFPhysVector3 & cuttercenterlinedirection,
                              const std::set<GFPhysSoftBodyFace*> faces,
                              const GFPhysVectorObj<GFPhysSoftBodyNode *> nodes,
                              const int & M);
        
        void UpdateRendSegments(GFPhysVectorObj<NailSegment>& segments);       
        void RendSegment(const int & begin,const float & delta);

        void deleteSegmentByFace(GFPhysSoftBodyFace * face);
        
    public:        
        GFPhysVectorObj<NailSegment>        m_SegmentVector;
        GFPhysVectorObj<NailPoint>          m_NailPoints;
        Ogre::SceneNode *                   m_SceneNode;

        Ogre::ManualObject *                m_Object; 
    private:
        //PaintingTool painting;
    protected:                
        MisMedicOrgan_Ordinary *            m_NailedOrgan;
        GFPhysVectorObj<NailRendSection>    m_NailRendSect;
        int                                 m_NailID;

		//GFPhysVectorObj<NailPoint> m_TempNailPoints; 
		//GFPhysVectorObj<Ogre::Vector3> m_TempropeNodes;
		//GFPhysVectorObj<Real> m_TempropeNodesAccumulateDist;
};
