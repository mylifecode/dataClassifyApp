#include "VolumeBlood.h"

#include "MXOgreGraphic.h"
#include "MXOgreWrapper.h"
#include "Ogre.h"
#include "stdio.h"

#include "MisNewTraining.h"

class VolumeBloodCollideSoftBodyFaceCallBack : public GFPhysNodeOverlapCallback
{
	
public:
	
	VolumeBlood* pVolumeBlood;

	VolumeBloodCollideSoftBodyFaceCallBack(VolumeBlood* pVolumeBlood_)
	{
		pVolumeBlood = pVolumeBlood_;
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{
	}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB)
	{
		int SegIndex = (int)dynNodeB->m_UserData;
		//printf("%d ", SegIndex);
		if (SegIndex < 3) return;
		if (SegIndex >= pVolumeBlood->collideSecIdx) return;
				
		
		GFPhysSoftBodyFace * pSoftFace = (GFPhysSoftBodyFace*)dynNodeA->m_UserData;
		GFPhysVector3 n0 = GFPhysVector3(pVolumeBlood->splineArr[3 * SegIndex], pVolumeBlood->splineArr[3 * SegIndex + 1], pVolumeBlood->splineArr[3*SegIndex+2]);
		GFPhysVector3 n1 = GFPhysVector3(pVolumeBlood->splineArr[3 * (SegIndex+1)], pVolumeBlood->splineArr[3 * (SegIndex+1) + 1], pVolumeBlood->splineArr[3 * (SegIndex+1) + 2]);

		n0 = GFPhysVector3(n0.GetX() * 100, n0.GetY() * 100, n0.GetZ() * 100);
		n1 = GFPhysVector3(n1.GetX() * 100, n1.GetY() * 100, n1.GetZ() * 100);

		Real rayWeight;
		GFPhysVector3 intersectpt;
		Real triangleWeight[3];

		bool isIntersect = LineIntersectTriangle(pSoftFace->m_Nodes[0]->m_CurrPosition,
			pSoftFace->m_Nodes[1]->m_CurrPosition,
			pSoftFace->m_Nodes[2]->m_CurrPosition,
			n0,
			n1,
			rayWeight,
			intersectpt, triangleWeight);

		if (!isIntersect) return;
		if (rayWeight<0.0 || rayWeight>1.0) return;
		
		pVolumeBlood->collideSecOrgan = pVolumeBlood->collideCheckOrgan;

		float tx1[2], tx2[2], tx3[2];
		tx1[0] = pSoftFace->m_TexCoordU[0];
		tx1[1] = pSoftFace->m_TexCoordV[0];
		tx2[0] = pSoftFace->m_TexCoordU[1];
		tx2[1] = pSoftFace->m_TexCoordV[1];
		tx3[0] = pSoftFace->m_TexCoordU[2];
		tx3[1] = pSoftFace->m_TexCoordV[2];
		pVolumeBlood->collideFaceTexCoord[0] = triangleWeight[0] * tx1[0] + triangleWeight[1] * tx2[0] + triangleWeight[2] * tx3[0];
		pVolumeBlood->collideFaceTexCoord[1] = triangleWeight[0] * tx1[1] + triangleWeight[1] * tx2[1] + triangleWeight[2] * tx3[1];

		pVolumeBlood->collideSecIdx = SegIndex;
	}
};


VolumeBlood::VolumeBlood(float* gravity_, int vQuality_, int hQuality_, float bloodLen_)
{
	int i;

	disturb = (float*)malloc(10000 * sizeof(float));
	for (i = 0; i < 10000; i++)
	{
		if (i % 3 == 0)
			disturb[i] = (rand() % 250) / 1000.0;
		else
			disturb[i] = 0;//
	}
	ts = 0.0;

	cw_copy3(gravity_, gravity);
	vQuality = vQuality_;
	hQuality = hQuality_;
	bloodLen = bloodLen_;

	splineArr = (float*)malloc(vQuality * 3 * sizeof(float));
	scaleArr = (float*)malloc(vQuality * sizeof(float));
	shapeArr = (float*)malloc(hQuality * 2 * sizeof(float));
	fceArr = (int*)malloc(hQuality*vQuality*2*sizeof(int) * 3);
	normArr = (float*)malloc(vQuality * hQuality* 2* 3 * sizeof(float));
	tgArr = (float*)malloc(vQuality * hQuality * 2 * 3 * sizeof(float));
	uvArr = (float*)malloc(vQuality * hQuality * 2 * 2 * sizeof(float));
	vtxArr = (float*)malloc(vQuality * hQuality * 3 *sizeof(float) * 3);
	for (i = 0; i < hQuality; i++)
	{
		shapeArr[2 * i] = 1.0f*cos(1.0*i / hQuality * 2 * cw_pi());
		shapeArr[2 * i + 1] = 1.0f*sin(1.0*i / hQuality * 2 * cw_pi());
	}

	static int id = 1;
	m_id = id++;
	Ogre::String suffix = Ogre::StringConverter::toString(m_id);
	m_manualObjectName = "bloodStream_" + suffix;
	m_materialName = "volumeBlood_" + suffix;

	Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingletonPtr()->getByName("volumeBlood");
	matPtr->clone(m_materialName);

}

VolumeBlood::~VolumeBlood()
{
	free(splineArr);
	free(scaleArr);
	free(shapeArr);
	free(fceArr);
	free(normArr);
	free(vtxArr);
	free(tgArr);
	free(uvArr);

	if (MXOgreWrapper::Get()->GetDefaultSceneManger()->hasManualObject(m_manualObjectName))
	{
		Ogre::ManualObject * objblood = MXOgreWrapper::Get()->GetDefaultSceneManger()->getManualObject(m_manualObjectName);
		
		Ogre::SceneNode * bloodscenenode = objblood->getParentSceneNode();

		objblood->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(MXOgreWrapper::Get()->GetDefaultSceneManger()->getManualObject(m_manualObjectName));
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroySceneNode(bloodscenenode);
	}

}


void VolumeBlood::step(float dt)
{
	ts += dt;
}

void VolumeBlood::setCutInfo(float cutRadius, float flow_, MisMedicOrgan_Ordinary* pOrgan_, int faceID_, float* weight_)
{	
	pOrgan = pOrgan_;
	faceID = faceID_;
	cw_copy3(weight_, faceWeight);

	area = cutRadius*cutRadius*cw_pi();
	flow = flow_;
	coneCoeff = 10.0;
	
}

int VolumeBlood::updateCutInfoByOrganFace() {
	GFPhysSoftBodyFace* pFace = pOrgan->m_physbody->GetFaceByUID(faceID);///pOrgan->m_OriginFaces[faceID].m_physface;
	GFPhysVector3 vtxCoord[3];

	if (!pFace) return 0;

	vtxCoord[0] = pFace->m_Nodes[0]->m_CurrPosition;
	vtxCoord[1] = pFace->m_Nodes[1]->m_CurrPosition;
	vtxCoord[2] = pFace->m_Nodes[2]->m_CurrPosition;

	float p12[3], p13[3], norm[3], normLen, pCenter[3];

	p12[0] = vtxCoord[1].GetX() - vtxCoord[0].GetX(); p12[1] = vtxCoord[1].GetY() - vtxCoord[0].GetY(); p12[2] = vtxCoord[1].GetZ() - vtxCoord[0].GetZ();
	p13[0] = vtxCoord[2].GetX() - vtxCoord[0].GetX(); p13[1] = vtxCoord[2].GetY() - vtxCoord[0].GetY(); p13[2] = vtxCoord[2].GetZ() - vtxCoord[0].GetZ();
	cw_cross3(p12, p13, norm);
	normLen = sqrt(cw_dotprod3(norm, norm));
	cw_scale3(norm, 1.0 / normLen, norm);
	pCenter[0] = vtxCoord[0].GetX()*faceWeight[0] + vtxCoord[1].GetX()*faceWeight[1] + vtxCoord[2].GetX()*faceWeight[2];
	pCenter[1] = vtxCoord[0].GetY()*faceWeight[0] + vtxCoord[1].GetY()*faceWeight[1] + vtxCoord[2].GetY()*faceWeight[2];
	pCenter[2] = vtxCoord[0].GetZ()*faceWeight[0] + vtxCoord[1].GetZ()*faceWeight[1] + vtxCoord[2].GetZ()*faceWeight[2];

	
	cw_copy3(norm, this->norm);
	cw_scale3(pCenter, 0.01, pos);
	normLen = sqrt(cw_dotprod3(norm, norm));
	cw_scale3(norm, 1.0 / normLen, norm);

	return 1;

}

void VolumeBlood::setCutInfo(float cutRadius, float flow_)
{
	area = cutRadius*cutRadius*cw_pi();
	flow = flow_;
}

void VolumeBlood::setCutInfo(float cutRadius, float flow_, float* pos_, float* norm_)
{
	float normLen;
	area = cutRadius*cutRadius*cw_pi();
	flow = flow_;
	coneCoeff = 10.0;
	cw_copy3(norm_, norm);
	cw_scale3(pos_, 0.01, pos);
	normLen = sqrt(cw_dotprod3(norm, norm));
	cw_scale3(norm, 1.0 / normLen, norm);
}


/*
void VolumeBlood::updateCollideTree()
{
	
	//printf("C");

}
*/

void VolumeBlood::updateCollideInfo(ITraining *pTraining)
{	
	GFPhysVector3 aabbMin, aabbMax;

	m_SegmentTree.Clear();

	float radius = 0.001;

	for (int i = 0; i < vQuality; i++)
	{	

		GFPhysVector3 aabbMin = GFPhysVector3(splineArr[3 * i] - radius, splineArr[3 * i + 1]-radius, splineArr[3 * i+2] - radius);
		GFPhysVector3 aabbMax = GFPhysVector3(splineArr[3 * i] + radius, splineArr[3 * i + 1]+radius, splineArr[3 * i + 2] - radius);

		aabbMin.SetMin(GFPhysVector3(splineArr[3 * (i+1) ] - radius, splineArr[3 * (i+1) + 1] - radius, splineArr[3 * (i+1) + 2] - radius));
		aabbMax.SetMax(GFPhysVector3(splineArr[3 * (i + 1)] + radius, splineArr[3 * (i + 1) + 1] + radius, splineArr[3 * (i + 1) + 2] + radius));

		aabbMin.SetX(aabbMin.GetX()*100.0); aabbMin.SetY(aabbMin.GetY()*100.0); aabbMin.SetZ(aabbMin.GetZ()*100.0);
		aabbMax.SetX(aabbMax.GetX()*100.0); aabbMax.SetY(aabbMax.GetY()*100.0); aabbMax.SetZ(aabbMax.GetZ()*100.0);

		GFPhysDBVNode * pTreeNode = m_SegmentTree.InsertAABBNode(aabbMin, aabbMax);
		pTreeNode->m_UserData = (void *)i;

	}

	VolumeBloodCollideSoftBodyFaceCallBack cb(this);

	collideSecIdx = vQuality;
	collideSecOrgan = NULL;

	std::vector<MisMedicOrganInterface*> pOrgans;
	pTraining->GetAllOrgan(pOrgans);
	for (size_t o = 0; o < pOrgans.size(); o++)
	{	MisMedicOrganInterface *pOrganInterface = pOrgans[o];
		if (pOrganInterface->GetCreateInfo().m_objTopologyType == DOT_VOLMESH || pOrganInterface->GetCreateInfo().m_objTopologyType == DOT_MEMBRANE)
		{	MisMedicOrgan_Ordinary *pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(pOrganInterface);
			if (pOrgan && pOrgan->m_physbody)
			{	//cb.SetCurrOrgan(pOrgan);
				GFPhysVectorObj<GFPhysDBVTree*> bvTrees = pOrgan->m_physbody->GetSoftBodyShape().GetFaceBVTrees();

				for (size_t t = 0; t < bvTrees.size(); t++)
				{
					//printf("D");
						collideCheckOrgan = pOrgan;
						GFPhysDBVTree * bvTree = bvTrees[t];
						bvTree->CollideWithDBVTree(m_SegmentTree, &cb);
				
				}
			}
		}
	}

	collideSecIdx++;

	/*
	// cpos附近全部可见!
	int refIdx=(int)((pbci->cpos[1] - pbii->projectStartY) / (pbii->projectEndY - pbii->projectStartY)*pbii->projectNum);
	for (i = refIdx; i < pbii->projectNum - 1; i++) {
		if (projects[i].sHide == 0) break;
		projects[i].sHide = 0;
	}
	for (i = refIdx-1; i > 0; i--) {
		if (projects[i].sHide == 0) break;
		projects[i].sHide = 0;
	}

	// 第一段隐藏开始全部隐藏
	refIdx = i;
	for (i = refIdx - 1; i > 0; i--) {
		if (projects[i].sHide == 1) break;
	}
	refIdx = i;
	for (i = refIdx - 1; i > 0; i--) {
		projects[i].sHide = 1;
	}

	*/

}



void VolumeBlood::setGravity(float gravity_[3]) {
	gravity[0] = gravity_[0];
	gravity[1] = gravity_[1];
	gravity[2] = gravity_[2];
}



void VolumeBlood::draw(ITraining *pTraining) {
	
	float t, flow1, v1len, v0[3], norm1[3], normX[3], normY[3], v0len, v, glen, tstep, s, f;
	int i, j, ij, sNew;

	
	cw_cross3(norm, gravity, normX);
	cw_cross3(norm, normX, normY);
	f = sqrt(cw_dotprod3(normY, normY));
	cw_scale3(normY, f, normY);
	cw_copy3(norm, norm1);
	cw_addsum3(normY, 0.003*sin(ts*20.0 * m_id), norm1);
	f = sqrt(cw_dotprod3(norm1, norm1));
	cw_scale3(norm1, f, norm1);

	v0len = flow / area;
	cw_scale3(norm1, flow / area, v0);
	v1len = coneCoeff*v0len;
	glen = sqrt(cw_dotprod3(gravity, gravity));
	s = sqrt(v0len*v0len + 2 * glen) - v0len;
	tstep = sqrt(2 * bloodLen / glen) / s / vQuality;
	for (i = 0; i < vQuality; i++)
	{
		t = tstep*i*s;
		splineArr[3 * i] = pos[0]+v0[0] * t + gravity[0] * t*t;
		splineArr[3 * i + 1] = pos[1] + v0[1] * t + gravity[1] * t*t;
		splineArr[3 * i + 2] = pos[2]+ v0[2] * t + gravity[2] * t*t;
		v = sqrt(v0len*v0len + (glen*t)*(glen*t)) + (v1len - v0len)*exp(-t*150.0);
		flow1 = flow + flow*0.5*disturb[(((int)(-ts*100.0 + t*500.0)) % 10000 + 10000) % 10000];
		scaleArr[i] = sqrt(flow1 / v / cw_pi());
	}

	int nFce, vtxCount;
	float splineVector[3], vecY[3], refVector[3], vecX[3], d, coord2d[2];

	vtxCount = hQuality*vQuality;
	for (i = 0; i < vQuality; i++)
	{
		if (i == 0)  
		{	cw_sub3(splineArr + 3, splineArr, splineVector); 
		}
		else if (i == vQuality - 1)
		{
			cw_sub3(splineArr + 3 * (vQuality - 1), splineArr + 3 * (vQuality - 2), splineVector);
		}
		else
		{
			cw_sub3(splineArr + 3 * (i + 1), splineArr + 3 * (i - 1), splineVector);
		}

		if (i == 0) 
		{
			cw_vec3(splineArr[0] - 0.5, splineArr[1] - 0.5, splineArr[2] - 0.5, refVector);
		}
		else
		{
			cw_copy3(vecY, refVector);
		}

		cw_cross3(refVector, splineVector, vecX);
		d = sqrt(cw_dotprod3(vecX, vecX));
		cw_scale3(vecX, 1.0/d, vecX);

		cw_cross3(splineVector, vecX, vecY);
		d = sqrt(cw_dotprod3(vecY, vecY));
		cw_scale3(vecY, 1.0 / d, vecY);
		
		for (j = 0; j < hQuality; j++)
		{
			coord2d[0] = shapeArr[j * 2];
			coord2d[1] = shapeArr[j * 2 + 1];
			coord2d[0] *= scaleArr[i];
			coord2d[1] *= scaleArr[i];

			cw_copy3(splineArr + 3 * i, vtxArr + (i*hQuality + j) * 3);
			cw_addsum3(vecX, coord2d[0], vtxArr + (i*hQuality + j) * 3);
			cw_addsum3(vecY, coord2d[1], vtxArr + (i*hQuality + j) * 3);
			
			ij = i*hQuality + j;

			cw_sub3(vtxArr + ij * 3, splineArr + 3*i, normArr+ij*3);
			d = sqrt(cw_dotprod3(normArr + ij * 3, normArr + ij * 3));
			cw_scale3(normArr + ij * 3, 1.0 / d, normArr + ij * 3);

			cw_cross3(vecY, normArr + ij * 3, tgArr+ij*3);
			d = sqrt(cw_dotprod3(tgArr, tgArr));
			cw_scale3(tgArr, 1.0 / d, tgArr);

			uvArr[ij * 2] = 1.0* j / hQuality;
			uvArr[ij * 2+1] = 1.0* i / vQuality;

			if (i == 0) {
				cw_copy3(splineArr + 3 * i, vtxArr + (i*hQuality + j) * 3);
			}

		}
	}


	updateCollideInfo(pTraining);

	nFce = 0;
	
	for (i = 0; i < vQuality - 1; i++)
	{
		if (collideSecIdx == i) break;

		for (j = 0; j < hQuality; j++)
		{
			fceArr[nFce * 3] = i*hQuality + j;
			fceArr[nFce * 3 + 1] = i*hQuality + (j + 1) % hQuality;
			fceArr[nFce * 3 + 2] = (i + 1)*hQuality + j;
			nFce++;
			fceArr[nFce*3 ] = i*hQuality + (j + 1) % hQuality;
			fceArr[nFce * 3 + 1] = (i + 1)*hQuality + (j + 1) % hQuality;
			fceArr[nFce * 3 + 2] = (i + 1)*hQuality + j;
			nFce++;
		}
	}
	/*
	for (i = 0; i < vQuality; i++)
	{	for (j = 0; j < hQuality; j++)
		{	ij = i*hQuality + j;
			normArr[3 * ij] = -shapeArr[2 * j ];
			normArr[3 * ij + 1] = -shapeArr[2 * j +1 ];
			normArr[3 * ij + 2] = 0.0;
		}
	}
	*/
	Ogre::ManualObject * obj;
	

	if (MXOgreWrapper::Get()->GetDefaultSceneManger()->hasManualObject(m_manualObjectName)) {
		obj = MXOgreWrapper::Get()->GetDefaultSceneManger()->getManualObject(m_manualObjectName);
		sNew = 0;
	}
	else {
		obj = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject(m_manualObjectName);
		sNew = 1;
	}

	Ogre::GpuProgramParametersSharedPtr ShaderPtr = GetShaderParamterPtr(m_materialName, VERTEX_PROGRAME, 0, 0);
	if (ShaderPtr.isNull() == false)
	{
		if (ShaderPtr->_findNamedConstantDefinition("ts", false))
		{
			ShaderPtr->setNamedConstant("ts", Ogre::Real(ts*20.0));
		}
	}


	
	if (sNew)
		obj->begin(m_materialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);	//  clampdebug
	else
		obj->beginUpdate(0);

	for (i = 0; i < vQuality; i++)
	{
		for (j = 0; j < hQuality; j++)
		{	ij = i*hQuality + j;
			obj->position(vtxArr[3 * ij], vtxArr[3 * ij + 1], vtxArr[3 * ij +2]);
			obj->normal(normArr[3 * ij], normArr[3 * ij + 1], normArr[3 * ij + 2]);
			obj->textureCoord(uvArr[2*ij], uvArr[2*ij+1]*20.0);
			obj->tangent(tgArr[3 * ij], tgArr[3 * ij + 1], tgArr[3 * ij + 2]);
		}
	}
	
	//printf("vtxArr %d %d\n", ij, pbii->projectNum*pbii->quality * 3);
	//printf("fceArr %d %d\n", nFce, pbii->projectNum*pbii->quality * 2 * 3);
	
	// empty index (when nFce==0) will cause memory access error, so push a dummy face here.
	obj->index(0);
	obj->index(0);
	obj->index(0);
	
	//if (nFce > 55) nFce = 55;
	for (i = 0; i < nFce; i++) {
		//printf("fce %d %d %d\n", fceArr[3 * i], fceArr[3 * i + 1], fceArr[3 * i+2]);
		obj->index(fceArr[3 * i]);
		obj->index(fceArr[3 * i + 1]);
		obj->index(fceArr[3 * i + 2]);
	};
	
	
	obj->end();

	if (collideSecOrgan) {

		//MisNewTraining_drawSingleFace("face", p1, p2, p3, 0.1);

		//MisMedicEffectRender* render = (MisMedicEffectRender*)collideSecOrgan->getPrivateMember("m_EffectRender");
		// D:\project\mis-laparo\media\effect\textures\cogBrandDefault.tga
		Ogre::TexturePtr m_Tex = Ogre::TextureManager::getSingleton().load("surface_bleed.dds", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		collideSecOrgan->ApplyEffect_Bleeding(Ogre::Vector2(collideFaceTexCoord[0], collideFaceTexCoord[1]), 0, 1.0, m_Tex);
	}
	
	if (sNew)
	{
		Ogre::SceneNode * scene;
		scene = MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->createChildSceneNode();
		scene->setScale(100.0, 100.0, 100.0);
		scene->attachObject(obj);
		//obj->setVisible(false);
	}
	
}

