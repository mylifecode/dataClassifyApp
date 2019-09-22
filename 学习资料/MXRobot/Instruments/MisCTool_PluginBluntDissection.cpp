
#include "MisCTool_PluginBluntDissection.h"
#include "Tool.h"

//#include "NewTraining.h"

MisCTool_PluginBluntDissection::MisCTool_PluginBluntDissection(CTool * tool, float forceThresholdOfPierce_, float forceThresholdOfEnlarge_) :MisMedicCToolPluginInterface(tool) {
	m_tool = tool;
	forceThresholdOfPierce = forceThresholdOfPierce_;
	forceThresholdOfEnlarge = forceThresholdOfEnlarge_;

}

MisCTool_PluginBluntDissection::~MisCTool_PluginBluntDissection() {
	timeSinceLastCut = 0.0;
}

void MisCTool_PluginBluntDissection::PhysicsSimulationEnd(int currStep, int TotalStep, float dt) {
	timeSinceLastCut += dt;
}

void MisCTool_PluginBluntDissection::onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{

	float angle;
	
	if (timeSinceLastCut < 0.5f) return;

	angle = m_tool->GetShaftAside();
	if (angle >= 12.0) distroyByEnlarge();
	else if (angle <= 0.1) distroyByPierce();

   


}

void MisCTool_PluginBluntDissection::distroyByEnlarge() {
	MisMedicOrgan_Ordinary* pOrgan;
	float refVal[3];
	int candidate;
	float forceVal;
	GFPhysVector3 refPoint1;
	GFPhysTransform transform;
	GFPhysVector3 dir, transformedRefPoint;
	int i;
	ToolCollidedFace cf;
	GFPhysSoftBodyFace* pFace;
	GFPhysVector3 vec;

	// left
	transform = m_tool->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform();
	dir = (transform.GetBasis()*GFPhysVector3(0.0, 1.0, 0.0)).Normalize();
	candidate = -1;
	refPoint1 = m_tool->m_CutBladeLeft.m_LinPoints[1];
	for (i = 0; i < m_tool->m_ToolColliedFaces.size(); i++) {
		cf = m_tool->m_ToolColliedFaces.at(i);
		if (cf.m_collideRigid != m_tool->m_lefttoolpartconvex.m_rigidbody) continue;
		if (!((MisMedicOrgan_Ordinary*)cf.m_collideSoft->GetUserPointer())->GetCreateInfo().m_bCanBluntDissection) continue;
		forceVal = (cf.m_FaceContactImpluse[0] + cf.m_FaceContactImpluse[1] + cf.m_FaceContactImpluse[2])*dir.Dot(cf.m_collideFace->m_FaceNormal);
		if (forceVal < 0.1) continue;
		if (candidate == -1) candidate = i;
		else if (Distance(cf.m_localpointInRigid, refPoint1) < Distance(m_tool->m_ToolColliedFaces.at(candidate).m_localpointInRigid, refPoint1)) candidate = i;
	}
	if (candidate >= 0)
	{
		cf = m_tool->m_ToolColliedFaces.at(candidate);
		pOrgan = (MisMedicOrgan_Ordinary*)cf.m_collideSoft->GetUserPointer();
		pFace = cf.m_collideFace;

		transformedRefPoint = m_tool->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*cf.m_localpointInRigid;

		refVal[0] = fabs(dir.Dot(pFace->m_Nodes[0]->m_CurrPosition - transformedRefPoint));
		refVal[1] = fabs(dir.Dot(pFace->m_Nodes[1]->m_CurrPosition - transformedRefPoint));
		refVal[2] = fabs(dir.Dot(pFace->m_Nodes[2]->m_CurrPosition - transformedRefPoint));

		if (refVal[0] < refVal[1] && refVal[0] < refVal[2]) candidate = 0;
		else if (refVal[1] < refVal[2]) candidate = 1;
		else candidate = 2;

		GFPhysVector3 ptOnFace = ClosestPtPointTriangle(transformedRefPoint,
			pFace->m_Nodes[0]->m_CurrPosition,
			pFace->m_Nodes[1]->m_CurrPosition,
			pFace->m_Nodes[2]->m_CurrPosition);

		float weights[3];
		CalcBaryCentric(pFace->m_Nodes[0]->m_CurrPosition,
			pFace->m_Nodes[1]->m_CurrPosition,
			pFace->m_Nodes[2]->m_CurrPosition,
			ptOnFace, weights[0], weights[1], weights[2]);

		pOrgan->DestroyTissueAroundNode(pFace , weights, false);// (pFace->m_Nodes[candidate]->m_CurrPosition, true);

		timeSinceLastCut = 0.0f;

	}



	transform = m_tool->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform();
	dir = (transform.GetBasis()*GFPhysVector3(0.0, -1.0, 0.0)).Normalize();
	//dir = (transform*GFPhysVector3(0.0, -1.0, 0.0) - transform.GetOrigin()).Normalize();

	candidate = -1;
	refPoint1 = m_tool->m_CutBladeRight.m_LinPoints[1];
	for (i = 0; i < m_tool->m_ToolColliedFaces.size(); i++) {
		cf = m_tool->m_ToolColliedFaces.at(i);
		if (cf.m_collideRigid != m_tool->m_righttoolpartconvex.m_rigidbody) continue;
		if (!((MisMedicOrgan_Ordinary*)cf.m_collideSoft->GetUserPointer())->GetCreateInfo().m_bCanBluntDissection) continue;
		forceVal = (cf.m_FaceContactImpluse[0] + cf.m_FaceContactImpluse[1] + cf.m_FaceContactImpluse[2])*dir.Dot(cf.m_collideFace->m_FaceNormal);
		if (forceVal < forceThresholdOfEnlarge) continue;
		if (candidate == -1) candidate = i;
		else if (Distance(cf.m_localpointInRigid, refPoint1) < Distance(m_tool->m_ToolColliedFaces.at(candidate).m_localpointInRigid, refPoint1)) candidate = i;
	}

	if (candidate != -1)
	{
		cf = m_tool->m_ToolColliedFaces.at(candidate);
		pOrgan = (MisMedicOrgan_Ordinary*)cf.m_collideSoft->GetUserPointer();
		pFace = cf.m_collideFace;

		transformedRefPoint = m_tool->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*cf.m_localpointInRigid;

		refVal[0] = fabs(dir.Dot(pFace->m_Nodes[0]->m_CurrPosition - transformedRefPoint));
		refVal[1] = fabs(dir.Dot(pFace->m_Nodes[1]->m_CurrPosition - transformedRefPoint));
		refVal[2] = fabs(dir.Dot(pFace->m_Nodes[2]->m_CurrPosition - transformedRefPoint));

		if (refVal[0] < refVal[1] && refVal[0] < refVal[2]) candidate = 0;
		else if (refVal[1] < refVal[2]) candidate = 1;
		else candidate = 2;

		GFPhysVector3 ptOnFace = ClosestPtPointTriangle(transformedRefPoint,
			pFace->m_Nodes[0]->m_CurrPosition,
			pFace->m_Nodes[1]->m_CurrPosition,
			pFace->m_Nodes[2]->m_CurrPosition);

		float weights[3];
		CalcBaryCentric(pFace->m_Nodes[0]->m_CurrPosition,
			pFace->m_Nodes[1]->m_CurrPosition,
			pFace->m_Nodes[2]->m_CurrPosition,
			ptOnFace, weights[0], weights[1], weights[2]);

		pOrgan->DestroyTissueAroundNode(pFace ,weights , false);// (pFace->m_Nodes[candidate]->m_CurrPosition, true);

		timeSinceLastCut = 0.0f;
	}

}

struct PierceFace
{
	float m_MaxPierceForce;
	GFPhysSoftBodyFace * m_PiercceFace;
	float m_weights[3];
	float m_DistToToolPiercePoint;
};
void MisCTool_PluginBluntDissection::distroyByPierce() {
	GFPhysTransform transformLeft, transformRight;
	GFPhysVector3 gpRef, dir, gp1, gp2, gp3, forceVal;// forceValSum[20]
	PierceFace pierceFace[20];//this is fix size danger fix me!!
	float p1[3];
	int i, j, organ_cx;
	ToolCollidedFace cf;
	GFPhysSoftBodyFace* pFace;
	MisMedicOrgan_Ordinary *pOrgan[10], *pOrganTemp;

	transformLeft = m_tool->m_lefttoolpartconvex.m_rigidbody->GetWorldTransform();
	transformRight = m_tool->m_righttoolpartconvex.m_rigidbody->GetWorldTransform();
	gpRef = transformLeft*GFPhysVector3(-0.35, -0.02, 2.09);

	//m_tool->m_lefttoolpartconvex.
	
	dir = (transformLeft.GetBasis()*GFPhysVector3(0.0, 0.0, -1.0)).Normalized();
	
	organ_cx = 0;
	//printf("MMM %d\n", m_tool->m_ToolColliedFaces.size());
	for (i = 0; i < m_tool->m_ToolColliedFaces.size(); i++) {
		cf = m_tool->m_ToolColliedFaces.at(i);
		if (!((MisMedicOrgan_Ordinary*)cf.m_collideSoft->GetUserPointer())->GetCreateInfo().m_bCanBluntDissection) continue;
		// get points of face
		
		GFPhysVector3 collidePtFace = cf.m_collideFace->m_Nodes[0]->m_CurrPosition*cf.m_collideWeights[0]
			                        + cf.m_collideFace->m_Nodes[1]->m_CurrPosition*cf.m_collideWeights[1]
			                        + cf.m_collideFace->m_Nodes[2]->m_CurrPosition*cf.m_collideWeights[2];
		
		float distToToolPiercePoint = (collidePtFace - gpRef).Dot(-dir);

		//float distToToolPiercePoint = collidePtFace.Distance(gpRef);
		if (distToToolPiercePoint < -0.1)
		    continue;

		pOrganTemp = (MisMedicOrgan_Ordinary*)cf.m_collideSoft->GetUserPointer();
		for (j = 0; j < organ_cx; j++) {
			if (pOrgan[j] == pOrganTemp) break;
		}
		if (j == organ_cx) {
			pierceFace[organ_cx].m_MaxPierceForce = 0.0f;// GFPhysVector3(0.0, 0.0, 0.0);
			pierceFace[organ_cx].m_DistToToolPiercePoint = -FLT_MAX;
			pierceFace[organ_cx].m_PiercceFace = 0;
			pOrgan[organ_cx++] = pOrganTemp;
		}
		forceVal = (cf.m_FaceContactImpluse[0] + cf.m_FaceContactImpluse[1] + cf.m_FaceContactImpluse[2])*cf.m_CollideNormal;// m_collideFace->m_FaceNormal;
		if (forceThresholdOfPierce < forceVal.Dot(dir))//valid face
		{
			
			if (distToToolPiercePoint > pierceFace[j].m_DistToToolPiercePoint)
			{ 
				
			   pierceFace[j].m_MaxPierceForce = forceVal.Dot(dir);
			   pierceFace[j].m_DistToToolPiercePoint = distToToolPiercePoint;
			   pierceFace[j].m_PiercceFace = cf.m_collideFace;
			   pierceFace[j].m_weights[0] = cf.m_collideWeights[0];
			   pierceFace[j].m_weights[1] = cf.m_collideWeights[1];
			   pierceFace[j].m_weights[2] = cf.m_collideWeights[2];
			}

		}
	}

	//printf("FFF %f\n", forceValSum.Dot(dir));
	for (i = 0; i < organ_cx; i++) {
		PierceFace & pFace = pierceFace[i];
		if (pFace.m_PiercceFace)
		{
			GFPhysVector3 pierecePoint;
			int pierceIndex;

			if (pFace.m_weights[0] > pFace.m_weights[1])
			{
				if (pFace.m_weights[0] > pFace.m_weights[2])
					pierceIndex = 0;
				else
					pierceIndex = 2;
			}
			else
			{
				if (pFace.m_weights[1] > pFace.m_weights[2])
					pierceIndex = 1;
				else
					pierceIndex = 2;
			}
			
			pOrgan[i]->DestroyTissueAroundNode(pFace.m_PiercceFace, pFace.m_weights, false);// (pFace.m_PiercceFace->m_Nodes[pierceIndex]->m_UnDeformedPos, false);
			timeSinceLastCut = 0.0f;
			//printf("OOOOOO %d\n", organ_cx);
		}
	}
		//cw_vec3(gp1.GetX(), gp1.GetY(), gp1.GetZ(), p1);
	//MisNewTraining_drawSingleSphere("aaa", p1, 0.2);
	//dir = (transform.GetBasis()*GFPhysVector3(0.0, 1.0, 0.0)).Normalize();
}


