#include "MisMedicTubeBody.h"
#include "MisNewTraining.h"
#include "Dynamic\Constraint\GoPhysSoftBodyDistConstraint.h"
#include "math\GoPhysTransformUtil.h"
#include "Math\GoPhysSIMDMath.h"
#include "Math\GoPhysMathUtil.h"
#include "CustomCollision.h"
#include "MisMedicOrganOrdinary.h"
#include "PhysicsWrapper.h"
#include "TrainingMgr.h"
#include "IObjDefine.h"
#include "MXOgreGraphic.h"
#include "Instruments/Tool.h"
#include "MXEventsDump.h"
#include "collision\NarrowPhase\GoPhysSoftbodyFaceConvexCollision.h"
float tube_rscollideMassScale = 0.35f;


//stupid method need optimize
float TUBE_SegmentTriangleClosetDist(GFPhysVector3 triVerts[3] , GFPhysVector3 segVerts[2])
{
	GFPhysVector3 closetPtTube[5];
	GFPhysVector3 closetPtFace[5];

	float s , t;

	closetPtFace[0]   = ClosestPtPointTriangle(segVerts[0], triVerts[0], triVerts[1], triVerts[2]);
	closetPtTube[0] = segVerts[0];

	closetPtFace[1]   = ClosestPtPointTriangle(segVerts[1], triVerts[0], triVerts[1], triVerts[2]);
	closetPtTube[1] = segVerts[1];

	GPClosestPtSegmentSegment( triVerts[0], 
		triVerts[1],
		segVerts[0], 
		segVerts[1],
		s, t, 
		closetPtFace[2], 
		closetPtTube[2]);

	GPClosestPtSegmentSegment(  triVerts[1], 
		triVerts[2],
		segVerts[0], 
		segVerts[1],
		s, t, 
		closetPtFace[3],
		closetPtTube[3]);

	GPClosestPtSegmentSegment(  triVerts[2], 
		triVerts[0],
		segVerts[0], 
		segVerts[1],
		s, t,
		closetPtFace[4],
		closetPtTube[4]);

	Real intersectWeight;
	GFPhysVector3 intersectpt;
	Real triangleWeight[3];
	bool intersectTri = LineIntersectTriangle(triVerts[0] , 
		triVerts[1] , 
		triVerts[2] ,
		segVerts[0], 
		segVerts[1],
		intersectWeight , 
		intersectpt , 
		triangleWeight);
	if(intersectTri == true)
	{
		return 0;
	}

	float minDist = FLT_MAX;
	for(int c = 0 ; c < 5 ; c++)
	{	
		float dist = (closetPtTube[c] - closetPtFace[c]).Length();

		if(dist < minDist)
		{
			minDist  = dist;
		}
	}
	return minDist;
}
//================================================================================================================
TUBESOFTCollidePair::TUBESOFTCollidePair(GFPhysSoftBody * sb)
{
	m_CollideBody = sb;
	m_ImpluseOnTube = Ogre::Vector3(0,0,0);
}
//================================================================================================================
TUBESOFTCollidePair::TUBESOFTCollidePair(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face)
{
	m_CollideBody = sb;
	m_SoftFace = face;
	m_CollideNormal = GPVec3ToOgre((m_SoftFace->m_Nodes[1]->m_CurrPosition-m_SoftFace->m_Nodes[0]->m_CurrPosition).Cross(m_SoftFace->m_Nodes[2]->m_CurrPosition-m_SoftFace->m_Nodes[0]->m_CurrPosition));
	m_CollideNormal.normalise();
	m_ImpluseOnTube = Ogre::Vector3(0,0,0);

}

//================================================================================================================
TUBESOFTCollidePair::TUBESOFTCollidePair(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face , MisMedicTubeBody * tube,int segment)
{
    m_CollideBody = sb;
    m_SoftFace = face;
    m_CollideNormal = GPVec3ToOgre((m_SoftFace->m_Nodes[1]->m_CurrPosition-m_SoftFace->m_Nodes[0]->m_CurrPosition).Cross(m_SoftFace->m_Nodes[2]->m_CurrPosition-m_SoftFace->m_Nodes[0]->m_CurrPosition));
    m_CollideNormal.normalise();
    m_ImpluseOnTube = Ogre::Vector3(0,0,0);

    m_Tube = tube;
    m_SegIndex =segment;

}
//================================================================================================================
int TUBESOFTCollidePair::GetCollideSegmentIndex() const
{
	if(m_CollideType == TUBESOFTCollidePair::TFCD_EE)
	   return m_e3;
	else if(m_CollideType == TUBESOFTCollidePair::TFCD_EE)
	   return m_e1;
	else
	   return -1;
}
//================================================================================================================
GFPhysSoftBody * TUBESOFTCollidePair::GetCollideBody()  const
{
	return m_CollideBody;
}
//================================================================================================================
GFPhysSoftBodyFace * TUBESOFTCollidePair::GetCollideSoftFace()  const
{
	return m_SoftFace;
}
//================================================================================================================
void TUBESOFTCollidePair::SetEE_Collision(int e1 , int e2 , int e3 , int e4 , const Ogre::Vector3 & collideNormal  , const Ogre::Vector3 & FaceNormal)
{
    m_CollideType = TUBESOFTCollidePair::TFCD_EE;
    m_CollideNormal = collideNormal;
    m_FaceNormal = FaceNormal;
    m_e1 = e1;
    m_e2 = e2;
    m_e3 = e3;
    m_e4 = e4;

    m_CollideNode[0] = m_SoftFace->m_Nodes[m_e1];
    m_CollideNode[1] = m_SoftFace->m_Nodes[m_e2];

    m_CollideNodeDepth[0] = 0;
    m_CollideNodeDepth[1] = 0;

    GFPhysSoftBodyTetrahedron * tetra = m_SoftFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
    m_NumCollideNode = 2;
    for(int v = 0 ; v < 4 ; v++)
    {
        GFPhysSoftBodyNode * collideNode = tetra->m_TetraNodes[v];
        if((collideNode != m_CollideNode[0]) && (collideNode != m_CollideNode[1]))
        {
            m_CollideNode[m_NumCollideNode] = collideNode;
		   m_CollideNodeDepth[m_NumCollideNode] = (collideNode->m_CurrPosition-m_CollideNode[0]->m_CurrPosition).Dot(OgreToGPVec3(m_CollideNormal));
		   m_NumCollideNode++;
		}
	}
}
//================================================================================================================
void TUBESOFTCollidePair::SetVF_Collision(int nt , const Ogre::Vector3 & collideNormal  , const Ogre::Vector3 & FaceNormal)
{
	m_CollideType = TUBESOFTCollidePair::TFCD_VF;
	m_CollideNormal = collideNormal;
	m_FaceNormal = FaceNormal;
	m_e1 = nt;

	m_CollideNode[0] = m_SoftFace->m_Nodes[0];
	m_CollideNode[1] = m_SoftFace->m_Nodes[1];
	m_CollideNode[2] = m_SoftFace->m_Nodes[2];

	m_CollideNodeDepth[0] = m_CollideNodeDepth[1] = m_CollideNodeDepth[2] = 0;

	GFPhysSoftBodyTetrahedron * tetra = m_SoftFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
	m_NumCollideNode = 3;
	for(int v = 0 ; v < 4 ; v++)
	{
		GFPhysSoftBodyNode * collideNode = tetra->m_TetraNodes[v];
		if((collideNode != m_CollideNode[0]) && (collideNode != m_CollideNode[1]) && (collideNode != m_CollideNode[2]))
		{
			m_CollideNode[m_NumCollideNode] = collideNode;
			m_CollideNodeDepth[m_NumCollideNode] = (collideNode->m_CurrPosition-m_CollideNode[0]->m_CurrPosition).Dot(OgreToGPVec3(m_CollideNormal));
			m_NumCollideNode++;
		}
	}
}
//================================================================================================================
void TUBESOFTCollidePair::SetSF_Collision(const GFPhysVector3 ClosetPoints[2] , const Real & depth , const GFPhysVector3 & collideormal)
{
    m_CollideType = TUBESOFTCollidePair::TFCD_SF;
    m_CollideNormal = GPVec3ToOgre(collideormal);
    //m_e1 = nt;

    m_CollideNode[0] = m_SoftFace->m_Nodes[0];
    m_CollideNode[1] = m_SoftFace->m_Nodes[1];
    m_CollideNode[2] = m_SoftFace->m_Nodes[2];

    //m_CollideNode[3] = new GFPhysSoftBodyNode();
    //m_CollideNode[4] = new GFPhysSoftBodyNode();

    //GFPhysVector3 n0,n1;
    //bool T = m_Tube->GetTubeSegmentNodePos(n0,n1,m_SegIndex);
    //m_CollideNode[3]->Reset(n0,GFPhysVector3(0,0,0),0.1);// 1.0 represent node mass need to be update!
    //m_CollideNode[4]->Reset(n1,GFPhysVector3(0,0,0),0.1);
    
    m_CollideNodeDepth[0] = m_CollideNodeDepth[1] = m_CollideNodeDepth[2] = depth*0.5;
    m_CollideNodeDepth[3] = m_CollideNodeDepth[4] = -depth*0.5;
    m_CollideDepth = depth;
    m_NumCollideNode = 5;

}

//================================================================================================================
TUBETUBECollidePair::TUBETUBECollidePair(MisMedicTubeBody * TubeA ,
							 MisMedicTubeBody * TubeB , 
							 int SegmentA,
							 int SegmentB)
{
	m_TubeA = TubeA;
	m_TubeB = TubeB;
	m_SegmentA = SegmentA;
	m_SegmentB = SegmentB;
}
//================================================================================================================
int TUBETUBECollidePair::GetCollideSegmentA()  const
{
	return m_SegmentA;
}
//================================================================================================================
int TUBETUBECollidePair::GetCollideSegmentB()  const
{
	return m_SegmentB;
}
//================================================================================================================
//@collision need optimize
//@also check whether SAT method have a lot  jitter effect in contact face(the SAT and closet method should behavier as the same)
class MisTubeSegSoftFaceCallback : public GFPhysNodeOverlapCallback
{
public:
	MisTubeSegSoftFaceCallback(float Margin,
								 GFPhysSoftBody * sb,
								 float collideradius, 
							     MisMedicTubeBody * Tube , 
								 std::vector<TUBESOFTCollidePair> & paircd,
								 bool useCCD) : m_sb(sb),
												m_collideRadius(collideradius+Margin),
												m_Tube(Tube),
												m_CollidePairs(paircd)
																									
	{
		m_UseCCD = useCCD;
	}
	virtual ~MisTubeSegSoftFaceCallback()
	{}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes , GFPhysAABBNode * staticnode)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
	{
		GFPhysSoftBodyFace * softFace = (GFPhysSoftBodyFace*)dynNodeA->m_UserData;		
        int SegIndex = (int)dynNodeB->m_UserData;
		
		GFPhysVector3 SoftFaceVerts[3];
		GFPhysVector3 TubeMovePath[3];

		SoftFaceVerts[0] = softFace->m_Nodes[0]->m_CurrPosition;
		SoftFaceVerts[1] = softFace->m_Nodes[1]->m_CurrPosition;
		SoftFaceVerts[2] = softFace->m_Nodes[2]->m_CurrPosition;

		TubeNode n0 , n1;

		bool succedc = m_Tube->GetTubeSegmentNode(n0 , n1 , SegIndex);

        //////////////////////////////////////////////////////////////////////////
        /// new add 
        GFPhysLineSegmentShape ConvexShape(n0.m_CurrPosition , n1.m_CurrPosition);
        
		GFPhysTransform boxTrans;
        boxTrans.SetIdentity();
        ConvexShape.SetMargin(m_Tube->m_TubeCollideRadius + m_Tube->m_Margin);

            //ConvexShape.SetMargin(m_Tube->GetCollideRadius()); //which margin should be used here?

        GFPhysVector3 SepDirWorld = (SoftFaceVerts[1]-SoftFaceVerts[0]).Cross(SoftFaceVerts[2]-SoftFaceVerts[1]).Normalized();
            
        GFPhysVector3 CenterOfMass = (n0.m_CurrPosition + n1.m_CurrPosition)/2;

        GFPhysVector3 ClosetPoints[2];
        Real PentrateDepth;
        GFPhysVector3 collideormal;
        
		//collidetype = TUBESOFTCollidePair::TFCD_SF;
        bool xenocollision = ConvexSoftFaceClosetPoint(
                SoftFaceVerts,
                ConvexShape, 
                boxTrans,
                SepDirWorld,
                CenterOfMass,
                ClosetPoints,
                PentrateDepth,
                collideormal);
            //
            if(xenocollision == true )
            {
                TUBESOFTCollidePair collidePair(m_sb,softFace,m_Tube,SegIndex);//without segindex we could not get the segment's nodes

               // int collideNormalSign = (collideormal.Dot(SepDirWorld) > 0 ? 1 : -1);

                collidePair.SetSF_Collision(ClosetPoints,PentrateDepth,collideormal);
              
                //if(collideNormalSign == 1)
                {
                    m_CollidePairs.push_back(collidePair);
                }
                //else
               // {
                  //  m_InvCollidePairs.push_back(collidePair);
                //}//for vertex - face collision add normal opposite to face collision to vector for further check
            }
	}
	GFPhysSoftBody * m_sb;
	MisMedicTubeBody * m_Tube;
	float m_collideRadius;

	std::vector<TUBESOFTCollidePair> & m_CollidePairs;
	std::vector<TUBESOFTCollidePair> m_InvCollidePairs;
	bool m_UseCCD;
};

//===================================================================================================
MisMedicTubeBody::MisMedicTubeBody(Ogre::SceneManager * sceneMgr  , MisNewTraining * ownertrain) 
   : m_ToolKernalNode(0) , m_AttachedFace(0) , m_ownertrain(ownertrain) ,m_IsCutAfterBound(false)
{
	static int s_TubeId = 0;
	s_TubeId++;

	Ogre::String strTubeName = "TubeTubeObject" + Ogre::StringConverter::toString(s_TubeId);
	m_RendObject.CreateRendPart(strTubeName , sceneMgr);

	m_UnitLen = 0.18f;//unit length in Tube
	m_TubeCollideRadius = 1.6f;//Tube physics radius
	m_TubeRendRadius = 0.8f;//Tube rend radius
	m_Margin = m_TubeCollideRadius*0.25f;//collision margin
	m_TubeFriction = 0.6f;//Tube vs soft collision coefficients
	m_TubeRSFriction = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.2f);//Tube vs rigid coefficients
	m_UseCCD = false;

	m_Gravity = GFPhysVector3(0,0,-6.0f);
	m_ForceFeed = GFPhysVector3(0,0,0);
	
	m_TotalItorNum = 14;
	
	m_TimesPerItor = m_TotalItorNum / GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum;

	if(m_TimesPerItor < 1)
	   m_TimesPerItor = 1;

	m_SingleItorNum = m_TotalItorNum-m_TimesPerItor*GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum;

	if(m_SingleItorNum < 0)
	   m_SingleItorNum = 0;

	m_TubeBendStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.9f);

	m_TubeStrechStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);

	m_SolveMassScale = 1.0f;

	m_Catergory = OPC_TUBEUNIVERSAL;

	m_EnableSelfCollision = false;

	m_bAttached = false;

	m_RendSegementTag = false;

	m_UseBendForce = true;

	m_NeedRend = true;

	m_InvMassArray = 0;
	
	m_CurrPosArray = 0;
	
	m_UndeformPosArray = 0;
	
	m_NumSMNode = 0;
}
//===================================================================================================
MisMedicTubeBody::~MisMedicTubeBody()
{
	DestoryTube();
}
//===================================================================================================
void MisMedicTubeBody::SetStretchStiffness(float set)
{
	m_TubeStrechStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);

}
//===================================================================================================
void MisMedicTubeBody::SetBendingStiffness(float set)
{
	m_TubeBendStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);
}
//===================================================================================================
void MisMedicTubeBody::SetGravity(const GFPhysVector3 & gravity)
{
	m_Gravity = gravity;
}
//===================================================================================================
#define ADDSOFTTubeFORCE
GFPhysVector3 MisMedicTubeBody::CalcLoopForceFeedBack()
{
	return GFPhysVector3(0,0,0);
}
//=========================================================================================================
void MisMedicTubeBody::DestoryTube()
{
	m_TubeNodes.clear();
	m_FixedNodaIndex.clear();

	m_AttachedFace = 0;
	m_SegmentBVTree.Clear();

	m_TUBESOFTCollidePair.clear();

	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);

	m_IsCutAfterBound = false;
}
//=========================================================================================================
void MisMedicTubeBody::CreateFreeTube(const GFPhysVector3 & StartFixPoint , const GFPhysVector3 & EndFixPoint , int segmetnCount)
{
	DestoryTube();

	m_UnitLen = (StartFixPoint-EndFixPoint).Length() / segmetnCount;

	for(int r = 0 ; r <= segmetnCount ; r++)
	{
		float weight = (float)r / (float)segmetnCount;

		GFPhysVector3 TubePos = StartFixPoint*(1-weight) + EndFixPoint*weight;

		TubeNode Tubenode(TubePos);

		m_TubeNodes.push_back(Tubenode);
	}
	
	m_TopoType = MisMedicTubeBody::TTT_FREE;

	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	m_SegmentState.resize(GetNumSegments());
	for(int s = 0 ; s < GetNumSegments() ; s++)
	{
		m_SegmentState[s] = 0;
	}
}
//=============================================================================================
void MisMedicTubeBody::AttachNodePointToFace(GFPhysSoftBodyFace * attachFace , float weights[3])
{
	m_AttachedFace = attachFace;
	m_AttachWeight[0] = weights[0];
	m_AttachWeight[1] = weights[1];
	m_AttachWeight[2] = weights[2];
}
//=========================================================================================================
void MisMedicTubeBody::DetachNodePointFromFace()
{
	m_AttachedFace = 0;
}
//=========================================================================================================
void MisMedicTubeBody::SimulateTubePhysics(float dt)
{
	UpdateFixedNodes();

	//update tree
	m_SegmentBVTree.Clear();//clear first
	for(int n = 0 ; n < m_TubeNodes.size()-1 ; n++)
	{
		GFPhysVector3 posn0 = m_TubeNodes[n].m_CurrPosition;
		GFPhysVector3 posn1 = m_TubeNodes[n+1].m_CurrPosition;
		
		GFPhysVector3 minPos = posn0;
		GFPhysVector3 maxPos = posn0;

		if(m_UseCCD)
		{
			GFPhysVector3 posl0 = m_TubeNodes[n].m_LastPosition;
			GFPhysVector3 posl1 = m_TubeNodes[n+1].m_LastPosition;

			minPos.SetMin(posn1);
			minPos.SetMin(posl0);
			minPos.SetMin(posl1);

			maxPos.SetMax(posn1);
			maxPos.SetMax(posl0);
			maxPos.SetMax(posl1);
		}
		else
		{
			minPos.SetMin(posn1);
			maxPos.SetMax(posn1);
		}
		
		GFPhysVector3 span(m_TubeCollideRadius+m_Margin,m_TubeCollideRadius+m_Margin,m_TubeCollideRadius+m_Margin);

		GFPhysDBVNode * bvNode = m_SegmentBVTree.InsertAABBNode(minPos-span , maxPos+span);
		bvNode->m_UserData = (void*)n;
	}
	
	//integrate position
	float dampingRate = 5.0f;

	for(size_t n = 0 ; n < m_TubeNodes.size() ; n++)
	{	
		if(m_TubeNodes[n].m_InvMass > FLT_EPSILON)
		{
			m_TubeNodes[n].m_Velocity += m_Gravity*dt;
			
			m_TubeNodes[n].m_FrameCollideTag = false;

			float realdamp = GPClamped(1.0f - dt * dampingRate , 0.0f, 1.0f);

			m_TubeNodes[n].m_Velocity *= realdamp;


			m_TubeNodes[n].m_LastPosition = m_TubeNodes[n].m_CurrPosition;
			m_TubeNodes[n].m_CurrPosition += m_TubeNodes[n].m_Velocity*dt;
		}
	}

	//check collision
	ITraining * itrain = CTrainingMgr::Instance()->GetCurTraining();
	
	MisNewTraining * newTrain = dynamic_cast<MisNewTraining * >(itrain);

	if(newTrain)
	{
		//check collision with soft body
		m_TUBESOFTCollidePair.clear();//clear previous fram pair first

		std::vector<MisMedicOrganInterface*> organsInTrain;

		newTrain->GetAllOrgan(organsInTrain);

		//check collision with soft organ
		for(size_t c = 0 ; c < organsInTrain.size() ; c++)
		{
			MisMedicOrganInterface * oif = organsInTrain[c];

			GFPhysSoftBody * physbody = 0;

			MisMedicOrgan_Ordinary * organOrdinary = dynamic_cast<MisMedicOrgan_Ordinary *>(oif);

			if(organOrdinary)
			{
				physbody = organOrdinary->m_physbody;
			}
			

			if(physbody)
			{
				GFPhysVectorObj<GFPhysDBVTree*> bvTrees = physbody->GetSoftBodyShape().GetFaceBVTrees();

				MisTubeSegSoftFaceCallback collideCallBack(m_Margin , physbody , m_TubeCollideRadius , this , m_TUBESOFTCollidePair , m_UseCCD);

				for(size_t t = 0 ; t < bvTrees.size() ; t++)
				{
					GFPhysDBVTree * bvTree = bvTrees[t];
					bvTree->CollideWithDBVTree(m_SegmentBVTree , &collideCallBack);
				}
			}
		}
	}
}

//=========================================================================================================
GFPhysVector3& MisMedicTubeBody::GetTubeNodePositionByIndex(int index)
{
	return m_TubeNodes[index].m_CurrPosition;
}
//=========================================================================================================
bool MisMedicTubeBody::SetTubeNodePositionByIndex(int index,GFPhysVector3& pos)
{
    if (index < 0 || index > this->GetNumTubeNodes()-1)
    {
        return false;
    } 
    else
    {
        m_TubeNodes[index].m_CurrPosition = pos;
        return true;
    }    
}
//=========================================================================================================
bool MisMedicTubeBody::IsCollideWithOrgan(Ogre::Plane* plane)
{
 	for(size_t t = 0  ; t < m_TUBESOFTCollidePair.size() ; t++)
  	{
  		TUBESOFTCollidePair & tfPair = m_TUBESOFTCollidePair[t];
  		//int segIndex = tfPair.GetCollideSegmentIndex(); 
 		//if(segIndex >= 0)
 		{
			Real mindist = 0.001f;
			Real dist    = mindist;

 			GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
 			if(softFace)
 			{
 				GFPhysSoftBodyNode * pSoftNode = softFace->m_Nodes[0];
 				if( pSoftNode )
 				{					
					Ogre::Vector3 ogrePos(pSoftNode->m_UnDeformedPos.GetX(),pSoftNode->m_UnDeformedPos.GetY(),pSoftNode->m_UnDeformedPos.GetZ());					 
					{
						 
						dist = fabs(plane->getDistance(ogrePos));
						if( dist<mindist )
						{
							mindist = dist;
							return true;
						}
					}
 				}
 			}
 		}
  	}
	return false;
}

Ogre::Vector3  MisMedicTubeBody::GetTubeDir()
{
	return GPVec3ToOgre(m_TubeNodes[m_TubeNodes.size() - 1].m_CurrPosition - m_TubeNodes[m_TubeNodes.size() - 2].m_CurrPosition);
}

//================================================================================================
void MisMedicTubeBody::EndSimuolateTubePhysics(float dt)
{
	//friction
	for(size_t c = 0 ; c < m_TUBESOFTCollidePair.size() ; c++)
	{
		const TUBESOFTCollidePair & cdpair = m_TUBESOFTCollidePair[c];
		if(cdpair.m_CollideType == TUBESOFTCollidePair::TFCD_VF)
		{
			GFPhysSoftBodyFace * softFace = cdpair.m_SoftFace;
			TubeNode & tnode = m_TubeNodes[cdpair.m_e1];

			GFPhysVector3 CollideNormal = OgreToGPVec3(cdpair.m_CollideNormal);//(softFace->m_Nodes[1]->m_CurrPosition-softFace->m_Nodes[0]->m_CurrPosition).Cross(softFace->m_Nodes[2]->m_CurrPosition-softFace->m_Nodes[0]->m_CurrPosition);

			Real normLen = CollideNormal.Length();

			if(normLen > FLT_EPSILON)
			{
				const Real frictionDamp = m_TubeFriction;//0.8f;

				CollideNormal /= normLen;

				GFPhysVector3 NodaVel = tnode.m_CurrPosition-tnode.m_LastPosition;

				GFPhysVector3 NormalVel = CollideNormal*NodaVel.Dot(CollideNormal);

				GFPhysVector3 TangVel = NodaVel-NormalVel;

				TangVel *= 0.9f;//GPClamped(1.0f - dt * frictionDamp , 0.0f, 1.0f);

				tnode.m_CurrPosition = tnode.m_LastPosition+NormalVel+TangVel;

				for(int n = 0 ; n < 3 ; n++)
				{
					GFPhysSoftBodyNode * faceNode = softFace->m_Nodes[n];

					GFPhysVector3 NodaVel = faceNode->m_Velocity;

					GFPhysVector3 NormalVel = CollideNormal*NodaVel.Dot(CollideNormal);

					GFPhysVector3 TangVel = NodaVel-NormalVel;

					TangVel *= GPClamped(1.0f - dt * frictionDamp , 0.0f, 1.0f);

					faceNode->m_Velocity = NormalVel+TangVel;
				}	
			}
		}
	}

	//update velocity
	for(size_t n = 0 ; n < m_TubeNodes.size() ; n++)
	{	
		if(m_TubeNodes[n].m_InvMass > FLT_EPSILON)
		{
			GFPhysVector3 trans = (m_TubeNodes[n].m_CurrPosition-m_TubeNodes[n].m_LastPosition);
			m_TubeNodes[n].m_Velocity = (m_TubeNodes[n].m_CurrPosition-m_TubeNodes[n].m_LastPosition) / dt;
		}
	}


	for(size_t t = 0  ; t < m_TUBESOFTCollidePair.size() ; t++)
	{
		TUBESOFTCollidePair & tfPair = m_TUBESOFTCollidePair[t];
		GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
		if(softFace)
		{
			softFace->m_Nodes[0]->m_InvM = softFace->m_Nodes[0]->m_OriginInvMass;
			softFace->m_Nodes[1]->m_InvM = softFace->m_Nodes[1]->m_OriginInvMass;
			softFace->m_Nodes[2]->m_InvM = softFace->m_Nodes[2]->m_OriginInvMass;

		}
	}
	//
}

//================================================================================================
void MisMedicTubeBody::SolveAdditionConstraint()
{
	if(m_TopoType == MisMedicTubeBody::TTT_FREE)
	   return;

	Ogre::Matrix4 transMat = m_ToolKernalNode->_getFullTransform();

	Ogre::Vector3 temp = transMat.extractQuaternion() * m_LoopAxisLocal;
	temp.normalise();

	GFPhysVector3 loopAxisWorld(temp.x , temp.y , temp.z);

	if(m_TopoType == MisMedicTubeBody::TTT_LOOP)
	{
		//en force virtual bend at root
		int RootBendInterval = 5;
		if((int)m_TubeNodes.size() > 2*(RootBendInterval+2))
		{
			SolveBend(m_VirtualStickNode , m_TubeNodes[0] , m_TubeNodes[RootBendInterval] , m_TubeBendStiffness);
			SolveBend(m_VirtualStickNode , m_TubeNodes[m_TubeNodes.size()-1] , m_TubeNodes[m_TubeNodes.size()-1-RootBendInterval] , m_TubeBendStiffness);
		}
	}
	else
	{
		int RootBendInterval = 5;
		
		if((int)m_TubeNodes.size() > RootBendInterval+2)
		{
		    SolveBend(m_VirtualStickNode , m_TubeNodes[0] , m_TubeNodes[RootBendInterval] , 0.95f);
		}
		if(m_AttachedFace)
		{
			SolveAttachment(m_AttachedFace , m_AttachWeight , GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.98f));
		}
	}
}
//================================================================================================
bool MisMedicTubeBody::GetTubeSegmentNode(TubeNode & n0 , TubeNode & n1 , int segIndex)
{
	if(segIndex >= 0 && segIndex < (int)m_TubeNodes.size()-1)
	{
		n0 = m_TubeNodes[segIndex];
		n1 = m_TubeNodes[segIndex+1];
		return true;
	}
	return false;
}
bool MisMedicTubeBody::GetTubeSegmentNodePos(GFPhysVector3 & n0 , GFPhysVector3 & n1 , int segIndex)
{
	if(segIndex >= 0 && segIndex < (int)m_TubeNodes.size()-1)
	{
		n0 = m_TubeNodes[segIndex].m_CurrPosition;
		n1 = m_TubeNodes[segIndex+1].m_CurrPosition;
		return true;
	}
	return false;
}
//================================================================================================
void  MisMedicTubeBody::SetUnitLen(float UnitLen)
{
	m_UnitLen = UnitLen;
}
//================================================================================================
float MisMedicTubeBody::GetUnitLen()
{
	return m_UnitLen;
}
//================================================================================================
float MisMedicTubeBody::GetTotalLen(bool deformed)
{
	int SegNum = m_TubeNodes.size()-1;
	
	if(deformed == false)
	   return m_UnitLen*SegNum;
	else
	{
	   float totalLen = 0;
	   for(int n = 0 ; n < SegNum ; n++)
	   {
			GFPhysVector3 p0 = m_TubeNodes[n].m_CurrPosition;
			GFPhysVector3 p1 = m_TubeNodes[n+1].m_CurrPosition;

			float t = (p0-p1).Length();
			totalLen += t;
	   }
	   return totalLen;
	}
}
//================================================================================================
MisMedicTubeBody::TubeTopoType MisMedicTubeBody::GetTubeTopoType()
{
	return m_TopoType;
}
//================================================================================================
int MisMedicTubeBody::GetNumTubeNodes()
{
	return m_TubeNodes.size();
}
//================================================================================================
int MisMedicTubeBody::GetNumSegments()
{
	return (int)m_TubeNodes.size()-1;
}	
//================================================================================================
TubeNode MisMedicTubeBody::GetTubeNode(int NodeIndex)
{
	return m_TubeNodes[NodeIndex];
}
//================================================================================================
TubeNode & MisMedicTubeBody::GetTubeNodeRef(int NodeIndex)
{
	return m_TubeNodes[NodeIndex];
}
//================================================================================================
const std::vector<TUBESOFTCollidePair> & MisMedicTubeBody::GetCollidePairs()
{
	return m_TUBESOFTCollidePair;
}
//================================================================================================
void MisMedicTubeBody::UpdateFixedNodes()
{
	if(m_TopoType == TTT_FREE)
	   return;

	Ogre::Matrix4 transMat = m_ToolKernalNode->_getFullTransform();

	for(size_t n = 0 ; n < m_FixedNodaIndex.size() ; n++)
	{
		int NodeIndex = m_FixedNodaIndex[n];

		TubeNode & FixNode = m_TubeNodes[NodeIndex];

		Ogre::Vector3 temp = Ogre::Vector3(FixNode.m_UnDeformedPos.x() , FixNode.m_UnDeformedPos.y() , FixNode.m_UnDeformedPos.z());
		temp = transMat * temp;

		FixNode.m_CurrPosition = FixNode.m_LastPosition = GFPhysVector3(temp.x , temp.y , temp.z);
	}

	Ogre::Vector3 temp = Ogre::Vector3(m_VirtualStickNode.m_UnDeformedPos.m_x , m_VirtualStickNode.m_UnDeformedPos.m_y , m_VirtualStickNode.m_UnDeformedPos.m_z);
	temp = transMat* temp;

	m_VirtualStickNode.m_CurrPosition = m_VirtualStickNode.m_LastPosition = GFPhysVector3(temp.x , temp.y , temp.z);
}

//===================================================================================================
void MisMedicTubeBody::UpdateMesh()
{	


	std::vector<Ogre::Vector3> RendNodes;
	for(size_t n = 0 ; n < m_TubeNodes.size(); n++)
	{
		GFPhysVector3 temp = m_TubeNodes[n].m_CurrPosition;
		RendNodes.push_back(Ogre::Vector3(temp.x() , temp.y() , temp.z()));
	}	
	m_RendObject.UpdateRendSegment(RendNodes , m_TubeRendRadius);


	if(m_RendSegementTag)
	{
		std::vector<bool> segTags;
		std::vector<Ogre::ColourValue> segColors;
		segTags.resize(GetNumSegments());
		segColors.resize(GetNumSegments());
		for(size_t c = 0 ; c < segTags.size() ; c++)
		{
			if(m_SegmentState[c] & TUBEST_INKNOT)
			{
			   segTags[c] = true;
			   segColors[c] = Ogre::ColourValue::Green;
			}
			else
			   segTags[c] = false;
		}

		m_RendObject.UpdateSegmentTagColor( RendNodes , 
											segTags,
											segColors,
											m_TubeRendRadius);
	}
	
}
void MisMedicTubeBody::UpdateMeshByCustomedRendNodes(std::vector<Ogre::Vector3> & RendNodes)
{
	m_RendObject.UpdateRendSegment(RendNodes , m_TubeRendRadius);
}
//===================================================================
void MisMedicTubeBody::SolveBend(TubeNode &  va , TubeNode & vb , TubeNode & vc  , float Stiffness)
{
		if(m_UseBendForce == false)
		   return;
		float wa = (va.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);
		float wb = (vb.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);
		float wc = (vc.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);

		if(va.m_FrameCollideTag)
		   wa *= tube_rscollideMassScale;
	    
		if(vb.m_FrameCollideTag)
		   wb *= tube_rscollideMassScale;
		
		if(vc.m_FrameCollideTag)
		   wc *= tube_rscollideMassScale;

		if(va.m_FrameCollideTag || vb.m_FrameCollideTag || vc.m_FrameCollideTag)
		{
			Stiffness *= 0.3f;
		}

		GFPhysVector3 vab = va.m_CurrPosition - vb.m_CurrPosition;
		GFPhysVector3 vcb = vc.m_CurrPosition - vb.m_CurrPosition;

		float lab = vab.Length();
		float lcb = vcb.Length();

		if(lab * lcb == 0)
		   return;

		float invAB = 1.0f / lab;
		float invCB = 1.0f / lcb;

		GFPhysVector3 n1 = vab*invAB;
		GFPhysVector3 n2 = vcb*invCB;

		float d = n1.Dot(n2);

		GPClamp(d , -1.0f , 1.0f);

		float dd = sqrtf(1-d*d);

		GFPhysVector3 Col0 = GFPhysVector3(1-n1.m_x*n1.m_x , -n1.m_y*n1.m_x , -n1.m_z*n1.m_x) * invAB;
		GFPhysVector3 Col1 = GFPhysVector3(-n1.m_x*n1.m_y , 1-n1.m_y*n1.m_y , -n1.m_z*n1.m_y) * invAB;
		GFPhysVector3 Col2 = GFPhysVector3(-n1.m_x*n1.m_z , -n1.m_y*n1.m_z , 1-n1.m_z*n1.m_z) * invAB;

		GFPhysVector3 gradVa = GFPhysVector3(Col0.Dot(n2) , Col1.Dot(n2) , Col2.Dot(n2));

		Col0 = GFPhysVector3(1-n2.m_x*n2.m_x , -n2.m_y*n2.m_x , -n2.m_z*n2.m_x) * invCB;
		Col1 = GFPhysVector3(-n2.m_x*n2.m_y , 1-n2.m_y*n2.m_y , -n2.m_z*n2.m_y) * invCB;
		Col2 = GFPhysVector3(-n2.m_x*n2.m_z , -n2.m_y*n2.m_z , 1-n2.m_z*n2.m_z) * invCB;

		//@note !! in fact gradVc is gradVc * (-1.0f / dd)
		//but 1.0f/dd may be large and the sumgrad is square of (1.0f/dd) may be large than the float type 
		//precise for numerical robust , we eliminate this factor(-1.0f / dd) with the denomination
		//of sum grad and , finally multiply to result
		GFPhysVector3 gradVc = GFPhysVector3(Col0.Dot(n1) , Col1.Dot(n1) , Col2.Dot(n1));//gradVa *= arcdx;//gradVc *= arcdx;
				
		GFPhysVector3 gradVb = -gradVa-gradVc;

		float sumgrad = wa*gradVa.Length2() + wc*gradVc.Length2() + wb*gradVb.Length2();

		float c = acosf(d) -  3.1415926f;//minus PI(180 degree)

		if( fabsf(sumgrad) > FLT_EPSILON)// && (c > FLT_EPSILON || sumgrad > FLT_EPSILON || dd > FLT_EPSILON))//FLT_EPSILON)
		{
			float s = c * (-dd) / sumgrad;

			GFPhysVector3 deltaVA = -gradVa*(s*wa);

			GFPhysVector3 deltaVB = -gradVb*(s*wb);

			GFPhysVector3 deltaVC = -gradVc*(s*wc);

			float detaA = deltaVA.Length();
			float detaB = deltaVB.Length();
			float detaC = deltaVC.Length();
		
			va.m_CurrPosition += deltaVA*Stiffness;
			vb.m_CurrPosition += deltaVB*Stiffness;
			vc.m_CurrPosition += deltaVC*Stiffness;
		}
}
//===================================================================
GFPhysVector3 MisMedicTubeBody::SolveStretch(TubeNode & Node1 , TubeNode & Node2 , float Stiffness , int interval)
{
	Real RestLen = m_UnitLen*interval;

	Real w1 = Node1.m_InvMass;

	Real w2 = Node2.m_InvMass;

	Real w = w1+w2;

	if(Node1.m_FrameCollideTag)
	   w1 *= tube_rscollideMassScale;
	
	if(Node2.m_FrameCollideTag)
	   w2 *= tube_rscollideMassScale;
	
	GFPhysVector3 impluseDelta(0,0,0);

	if(w > FLT_EPSILON)
	{
		GFPhysVector3 Diff = Node1.m_CurrPosition - Node2.m_CurrPosition;
		Real Length = Diff.Length();

		if(Length > FLT_EPSILON)
		{
			//if(Node1.m_FrameCollideTag && Node2.m_FrameCollideTag)
			//   Stiffness = 0.9f;

			Real InvLength = 1.0f / Length;

			Real Invw1w2 = 1.0f / (w1+w2);

			Real Temp = (Length-RestLen) * InvLength;///Length;

			GFPhysVector3 Deta1 = -(w1 * Invw1w2) * Temp * Diff;

			GFPhysVector3 Deta2 =  (w2 * Invw1w2) * Temp * Diff;

			Node1.m_CurrPosition += Deta1*Stiffness;

			Node2.m_CurrPosition += Deta2*Stiffness;

			if(Length-RestLen > 0)
			   impluseDelta = Diff*InvLength*(Length-RestLen)*Stiffness;
		}
	}
	return impluseDelta;
}
//=============================================================================================
void MisMedicTubeBody::SolveAttachment(GFPhysSoftBodyFace * attachFace , float weights[3] , float stiffness)
{
	GFPhysVector3 pointInFace = attachFace->m_Nodes[0]->m_CurrPosition*weights[0]
							   +attachFace->m_Nodes[1]->m_CurrPosition*weights[1]
							   +attachFace->m_Nodes[2]->m_CurrPosition*weights[2];

	TubeNode & nodeAttach = m_TubeNodes[m_TubeNodes.size()-1];

	GFPhysVector3 Diff = (pointInFace-nodeAttach.m_CurrPosition);

	float diffLen = Diff.Length();

	if(diffLen > FLT_EPSILON)
	{	
		Diff /= diffLen;

		float faceInvMass[3];
		faceInvMass[0] = 1.0f;//attachFace->m_Nodes[0]->m_InvM;
		faceInvMass[1] = 1.0f;//attachFace->m_Nodes[1]->m_InvM;
		faceInvMass[2] = 1.0f;//attachFace->m_Nodes[2]->m_InvM;

		float nodeInvMass =  1.0f;///m_SolveMassScale;//(faceInvMass[0]+faceInvMass[1]+faceInvMass[2]);//*0.33f*0.2f;

		float sumgrad =  weights[0]*weights[0]*faceInvMass[0]
						+weights[1]*weights[1]*faceInvMass[1]
						+weights[2]*weights[2]*faceInvMass[2]
						+nodeInvMass;

		float scale = stiffness*(-diffLen) / sumgrad;

		GFPhysVector3 grad00 = Diff*weights[0];
		GFPhysVector3 grad01 = Diff*weights[1];
		GFPhysVector3 grad02 = Diff*weights[2];

		GFPhysVector3 gradNoda = -Diff;

		GFPhysVector3 delta00 = scale*grad00*faceInvMass[0];
		GFPhysVector3 delta01 = scale*grad01*faceInvMass[1];
		GFPhysVector3 delta02 = scale*grad02*faceInvMass[2];
		GFPhysVector3 deltaNode = scale*gradNoda*nodeInvMass;

		attachFace->m_Nodes[0]->m_CurrPosition += delta00;
		attachFace->m_Nodes[1]->m_CurrPosition += delta01;
		attachFace->m_Nodes[2]->m_CurrPosition += delta02;

		nodeAttach.m_CurrPosition += deltaNode;
	}
}
//===================================================================================================
GFPhysVector3 MisMedicTubeBody::SolveEECollide(const GFPhysVector3 & collideNormal , 
												 GFPhysSoftBodyNode * e1 , 
												 GFPhysSoftBodyNode * e2 ,
												 TubeNode & e3 , 
												 TubeNode & e4 , 
												 const TUBESOFTCollidePair & cdPair)
{
	float Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);
	GFPhysVector3 impluse(0,0,0);

	TubeNode * tNodes[2];
	GFPhysSoftBodyNode * sNodes[4];
	tNodes[0] = &e3;
	tNodes[1] = &e4;

	sNodes[0] = cdPair.m_CollideNode[0];
	sNodes[1] = cdPair.m_CollideNode[1];
	sNodes[2] = cdPair.m_CollideNode[2];
	sNodes[3] = cdPair.m_CollideNode[3];

	for(int t = 0 ; t < 2 ; t++)
	{
		for(int s = 0 ; s < cdPair.m_NumCollideNode ; s++)
		{
			GFPhysVector3 tpos = tNodes[t]->m_CurrPosition;
			GFPhysVector3 spos = sNodes[s]->m_CurrPosition;
			float distCurr = (tpos-spos).Dot(collideNormal)-m_TubeCollideRadius;
			if(distCurr < 0)//solve constraint when negative
			{
				distCurr *= Stiffness;
				if(distCurr < -0.3f)
				   distCurr = -0.3f;

				float wt = tNodes[t]->m_InvMass > 0.0f ? 1.0f / m_SolveMassScale : 0.0f;
				float ws = sNodes[s]->m_InvM > 0.0f ? 1.0f : 0.0f;

				float sumInvW = wt+ws;
				
				if(sumInvW > FLT_EPSILON)
				{
					float moveTube   = -(wt / sumInvW)*distCurr;
					float moveFaceNoda = (ws / sumInvW)*distCurr;

					tNodes[t]->m_CurrPosition += collideNormal*moveTube;
					sNodes[s]->m_CurrPosition += collideNormal*moveFaceNoda;

					impluse += collideNormal*moveTube;
				}
			}
		}
	}
	return impluse;
}
//===================================================================================================
GFPhysVector3 MisMedicTubeBody::SolveVTCollide(const GFPhysVector3 & collideNormal , GFPhysSoftBodyNode * n0 , GFPhysSoftBodyNode * n1 , GFPhysSoftBodyNode * n2 , TubeNode & tn , const TUBESOFTCollidePair & cdPair)
{
	float Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);

	GFPhysVector3 impluse(0,0,0);

	GFPhysVector3 CollideNodePos[4];
	float invMassNode[4];
	GFPhysSoftBodyNode * CollideNodes[4];

	for(int s = 0 ; s < 4 ; s++)
	{
		CollideNodes[s] = cdPair.m_CollideNode[s];
		CollideNodePos[s] = CollideNodes[s]->m_CurrPosition;
		invMassNode[s] = (CollideNodes[s]->m_InvM > 0.0f ? 1.0f : 0.0f);
	}

	for(int n = 0 ; n < cdPair.m_NumCollideNode ; n++)
	{
		GFPhysVector3 q = tn.m_CurrPosition-CollideNodePos[n];
		float nDotQ = collideNormal.Dot(q);
		float distCurr = nDotQ - m_TubeCollideRadius;

		if(distCurr < 0)//solve constraint when negative
		{
			distCurr *= Stiffness;

			if(distCurr < -0.3f)
			   distCurr = -0.3f;

			float InvTubeMass = (tn.m_InvMass > 0.0f ? 1.0f : 0.0f) / m_SolveMassScale;

			float SumInvMass = invMassNode[n] + InvTubeMass;

			if(SumInvMass > FLT_EPSILON)
			{
				float moveTube   = -(InvTubeMass / SumInvMass)*distCurr;
				float moveFaceNoda = (invMassNode[n] / SumInvMass)*distCurr;

				tn.m_CurrPosition += collideNormal*moveTube;
				CollideNodes[n]->m_CurrPosition += collideNormal*moveFaceNoda;

				impluse += collideNormal*moveTube;
			}	
		}
	}
	
	return impluse;
}

//===================================================================================================
GFPhysVector3 MisMedicTubeBody::SolveSFCollide(const GFPhysVector3 & collideNormal , 
                                               GFPhysSoftBodyNode * n0 , 
                                               GFPhysSoftBodyNode * n1 , 
                                               GFPhysSoftBodyNode * n2 , 
                                               TubeNode & tn1 ,
                                               TubeNode & tn2 , 
                                               const TUBESOFTCollidePair & cdPair)
{
    float Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);
    GFPhysVector3 impluse(0,0,0);

    TubeNode * tNodes[2];
    GFPhysSoftBodyNode * sNodes[3];
    tNodes[0] = &tn1;
    tNodes[1] = &tn2;

    sNodes[0] = cdPair.m_CollideNode[0];
    sNodes[1] = cdPair.m_CollideNode[1];
    sNodes[2] = cdPair.m_CollideNode[2];
    

    for(int t = 0 ; t < 2 ; t++)
    {
        for(int s = 0 ; s < 3/*cdPair.m_NumCollideNode*/ ; s++)
        {
            GFPhysVector3 tpos = tNodes[t]->m_CurrPosition;
            GFPhysVector3 spos = sNodes[s]->m_CurrPosition;
            float distCurr = (tpos-spos).Dot(collideNormal)-m_TubeCollideRadius;//fabsf(cdPair.m_CollideDepth);//m_TubeCollideRadius;
            if(distCurr < 0)//solve constraint when negative
            {
                distCurr *= Stiffness;

                float wt = tNodes[t]->m_InvMass > 0.0f ? 1.0f / m_SolveMassScale : 0.0f;
                float ws = sNodes[s]->m_InvM > 0.0f ? 1.0f : 0.0f;

                float sumInvW = wt+ws;

                if(sumInvW > FLT_EPSILON)
                {
                    float moveTube   = -(wt / sumInvW)*distCurr;
                    float moveFaceNoda = (ws / sumInvW)*distCurr;

                    tNodes[t]->m_CurrPosition += collideNormal*moveTube;
                    sNodes[s]->m_CurrPosition += collideNormal*moveFaceNoda;

                    impluse += collideNormal*moveTube;
                }
            }
        }
    }
    return impluse;
}
//===================================================================================================
void MisMedicTubeBody::SolveSoftTubeCollisions()
{
	
	float Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.9f);

	for(size_t c = 0 ; c < m_TUBESOFTCollidePair.size() ; c++)
	{
		TUBESOFTCollidePair & cdpair = m_TUBESOFTCollidePair[c];
		
		GFPhysSoftBodyFace * softFace = cdpair.m_SoftFace;

        

		if(cdpair.m_CollideType == TUBESOFTCollidePair::TFCD_EE)
		{
			/*if(m_TubeNodes[cdpair.m_e3].m_bCollideSoft && m_TubeNodes[cdpair.m_e4].m_bCollideSoft)
			{
				GFPhysVector3 impluse = SolveEECollide(
										OgreToGPVec3(cdpair.m_CollideNormal),
										softFace->m_Nodes[cdpair.m_e1] , 
										softFace->m_Nodes[cdpair.m_e2] , 
										m_TubeNodes[cdpair.m_e3] , 
										m_TubeNodes[cdpair.m_e4] ,
										cdpair);

				cdpair.m_ImpluseOnTube += GPVec3ToOgre(impluse);

			}*/
		}
		else if(cdpair.m_CollideType == TUBESOFTCollidePair::TFCD_VF)
		{
			/*if(m_TubeNodes[cdpair.m_e1].m_bCollideSoft)
			{
				GFPhysVector3 impluse = SolveVTCollide( OgreToGPVec3(cdpair.m_CollideNormal),
														softFace->m_Nodes[0] , 
														softFace->m_Nodes[1] , 
														softFace->m_Nodes[2] ,
														m_TubeNodes[cdpair.m_e1],
														cdpair);

				cdpair.m_ImpluseOnTube += GPVec3ToOgre(impluse);
			}*/

		}
        else if(cdpair.m_CollideType == TUBESOFTCollidePair::TFCD_SF)
        {
            //if(m_TubeNodes[cdpair.m_e1].m_bCollideSoft)
            {
                TubeNode n0,n1;
                bool T = this->GetTubeSegmentNode(n0,n1,cdpair.m_SegIndex);
                GFPhysVector3 impluse = SolveSFCollide( OgreToGPVec3(cdpair.m_CollideNormal),
                    softFace->m_Nodes[0] , 
                    softFace->m_Nodes[1] , 
                    softFace->m_Nodes[2] ,
                    n0,
                    n1,
                    cdpair);

                cdpair.m_ImpluseOnTube += GPVec3ToOgre(impluse);
            }

        }
	}
}
//============================================================================================================================================
void MisMedicTubeBody::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	//
	for(size_t t = 0  ; t < m_TUBESOFTCollidePair.size() ; t++)
	{
		TUBESOFTCollidePair & tfPair = m_TUBESOFTCollidePair[t];

		int segIndex = tfPair.GetCollideSegmentIndex();

		if(segIndex >= 0)
		{
			m_TubeNodes[segIndex].m_FrameCollideTag = true;
			m_TubeNodes[segIndex+1].m_FrameCollideTag = true;
		}

		GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
		if(softFace)
		{
			//softFace->m_Nodes[0]->m_InvM = softFace->m_Nodes[0]->m_OriginInvMass*0.2f;
			//softFace->m_Nodes[1]->m_InvM = softFace->m_Nodes[1]->m_OriginInvMass*0.2f;
			//softFace->m_Nodes[2]->m_InvM = softFace->m_Nodes[2]->m_OriginInvMass*0.2f;
            
           // softFace->m_Nodes[0]->m_StateFlag |= GPSESF_COLLIDRIGID;
           // softFace->m_Nodes[1]->m_StateFlag |= GPSESF_COLLIDRIGID;
           // softFace->m_Nodes[2]->m_StateFlag |= GPSESF_COLLIDRIGID;

		}
	}
	//float originLen = GetTotalLen(false);
	//float currLen   = GetTotalLen(true);
	m_SolveMassScale = 1.0f;
	//if(currLen > originLen)
	//{
	//	float t = currLen / originLen;
		//m_SolveMassScale *= (t*t*t);
	//}
	
#if(0)
	//apply rigid force
	int numNode = GetNumTubeNodes();
	if(m_NumSMNode < numNode)
	{
		if(m_InvMassArray)
		   delete []m_InvMassArray;
		m_InvMassArray = new float[numNode];

		if(m_CurrPosArray)
		   delete []m_CurrPosArray;
		m_CurrPosArray = new GFPhysVector3[numNode];

		if(m_UndeformPosArray)
		   delete []m_UndeformPosArray;
		m_UndeformPosArray = new GFPhysVector3[numNode];


		int interval = numNode;

		for(int n = 0 ; n <= numNode-interval ; n++)
		{
			for(int k = 0 ; k < interval ; k++)
			{
				const TubeNode & tnode = GetTubeNodeRef(n+k);

				m_UndeformPosArray[k] = tnode.m_UnDeformedPos;

				m_CurrPosArray[k] = tnode.m_CurrPosition;

				float invMass = (tnode.m_InvMass > FLT_EPSILON ? 1 : 100);

				m_InvMassArray[k] = invMass;
			}
			
			GFPhysSoftShapeMatchSolver::Solve(m_InvMassArray , m_CurrPosArray, m_UndeformPosArray , interval , 0.01f);
			
			for(int k = 0 ; k < interval ; k++)
			{
				TubeNode & tnode = GetTubeNodeRef(n+k);
				if(tnode.m_InvMass > FLT_EPSILON)
				   tnode.m_CurrPosition = m_CurrPosArray[k];
			}

		}
	}
#endif
	return;
}

#define ADDITIONALBENDSUPPORT 1
void MisMedicTubeBody::SolveConstraint(Real Stiffness,Real TimeStep)
{
	//solve constraint
	float stretchStiffness = m_TubeStrechStiffness;//1.0f;//GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(1.0f);

	float bendStiffness = m_TubeBendStiffness;//0.9f;//GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.999f);

	//for(int i = 0 ; i < 2 ; i++)
	{
		//solve stretch constraint
		int lastSeg = -1;
		for(int n = 0 ; n < (int)m_TubeNodes.size()-1 ; n += 2)
		{	
			lastSeg = n;
			SolveStretch(m_TubeNodes[n] , m_TubeNodes[n+1] , stretchStiffness , 1);
		}

		for(int k = (lastSeg == m_TubeNodes.size()-2 ? m_TubeNodes.size()-3 : m_TubeNodes.size()-2) ; k >= 0 ; k -= 2)
		{	
			SolveStretch(m_TubeNodes[k] , m_TubeNodes[k+1] , stretchStiffness , 1);
		}

#if(ADDITIONALBENDSUPPORT)
		for(int n = 0 ; n < (int)m_TubeNodes.size()-4 ; n += 2)
		{	
			lastSeg = n;
			SolveBend(m_TubeNodes[n] , m_TubeNodes[n+2] , m_TubeNodes[n+4] ,bendStiffness);
		}

		for(int k = (lastSeg == m_TubeNodes.size()-5 ? m_TubeNodes.size()-6 : m_TubeNodes.size()-5) ; k >= 0 ; k -= 2)
		{	
			SolveBend(m_TubeNodes[k] , m_TubeNodes[k+2] , m_TubeNodes[k+4] ,bendStiffness);
		}
#endif	

		//solve bend angle
		for(int n = 0 ; n < (int)m_TubeNodes.size()-2 ; n += 2)
		{	
			lastSeg = n;
			SolveBend(m_TubeNodes[n] , m_TubeNodes[n+1] , m_TubeNodes[n+2] ,bendStiffness);
		}

		for(int k = (lastSeg == m_TubeNodes.size()-3 ? m_TubeNodes.size()-4 : m_TubeNodes.size()-3) ; k >= 0 ; k -= 2)
		{	
			SolveBend(m_TubeNodes[k] , m_TubeNodes[k+1] , m_TubeNodes[k+2] ,bendStiffness);
		}

		SolveAdditionConstraint();//addition constraint

		//solve soft-Tube collision
		SolveSoftTubeCollisions();

	}

}