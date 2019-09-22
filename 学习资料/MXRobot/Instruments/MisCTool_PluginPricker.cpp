
#include "MisCTool_PluginPricker.h"
#include "Instruments/Tool.h"
//#include "../NewTrain/NewTraining.h"

MisCTool_PluginPricker_FixConstraint::MisCTool_PluginPricker_FixConstraint(MisMedicOrgan_Ordinary* organ, int faceIdx, float* weights_, GFPhysTransform* transform_, const float* coord0) {
	int nodeUIDs[3];
	float p[9];


	GFPhysSoftBodyFace* pFace = organ->m_physbody->GetFaceByUID(faceIdx);

	pNode1 = organ->m_physbody->GetNode(pFace->m_Nodes[0]->m_uid);
	pNode2 = organ->m_physbody->GetNode(pFace->m_Nodes[1]->m_uid);
	pNode3 = organ->m_physbody->GetNode(pFace->m_Nodes[2]->m_uid);
	cw_copy3(weights_, weights);

	cw_copy3(coord0, coord);
	cw_copy3(coord, coordOrig);

	cw_vec3(0.0, 0.0, 0.0, f);

	transformOrig = new GFPhysTransform(*transform_);

	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}

void MisCTool_PluginPricker_FixConstraint::SetCoord(const float* coord_) {
	cw_copy3(coord_, coord);
}

void MisCTool_PluginPricker_FixConstraint::SetCoord(GFPhysTransform* newTransform) {
	GFPhysVector3 newpos = (*newTransform) * (transformOrig->Inverse())*GFPhysVector3(coordOrig[0], coordOrig[1], coordOrig[2]);
	cw_vec3(newpos.GetX(), newpos.GetY(), newpos.GetZ(), coord);
}


void MisCTool_PluginPricker_FixConstraint::PrepareSolveConstraint(Real Stiffness, Real TimeStep){
	cw_vec3(0.0, 0.0, 0.0, f);
}

void MisCTool_PluginPricker_FixConstraint::SolveConstraint(Real Stiffniss, Real TimeStep){
	float Cp, coordCurrent[3], coordDiff[3], p1[3], p2[3], p3[3], s, dp1[3], dp2[3], dp3[3];

	if (!(pNode1&&pNode2&&pNode3)) return;

	cw_vec3(pNode1->m_CurrPosition.GetX(), pNode1->m_CurrPosition.GetY(), pNode1->m_CurrPosition.GetZ(), p1);
	cw_vec3(pNode2->m_CurrPosition.GetX(), pNode2->m_CurrPosition.GetY(), pNode2->m_CurrPosition.GetZ(), p2);
	cw_vec3(pNode3->m_CurrPosition.GetX(), pNode3->m_CurrPosition.GetY(), pNode3->m_CurrPosition.GetZ(), p3);

	coordCurrent[0] = weights[0] * p1[0] + weights[1] * p2[0] + weights[2] * p3[0];
	coordCurrent[1] = weights[0] * p1[1] + weights[1] * p2[1] + weights[2] * p3[1];
	coordCurrent[2] = weights[0] * p1[2] + weights[1] * p2[2] + weights[2] * p3[2];

	cw_sub3(coordCurrent, coord, coordDiff);
	Cp = sqrt(cw_dotprod3(coordDiff, coordDiff));

	s = 0.0;
	dp1[0] = 1.0 / Cp*(coordDiff[0])*weights[0]; s += pNode1->m_InvM*dp1[0] * dp1[0]; dp1[0] *= pNode1->m_InvM;
	dp1[1] = 1.0 / Cp*(coordDiff[1])*weights[0]; s += pNode1->m_InvM*dp1[1] * dp1[1]; dp1[1] *= pNode1->m_InvM;
	dp1[2] = 1.0 / Cp*(coordDiff[2])*weights[0]; s += pNode1->m_InvM*dp1[2] * dp1[2]; dp1[2] *= pNode1->m_InvM;
	dp2[0] = 1.0 / Cp*(coordDiff[0])*weights[1]; s += pNode2->m_InvM*dp2[0] * dp2[0]; dp2[0] *= pNode2->m_InvM;
	dp2[1] = 1.0 / Cp*(coordDiff[1])*weights[1]; s += pNode2->m_InvM*dp2[1] * dp2[1]; dp2[1] *= pNode2->m_InvM;
	dp2[2] = 1.0 / Cp*(coordDiff[2])*weights[1]; s += pNode2->m_InvM*dp2[2] * dp2[2]; dp2[2] *= pNode2->m_InvM;
	dp3[0] = 1.0 / Cp*(coordDiff[0])*weights[2]; s += pNode3->m_InvM*dp3[0] * dp3[0]; dp3[0] *= pNode3->m_InvM;
	dp3[1] = 1.0 / Cp*(coordDiff[1])*weights[2]; s += pNode3->m_InvM*dp3[1] * dp3[1]; dp3[1] *= pNode3->m_InvM;
	dp3[2] = 1.0 / Cp*(coordDiff[2])*weights[2]; s += pNode3->m_InvM*dp3[2] * dp3[2]; dp3[2] *= pNode3->m_InvM;

	s = Cp / s;
	dp1[0] *= -s; dp1[1] *= -s; dp1[2] *= -s;
	dp2[0] *= -s; dp2[1] *= -s; dp2[2] *= -s;
	dp3[0] *= -s; dp3[1] *= -s; dp3[2] *= -s;


	//pNode1->m_CurrPosition = GFPhysVector3(coord[0], coord[1], coord[2]); 
	pNode1->m_CurrPosition = GFPhysVector3(p1[0] + dp1[0], p1[1] + dp1[1], p1[2] + dp1[2]);
	pNode2->m_CurrPosition = GFPhysVector3(p2[0] + dp2[0], p2[1] + dp2[1], p2[2] + dp2[2]);
	pNode3->m_CurrPosition = GFPhysVector3(p3[0] + dp3[0], p3[1] + dp3[1], p3[2] + dp3[2]);

	cw_add3(f, coordDiff, f);

};

MisCTool_PluginPricker_FixConstraint::~MisCTool_PluginPricker_FixConstraint(){
	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}

MisCTool_PluginPricker::MisCTool_PluginPricker(CTool * tool, float resistOfPrick) : MisMedicCToolPluginInterface(tool) {
	this->m_tool = tool;
	this->m_constraint = NULL;
	this->m_part[0]=0;
	this->m_resistOfPrick = resistOfPrick;
	this->pHitOrgan = NULL;

	if (this->m_tool->GetOwnerTraining()->m_pToolsMgr->GetLeftTool() == tool) strcpy(this->m_side, "left");
	else if (this->m_tool->GetOwnerTraining()->m_pToolsMgr->GetLeftTool() == tool) strcpy(this->m_side, "right");
}

MisCTool_PluginPricker::~MisCTool_PluginPricker(){
	if (this->m_constraint) {
		delete this->m_constraint;
		this->m_constraint = NULL;
	}
}

void MisCTool_PluginPricker::setFaceEx(MisMedicOrgan_Ordinary* pOrgan, int faceUID, int collisionEnabled) {
	GFPhysSoftBodyFace* pFace = pOrgan->m_physbody->GetFaceByUID(faceUID);
	if (!pFace) return ;
	if (collisionEnabled) {
		pFace->m_RSCollisionMask |= (!strcmp(m_side, "left"))?MMRC_LeftTool:MMRC_RightTool;
	}
	else {
		pFace->m_RSCollisionMask &= ~((!strcmp(m_side, "left")) ? MMRC_LeftTool : MMRC_RightTool);
	}
}

int MisCTool_PluginPricker::getFaceEx(MisMedicOrgan_Ordinary* organ, int faceUID, float* pCoord) {
	GFPhysSoftBodyFace* pFace = organ->m_physbody->GetFaceByUID(faceUID);
	if (!pFace) return false;

	cw_vec3(pFace->m_Nodes[0]->m_CurrPosition.GetX(), pFace->m_Nodes[0]->m_CurrPosition.GetY(), pFace->m_Nodes[0]->m_CurrPosition.GetZ(), pCoord);
	cw_vec3(pFace->m_Nodes[1]->m_CurrPosition.GetX(), pFace->m_Nodes[1]->m_CurrPosition.GetY(), pFace->m_Nodes[1]->m_CurrPosition.GetZ(), pCoord + 3);
	cw_vec3(pFace->m_Nodes[2]->m_CurrPosition.GetX(), pFace->m_Nodes[2]->m_CurrPosition.GetY(), pFace->m_Nodes[2]->m_CurrPosition.GetZ(), pCoord + 6);
	
	return true;
}


void MisCTool_PluginPricker::tryReset() {
	if (m_constraint) {
		delete this->m_constraint;
		this->m_constraint = NULL;
		for (int j = 0; j < this->disabledFaces.size(); j++) {
			setFaceEx(this->disabledOrgans.at(j), this->disabledFaces.at(j), true);
		}
		this->disabledFaces.clear();
		this->disabledOrgans.clear();
	}
}


void MisCTool_PluginPricker::PhysicsSimulationEnd(int currStep, int TotalStep, float dt) {
	int i;
	float fval;
	GFPhysSoftBodyFace* pFace;

	// force feesback: accumulated from constraint
	if (this->m_constraint) {
		cw_copy3(this->m_constraint->f, this->m_tool->m_currentExternalForce);
		cw_scale3(this->m_tool->m_currentExternalForce, 15.0, this->m_tool->m_currentExternalForce);
	}

	// Ô¼Êø½â³ý
	if (this->m_constraint)
	{
		for (i = 0; i < 1; i++) {
			fval = cw_dotprod3(this->m_constraint->f, this->forceDir);
			if (fval > 0.0) break; // opposite direction between current force and original force is required.
			fval = sqrt(cw_dotprod3(this->m_constraint->f, this->m_constraint->f));
			if (fval < 0.75) break;

			printf("Constraint Delete: %f\n", fval);
			tryReset();
		}
	}

	std::vector<GFPhysSoftBodyFace*> ResultFaces;
	char status[1000];
	int faceID;
	float faceWeight[3], force[3], p[9], vtx[3], p1[3], p2[3], p3[3];
	GFPhysTransform transforms[3];
	GFPhysVector3 pointCoord;
	MisMedicOrgan_Ordinary* organ;

	//CTool_GetStatus(this->m_tool, status, organName, &faceID, faceWeight, force, transforms);
	organ = NULL;
	getStatusEx(status, &organ, &faceID, faceWeight, force, transforms);

	// Ô¼Êø
	// calculate the project force alongside the blade
	if (strstr(status, "collideLeft") )
	{
		strcpy(m_part, "left");
		pointCoord = transforms[0] * GFPhysVector3(0.0f, 1.0f, 0.0f);
		fval = cw_dotprod3(pointCoord, force);
	}
	else if (strstr(status, "collideRight"))
	{
		strcpy(m_part, "right");
		pointCoord = transforms[1] * GFPhysVector3(0.0f, -1.0f, 0.0f);
		fval = cw_dotprod3(pointCoord, force);
	}

	if (strstr(status, "collide") && -fval > this->m_resistOfPrick)
	{
		if (!this->getFaceEx(organ, faceID, p)) {
			tryReset();
			return;
		}

		if (this->m_constraint != NULL) {
			delete this->m_constraint;
			this->m_constraint = NULL;
		}

		vtx[0] = faceWeight[0] * p[0] + faceWeight[1] * p[3] + faceWeight[2] * p[6];
		vtx[1] = faceWeight[0] * p[1] + faceWeight[1] * p[4] + faceWeight[2] * p[7];
		vtx[2] = faceWeight[0] * p[2] + faceWeight[1] * p[5] + faceWeight[2] * p[8];

		cw_copy3(force, this->forceDir);
		cw_scale3(this->forceDir, 1.0 / sqrt(cw_dotprod3(force, force)), this->forceDir);
		vtx[0] += 0.4*this->forceDir[0];
		vtx[1] += 0.4*this->forceDir[1];
		vtx[2] += 0.4*this->forceDir[2];

		if (!strcmp(m_part, "left"))
			pointCoord = transforms[0] * GFPhysVector3(0.0f, -0.015f, 0.7f);
		else
			pointCoord = transforms[1] * GFPhysVector3(-0.02f, 0.035f, 0.7f);

		vtx[0] = pointCoord.GetX();
		vtx[1] = pointCoord.GetY();
		vtx[2] = pointCoord.GetZ();

		this->m_constraint = new MisCTool_PluginPricker_FixConstraint(organ, faceID, faceWeight, transforms + (strcmp(m_part, "left")?1:0), vtx);

		printf("Constraint Create %s\n", m_part);

		pHitOrgan=organ;
		hitFaceID=faceID;
		vtx[0] = faceWeight[0] * p[0] + faceWeight[1] * p[3] + faceWeight[2] * p[6];
		vtx[1] = faceWeight[0] * p[1] + faceWeight[1] * p[4] + faceWeight[2] * p[7];
		vtx[2] = faceWeight[0] * p[2] + faceWeight[1] * p[5] + faceWeight[2] * p[8];
		cw_copy3(vtx, hitPoint);
	}
	if (this->m_constraint) {
		//fixConstraint->SetCoord(transforms);
		if (!strcmp(m_part, "left"))
		{
			cw_vec3(0.0f, 0.0f, 1.1f, p1);
			cw_vec3(0.0f, -0.06f, -0.5f, p2);

		}
		else
		{
			cw_vec3(-0.02f, 0.02f, 1.1f, p1);
			cw_vec3(-0.02f, 0.08f, -0.5f, p2);
		}
		for (i = 0; i < 3; i++) p3[i] = p1[i] * 0.75 + p2[i]*0.25;
		pointCoord = transforms[strcmp(m_part, "left")?1:0] * GFPhysVector3(p3[0], p3[1], p3[2]);

		vtx[0] = pointCoord.GetX();
		vtx[1] = pointCoord.GetY();
		vtx[2] = pointCoord.GetZ();
		this->m_constraint->SetCoord(vtx);
	}


	// ´Á¶´
	//CTool_GetStatus(this->m_tool, status, organName, &faceID, faceWeight, force, NULL);
	

	if (strstr(status, "collide") && fval < -3.0)
	{
		this->getFaceEx(organ, faceID, p);
		vtx[0] = faceWeight[0] * p[0] + faceWeight[1] * p[3] + faceWeight[2] * p[6];
		vtx[1] = faceWeight[0] * p[1] + faceWeight[1] * p[4] + faceWeight[2] * p[7];
		vtx[2] = faceWeight[0] * p[2] + faceWeight[1] * p[5] + faceWeight[2] * p[8];
		organ->SelectPhysFaceAroundPoint(ResultFaces, GFPhysVector3(vtx[0], vtx[1], vtx[2]), 0.1, false);
		for (int i = 0; i < ResultFaces.size(); i++) {
			setFaceEx(organ, ResultFaces.at(i)->m_uid, false);
			this->disabledFaces.push_back(ResultFaces.at(i)->m_uid);
			this->disabledOrgans.push_back(organ);
		}
		setFaceEx(organ, faceID, false);
		this->disabledFaces.push_back(faceID);
		this->disabledOrgans.push_back(organ);
	}
	
}

void MisCTool_PluginPricker::OneFrameUpdateEnded() {
	float p[9];
	int i;

	/*
	if (pHitOrgan) {
		this->getFaceEx(pHitOrgan, hitFaceID, p);
		MisNewTraining_drawSingleFace("aaa", p, p + 3, p + 6, 0.1);
	}
	else {
		for (i = 0; i < 9; i++) p[i] = 0.0;
		MisNewTraining_drawSingleFace("aaa", p, p + 3, p + 6, 0.001);
	}
	*/

	/*
	p[0] = 0.0; p[1] = 0.0; p[2] = 0.0;
	for (int i = 0; i < 20; i++) MisNewTraining_drawSingleFace("aaa"+i, p, p , p, 0.001);
	for (int i = 0; i < disabledFaces.size(); i++) {
		getFaceEx(disabledOrgans.at(i), disabledFaces.at(i), p);
		MisNewTraining_drawSingleFace("aaa"+i, p, p + 3, p + 6, 0.1);
	}*/

	//MisNewTraining_drawSingleDir("dir", hitPoint, forceDir);
}

void MisCTool_PluginPricker::getStatusEx(char* pStatus, MisMedicOrgan_Ordinary** ppOrgan, int* pFaceID, float* pFaceWeight, float* force, GFPhysTransform* transforms) {
	const GFPhysCollideObject * pCollideObject=NULL;
	Ogre::Vector3 contactForce, dragForce;
	const GFPhysSoftBodyFace * pDistFace;
	char status[100], organName[100];
	float faceWeight[3], projDst, projCurrent, norm[3];
	int materialid, faceID;
	MisMedicOrgan_Ordinary* pOrgan;
	int i, iDst;

	pOrgan = NULL;
	force[0] = 0.0; force[1] = 0.0; force[2] = 0.0;


	if(m_tool->m_ToolColliedFaces.size() > 0) {
		m_tool->GetForceFeedBack(contactForce, dragForce);
		force[0] = contactForce.x; force[1] = contactForce.y; force[2] = contactForce.z;

		
		projDst = -1e100;
		for (i = 0; i < m_tool->m_ToolColliedFaces.size(); i++) {
			pDistFace = m_tool->m_ToolColliedFaces.at(i).m_collideFace;
			norm[0] = pDistFace->m_FaceNormal.GetX();
			norm[1] = pDistFace->m_FaceNormal.GetY();
			norm[2] = pDistFace->m_FaceNormal.GetZ();
			projCurrent = cw_dotprod3(norm, force);
			//printf("FFF %d %f\n", projCurrent);
			if (projCurrent > projDst) {
				projDst = projCurrent;
				iDst = i;
			}
		}

		pCollideObject = m_tool->m_ToolColliedFaces.at(iDst).m_collideSoft;

		if (m_tool->m_ToolColliedFaces.at(iDst).m_collideRigid == m_tool->m_lefttoolpartconvex.m_rigidbody) strcat(status, "collideLeft|");
		else if (m_tool->m_ToolColliedFaces.at(iDst).m_collideRigid == m_tool->m_righttoolpartconvex.m_rigidbody) strcat(status, "collideRight|");
		faceWeight[0] = m_tool->m_ToolColliedFaces.at(iDst).m_collideWeights[0];
		faceWeight[1] = m_tool->m_ToolColliedFaces.at(iDst).m_collideWeights[1];
		faceWeight[2] = m_tool->m_ToolColliedFaces.at(iDst).m_collideWeights[2];
		pDistFace = m_tool->m_ToolColliedFaces.at(iDst).m_collideFace;

	}
	transforms[0] = m_tool->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform();
	transforms[1] = m_tool->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform();
	
	if (pCollideObject && pCollideObject->GetUserPointer()) {
		strcat(status, "collide|");
		pOrgan = (MisMedicOrgan_Ordinary*)pCollideObject->GetUserPointer();
		strcpy(organName, pOrgan->getName().c_str());
		pOrgan->ExtractFaceIdAndMaterialIdFromUsrData(const_cast<GFPhysSoftBodyFace*>(pDistFace), materialid, faceID);
	}

	strcpy(pStatus, status);
	*pFaceID = faceID;
	memcpy(pFaceWeight, faceWeight, 3 * sizeof(float));
	if (ppOrgan) *ppOrgan = pOrgan;
	

	
}


