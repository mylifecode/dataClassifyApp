#include "CustomConstraint.h"
#include "Math/GoPhysSIMDMath.h"
/**
 * Calculate the determinant for a 4x4 matrix based on this example:
 * http://www.euclideanspace.com/maths/algebra/matrix/functions/determinant/fourD/index.htm
 * This function takes four Vec4f as row vectors and calculates the resulting matrix' determinant
 * using the Laplace expansion.
 *
 */
/*const float Determinant( const GFPhysVector3& v0,
										  const GFPhysVector3& v1,
										  const GFPhysVector3& v2,
										  const GFPhysVector3& v3 )
{
	return ((v2-v1).Cross(v3-v1)).Dot(v0-v1);
}*/
 
/**
 * Calculate the actual barycentric coordinate from a point p0_ and the four 
 * vertices v0_ .. v3_ from a tetrahedron.
 */
bool GetPointBarycentricCoordinate( const GFPhysVector3& v0,
                                      const GFPhysVector3& v1,
                                      const GFPhysVector3& v2,
                                      const GFPhysVector3& v3,
                                      const GFPhysVector3& p0,
									  float   weights[4])
{
			const float det0 = Determinant(v0, v1, v2, v3);
			const float det1 = Determinant(p0, v1, v2, v3);
			const float det2 = Determinant(v0, p0, v2, v3);
			const float det3 = Determinant(v0, v1, p0, v3);
			const float det4 = Determinant(v0, v1, v2, p0);
			weights[0] = (det1/det0);
			weights[1] = (det2/det0);
			weights[2] = (det3/det0);
			weights[3] = (det4/det0);
			return true;
}
//=================================================================================================================
VeinTubeTubeConnect::VeinTubeTubeConnect(GFPhysSoftBodyNode * NodePairInTubeA[6] , 
													   GFPhysSoftBodyNode * NodePairInTubeB[6] ,
													   float weightA , float weightB, 
													   const GFPhysVector3 & offsetA , 
													   const GFPhysVector3 & offsetB)
{
	m_NodePairInTubeA[0] = NodePairInTubeA[0];
	m_NodePairInTubeA[1] = NodePairInTubeA[1];
	m_NodePairInTubeA[2] = NodePairInTubeA[2];
	m_NodePairInTubeA[3] = NodePairInTubeA[3];
	m_NodePairInTubeA[4] = NodePairInTubeA[4];
	m_NodePairInTubeA[5] = NodePairInTubeA[5];

	m_NodePairInTubeB[0] = NodePairInTubeB[0];
	m_NodePairInTubeB[1] = NodePairInTubeB[1];
	m_NodePairInTubeB[2] = NodePairInTubeB[2];
	m_NodePairInTubeB[3] = NodePairInTubeB[3];
	m_NodePairInTubeB[4] = NodePairInTubeB[4];
	m_NodePairInTubeB[5] = NodePairInTubeB[5];

	m_weightA = weightA;
	m_weightB = weightB;

	GFPhysVector3 centerA0 = m_NodePairInTubeA[0]->m_CurrPosition+m_NodePairInTubeA[1]->m_CurrPosition+m_NodePairInTubeA[2]->m_CurrPosition;
	centerA0 /= 3;

	GFPhysVector3 centerA1 = m_NodePairInTubeA[3]->m_CurrPosition+m_NodePairInTubeA[4]->m_CurrPosition+m_NodePairInTubeA[5]->m_CurrPosition;
	centerA1 /= 3;

	GFPhysVector3 centerB0 = m_NodePairInTubeB[0]->m_CurrPosition+m_NodePairInTubeB[1]->m_CurrPosition+m_NodePairInTubeB[2]->m_CurrPosition;
	centerB0 /= 3;

	GFPhysVector3 centerB1 = m_NodePairInTubeB[3]->m_CurrPosition+m_NodePairInTubeB[4]->m_CurrPosition+m_NodePairInTubeB[5]->m_CurrPosition;
	centerB1 /= 3;

	GFPhysVector3 pointA = centerA0*m_weightA + centerA1*(1-m_weightA);
	
	GFPhysVector3 pointB = centerB0*m_weightB + centerB1*(1-m_weightB);

	m_RestLen = (pointA-pointB).Length();

	GFPhysVector3 tetraVerts[4];

	m_offsetA = offsetA;
	m_offsetB = offsetB;

	m_inContactStick = 0;
}
//===========================================================================================================
void VeinTubeTubeConnect::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
//============================================================================================================
void VeinTubeTubeConnect::SolveConstraint(Real globalstiffness,Real TimeStep)
{
	float Stiffness = m_Stiffness*globalstiffness;

	GFPhysVector3 centerA0 = m_NodePairInTubeA[0]->m_CurrPosition+m_NodePairInTubeA[1]->m_CurrPosition+m_NodePairInTubeA[2]->m_CurrPosition;
	centerA0 /= 3;

	GFPhysVector3 centerA1 = m_NodePairInTubeA[3]->m_CurrPosition+m_NodePairInTubeA[4]->m_CurrPosition+m_NodePairInTubeA[5]->m_CurrPosition;
	centerA1 /= 3;

	GFPhysVector3 centerB0 = m_NodePairInTubeB[0]->m_CurrPosition+m_NodePairInTubeB[1]->m_CurrPosition+m_NodePairInTubeB[2]->m_CurrPosition;
	centerB0 /= 3;

	GFPhysVector3 centerB1 = m_NodePairInTubeB[3]->m_CurrPosition+m_NodePairInTubeB[4]->m_CurrPosition+m_NodePairInTubeB[5]->m_CurrPosition;
	centerB1 /= 3;

	GFPhysVector3 pointA = centerA0*m_weightA + centerA1*(1-m_weightA);
	GFPhysVector3 pointB = centerB0*m_weightB + centerB1*(1-m_weightB);
	pointA = pointA+m_offsetA;
	pointB = pointB+m_offsetB;

	GFPhysVector3 Diff = pointA - pointB;
	
	float Length = Diff.Length();
	
	if(m_inContactStick == true)
	{
		float tubePartA = (pointA-m_StickPoint).Length();

		float tubePartB = (pointB-m_StickPoint).Length();

		Length = tubePartA + tubePartB;

		if(Length > m_RestLen && Length > GP_EPSILON)
		{
			//
			/*float clampLen = m_RestLen+0.2f;
			if(Length > clampLen)
			{
				Length = (Length-clampLen)*0.05f+clampLen;
			}*/
			//

			float PartADiff = 0.5f*(Length-m_RestLen)*tubePartA / Length;

			float PartBDiff = 0.5f*(Length-m_RestLen)*tubePartB / Length;

			GFPhysVector3 DetaA = -Stiffness * PartADiff * (pointA-m_StickPoint) / tubePartA;//-0.5f * PartADiff * (pointA-m_StickPoint);

			GFPhysVector3 DetaB = -Stiffness * PartBDiff * (pointB-m_StickPoint) / tubePartB;
			
			if(m_NodePairInTubeA[0]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[0]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[1]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[1]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[2]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[2]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[3]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[3]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[4]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[4]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[5]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[5]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeB[0]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeB[0]->m_CurrPosition += DetaB;

			if(m_NodePairInTubeB[1]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeB[1]->m_CurrPosition += DetaB;

			if(m_NodePairInTubeB[2]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeB[2]->m_CurrPosition += DetaB;

			if(m_NodePairInTubeB[3]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeB[3]->m_CurrPosition += DetaB;

			if(m_NodePairInTubeB[4]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeB[4]->m_CurrPosition += DetaB;

			if(m_NodePairInTubeB[5]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeB[5]->m_CurrPosition += DetaB;
		}
	}
	else
	{
		if(Length > FLT_EPSILON)
		{
			Stiffness = Stiffness;//*0.5f;

			Real Temp = (Length-m_RestLen)/Length;
			GFPhysVector3 DetaA = -0.5f * Temp * Diff;
			GFPhysVector3 DetaB =  0.5f * Temp * Diff;

			DetaA = DetaA* Stiffness;

			DetaB = DetaB* Stiffness;

			m_NodePairInTubeA[0]->m_CurrPosition += DetaA;
			m_NodePairInTubeA[1]->m_CurrPosition += DetaA;
			m_NodePairInTubeA[2]->m_CurrPosition += DetaA;
			m_NodePairInTubeA[3]->m_CurrPosition += DetaA;
			m_NodePairInTubeA[4]->m_CurrPosition += DetaA;
			m_NodePairInTubeA[5]->m_CurrPosition += DetaA;

			m_NodePairInTubeB[0]->m_CurrPosition += DetaB;
			m_NodePairInTubeB[1]->m_CurrPosition += DetaB;
			m_NodePairInTubeB[2]->m_CurrPosition += DetaB;
			m_NodePairInTubeB[3]->m_CurrPosition += DetaB;
			m_NodePairInTubeB[4]->m_CurrPosition += DetaB;
			m_NodePairInTubeB[5]->m_CurrPosition += DetaB;
		}
		else
		{
			//To do 
		}
	}
	
}

//============================================================================================================
VeinTubeFaceConnect::VeinTubeFaceConnect(GFPhysSoftBodyNode * NodePairInTube[6] , GFPhysSoftBodyFace * Face,
										 float Tubeweight , float faceWeight[3] , 
										 const GFPhysVector3 & offsetTube , float offsetFace)
{
	m_NodePairInTubeA[0] = NodePairInTube[0];
	m_NodePairInTubeA[1] = NodePairInTube[1];
	m_NodePairInTubeA[2] = NodePairInTube[2];
	m_NodePairInTubeA[3] = NodePairInTube[3];
	m_NodePairInTubeA[4] = NodePairInTube[4];
	m_NodePairInTubeA[5] = NodePairInTube[5];

	m_Face = Face;

	m_weightTube = Tubeweight;
	m_weightFace[0] = faceWeight[0];
	m_weightFace[1] = faceWeight[1];
	m_weightFace[2] = faceWeight[2];

	m_offsetTube = offsetTube;
	m_offsetFace = offsetFace;

	GFPhysVector3 centerA0 = m_NodePairInTubeA[0]->m_CurrPosition+m_NodePairInTubeA[1]->m_CurrPosition+m_NodePairInTubeA[2]->m_CurrPosition;
	centerA0 /= 3;

	GFPhysVector3 centerA1 = m_NodePairInTubeA[3]->m_CurrPosition+m_NodePairInTubeA[4]->m_CurrPosition+m_NodePairInTubeA[5]->m_CurrPosition;
	centerA1 /= 3;

	GFPhysVector3 pointTube = centerA0*m_weightTube + centerA1*(1-m_weightTube);
	//calculate offset vector
	pointTube = pointTube+m_offsetTube;

	GFPhysVector3 PointFace = m_weightFace[0]*m_Face->m_Nodes[0]->m_CurrPosition+
						      m_weightFace[1]*m_Face->m_Nodes[1]->m_CurrPosition+
							  m_weightFace[2]*m_Face->m_Nodes[2]->m_CurrPosition;

	//PointFace = PointFace+m_offsetFace;

	m_RestLen = (pointTube-PointFace).Length();

	m_inContactStick = false;

	m_TubeConnectMass = 4.0f;

	m_FaceConnectMass = 1.0f;
}

void VeinTubeFaceConnect::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
void VeinTubeFaceConnect::SolveConstraint(Real globalstiffness,Real TimeStep)
{
	float Stiffness = m_Stiffness*globalstiffness;

	GFPhysVector3 centerTube0 = m_NodePairInTubeA[0]->m_CurrPosition+m_NodePairInTubeA[1]->m_CurrPosition+m_NodePairInTubeA[2]->m_CurrPosition;
	centerTube0 /= 3;

	GFPhysVector3 centerTube1 = m_NodePairInTubeA[3]->m_CurrPosition+m_NodePairInTubeA[4]->m_CurrPosition+m_NodePairInTubeA[5]->m_CurrPosition;
	centerTube1 /= 3;

	GFPhysVector3 pointTube = centerTube0*m_weightTube + centerTube1*(1-m_weightTube);
	
	//calculate offset vector
	pointTube = pointTube+m_offsetTube;

	
	GFPhysVector3 PointFace = m_weightFace[0]*m_Face->m_Nodes[0]->m_CurrPosition+
							  m_weightFace[1]*m_Face->m_Nodes[1]->m_CurrPosition+
							  m_weightFace[2]*m_Face->m_Nodes[2]->m_CurrPosition;

	GFPhysVector3 Diff = pointTube - PointFace;
	
	float Length = Diff.Length();

	float InvFaceMass = (m_FaceConnectMass > FLT_EPSILON ? 1.0f / m_FaceConnectMass : 0);

	float InvTubeMass = (m_TubeConnectMass > FLT_EPSILON ? 1.0f / m_TubeConnectMass : 0);

	float InvSum = InvTubeMass+InvFaceMass;

	float weightFace = (InvSum > FLT_EPSILON ? InvFaceMass / InvSum : 0);

	float weightTube = (InvSum > FLT_EPSILON ? InvTubeMass / InvSum : 0);
	
	if(m_inContactStick == true)
	{
		float tubePartLen = (pointTube-m_StickPoint).Length();

		float facePartLen = (PointFace-m_StickPoint).Length();

		Length = tubePartLen + facePartLen;

		if(Length > m_RestLen)
		{
			//
			//float clampLen = m_RestLen+0.2f;
			//if(Length > clampLen)
			//{
			//	Length = (Length-clampLen)*0.1f+clampLen;
			//}
			//

			float Facesiffness  = Stiffness;

			float TubeStiffness = Stiffness;//*0.05f;

			float FaceDiff = 0.5f*(Length-m_RestLen)*facePartLen / Length;
		
			float TubeDiff = 0.5f*(Length-m_RestLen)*tubePartLen / Length;
			
			Real Invsumgrad = 1.0f / (m_weightFace[0]*m_weightFace[0] + m_weightFace[1]*m_weightFace[1] + m_weightFace[2]*m_weightFace[2]);

			GFPhysVector3 s = -Facesiffness * Invsumgrad * FaceDiff * (PointFace-m_StickPoint) / facePartLen;
	
			m_Face->m_Nodes[0]->m_CurrPosition += (s * m_weightFace[0]);
			m_Face->m_Nodes[1]->m_CurrPosition += (s * m_weightFace[1]);
			m_Face->m_Nodes[2]->m_CurrPosition += (s * m_weightFace[2]);


			GFPhysVector3 DetaA = -TubeStiffness * TubeDiff * (pointTube-m_StickPoint) / tubePartLen;

			if(m_NodePairInTubeA[0]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[0]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[1]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[1]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[2]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[2]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[3]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[3]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[4]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[4]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[5]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[5]->m_CurrPosition += DetaA;
		}
	}
	else
	{	
		if(Length > FLT_EPSILON)
		{
			Real Temp = (Length-m_RestLen)/Length;
			
			GFPhysVector3 DetaA = -/*0.2f*/weightTube * Temp * Diff;
			
			GFPhysVector3 DetaB =  /*0.8f*/weightFace * Temp * Diff;

			DetaA = DetaA* Stiffness;
			DetaB = DetaB* Stiffness;

			if(m_NodePairInTubeA[0]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[0]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[1]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[1]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[2]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[2]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[3]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[3]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[4]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[4]->m_CurrPosition += DetaA;

			if(m_NodePairInTubeA[5]->m_InvM > GP_EPSILON)
			   m_NodePairInTubeA[5]->m_CurrPosition += DetaA;

			m_Face->m_Nodes[0]->m_CurrPosition += DetaB;
			m_Face->m_Nodes[1]->m_CurrPosition += DetaB;
			m_Face->m_Nodes[2]->m_CurrPosition += DetaB;
		}
		else
		{
			//To do 
		}
	}

}

//=======================================================================================================

//=================================================================================================================
TubeSectionBindConnect::TubeSectionBindConnect(GFPhysSoftBodyNode * NodePairInSectionA[3] , 
											   GFPhysSoftBodyNode * NodePairInSectionB[3])
{
	m_NodePairInSectionA[0] = NodePairInSectionA[0];
	m_NodePairInSectionA[1] = NodePairInSectionA[1];
	m_NodePairInSectionA[2] = NodePairInSectionA[2];
	
	m_NodePairInSectionB[0] = NodePairInSectionB[0];
	m_NodePairInSectionB[1] = NodePairInSectionB[1];
	m_NodePairInSectionB[2] = NodePairInSectionB[2];
}
//===========================================================================================================
void TubeSectionBindConnect::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
//============================================================================================================
void TubeSectionBindConnect::SolveConstraint(Real globalstiffness,Real TimeStep)
{
	float Stiffness = m_Stiffness*globalstiffness;

	GFPhysVector3 centerA = m_NodePairInSectionA[0]->m_CurrPosition+m_NodePairInSectionA[1]->m_CurrPosition+m_NodePairInSectionA[2]->m_CurrPosition;
	centerA /= 3;

	GFPhysVector3 centerB = m_NodePairInSectionB[0]->m_CurrPosition+m_NodePairInSectionB[1]->m_CurrPosition+m_NodePairInSectionB[2]->m_CurrPosition;
	centerB /= 3;

	GFPhysVector3 Diff = centerA - centerB;

	float Length = Diff.Length();

	float invMassA = m_NodePairInSectionA[0]->m_InvM
					+m_NodePairInSectionA[1]->m_InvM
					+m_NodePairInSectionA[2]->m_InvM;

	float invMassB = m_NodePairInSectionB[0]->m_InvM
					+m_NodePairInSectionB[1]->m_InvM
					+m_NodePairInSectionB[2]->m_InvM;

	float invMass = invMassA+invMassB;
	
	if(invMass < FLT_EPSILON)
	   return;
	
	float weightA = invMassA / invMass;
	
	float weightB = invMassB / invMass;

	if(Length > FLT_EPSILON)
	{
		GFPhysVector3 DetaA = -weightA * Diff;
		
		GFPhysVector3 DetaB =  weightB * Diff;

		DetaA = DetaA*Stiffness;

		DetaB = DetaB*Stiffness;

		m_NodePairInSectionA[0]->m_CurrPosition += DetaA;
		m_NodePairInSectionA[1]->m_CurrPosition += DetaA;
		m_NodePairInSectionA[2]->m_CurrPosition += DetaA;
		
		m_NodePairInSectionB[0]->m_CurrPosition += DetaB;
		m_NodePairInSectionB[1]->m_CurrPosition += DetaB;
		m_NodePairInSectionB[2]->m_CurrPosition += DetaB;
	}
}






//=============================================================================================
ClampPointConstraint::ClampPointConstraint(GFPhysSoftBodyNode * Node,GFPhysRigidBody * RigidBody , const GFPhysVector3 & contactNormal)
{
	m_Node		  = Node;
	m_Rigid		  = RigidBody;
	m_LocalPoint  = RigidBody->GetWorldTransform().Inverse()*m_Node->m_CurrPosition;
	m_LocalNormal = contactNormal;
	m_NormalStiffScale = 1;
	m_TangentStiffScale = 1;
}
//=============================================================================================
void ClampPointConstraint::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	const GFPhysTransform & Trans = m_Rigid->GetWorldTransform();
	
	m_WorldPoint  = Trans * m_LocalPoint; 
	m_WorldNormal = Trans.GetBasis()*m_LocalNormal;

	m_NormalImpluseAccm  = GFPhysVector3(0,0,0);
	m_TangentImpluseAccm = GFPhysVector3(0,0,0);
}
//=============================================================================================
void ClampPointConstraint::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	/*float normalstiffness  = m_NormalStiffScale;

	float tangentstiffness = m_TangentStiffScale;

	GFPhysVector3 Delta = m_Node->m_CurrPosition-m_WorldPoint;
	
	GFPhysVector3 normalDelta  = m_WorldNormal * Delta.Dot(m_WorldNormal);

	GFPhysVector3 tangentDelta = Delta-normalDelta;

	GFPhysVector3 NormalImpluse  = -normalstiffness  * normalDelta;

	GFPhysVector3 TangentImpluse = -tangentstiffness * tangentDelta;

	m_NormalImpluseAccm += NormalImpluse;

	m_TangentImpluseAccm += TangentImpluse;

	m_Node->m_CurrPosition += (NormalImpluse+TangentImpluse);*/
	
	GFPhysVector3 TangentImpluse = m_Stiffness * (m_WorldPoint-m_Node->m_CurrPosition);
	m_Node->m_CurrPosition += TangentImpluse;

	m_TangentImpluseAccm += TangentImpluse;
}

GFPhysVector3 ClampPointConstraint::GetConstraintImpluse()
{
	return m_NormalImpluseAccm*0.3f+m_TangentImpluseAccm;
}




TetrahedronAttachConstraint::TetrahedronAttachConstraint(GFPhysSoftBodyNode * nodeattach , 
														 GFPhysSoftBodyTetrahedron * Tetra , //GFPhysSoftBodyNode*teranodes[4],
														 float weights[4])//,
														 //float MassTetra,
														// float MassNode)
{
	m_Tetra = Tetra;
	m_Face = 0;
	//m_Teranodes[0] = m_Tetra->m_TetraNodes[0];
	//m_Teranodes[1] = m_Tetra->m_TetraNodes[1];
	//m_Teranodes[2] = m_Tetra->m_TetraNodes[2];
	//m_Teranodes[3] = m_Tetra->m_TetraNodes[3];

	m_AttachNode = nodeattach;

	m_Weights[0] = weights[0];
	m_Weights[1] = weights[1];
	m_Weights[2] = weights[2];
	m_Weights[3] = weights[3];

	m_InvMassTetras[0] = m_Tetra->m_TetraNodes[0]->m_InvM;
	m_InvMassTetras[1] = m_Tetra->m_TetraNodes[1]->m_InvM;
	m_InvMassTetras[2] = m_Tetra->m_TetraNodes[2]->m_InvM;
	m_InvMassTetras[3] = m_Tetra->m_TetraNodes[3]->m_InvM;
	m_InvMassNode = m_AttachNode->m_InvM;

	float massTetra = (m_Tetra->m_TetraNodes[0]->m_Mass + m_Tetra->m_TetraNodes[1]->m_Mass + m_Tetra->m_TetraNodes[2]->m_Mass + m_Tetra->m_TetraNodes[3]->m_Mass);

	if(massTetra > FLT_EPSILON)
	   m_InvMassPerTetra = 1.0f / massTetra;
	else
	   m_InvMassPerTetra = 0.0f;

	m_IsValid = true;
}

TetrahedronAttachConstraint::TetrahedronAttachConstraint(GFPhysSoftBodyNode * nodeattach,
	                                                     GFPhysSoftBodyFace * face,
	                                                     float weights[3])
{
	m_Tetra = 0;
	m_Face = face;

	m_AttachNode = nodeattach;

	m_Weights[0] = weights[0];
	m_Weights[1] = weights[1];
	m_Weights[2] = weights[2];


	m_InvMassTetras[0] = m_Face->m_Nodes[0]->m_InvM;
	m_InvMassTetras[1] = m_Face->m_Nodes[1]->m_InvM;
	m_InvMassTetras[2] = m_Face->m_Nodes[2]->m_InvM;
	m_InvMassNode = m_AttachNode->m_InvM;

	m_IsValid = true;
}

TetrahedronAttachConstraint::~TetrahedronAttachConstraint()
{
	
}

void TetrahedronAttachConstraint::ReBuild( GFPhysSoftBodyNode * nodeattach , 
					                                 GFPhysSoftBodyTetrahedron * Tetra , 
					                                 float weights[4])
{
	if (m_Face != 0)
		return;

	m_Tetra = Tetra;

	//m_Teranodes[0] = m_Tetra->m_TetraNodes[0];
	//m_Teranodes[1] = m_Tetra->m_TetraNodes[1];
	//m_Teranodes[2] = m_Tetra->m_TetraNodes[2];
	//m_Teranodes[3] = m_Tetra->m_TetraNodes[3];

	m_AttachNode = nodeattach;

	m_Weights[0] = weights[0];
	m_Weights[1] = weights[1];
	m_Weights[2] = weights[2];
	m_Weights[3] = weights[3];

	m_InvMassTetras[0] = m_Tetra->m_TetraNodes[0]->m_InvM;
	m_InvMassTetras[1] = m_Tetra->m_TetraNodes[1]->m_InvM;
	m_InvMassTetras[2] = m_Tetra->m_TetraNodes[2]->m_InvM;
	m_InvMassTetras[3] = m_Tetra->m_TetraNodes[3]->m_InvM;
	m_InvMassNode = m_AttachNode->m_InvM;

	float massTetra = (m_Tetra->m_TetraNodes[0]->m_Mass + m_Tetra->m_TetraNodes[1]->m_Mass + m_Tetra->m_TetraNodes[2]->m_Mass + m_Tetra->m_TetraNodes[3]->m_Mass);

	if(massTetra > FLT_EPSILON)
		m_InvMassPerTetra = 1.0f / massTetra;
	else
		m_InvMassPerTetra = 0.0f;
	m_IsValid = true;
}

void TetrahedronAttachConstraint::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
void TetrahedronAttachConstraint::SolveConstraint(Real Stiffniss , Real TimeStep)
{
	if (m_Tetra)
	{
		GFPhysVector3 pTetra(0, 0, 0);

		GFPhysVector3 pAttach(0, 0, 0);

		pTetra = m_Tetra->m_TetraNodes[0]->m_CurrPosition*m_Weights[0]
			+ m_Tetra->m_TetraNodes[1]->m_CurrPosition*m_Weights[1]
			+ m_Tetra->m_TetraNodes[2]->m_CurrPosition*m_Weights[2]
			+ m_Tetra->m_TetraNodes[3]->m_CurrPosition*m_Weights[3];

		pAttach = m_AttachNode->m_CurrPosition;

		GFPhysVector3 Diff = (pTetra - pAttach);

		float diffLen = Diff.Length();

		if (diffLen > FLT_EPSILON)
		{
			Diff /= diffLen;

			float sumgrad = m_Weights[0] * m_Weights[0] * m_InvMassTetras[0]
				+ m_Weights[1] * m_Weights[1] * m_InvMassTetras[1]
				+ m_Weights[2] * m_Weights[2] * m_InvMassTetras[2]
				+ m_Weights[3] * m_Weights[3] * m_InvMassTetras[3]
				+ m_InvMassNode;

			Stiffniss = Stiffniss*m_Stiffness;

			float scale = Stiffniss*(-diffLen) / sumgrad;

			GFPhysVector3 grad00 = Diff*m_Weights[0];
			GFPhysVector3 grad01 = Diff*m_Weights[1];
			GFPhysVector3 grad02 = Diff*m_Weights[2];
			GFPhysVector3 grad03 = Diff*m_Weights[3];

			GFPhysVector3 gradNoda = -Diff;

			GFPhysVector3 delta00 = scale*grad00*m_InvMassTetras[0];
			GFPhysVector3 delta01 = scale*grad01*m_InvMassTetras[1];
			GFPhysVector3 delta02 = scale*grad02*m_InvMassTetras[2];
			GFPhysVector3 delta03 = scale*grad03*m_InvMassTetras[3];

			GFPhysVector3 deltaNode = scale*gradNoda*m_InvMassNode;

			//--new method
			float InvMassTotal = m_InvMassPerTetra + m_InvMassNode;

			if (InvMassTotal > FLT_EPSILON)
			{
				float wt = m_InvMassPerTetra / InvMassTotal;
				float wn = m_InvMassNode / InvMassTotal;

				GFPhysVector3 deltat = Diff * (-diffLen)  * m_Stiffness * wt;
				GFPhysVector3 deltan = -Diff * (-diffLen) * m_Stiffness * wn;

				m_Tetra->m_TetraNodes[0]->m_CurrPosition += deltat;
				m_Tetra->m_TetraNodes[1]->m_CurrPosition += deltat;
				m_Tetra->m_TetraNodes[2]->m_CurrPosition += deltat;
				m_Tetra->m_TetraNodes[3]->m_CurrPosition += deltat;

				m_AttachNode->m_CurrPosition += deltan;
			}
		}
	}
	else//face
	{
		GFPhysVector3 pFace(0, 0, 0);

		GFPhysVector3 pAttach(0, 0, 0);

		pFace  = m_Face->m_Nodes[0]->m_CurrPosition*m_Weights[0]
			   + m_Face->m_Nodes[1]->m_CurrPosition*m_Weights[1]
			   + m_Face->m_Nodes[2]->m_CurrPosition*m_Weights[2];

		pAttach = m_AttachNode->m_CurrPosition;

		GFPhysVector3 Diff = (pFace - pAttach);

		float diffLen = Diff.Length();

		if (diffLen > FLT_EPSILON)
		{
			Diff /= diffLen;

			float sumgrad = m_Weights[0] * m_Weights[0] * m_Face->m_Nodes[0]->m_InvM
				          + m_Weights[1] * m_Weights[1] * m_Face->m_Nodes[1]->m_InvM
				          + m_Weights[2] * m_Weights[2] * m_Face->m_Nodes[2]->m_InvM
						  + m_AttachNode->m_InvM;

			Stiffniss = Stiffniss*m_Stiffness;

			float scale = Stiffniss*(-diffLen) / sumgrad;

			GFPhysVector3 grad00 = Diff*m_Weights[0];
			GFPhysVector3 grad01 = Diff*m_Weights[1];
			GFPhysVector3 grad02 = Diff*m_Weights[2];


			GFPhysVector3 gradNoda = -Diff;

			GFPhysVector3 delta00 = scale * grad00 * m_Face->m_Nodes[0]->m_InvM;
			GFPhysVector3 delta01 = scale * grad01 * m_Face->m_Nodes[1]->m_InvM;
			GFPhysVector3 delta02 = scale * grad02 * m_Face->m_Nodes[2]->m_InvM;

			GFPhysVector3 deltaNode = scale * gradNoda * m_AttachNode->m_InvM;

			m_Face->m_Nodes[0]->m_CurrPosition += delta00;
			m_Face->m_Nodes[1]->m_CurrPosition += delta01;
			m_Face->m_Nodes[2]->m_CurrPosition += delta02;
		
			m_AttachNode->m_CurrPosition += deltaNode;
		}
	}

}


//----------------------------------------------------------------------------------
VeinFaceFaceWeakConstraint::VeinFaceFaceWeakConstraint()
{

}
//------------------------------------------------------------------------------------------------
void VeinFaceFaceWeakConstraint::Construct(GFPhysSoftBodyFace * FaceA0 , 
										   GFPhysSoftBodyFace * FaceA1 ,
										   GFPhysSoftBodyFace * FaceB0 ,
										   GFPhysSoftBodyFace * FaceB1 ,
										   float weightA0[3] ,
										   float weightA1[3] ,
										   float weightB0[3] ,
										   float weightB1[3] ,
										   float MassA , 
										   float MassB)
{
		m_Face[0] = FaceA0;
		m_Face[1] = FaceA1;
		m_Face[2] = FaceB0;
		m_Face[3] = FaceB1;

		m_FaceNodes[0][0] = FaceA0->m_Nodes[0];
		m_FaceNodes[0][1] = FaceA0->m_Nodes[1];
		m_FaceNodes[0][2] = FaceA0->m_Nodes[2];

		m_FaceNodes[1][0] = FaceA1->m_Nodes[0];
		m_FaceNodes[1][1] = FaceA1->m_Nodes[1];
		m_FaceNodes[1][2] = FaceA1->m_Nodes[2];

		m_FaceNodes[2][0] = FaceB0->m_Nodes[0];
		m_FaceNodes[2][1] = FaceB0->m_Nodes[1];
		m_FaceNodes[2][2] = FaceB0->m_Nodes[2];

		m_FaceNodes[3][0] = FaceB1->m_Nodes[0];
		m_FaceNodes[3][1] = FaceB1->m_Nodes[1];
		m_FaceNodes[3][2] = FaceB1->m_Nodes[2];

		m_weights[0][0] = weightA0[0];
		m_weights[0][1] = weightA0[1];
		m_weights[0][2] = weightA0[2];
		m_weights[1][0] = weightA1[0];
		m_weights[1][1] = weightA1[1];
		m_weights[1][2] = weightA1[2];

		m_weights[2][0] = weightB0[0];
		m_weights[2][1] = weightB0[1];
		m_weights[2][2] = weightB0[2];
		m_weights[3][0] = weightB1[0];
		m_weights[3][1] = weightB1[1];
		m_weights[3][2] = weightB1[2];

		GFPhysVector3 A0 = m_FaceNodes[0][0]->m_CurrPosition*weightA0[0]+
			m_FaceNodes[0][1]->m_CurrPosition*weightA0[1]+
			m_FaceNodes[0][2]->m_CurrPosition*weightA0[2];

		GFPhysVector3 A1 = m_FaceNodes[1][0]->m_CurrPosition*weightA1[0]+
			m_FaceNodes[1][1]->m_CurrPosition*weightA1[1]+
			m_FaceNodes[1][2]->m_CurrPosition*weightA1[2];


		GFPhysVector3 B0 = m_FaceNodes[2][0]->m_CurrPosition*weightB0[0]+
			m_FaceNodes[2][1]->m_CurrPosition*weightB0[1]+
			m_FaceNodes[2][2]->m_CurrPosition*weightB0[2];

		GFPhysVector3 B1 = m_FaceNodes[3][0]->m_CurrPosition*weightB1[0]+
			m_FaceNodes[3][1]->m_CurrPosition*weightB1[1]+
			m_FaceNodes[3][2]->m_CurrPosition*weightB1[2];

		m_RestLength[0] = (A0-A1).Length();
		m_RestLength[1] = (B0-B1).Length();
		m_RestLength[2] = (A0-B0).Length();
		m_RestLength[3] = (A1-B1).Length();
		m_RestLength[4] = (A1-B0).Length();
		m_RestLength[5] = (A0-B1).Length();

		GFPhysVector3 c = B1-A0;
		GFPhysVector3 a = A1-A0;
		GFPhysVector3 b = B0-A0;

		const Real denom = Real(1)/Real(6);

		m_RestVolume = denom*(a.Cross(b)).Dot(c);

		m_InvMass[0] = m_InvMass[1] = (MassA < FLT_EPSILON ? 0 : 1.0f / MassA);

		m_InvMass[2] = m_InvMass[3] = (MassB < FLT_EPSILON ? 0 : 1.0f / MassB);

		//cache edge solve sumgrad
		int solveedgeindex[] = {0,1,2,3,0,2,1,3,1,2,0,3};

		for(int e = 0 ; e < 6 ; e++)
		{
			int t0 = solveedgeindex[e*2];

			int t1 = solveedgeindex[e*2+1];

			GFPhysSoftBodyNode * * Face0Nodes = m_FaceNodes[t0];

			GFPhysSoftBodyNode * * Face1Nodes = m_FaceNodes[t1];

			float * weights0 = m_weights[t0];

			float * weights1 = m_weights[t1];

			Real Face0NodesMass = m_InvMass[t0];

			Real Face1NodesMass = m_InvMass[t1];

			m_CachedSumGrad[e] = weights0[0]*weights0[0]*Face0NodesMass
				+weights0[1]*weights0[1]*Face0NodesMass
				+weights0[2]*weights0[2]*Face0NodesMass
				+weights1[0]*weights1[0]*Face1NodesMass
				+weights1[1]*weights1[1]*Face1NodesMass
				+weights1[2]*weights1[2]*Face1NodesMass;
		}
		m_InHookState = false;
		m_Stiffness = 0.28f;
}

void VeinFaceFaceWeakConstraint::OnAttachFaceChanged(GFPhysSoftBodyFace * FaceA0,
	                                                 GFPhysSoftBodyFace * FaceA1,
	                                                 GFPhysSoftBodyFace * FaceB0,
	                                                 GFPhysSoftBodyFace * FaceB1,
	                                                 float weightA0[3],
	                                                 float weightA1[3],
	                                                 float weightB0[3],
	                                                 float weightB1[3])
{
	m_Face[0] = FaceA0;
	m_Face[1] = FaceA1;
	m_Face[2] = FaceB0;
	m_Face[3] = FaceB1;

	m_FaceNodes[0][0] = FaceA0->m_Nodes[0];
	m_FaceNodes[0][1] = FaceA0->m_Nodes[1];
	m_FaceNodes[0][2] = FaceA0->m_Nodes[2];

	m_FaceNodes[1][0] = FaceA1->m_Nodes[0];
	m_FaceNodes[1][1] = FaceA1->m_Nodes[1];
	m_FaceNodes[1][2] = FaceA1->m_Nodes[2];

	m_FaceNodes[2][0] = FaceB0->m_Nodes[0];
	m_FaceNodes[2][1] = FaceB0->m_Nodes[1];
	m_FaceNodes[2][2] = FaceB0->m_Nodes[2];

	m_FaceNodes[3][0] = FaceB1->m_Nodes[0];
	m_FaceNodes[3][1] = FaceB1->m_Nodes[1];
	m_FaceNodes[3][2] = FaceB1->m_Nodes[2];

	m_weights[0][0] = weightA0[0];
	m_weights[0][1] = weightA0[1];
	m_weights[0][2] = weightA0[2];
	m_weights[1][0] = weightA1[0];
	m_weights[1][1] = weightA1[1];
	m_weights[1][2] = weightA1[2];

	m_weights[2][0] = weightB0[0];
	m_weights[2][1] = weightB0[1];
	m_weights[2][2] = weightB0[2];
	m_weights[3][0] = weightB1[0];
	m_weights[3][1] = weightB1[1];
	m_weights[3][2] = weightB1[2];

	GFPhysVector3 A0 = m_FaceNodes[0][0]->m_UnDeformedPos*weightA0[0] +
		               m_FaceNodes[0][1]->m_UnDeformedPos*weightA0[1] +
		               m_FaceNodes[0][2]->m_UnDeformedPos*weightA0[2];

	GFPhysVector3 A1 = m_FaceNodes[1][0]->m_UnDeformedPos*weightA1[0] +
		               m_FaceNodes[1][1]->m_UnDeformedPos*weightA1[1] +
		               m_FaceNodes[1][2]->m_UnDeformedPos*weightA1[2];


	GFPhysVector3 B0 = m_FaceNodes[2][0]->m_UnDeformedPos*weightB0[0] +
		               m_FaceNodes[2][1]->m_UnDeformedPos*weightB0[1] +
		               m_FaceNodes[2][2]->m_UnDeformedPos*weightB0[2];

	GFPhysVector3 B1 = m_FaceNodes[3][0]->m_UnDeformedPos*weightB1[0] +
		               m_FaceNodes[3][1]->m_UnDeformedPos*weightB1[1] +
		               m_FaceNodes[3][2]->m_UnDeformedPos*weightB1[2];

	m_RestLength[0] = (A0 - A1).Length();
	m_RestLength[1] = (B0 - B1).Length();
	m_RestLength[2] = (A0 - B0).Length();
	m_RestLength[3] = (A1 - B1).Length();
	m_RestLength[4] = (A1 - B0).Length();
	m_RestLength[5] = (A0 - B1).Length();

	GFPhysVector3 c = B1 - A0;
	GFPhysVector3 a = A1 - A0;
	GFPhysVector3 b = B0 - A0;

	const Real denom = Real(1) / Real(6);

	m_RestVolume = denom*(a.Cross(b)).Dot(c);

	//cache edge solve sumgrad
	int solveedgeindex[] = { 0, 1, 2, 3, 0, 2, 1, 3, 1, 2, 0, 3 };

	for (int e = 0; e < 6; e++)
	{
		int t0 = solveedgeindex[e * 2];

		int t1 = solveedgeindex[e * 2 + 1];

		GFPhysSoftBodyNode * * Face0Nodes = m_FaceNodes[t0];

		GFPhysSoftBodyNode * * Face1Nodes = m_FaceNodes[t1];

		float * weights0 = m_weights[t0];

		float * weights1 = m_weights[t1];

		Real Face0NodesMass = m_InvMass[t0];

		Real Face1NodesMass = m_InvMass[t1];

		m_CachedSumGrad[e] = weights0[0] * weights0[0] * Face0NodesMass
			+ weights0[1] * weights0[1] * Face0NodesMass
			+ weights0[2] * weights0[2] * Face0NodesMass
			+ weights1[0] * weights1[0] * Face1NodesMass
			+ weights1[1] * weights1[1] * Face1NodesMass
			+ weights1[2] * weights1[2] * Face1NodesMass;
	}
}

void VeinFaceFaceWeakConstraint::PrepareSolveConstraints()
{

}
//------------------------------------------------------------------------------------------------
void VeinFaceFaceWeakConstraint::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	Stiffniss = Stiffniss * m_Stiffness;

	GFPhysVector3 FacePoints[4];

	for(int f = 0 ; f < 4 ; f++)
	{
		FacePoints[f] =  m_FaceNodes[f][0]->m_CurrPosition*m_weights[f][0]
						+m_FaceNodes[f][1]->m_CurrPosition*m_weights[f][1]
						+m_FaceNodes[f][2]->m_CurrPosition*m_weights[f][2];
	}

	//check whether we need use hook
	bool HasHookDeviate = false;

	if(m_InHookState == true)
	{
		Stiffniss = 0.02f;//*0.25f;//hook stiffniss

		GFPhysVector3 FacePointA = FacePoints[0];

		GFPhysVector3 FacePointB = FacePoints[2];

		GFPhysSoftBodyFace * FaceA = m_Face[0];

		GFPhysSoftBodyFace * FaceB = m_Face[2];

		float FaceAPartLen = (FacePointA-m_StickPoint).Length();
		float FaceBPartLen = (FacePointB-m_StickPoint).Length();
		float TotalLen = FaceAPartLen+FaceBPartLen;
		float RestLen = m_RestLength[2];
        if (TotalLen > RestLen && TotalLen > GP_EPSILON  && FaceBPartLen > GP_EPSILON && FaceAPartLen > GP_EPSILON)
		{
			float FaceADiff = (TotalLen-RestLen)*FaceAPartLen / TotalLen;

			float FaceBDiff = (TotalLen-RestLen)*FaceBPartLen / TotalLen;

			float * weightsA = m_weights[0];
			
			float * weightsB = m_weights[2];
			
			Real Invsumgrad = 1.0f / (weightsA[0]*weightsA[0]+weightsA[1]*weightsA[1]+weightsA[2]*weightsA[2]);
		
			GFPhysVector3 s = -Stiffniss * Invsumgrad * FaceADiff * (FacePointA-m_StickPoint) / FaceAPartLen;

			if(m_InvMass[0] > GP_EPSILON)
			{
				FaceA->m_Nodes[0]->m_CurrPosition += (s * weightsA[0]);
				FaceA->m_Nodes[1]->m_CurrPosition += (s * weightsA[1]);
				FaceA->m_Nodes[2]->m_CurrPosition += (s * weightsA[2]);
			}
			

			Invsumgrad = 1.0f / (weightsB[0]*weightsB[0]+weightsB[1]*weightsB[1]+weightsB[2]*weightsB[2]);

			s = -Stiffniss * Invsumgrad * FaceBDiff * (FacePointB-m_StickPoint) / FaceBPartLen;

            if (m_InvMass[2] > GP_EPSILON)
			{
				FaceB->m_Nodes[0]->m_CurrPosition += (s * weightsB[0]);
				FaceB->m_Nodes[1]->m_CurrPosition += (s * weightsB[1]);
				FaceB->m_Nodes[2]->m_CurrPosition += (s * weightsB[2]);
			}
		}
	}
	else
	{
		//solve volume
		/*
		const Real denom = Real(1)/Real(6);

		GFPhysVector3 c = FacePoints[3]-FacePoints[0];
		GFPhysVector3 a = FacePoints[1]-FacePoints[0];
		GFPhysVector3 b = FacePoints[2]-FacePoints[0];

		GFPhysVector3 IntermGrad[4];

		IntermGrad[1] = denom*b.Cross(c);
		IntermGrad[2] = denom*c.Cross(a);
		IntermGrad[3] = denom*a.Cross(b);
		IntermGrad[0] = -IntermGrad[1]-IntermGrad[2]-IntermGrad[3];

		GFPhysVector3 GradTerm[4][3];
		//Real GradWeights[4][3];

		Real sumgradlen = 0;

		for(int f = 0 ; f < 4 ; f++)
		{
			GradTerm[f][0] = IntermGrad[f]*m_weights[f][0];
			GradTerm[f][1] = IntermGrad[f]*m_weights[f][1];
			GradTerm[f][2] = IntermGrad[f]*m_weights[f][2];

			//GradWeights[f][0] = m_InvMass[f];
			//GradWeights[f][1] = m_InvMass[f];
			//GradWeights[f][2] = m_InvMass[f];

			sumgradlen += m_InvMass[f]*GradTerm[f][0].Length2()+m_InvMass[f]*GradTerm[f][1].Length2()+m_InvMass[f]*GradTerm[f][2].Length2();
		}

		if(sumgradlen > GP_EPSILON)
		{
			Real currvolume = IntermGrad[3].Dot(c);

			Real volumediff = currvolume-m_RestVolume;

			Real s = -volumediff/ sumgradlen;

			float smulstiff = s*Stiffniss;

			for(int f = 0 ; f < 4 ; f++)
			{
				m_FaceNodes[f][0]->m_CurrPosition += GradTerm[f][0] * m_InvMass[f] * smulstiff;
				m_FaceNodes[f][1]->m_CurrPosition += GradTerm[f][1] * m_InvMass[f] * smulstiff;
				m_FaceNodes[f][2]->m_CurrPosition += GradTerm[f][2] * m_InvMass[f] * smulstiff;
			}
		}*/
		//solve 5 Edge
		int solveedgeindex[] = {0,1,2,3,0,2,1,3,1,2,0,3};
		
		for(int e = 2 ; e < 6 ; e++)
		{
			int t0 = solveedgeindex[e*2];
			
			int t1 = solveedgeindex[e*2+1];

			GFPhysSoftBodyNode * * Face0Nodes = m_FaceNodes[t0];

			GFPhysSoftBodyNode * * Face1Nodes = m_FaceNodes[t1];

			float * weights0 = m_weights[t0];

			float * weights1 = m_weights[t1];

			Real Face0NodesMass = m_InvMass[t0];
			
			Real Face1NodesMass = m_InvMass[t1];

			GFPhysVector3 p0 =  Face0Nodes[0]->m_CurrPosition*weights0[0]
							   +Face0Nodes[1]->m_CurrPosition*weights0[1]
							   +Face0Nodes[2]->m_CurrPosition*weights0[2];

			GFPhysVector3 p1 =  Face1Nodes[0]->m_CurrPosition*weights1[0]
							   +Face1Nodes[1]->m_CurrPosition*weights1[1] 
							   +Face1Nodes[2]->m_CurrPosition*weights1[2];

			GFPhysVector3 Diff = (p0-p1);

			float  Length = Diff.Length();

			float  diffLen = Length-m_RestLength[e];

			if (Length > GP_EPSILON)
			{	
				Diff /= Length;//diff is normalized now 

				GFPhysVector3 grad00 = Diff*weights0[0];
				GFPhysVector3 grad01 = Diff*weights0[1];
				GFPhysVector3 grad02 = Diff*weights0[2];

				GFPhysVector3 grad10 = -Diff*weights1[0];
				GFPhysVector3 grad11 = -Diff*weights1[1];
				GFPhysVector3 grad12 = -Diff*weights1[2];

				
				//weighted by inverse mass 's sum of gradient !!!optimize me can be cached
				Real sumgrad = m_CachedSumGrad[e];

                if (sumgrad > GP_EPSILON)
				{
					Real scale = Stiffniss*(-diffLen) / sumgrad;

					Real ms0 = scale * Face0NodesMass;
					Real ms1 = scale * Face1NodesMass;

					GFPhysVector3 delta00 = grad00 * ms0;
					GFPhysVector3 delta01 = grad01 * ms0;
					GFPhysVector3 delta02 = grad02 * ms0;

					GFPhysVector3 delta10 = grad10 * ms1;
					GFPhysVector3 delta11 = grad11 * ms1;
					GFPhysVector3 delta12 = grad12 * ms1;

					Face0Nodes[0]->m_CurrPosition += delta00;
					Face0Nodes[1]->m_CurrPosition += delta01;
					Face0Nodes[2]->m_CurrPosition += delta02;

					Face1Nodes[0]->m_CurrPosition += delta10;
					Face1Nodes[1]->m_CurrPosition += delta11;
					Face1Nodes[2]->m_CurrPosition += delta12;
				}
			}
		}
	}
}


NodeOnPlaneCS::NodeOnPlaneCS(GFPhysSoftBodyNode * node)
{
	m_Node = node;
}
void NodeOnPlaneCS::UpdatePlane(const GFPhysVector3 & planeNormal , const GFPhysVector3 & planePoint)
{
	m_PlaneNormal = planeNormal;
	m_planePoint = planePoint;
}
void NodeOnPlaneCS::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	GFPhysVector3 nodePos = m_Node->m_CurrPosition;
	float dist = (nodePos-m_planePoint).Dot(m_PlaneNormal);
	m_Node->m_CurrPosition -= m_PlaneNormal*dist*m_Stiffness;
}

//===========================================================================================
FaceEdgePointAttachConstraint::FaceEdgePointAttachConstraint( 
												//GFPhysSoftBodyNode * Facenodes[3] , 
												//GFPhysSoftBodyNode * EdgeNodes[2] , 
												GFPhysSoftBodyFace * AttachFace,
												GFPhysSoftBodyEdge * AttachEdge,
												float FaceWeights[3] ,
												float EdgeWeights[2],
												float MassFace,
												float MassEdge)
{
		m_AttachEdge = AttachEdge;
		m_AttachFace = AttachFace;

		m_FaceNodes[0] = m_AttachFace->m_Nodes[0];
		m_FaceNodes[1] = m_AttachFace->m_Nodes[1];
		m_FaceNodes[2] = m_AttachFace->m_Nodes[2];

		m_EdgeNodes[0] = m_AttachEdge->m_Nodes[0];
		m_EdgeNodes[1] = m_AttachEdge->m_Nodes[1];

		m_FaceWeights[0] = FaceWeights[0];
		m_FaceWeights[1] = FaceWeights[1];
		m_FaceWeights[2] = FaceWeights[2];

		m_EdgeWeights[0] = EdgeWeights[0];
		m_EdgeWeights[1] = EdgeWeights[1];
	
		m_InvMassFace[0] = m_FaceNodes[0]->m_InvM;//(MassFace > FLT_EPSILON ? 1.0f / MassFace : 0);
		m_InvMassFace[1] = m_FaceNodes[1]->m_InvM;
		m_InvMassFace[2] = m_FaceNodes[2]->m_InvM;
		
		m_InvMassEdge[0] = m_EdgeNodes[0]->m_InvM;//(MassEdge > FLT_EPSILON ? 1.0f / MassEdge : 0);
		m_InvMassEdge[1] = m_EdgeNodes[1]->m_InvM;

		float facemass = m_FaceNodes[0]->m_Mass+m_FaceNodes[1]->m_Mass+m_FaceNodes[2]->m_Mass;
		float edgemass = m_EdgeNodes[0]->m_Mass+m_EdgeNodes[1]->m_Mass;

		if(facemass > FLT_EPSILON)
		   m_InvMassPerFace = 1.0f / facemass;
		else
		   m_InvMassPerFace = 0.0f;

		if(edgemass > FLT_EPSILON)
		   m_InvMassPerEdge = 1.0f / edgemass;
		else
		   m_InvMassPerEdge = 0.0f;

		m_IsValid = true;
}
//===========================================================================================
FaceEdgePointAttachConstraint::~FaceEdgePointAttachConstraint()
{

}
//===========================================================================================
void FaceEdgePointAttachConstraint::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
//===========================================================================================
void FaceEdgePointAttachConstraint::SolveConstraint(Real Stiffniss,Real TimeStep)
{

	GFPhysVector3 pTetra = m_FaceNodes[0]->m_CurrPosition*m_FaceWeights[0]
						  +m_FaceNodes[1]->m_CurrPosition*m_FaceWeights[1]
						  +m_FaceNodes[2]->m_CurrPosition*m_FaceWeights[2];

	GFPhysVector3 pAttach = m_EdgeNodes[0]->m_CurrPosition*m_EdgeWeights[0]
						   +m_EdgeNodes[1]->m_CurrPosition*m_EdgeWeights[1];

	GFPhysVector3 Diff = (pTetra-pAttach);

	float diffLen = Diff.Length();

	if(diffLen > FLT_EPSILON)
	{	
		Diff /= diffLen;

		float sumgrad =  m_FaceWeights[0]*m_FaceWeights[0]*m_InvMassFace[0]
						+m_FaceWeights[1]*m_FaceWeights[1]*m_InvMassFace[1]
						+m_FaceWeights[2]*m_FaceWeights[2]*m_InvMassFace[2]
						+m_EdgeWeights[0]*m_EdgeWeights[0]*m_InvMassEdge[0]
						+m_EdgeWeights[1]*m_EdgeWeights[1]*m_InvMassEdge[1];

		//Stiffniss = Stiffniss*m_Stiffness;

		float scale = m_Stiffness*(-diffLen) / sumgrad;

		GFPhysVector3 gradF0 = Diff*m_FaceWeights[0];
		GFPhysVector3 gradF1 = Diff*m_FaceWeights[1];
		GFPhysVector3 gradF2 = Diff*m_FaceWeights[2];

		GFPhysVector3 gradE0 = -Diff*m_EdgeWeights[0];
		GFPhysVector3 gradE1 = -Diff*m_EdgeWeights[1];

		GFPhysVector3 deltaF0 = scale*gradF0*m_InvMassFace[0];
		GFPhysVector3 deltaF1 = scale*gradF1*m_InvMassFace[1];
		GFPhysVector3 deltaF2 = scale*gradF2*m_InvMassFace[2];
	
		GFPhysVector3 deltaE0 = scale*gradE0*m_InvMassEdge[0];
		GFPhysVector3 deltaE1 = scale*gradE1*m_InvMassEdge[1];
		
		//new method
		float InvMassTotal = m_InvMassPerFace + m_InvMassPerEdge;

		if(InvMassTotal > FLT_EPSILON)
		{
			float wf = m_InvMassPerFace / InvMassTotal;
			float we = m_InvMassPerEdge / InvMassTotal;

			GFPhysVector3 deltaf = Diff * (-diffLen)  * m_Stiffness * wf;
			GFPhysVector3 deltae = -Diff * (-diffLen) * m_Stiffness * we;
#if(0)
			m_FaceNodes[0]->m_CurrPosition += deltaf;
			m_FaceNodes[1]->m_CurrPosition += deltaf;
			m_FaceNodes[2]->m_CurrPosition += deltaf;
			

			m_EdgeNodes[0]->m_CurrPosition += deltae;
			m_EdgeNodes[1]->m_CurrPosition += deltae;
#else
			m_FaceNodes[0]->m_CurrPosition += deltaF0;
			m_FaceNodes[1]->m_CurrPosition += deltaF1;
			m_FaceNodes[2]->m_CurrPosition += deltaF2;

			m_EdgeNodes[0]->m_CurrPosition += deltaE0;
			m_EdgeNodes[1]->m_CurrPosition += deltaE1;
#endif
			m_EdgeNodes[0]->m_StateFlag |= GPSESF_ATTACHED;
			m_EdgeNodes[1]->m_StateFlag |= GPSESF_ATTACHED;
		}

		return;
		//
		m_FaceNodes[0]->m_CurrPosition += deltaF0;
		m_FaceNodes[1]->m_CurrPosition += deltaF1;
		m_FaceNodes[2]->m_CurrPosition += deltaF2;
		
		m_EdgeNodes[0]->m_CurrPosition += deltaE0;
		m_EdgeNodes[1]->m_CurrPosition += deltaE1;

	}
}


TestPointPointDistanceConstraint::TestPointPointDistanceConstraint(GoPhys::GFPhysSoftBodyNode *p1, GoPhys::GFPhysSoftBodyNode *p2, float w1, float w2, float d):
m_p1(p1), m_p2(p2), m_w1(w1), m_w2(w2), m_d(d)
{

}

void TestPointPointDistanceConstraint::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	GFPhysVector3 Diff = m_p1->m_CurrPosition-m_p2->m_CurrPosition;

	Real Length = Diff.Length();

	Real diffLen = Length-m_d;


	if(diffLen > FLT_EPSILON && diffLen > 0)
	{	
		Diff /= Length;//diff is normalized now 
		GFPhysVector3 detaP1 =  (-m_w1)/(m_w1+m_w2)*diffLen*Diff;
		GFPhysVector3 detaP2 =  (m_w2)/(m_w1+m_w2)*diffLen*Diff;

		m_p1->m_CurrPosition += detaP1;
		m_p2->m_CurrPosition += detaP2;
	}
}

Face_FaceConnection::Face_FaceConnection(GFPhysSoftBodyFace * face0 , GFPhysSoftBodyFace * face1 , float EdgeStiffness , float VolumeStiffness)
{
	m_Face[0] = face0;
	m_Face[1] = face1;

	GFPhysSoftBodyNode * Nodes0[3];
	GFPhysSoftBodyNode * Nodes1[3];

	Nodes0[0] = face0->m_Nodes[0];
	Nodes0[1] = face0->m_Nodes[1];
	Nodes0[2] = face0->m_Nodes[2];

	int TestLinkOrder[] = {0,1,2, 0,2,1, 1,0,2, 1,2,0 , 2,0,1, 2,1,0};
	//choose min total length
	float minTotalDist = FLT_MAX;
	int   chooseIndex = 0;
	for(int c = 0 ; c < 6 ; c++)
	{
		int index0 = TestLinkOrder[c*3];
		int index1 = TestLinkOrder[c*3+1];
		int index2 = TestLinkOrder[c*3+2];
		float dist0 = (Nodes0[0]->m_UnDeformedPos-face1->m_Nodes[index0]->m_UnDeformedPos).Length();
		float dist1 = (Nodes0[1]->m_UnDeformedPos-face1->m_Nodes[index1]->m_UnDeformedPos).Length();
		float dist2 = (Nodes0[2]->m_UnDeformedPos-face1->m_Nodes[index2]->m_UnDeformedPos).Length();
		float dist = dist0+dist1+dist2;
		if(dist < minTotalDist)
		{
			minTotalDist = dist;
			chooseIndex = c;
		}
	}
	Nodes1[0] = face1->m_Nodes[TestLinkOrder[chooseIndex*3]];
	Nodes1[1] = face1->m_Nodes[TestLinkOrder[chooseIndex*3+1]];
	Nodes1[2] = face1->m_Nodes[TestLinkOrder[chooseIndex*3+2]];


	m_EdgeNode[0] = Nodes0[0];
	m_EdgeNode[1] = Nodes0[1];

	m_EdgeNode[2] = Nodes0[1];
	m_EdgeNode[3] = Nodes0[2];

	m_EdgeNode[4] = Nodes0[2];
	m_EdgeNode[5] = Nodes0[0];

	m_EdgeNode[6] = Nodes1[0];
	m_EdgeNode[7] = Nodes1[1];

	m_EdgeNode[8] = Nodes1[1];
	m_EdgeNode[9] = Nodes1[2];

	m_EdgeNode[10] = Nodes1[2];
	m_EdgeNode[11] = Nodes1[0];

	m_EdgeNode[12] = Nodes0[0];
	m_EdgeNode[13] = Nodes1[0];

	m_EdgeNode[14] = Nodes0[1];
	m_EdgeNode[15] = Nodes1[1];

	m_EdgeNode[16] = Nodes0[2];
	m_EdgeNode[17] = Nodes1[2];

	m_EdgeNode[18] = Nodes0[0];
	m_EdgeNode[19] = Nodes1[1];

	m_EdgeNode[20] = Nodes0[2];
	m_EdgeNode[21] = Nodes1[1];

	m_EdgeNode[22] = Nodes0[0];
	m_EdgeNode[23] = Nodes1[2];

	m_VolumeNode[0] = Nodes0[0];
	m_VolumeNode[1] = Nodes0[1];
	m_VolumeNode[2] = Nodes0[2];
	m_VolumeNode[3] = Nodes1[1];

	m_VolumeNode[4] = Nodes0[0];
	m_VolumeNode[5] = Nodes0[2];
	m_VolumeNode[6] = Nodes1[1];
	m_VolumeNode[7] = Nodes1[2];

	m_VolumeNode[8]  = Nodes1[0];
	m_VolumeNode[9]  = Nodes1[1];
	m_VolumeNode[10] = Nodes1[2];
	m_VolumeNode[11] = Nodes0[0];

	for(int e = 0 ; e  < 12 ; e++)
	{
		GFPhysVector3 Pos0 = m_EdgeNode[e*2]->m_UnDeformedPos;
		GFPhysVector3 Pos1 = m_EdgeNode[e*2+1]->m_UnDeformedPos;
		m_EdgeRest[e] = (Pos0-Pos1).Length();
	}

	for(int v = 0 ; v < 3 ; v++)
	{
		GFPhysVector3 Pos0 = m_VolumeNode[v*4]->m_UnDeformedPos;
		GFPhysVector3 Pos1 = m_VolumeNode[v*4+1]->m_UnDeformedPos;
		GFPhysVector3 Pos2 = m_VolumeNode[v*4+2]->m_UnDeformedPos;
		GFPhysVector3 Pos3 = m_VolumeNode[v*4+3]->m_UnDeformedPos;
		m_VolumRest[v] = (Pos1-Pos0).Dot((Pos2-Pos0).Cross(Pos3-Pos0));
		m_VolumRest[v] /= Real(6);
	}

	m_EdgeStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(EdgeStiffness);
	m_VolumStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(VolumeStiffness);
}

void Face_FaceConnection::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	float edgeStiffness  = m_EdgeStiffness;
	float volumStiffness = m_VolumStiffness;
	//solve edge first
	for(int e = 0 ; e < 12 ; e++)
	{
		GFPhysSoftBodyNode * Node1 = m_EdgeNode[2*e];
		GFPhysSoftBodyNode * Node2 = m_EdgeNode[2*e+1];

		Real RestLen  = m_EdgeRest[e];

		assert(Node1 && Node2);

		Real w1 = Node1 -> m_InvM;

		Real w2 = Node2 -> m_InvM;

		Real w = w1+w2;

		Real Invw1w2 = (w > GP_EPSILON ? 1.0f / w : 0.0f);

		GFPhysVector3 Diff = Node1->m_CurrPosition - Node2->m_CurrPosition;
		Real Length = Diff.Length();

		Real InvLength = (Length > GP_EPSILON ? 1.0f / Length : 0);

		Real DstRecoverLen = (Length-RestLen)*edgeStiffness;

		Real Temp = DstRecoverLen * InvLength;

		Real itc = Invw1w2 * Temp;


		GFPhysVector3 Deta1 = -(w1 * itc) * Diff;

		GFPhysVector3 Deta2 =  (w2 * itc) * Diff;

		if((Node1->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
		    Node1->m_CurrPosition += Deta1;

		if((Node2->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
		    Node2->m_CurrPosition += Deta2;
	}
	//solve column
	for(int v = 0 ; v < 3 ;v++)
	{
		float RestValue = m_VolumRest[v];
		
		GFPhysSoftBodyNode * NodeA = m_VolumeNode[v*4+0];
		GFPhysSoftBodyNode * NodeB = m_VolumeNode[v*4+1];
		GFPhysSoftBodyNode * NodeC = m_VolumeNode[v*4+2];
		GFPhysSoftBodyNode * NodeD = m_VolumeNode[v*4+3];

		Real wa = NodeA -> m_InvM;
		Real wb = NodeB -> m_InvM;
		Real wc = NodeC -> m_InvM;
		Real wd = NodeD -> m_InvM;
		
		const Real denom = Real(1)/Real(6);

		GFPhysVector3 ba = (NodeB->m_CurrPosition-NodeA->m_CurrPosition);
		GFPhysVector3 ca = (NodeC->m_CurrPosition-NodeA->m_CurrPosition);
		GFPhysVector3 da = (NodeD->m_CurrPosition-NodeA->m_CurrPosition);
		GFPhysVector3 cb = (NodeC->m_CurrPosition-NodeB->m_CurrPosition);
		GFPhysVector3 db = (NodeD->m_CurrPosition-NodeB->m_CurrPosition);
				
		/*Gradient of second node(node b)*/
		GFPhysVector3 gradientb = ca.Cross(da) * denom;
		
		/*Gradient of third node(node c)*/
		GFPhysVector3 gradientc = da.Cross(ba) * denom;
		
		/*Gradient of forth node(node d)*/
		GFPhysVector3 gradientd = ba.Cross(ca) * denom;
		
		/*Gradient of first node(node a)*/
		GFPhysVector3 gradienta = -gradientb-gradientc-gradientd;
				
		/*Current Signed Volume*/
		Real currvolume = ba.Dot(gradientb);
				
		Real volumediff = currvolume-RestValue;
				
		Real sumgradlen = (wa*gradienta.Length2()+wb*gradientb.Length2()+wc*gradientc.Length2()+wd*gradientd.Length2());
						
		assert(sumgradlen >=0 );//=.=
		
		Real InvSumGradLen = (sumgradlen > GP_EPSILON ? 1.0f / sumgradlen : 0.0f);

		Real s = -volumediff * InvSumGradLen;

		Real smulstiff = s*volumStiffness;

		GFPhysVector3 DetaA = (gradienta * wa*smulstiff);
		GFPhysVector3 DetaB = (gradientb * wb*smulstiff);
		GFPhysVector3 DetaC = (gradientc * wc*smulstiff);
		GFPhysVector3 DetaD = (gradientd * wd*smulstiff);
		
		if((NodeA->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
		    NodeA->m_CurrPosition += DetaA;

		if((NodeB->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
		    NodeB->m_CurrPosition += DetaB;

		if((NodeC->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
		    NodeC->m_CurrPosition += DetaC;

		if((NodeD->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
		    NodeD->m_CurrPosition += DetaD;
	}
}


VeinFaceFaceConnect::VeinFaceFaceConnect()
{
	m_inContactStick = false;
	m_Stiffness = 0.28f;
	m_Damping = 0.02f;
	m_Valid = true;
	m_RestLen = 0;
}
//=========================================================================================================
void VeinFaceFaceConnect::Construct(GFPhysSoftBodyFace * FaceA , 
									GFPhysSoftBodyFace * FaceB , 
									float faceAweight[3] ,
									float faceBweight[3] ,
									float FaceAInvMass,
									float FaceBInvMass)
{
	//volume
	m_Face[0] = FaceA;
	m_Face[1] = FaceB;

	weightsinFaceA[0] = faceAweight[0];
	weightsinFaceA[1] = faceAweight[1];
	weightsinFaceA[2] = faceAweight[2];

	weightsinFaceB[0] = faceBweight[0];
	weightsinFaceB[1] = faceBweight[1];
	weightsinFaceB[2] = faceBweight[2];

	GFPhysVector3 PointInFaceA = weightsinFaceA[0]*m_Face[0]->m_Nodes[0]->m_CurrPosition
								+weightsinFaceA[1]*m_Face[0]->m_Nodes[1]->m_CurrPosition
								+weightsinFaceA[2]*m_Face[0]->m_Nodes[2]->m_CurrPosition;

	GFPhysVector3 PointInFaceB = weightsinFaceB[0]*m_Face[1]->m_Nodes[0]->m_CurrPosition
								+weightsinFaceB[1]*m_Face[1]->m_Nodes[1]->m_CurrPosition
								+weightsinFaceB[2]*m_Face[1]->m_Nodes[2]->m_CurrPosition;

	m_RestLen = (PointInFaceA - PointInFaceB).Length();
	
	m_faceAInvM = FaceAInvMass;

	m_faceBInvM = FaceBInvMass;

	GFPhysSoftBodyNode * Nodes0[3];
	GFPhysSoftBodyNode * Nodes1[3];

	Nodes0[0] = FaceA->m_Nodes[0];
	Nodes0[1] = FaceA->m_Nodes[1];
	Nodes0[2] = FaceA->m_Nodes[2];

	int TestLinkOrder[] = {0,1,2, 0,2,1, 1,0,2, 1,2,0 , 2,0,1, 2,1,0};
	//choose min total length
	float minTotalDist = FLT_MAX;
	int   chooseIndex = 0;
	for(int c = 0 ; c < 6 ; c++)
	{
		int index0 = TestLinkOrder[c*3];
		int index1 = TestLinkOrder[c*3+1];
		int index2 = TestLinkOrder[c*3+2];
		float dist0 = (Nodes0[0]->m_CurrPosition-FaceB->m_Nodes[index0]->m_CurrPosition).Length();
		float dist1 = (Nodes0[1]->m_CurrPosition-FaceB->m_Nodes[index1]->m_CurrPosition).Length();
		float dist2 = (Nodes0[2]->m_CurrPosition-FaceB->m_Nodes[index2]->m_CurrPosition).Length();
		float dist = dist0+dist1+dist2;
		if(dist < minTotalDist)
		{
			minTotalDist = dist;
			chooseIndex = c;
		}
	}
	Nodes1[0] = FaceB->m_Nodes[TestLinkOrder[chooseIndex*3]];
	Nodes1[1] = FaceB->m_Nodes[TestLinkOrder[chooseIndex*3+1]];
	Nodes1[2] = FaceB->m_Nodes[TestLinkOrder[chooseIndex*3+2]];


	m_EdgeNode[0] = Nodes0[0];
	m_EdgeNode[1] = Nodes0[1];

	m_EdgeNode[2] = Nodes0[1];
	m_EdgeNode[3] = Nodes0[2];

	m_EdgeNode[4] = Nodes0[2];
	m_EdgeNode[5] = Nodes0[0];

	m_EdgeNode[6] = Nodes1[0];
	m_EdgeNode[7] = Nodes1[1];

	m_EdgeNode[8] = Nodes1[1];
	m_EdgeNode[9] = Nodes1[2];

	m_EdgeNode[10] = Nodes1[2];
	m_EdgeNode[11] = Nodes1[0];

	m_EdgeNode[12] = Nodes0[0];
	m_EdgeNode[13] = Nodes1[0];

	m_EdgeNode[14] = Nodes0[1];
	m_EdgeNode[15] = Nodes1[1];

	m_EdgeNode[16] = Nodes0[2];
	m_EdgeNode[17] = Nodes1[2];

	m_EdgeNode[18] = Nodes0[0];
	m_EdgeNode[19] = Nodes1[1];

	m_EdgeNode[20] = Nodes0[2];
	m_EdgeNode[21] = Nodes1[1];

	m_EdgeNode[22] = Nodes0[0];
	m_EdgeNode[23] = Nodes1[2];

	m_VolumeNode[0] = Nodes0[0];
	m_VolumeNode[1] = Nodes0[1];
	m_VolumeNode[2] = Nodes0[2];
	m_VolumeNode[3] = Nodes1[1];

	m_VolumeNode[4] = Nodes0[0];
	m_VolumeNode[5] = Nodes0[2];
	m_VolumeNode[6] = Nodes1[1];
	m_VolumeNode[7] = Nodes1[2];

	m_VolumeNode[8]  = Nodes1[0];
	m_VolumeNode[9]  = Nodes1[1];
	m_VolumeNode[10] = Nodes1[2];
	m_VolumeNode[11] = Nodes0[0];

	for(int e = 0 ; e  < 12 ; e++)
	{
		GFPhysVector3 Pos0 = m_EdgeNode[e*2]->m_CurrPosition;
		GFPhysVector3 Pos1 = m_EdgeNode[e*2+1]->m_CurrPosition;
		m_EdgeRest[e] = (Pos0-Pos1).Length();
	}

	for(int v = 0 ; v < 3 ; v++)
	{
		GFPhysVector3 Pos0 = m_VolumeNode[v*4]->m_CurrPosition;
		GFPhysVector3 Pos1 = m_VolumeNode[v*4+1]->m_CurrPosition;
		GFPhysVector3 Pos2 = m_VolumeNode[v*4+2]->m_CurrPosition;
		GFPhysVector3 Pos3 = m_VolumeNode[v*4+3]->m_CurrPosition;
		m_VolumRest[v] = (Pos1-Pos0).Dot((Pos2-Pos0).Cross(Pos3-Pos0));
		m_VolumRest[v] /= Real(6);
	}

}
//==========================================================================================================
void VeinFaceFaceConnect::OnAttachFaceChanged(GFPhysSoftBodyFace * FaceA0,
	                                          GFPhysSoftBodyFace * FaceA1,
	                                          GFPhysSoftBodyFace * FaceB0,
	                                          GFPhysSoftBodyFace * FaceB1,
	                                          float weightA0[3],
	                                          float weightA1[3],
	                                          float weightB0[3],
	                                          float weightB1[3])
{
	  //volume
	  m_Face[0] = FaceA0;
	  m_Face[1] = FaceB0;

	  weightsinFaceA[0] = weightA0[0];
	  weightsinFaceA[1] = weightA0[1];
	  weightsinFaceA[2] = weightA0[2];

	  weightsinFaceB[0] = weightB0[0];
	  weightsinFaceB[1] = weightB0[1];
	  weightsinFaceB[2] = weightB0[2];

	  GFPhysVector3 PointInFaceA = weightsinFaceA[0] * m_Face[0]->m_Nodes[0]->m_UnDeformedPos
		                         + weightsinFaceA[1] * m_Face[0]->m_Nodes[1]->m_UnDeformedPos
		                         + weightsinFaceA[2] * m_Face[0]->m_Nodes[2]->m_UnDeformedPos;

	  GFPhysVector3 PointInFaceB = weightsinFaceB[0] * m_Face[1]->m_Nodes[0]->m_UnDeformedPos
		                         + weightsinFaceB[1] * m_Face[1]->m_Nodes[1]->m_UnDeformedPos
		                         + weightsinFaceB[2] * m_Face[1]->m_Nodes[2]->m_UnDeformedPos;

	  m_RestLen = (PointInFaceA - PointInFaceB).Length();



	  GFPhysSoftBodyNode * Nodes0[3];
	  GFPhysSoftBodyNode * Nodes1[3];

	  Nodes0[0] = FaceA0->m_Nodes[0];
	  Nodes0[1] = FaceA0->m_Nodes[1];
	  Nodes0[2] = FaceA0->m_Nodes[2];

	  int TestLinkOrder[] = { 0, 1, 2, 0, 2, 1, 1, 0, 2, 1, 2, 0, 2, 0, 1, 2, 1, 0 };
	
	  //choose min total length
	  float minTotalDist = FLT_MAX;
	  int   chooseIndex = 0;
	  for (int c = 0; c < 6; c++)
	  {
		  int index0 = TestLinkOrder[c * 3];
		  int index1 = TestLinkOrder[c * 3 + 1];
		  int index2 = TestLinkOrder[c * 3 + 2];
		  float dist0 = (Nodes0[0]->m_UnDeformedPos - FaceB0->m_Nodes[index0]->m_UnDeformedPos).Length();
		  float dist1 = (Nodes0[1]->m_UnDeformedPos - FaceB0->m_Nodes[index1]->m_UnDeformedPos).Length();
		  float dist2 = (Nodes0[2]->m_UnDeformedPos - FaceB0->m_Nodes[index2]->m_UnDeformedPos).Length();
		  float dist = dist0 + dist1 + dist2;
		  if (dist < minTotalDist)
		  {
			minTotalDist = dist;
			chooseIndex = c;
		  }
	  }
	  Nodes1[0] = FaceB0->m_Nodes[TestLinkOrder[chooseIndex * 3]];
	  Nodes1[1] = FaceB0->m_Nodes[TestLinkOrder[chooseIndex * 3 + 1]];
	  Nodes1[2] = FaceB0->m_Nodes[TestLinkOrder[chooseIndex * 3 + 2]];


	  m_EdgeNode[0] = Nodes0[0];
	  m_EdgeNode[1] = Nodes0[1];

	  m_EdgeNode[2] = Nodes0[1];
	  m_EdgeNode[3] = Nodes0[2];

	  m_EdgeNode[4] = Nodes0[2];
	  m_EdgeNode[5] = Nodes0[0];

	  m_EdgeNode[6] = Nodes1[0];
	  m_EdgeNode[7] = Nodes1[1];

	  m_EdgeNode[8] = Nodes1[1];
	  m_EdgeNode[9] = Nodes1[2];

	  m_EdgeNode[10] = Nodes1[2];
	  m_EdgeNode[11] = Nodes1[0];

	  m_EdgeNode[12] = Nodes0[0];
	  m_EdgeNode[13] = Nodes1[0];

	  m_EdgeNode[14] = Nodes0[1];
	  m_EdgeNode[15] = Nodes1[1];

	  m_EdgeNode[16] = Nodes0[2];
	  m_EdgeNode[17] = Nodes1[2];

	  m_EdgeNode[18] = Nodes0[0];
	  m_EdgeNode[19] = Nodes1[1];

	  m_EdgeNode[20] = Nodes0[2];
	  m_EdgeNode[21] = Nodes1[1];

	  m_EdgeNode[22] = Nodes0[0];
	  m_EdgeNode[23] = Nodes1[2];

	  m_VolumeNode[0] = Nodes0[0];
	  m_VolumeNode[1] = Nodes0[1];
	  m_VolumeNode[2] = Nodes0[2];
	  m_VolumeNode[3] = Nodes1[1];

	  m_VolumeNode[4] = Nodes0[0];
	  m_VolumeNode[5] = Nodes0[2];
	  m_VolumeNode[6] = Nodes1[1];
	  m_VolumeNode[7] = Nodes1[2];

	  m_VolumeNode[8] = Nodes1[0];
	  m_VolumeNode[9] = Nodes1[1];
	  m_VolumeNode[10] = Nodes1[2];
	  m_VolumeNode[11] = Nodes0[0];

	 for (int e = 0; e < 12; e++)
	 {
		GFPhysVector3 Pos0 = m_EdgeNode[e * 2]->m_UnDeformedPos;
		GFPhysVector3 Pos1 = m_EdgeNode[e * 2 + 1]->m_UnDeformedPos;
		m_EdgeRest[e] = (Pos0 - Pos1).Length();
	 }

	 for (int v = 0; v < 3; v++)
	 {
		 GFPhysVector3 Pos0 = m_VolumeNode[v * 4]->m_UnDeformedPos;
		 GFPhysVector3 Pos1 = m_VolumeNode[v * 4 + 1]->m_UnDeformedPos;
		 GFPhysVector3 Pos2 = m_VolumeNode[v * 4 + 2]->m_UnDeformedPos;
		 GFPhysVector3 Pos3 = m_VolumeNode[v * 4 + 3]->m_UnDeformedPos;
		 m_VolumRest[v] = (Pos1 - Pos0).Dot((Pos2 - Pos0).Cross(Pos3 - Pos0));
		 m_VolumRest[v] /= Real(6);
	 }

}
//=============================================================================================
void VeinFaceFaceConnect::PrepareSolveConstraints()//edge collapse modify algo may change node value refresh!
{
	
}
//======================================================================================================
void VeinFaceFaceConnect::SolveConstraint(Real globalstiffness,Real TimeStep , bool solvevol)
{
	//solve edge first
	for(int e = 0 ; e < 12 ; e++)
	{
		GFPhysSoftBodyNode * Node1 = m_EdgeNode[2*e];
		GFPhysSoftBodyNode * Node2 = m_EdgeNode[2*e+1];

		Real RestLen  = m_EdgeRest[e];

		assert(Node1 && Node2);

		Real w1 = Node1 -> m_InvM;

		Real w2 = Node2 -> m_InvM;

		Real w = w1+w2;

		Real Invw1w2 = (w > GP_EPSILON ? 1.0f / w : 0.0f);

		GFPhysVector3 Diff = Node1->m_CurrPosition - Node2->m_CurrPosition;
		Real Length = Diff.Length();

		Real InvLength = (Length > GP_EPSILON ? 1.0f / Length : 0);

		Real DstRecoverLen = (Length-RestLen)*m_Stiffness;

		Real Temp = DstRecoverLen * InvLength;

		Real itc = Invw1w2 * Temp;


		GFPhysVector3 Deta1 = -(w1 * itc) * Diff;

		GFPhysVector3 Deta2 =  (w2 * itc) * Diff;

		//if((Node1->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
			Node1->m_CurrPosition += Deta1;

		//if((Node2->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
			Node2->m_CurrPosition += Deta2;
	}
	//solve volume
	for(int v = 0 ; v < 3 ;v++)
	{
		float RestValue = m_VolumRest[v];

		GFPhysSoftBodyNode * NodeA = m_VolumeNode[v*4+0];
		GFPhysSoftBodyNode * NodeB = m_VolumeNode[v*4+1];
		GFPhysSoftBodyNode * NodeC = m_VolumeNode[v*4+2];
		GFPhysSoftBodyNode * NodeD = m_VolumeNode[v*4+3];

		Real wa = NodeA -> m_InvM;
		Real wb = NodeB -> m_InvM;
		Real wc = NodeC -> m_InvM;
		Real wd = NodeD -> m_InvM;

		const Real denom = Real(1)/Real(6);

		GFPhysVector3 ba = (NodeB->m_CurrPosition-NodeA->m_CurrPosition);
		GFPhysVector3 ca = (NodeC->m_CurrPosition-NodeA->m_CurrPosition);
		GFPhysVector3 da = (NodeD->m_CurrPosition-NodeA->m_CurrPosition);
		GFPhysVector3 cb = (NodeC->m_CurrPosition-NodeB->m_CurrPosition);
		GFPhysVector3 db = (NodeD->m_CurrPosition-NodeB->m_CurrPosition);

		/*Gradient of second node(node b)*/
		GFPhysVector3 gradientb = ca.Cross(da) * denom;

		/*Gradient of third node(node c)*/
		GFPhysVector3 gradientc = da.Cross(ba) * denom;

		/*Gradient of forth node(node d)*/
		GFPhysVector3 gradientd = ba.Cross(ca) * denom;

		/*Gradient of first node(node a)*/
		GFPhysVector3 gradienta = -gradientb-gradientc-gradientd;

		/*Current Signed Volume*/
		Real currvolume = ba.Dot(gradientb);

		Real volumediff = currvolume-RestValue;

		Real sumgradlen = (wa*gradienta.Length2()+wb*gradientb.Length2()+wc*gradientc.Length2()+wd*gradientd.Length2());

		assert(sumgradlen >=0 );//=.=

		Real InvSumGradLen = (sumgradlen > GP_EPSILON ? 1.0f / sumgradlen : 0.0f);

		Real s = -volumediff * InvSumGradLen;

		Real smulstiff = s*m_Stiffness;

		GFPhysVector3 DetaA = (gradienta * wa*smulstiff);
		GFPhysVector3 DetaB = (gradientb * wb*smulstiff);
		GFPhysVector3 DetaC = (gradientc * wc*smulstiff);
		GFPhysVector3 DetaD = (gradientd * wd*smulstiff);

		//if((NodeA->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
			NodeA->m_CurrPosition += DetaA;

		//if((NodeB->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
			NodeB->m_CurrPosition += DetaB;

		//if((NodeC->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
			NodeC->m_CurrPosition += DetaC;

		//if((NodeD->m_StateFlag & GPSESF_COLLIDRIGID) == 0)
			NodeD->m_CurrPosition += DetaD;
	}
}

SoftBodyFaceNodeDistConstraint::SoftBodyFaceNodeDistConstraint()
{

}
//=======================================================================================================
void SoftBodyFaceNodeDistConstraint::Initalize(GFPhysSoftBodyFace * face , GFPhysSoftBodyNode * node , Real weight[3])
{
	m_Face = face;
	m_Node = node;

	m_FaceNodes[0] = face->m_Nodes[0];
	m_FaceNodes[1] = face->m_Nodes[1];
	m_FaceNodes[2] = face->m_Nodes[2];

	m_FaceNodesMass[0] = m_FaceNodes[0]->m_InvM;
	m_FaceNodesMass[1] = m_FaceNodes[1]->m_InvM;
	m_FaceNodesMass[2] = m_FaceNodes[2]->m_InvM;

	m_NodeMass = m_Node->m_InvM;

	m_weight[0] = weight[0];
	m_weight[1] = weight[1];
	m_weight[2] = weight[2];

	GFPhysVector3 p0 = m_FaceNodes[0]->m_CurrPosition*m_weight[0]+
		m_FaceNodes[1]->m_CurrPosition*m_weight[1]+
		m_FaceNodes[2]->m_CurrPosition*m_weight[2];

	GFPhysVector3 p1 = m_Node->m_CurrPosition;

	m_RestLength = (p0-p1).Length();

}
//===========================================================================================================
void SoftBodyFaceNodeDistConstraint::SetRestLength(Real restlen)
{
	m_RestLength = restlen;
}
//===========================================================================================================
void SoftBodyFaceNodeDistConstraint::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	//cache data to use in solve constraint
	m_FaceNodes[0] = m_Face->m_Nodes[0];
	m_FaceNodes[1] = m_Face->m_Nodes[1];
	m_FaceNodes[2] = m_Face->m_Nodes[2];

	m_FaceNodesMass[0] = m_FaceNodes[0]->m_InvM;
	m_FaceNodesMass[1] = m_FaceNodes[1]->m_InvM;
	m_FaceNodesMass[2] = m_FaceNodes[2]->m_InvM;

	m_NodeMass = m_Node->m_InvM;
}
//===========================================================================================================
void SoftBodyFaceNodeDistConstraint::SolveConstraint(Real Stiffniss , Real TimeStep)
{
	GFPhysVector3 p0 = m_FaceNodes[0]->m_CurrPosition*m_weight[0] 
	+m_FaceNodes[1]->m_CurrPosition*m_weight[1]
	+m_FaceNodes[2]->m_CurrPosition*m_weight[2];

	GFPhysVector3 p1 =  m_Node->m_CurrPosition;

	GFPhysVector3 Diff = (p0-p1);

	Real Length = Diff.Length();

	Real diffLen = Length-m_RestLength;

	Stiffniss = Stiffniss*m_Stiffness;

	if(Length > FLT_EPSILON)
	{	
		Diff  /= Length;//diff is normalized now 

		GFPhysVector3 grad00 = Diff*m_weight[0];
		GFPhysVector3 grad01 = Diff*m_weight[1];
		GFPhysVector3 grad02 = Diff*m_weight[2];

		GFPhysVector3 gradNoda = -Diff;

		//weighted by invmass 's sum of gradient
		Real sumgrad = m_weight[0]*m_weight[0]*m_FaceNodesMass[0]
		+m_weight[1]*m_weight[1]*m_FaceNodesMass[1]
		+m_weight[2]*m_weight[2]*m_FaceNodesMass[2]
		+m_NodeMass;

		Real scale = Stiffniss*(-diffLen) / sumgrad;


		GFPhysVector3 delta00 = scale*grad00*m_FaceNodesMass[0];
		GFPhysVector3 delta01 = scale*grad01*m_FaceNodesMass[1];
		GFPhysVector3 delta02 = scale*grad02*m_FaceNodesMass[2];

		GFPhysVector3 deltaNode = scale*gradNoda*m_NodeMass;

		m_FaceNodes[0]->m_CurrPosition += delta00;
		m_FaceNodes[1]->m_CurrPosition += delta01;
		m_FaceNodes[2]->m_CurrPosition += delta02;

		m_Node->m_CurrPosition += deltaNode;
	}
}
