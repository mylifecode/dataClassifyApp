#ifndef _CUSTOMCONSTRAINT_
#define _CUSTOMCONSTRAINT_
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"

using namespace GoPhys;

bool GetPointBarycentricCoordinate(const GFPhysVector3& v0,
								   const GFPhysVector3& v1,
								   const GFPhysVector3& v2,
								   const GFPhysVector3& v3,
								   const GFPhysVector3& p0,
								   float   weights[4]);

class VeinFaceFaceWeakConstraint //: public GFPhysSoftBodyConstraint
{
public:
	VeinFaceFaceWeakConstraint();

	void Construct( GFPhysSoftBodyFace * FaceA0 , 
					GFPhysSoftBodyFace * FaceA1 ,
					GFPhysSoftBodyFace * FaceB0 ,
					GFPhysSoftBodyFace * FaceB1 ,
					float weightA0[3] ,
					float weightA1[3] ,
					float weightB0[3] ,
					float weightB1[3] ,
					float MassA , 
					float MassB);
	void OnAttachFaceChanged(GFPhysSoftBodyFace * FaceA0,
		GFPhysSoftBodyFace * FaceA1,
		GFPhysSoftBodyFace * FaceB0,
		GFPhysSoftBodyFace * FaceB1,
		float weightA0[3],
		float weightA1[3],
		float weightB0[3],
		float weightB1[3]);

	void PrepareSolveConstraints();

	
	void SolveConstraint(Real Stiffniss,Real TimeStep);

	void SetStiffness(Real StiffNess)
	{
		m_Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(StiffNess);	
	}
	Real GetStiffness()
	{
		return m_Stiffness;
	}
	//store order A0 A1 B0 B1
	GFPhysSoftBodyFace * m_Face[4];

	GFPhysSoftBodyNode * m_FaceNodes[4][3];

	float m_weights[4][3];

	float m_RestLength[6];

	float m_CachedSumGrad[6];//for solve link

	float m_RestVolume;//order a0 a1 b0 b1

	float m_InvMass[4];

	//Real  m_SumGrad;//cached

	bool  m_InHookState;

	GFPhysVector3 m_StickPoint;

	float m_Stiffness;
};

//
//@ constraint between two tetrahedron tube 
//
class VeinTubeTubeConnect : public GoPhys::GFPhysSoftBodyConstraint
{
public:
	VeinTubeTubeConnect(GFPhysSoftBodyNode * NodePairInTubeA[6] , 
						GFPhysSoftBodyNode * NodePairInTubeB[6],
						float weightA , float weightB , 
						const GFPhysVector3 & offsetA , 
						const GFPhysVector3 & offsetB);

	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	void SolveConstraint(Real globalstiffness,Real TimeStep);

	GFPhysSoftBodyNode * m_NodePairInTubeA[6];

	GFPhysSoftBodyNode * m_NodePairInTubeB[6];

	float m_weightA;

	float m_weightB;

	float m_RestLen;

	bool  m_inContactStick;

	GFPhysVector3 m_StickPoint;

	GFPhysVector3 m_offsetA;
	GFPhysVector3 m_offsetB;

};

//@ constraint between organ surface and tube object
//
class VeinTubeFaceConnect : public GoPhys::GFPhysSoftBodyConstraint
{
public:
	VeinTubeFaceConnect(GFPhysSoftBodyNode * NodePairInTube[6] , GFPhysSoftBodyFace * Face,
						float Tubeweight , float faceWeight[3] , 
						const GFPhysVector3 & offsetTube , float offsetFace);

	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	void SolveConstraint(Real globalstiffness,Real TimeStep);

	GFPhysSoftBodyNode * m_NodePairInTubeA[6];

	GFPhysSoftBodyFace * m_Face;

	float m_weightTube;

	float m_weightFace[3];

	float m_RestLen;

	float m_TubeConnectMass;

	float m_FaceConnectMass;

	GFPhysVector3 m_offsetTube;

	float m_offsetFace;

	bool  m_inContactStick;

	GFPhysVector3 m_StickPoint;

};

//
class TubeSectionBindConnect : public GoPhys::GFPhysSoftBodyConstraint
{
public:
	TubeSectionBindConnect(GFPhysSoftBodyNode * NodePairInSectionA[3], 
					       GFPhysSoftBodyNode * NodePairInSectionB[6]
						  );

	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	void SolveConstraint(Real globalstiffness,Real TimeStep);

	GFPhysSoftBodyNode * m_NodePairInSectionA[3];

	GFPhysSoftBodyNode * m_NodePairInSectionB[3];
};

//
class ClampPointConstraint : public GFPhysSoftBodyConstraint
{
public:
	ClampPointConstraint(GFPhysSoftBodyNode * , GFPhysRigidBody * , const GFPhysVector3 & contactNormal);
	
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
	
	void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysVector3 GetConstraintImpluse();

	float m_NormalStiffScale;
	float m_TangentStiffScale;
protected:
	
	GFPhysSoftBodyNode * m_Node;			// Node pointer
	
	GFPhysRigidBody    * m_Rigid;			// Body

	GFPhysVector3		 m_LocalPoint;		// Anchor position in body space
	
	GFPhysVector3		 m_LocalNormal;

	GFPhysVector3		 m_WorldPoint;

	GFPhysVector3		 m_WorldNormal;

	GFPhysVector3		 m_NormalImpluseAccm;
	
	GFPhysVector3		 m_TangentImpluseAccm;
	
	float				 m_u;
};



class NodeOnPlaneCS : public GFPhysSoftBodyConstraint
{
public:
	NodeOnPlaneCS(GFPhysSoftBodyNode * node);

	void UpdatePlane(const GFPhysVector3 & planeNormal , const GFPhysVector3 & planePoint);

	void SolveConstraint(Real Stiffniss,Real TimeStep);
  
	GFPhysVector3 m_PlaneNormal;
	GFPhysVector3 m_planePoint;
	GFPhysSoftBodyNode * m_Node;
};


//@TetrahedronAttachConstraint
//attach a single node in to a tetrahedron
class TetrahedronAttachConstraint : public GFPhysSoftBodyConstraint
{
public:
	TetrahedronAttachConstraint(GFPhysSoftBodyNode * nodeattach , 
								GFPhysSoftBodyTetrahedron * Tetra , 
								float weights[4]);

	TetrahedronAttachConstraint(GFPhysSoftBodyNode * nodeattach,
		                        GFPhysSoftBodyFace * face,
		                        float weights[3]);


	~TetrahedronAttachConstraint();

	void ReBuild( GFPhysSoftBodyNode * nodeattach , 
				  GFPhysSoftBodyTetrahedron * Tetra , 
				  float weights[4]);

	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysSoftBodyTetrahedron * m_Tetra;
	GFPhysSoftBodyFace * m_Face;
	//GFPhysSoftBodyNode * m_Teranodes[4];

	GFPhysSoftBodyNode * m_AttachNode;

	float m_Weights[4];

	float m_InvMassTetras[4];//m_InvMassTetra;
	float m_InvMassNode;//m_InvMassNode;
	
	float m_InvMassPerTetra;

	bool  m_IsValid;

};


//attach a point on face to a point on Edge
class FaceEdgePointAttachConstraint : public GFPhysSoftBodyConstraint
{
public:
	FaceEdgePointAttachConstraint( GFPhysSoftBodyFace * AttachFace,
								   GFPhysSoftBodyEdge * AttachEdge,
	//GFPhysSoftBodyNode * Facenodes[3] , 
									//GFPhysSoftBodyNode * EdgeNodes[2] , 
									
									float FaceWeights[3] ,
									float EdgeWeights[2],
									float MassFace,
									float MassEdge);

	~FaceEdgePointAttachConstraint();

	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysSoftBodyNode * m_FaceNodes[4];

	GFPhysSoftBodyNode * m_EdgeNodes[4];

	float m_FaceWeights[3];
	float m_EdgeWeights[3];

	float m_InvMassFace[3];
	float m_InvMassEdge[2];

	float m_InvMassPerFace;
	float m_InvMassPerEdge;

	GFPhysSoftBodyFace * m_AttachFace;
	GFPhysSoftBodyEdge * m_AttachEdge;

	bool m_IsValid;
};



class TestPointPointDistanceConstraint : public GFPhysSoftBodyConstraint
{
public:
	TestPointPointDistanceConstraint(GFPhysSoftBodyNode * p1 , GFPhysSoftBodyNode* p2, float w1, float w2, float d);

	void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysSoftBodyNode *m_p1;
	GFPhysSoftBodyNode *m_p2;
	float m_w1;
	float m_w2;
	float m_d;
};

class Face_FaceConnection : public GFPhysSoftBodyConstraint
{
public:
	Face_FaceConnection(GFPhysSoftBodyFace * face0 , 
						GFPhysSoftBodyFace * face1 , 
						float EdgeStiffness = 0.4f , 
						float VolumeStiffness = 0.4f);

	void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysSoftBodyFace * m_Face[2];
	GFPhysSoftBodyNode * m_EdgeNode[24];//12 edge 2 node per edge

	GFPhysSoftBodyNode * m_VolumeNode[12];//3 tetra 4 node per tetra
	float m_EdgeRest[12];

	float m_VolumRest[3];

	float m_EdgeStiffness;
	float m_VolumStiffness;
};

//
//@ constraint between one point lie in the organ surface and the face of the other
//@ the constraint is usually used between 2 organ
class VeinFaceFaceConnect //: public GoPhys::GFPhysSoftBodyConstraint
{
public:
	VeinFaceFaceConnect();

	void Construct( GFPhysSoftBodyFace * FaceA , 
					GFPhysSoftBodyFace * FaceB , 
					float faceAweight[3] ,
					float faceBweight[3] ,
					float FaceAInvMass,
					float FaceBInvMass);

	void OnAttachFaceChanged(GFPhysSoftBodyFace * FaceA0,
		GFPhysSoftBodyFace * FaceA1,
		GFPhysSoftBodyFace * FaceB0,
		GFPhysSoftBodyFace * FaceB1,
		float weightA0[3],
		float weightA1[3],
		float weightB0[3],
		float weightB1[3]);

	//void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
	void PrepareSolveConstraints();

	void SolveConstraint(Real globalstiffness,Real TimeStep,bool solvevol = true);

	void SetStiffness(Real StiffNess)
	{
		m_Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(StiffNess);	
	}
	Real GetStiffness()
	{
		return m_Stiffness;
	}

	GFPhysSoftBodyFace * m_Face[2];
	GFPhysSoftBodyNode * m_EdgeNode[24];//12 edge 2 node per edge

	GFPhysSoftBodyNode * m_VolumeNode[12];//3 tetra 4 node per tetra
	float m_EdgeRest[12];

	float m_VolumRest[3];

	float m_Stiffness;

	float weightsinFaceA[3];
	float weightsinFaceB[3];

	float m_RestLen;//
	bool  m_inContactStick;
	GFPhysVector3 m_StickPoint;

	float m_faceAInvM;
	float m_faceBInvM;

	float m_Damping;

	bool  m_Valid;
	/*
	float m_Stiffness;

	float weightsinFace[3];

	GFPhysSoftBodyNode * m_Nodes[3];

	GFPhysSoftBodyFace * m_Face;

	float m_RestVolume;

	float m_RestLength[3];

	

	

	

	float m_FaceInvMass;

	float m_NodeInvMass;

	
	*/
};

//copy from the GFPhysSoftBodyFaceNodeDistConstraint of GoPhysSoftBodyDistConstraint.h and GoPhysSoftBodyDistConstraint.cpp
class SoftBodyFaceNodeDistConstraint : public GFPhysSoftBodyConstraint
{
public:
	SoftBodyFaceNodeDistConstraint();

	void Initalize(GFPhysSoftBodyFace * face , GFPhysSoftBodyNode * node , Real weight[3]);

	void SetRestLength(Real restlen);

	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	void SolveConstraint(Real Stiffniss,Real TimeStep);
protected:
	GFPhysSoftBodyNode * m_Node;

	GFPhysSoftBodyFace * m_Face;

	GFPhysSoftBodyNode * m_FaceNodes[3];//cached

	Real m_FaceNodesMass[3];//cached

	Real m_NodeMass;//cached

	Real m_weight[3];

	Real m_RestLength;
};

#endif