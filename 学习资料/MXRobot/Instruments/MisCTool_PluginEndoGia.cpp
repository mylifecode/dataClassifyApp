#include "MisCTool_PluginEndoGia.h"
#include "Tool.h"
#include "MisMedicOrganOrdinary.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "TipMgr.h"
#include "ITraining.h"
#include "Math/GoPhysTransformUtil.h"
#include "Math/GoPhysSIMDMath.h"
#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "PhysicsWrapper.h"

//#include "NewTraining.h"
#include "MisMedicOrganAttachment.h"


class MisCTool_PluginEndoGiaRegCB : public GFPhysNodeOverlapCallback
{
public:
	MisCTool_PluginEndoGiaRegCB(std::vector<GFPhysSoftBodyFace*>* faces)
	{	m_SoftFacesToCheck = faces;
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{
		GFPhysSoftBodyFace * face = (GFPhysSoftBodyFace*)UserData;
		m_SoftFacesToCheck->push_back(face);
	}

	std::vector<GFPhysSoftBodyFace*>* m_SoftFacesToCheck;
};

class NailFaceInterval
{
public:
    NailFaceInterval(GFPhysSoftBodyFace * face , 
        float t0 , 
        float t1 , 
        const GFPhysVector3 & minPos,
        const GFPhysVector3 & maxPos) : 
    m_face(face) , m_tMin(t0) , m_tMax(t1) , m_PosMin(minPos) ,m_PosMax(maxPos)
    {}
    GFPhysSoftBodyFace * m_face;
    GFPhysVector3 m_PosMin;
    GFPhysVector3 m_PosMax;
    float m_tMin;
    float m_tMax;
};



MisCTool_PluginEndoGia::MisCTool_PluginEndoGia(CTool * tool)
										: MisMedicCToolPluginInterface(tool)
{
	m_canNail = true;
	m_TimeElapsedSinceLastNail = 0;	
	m_AppAllowClamp = false;
    m_hasNailOrganization = false;
	m_haseffectApplyed = false;
	m_state = 0;
	m_prepareTime = 0.0;
}

MisCTool_PluginEndoGia::~MisCTool_PluginEndoGia()
{

}

void MisCTool_PluginEndoGia::setState(int state)
{	
	if (m_state == 3) {
		if (state == 0) m_state = 0;
	}
	else
		m_state = state;

}

typedef struct {
	GFPhysSoftBodyFace* face;
	GFPhysSoftBodyNode* lineA;
	GFPhysSoftBodyNode* lineB;
	MisMedicOrgan_Ordinary* organ;
	float lambda;
	GFPhysVector3 currentPos;
	GFPhysVector3 origPos;
	int next;
	int used;
} cross_vtx_t;



void MisCTool_PluginEndoGia::getROIFaces(std::vector<GFPhysSoftBodyFace*>* faces, MisMedicOrgan_Ordinary* pOrgan, GFPhysVector3& regMin, GFPhysVector3& regMax) {
	MisCTool_PluginEndoGiaRegCB callback(faces);
	pOrgan->m_physbody->GetSoftBodyShape().TraverseFaceTreeAgainstAABB(&callback, regMin, regMax);
}


void MisCTool_PluginEndoGia::createNail(MisMedicOrgan_Ordinary* organ, std::vector<GFPhysSoftBodyFace*>* faces, float len, float offset) {

	GFPhysVector3 gp1, gp2, gp3, gp4, gp12, gp13, gpABC, gpProj, gpMax, gpMin, gpCenter;
	GFPhysVector3 gp2D[4], gp2DSAT[4], gpProj2D[2], gpProj2DSAT[1];
	float planeOffset, d1, d2, d3, D, gp12len, gp13len;
	cross_vtx_t crossVtxArr[1000];
	int lineCount, i, crossVtxCount, lastI, lastPairI, findI, nextI, uid1, uid2, sFind, sCross;
	GFPhysSoftBodyFace* pFace;
	char name[100];
	float lambda, nailStep, segmentRemain, segmentLen, t, f;
	MisMedicEndoGiaClips* clips;
	GFPhysVector3 triVertsV[3], triVertsU[3], ResultPoint1[2], ResultPoint2[2], ResultPoint[2];
	bool r1, r2;

	// 三点构造平面
	gp1 = m_ToolObject->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeLeft.m_LinPoints[0];
	gp2 = m_ToolObject->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeLeft.m_LinPoints[1];
	gp3 = m_ToolObject->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeRight.m_LinPoints[0];
	gp4 = m_ToolObject->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeRight.m_LinPoints[1];


	// 修正gp
	gpCenter = (gp1 + gp2 + gp3 + gp4) / 4;
	gp12 = ((gp2 - gp1) + (gp4 - gp3)) / 2;
	gp13 = ((gp3 - gp1) + (gp4 - gp2)) / 2;
	gp13 = gp13 - gp12.Dot(gp13) / gp12.Length()*(gp12 / gp12.Length());
	gp1 = gpCenter - 0.5*gp12 - 0.5*gp13;
	gp2 = gpCenter + 0.5*gp12 - 0.5*gp13;
	gp3 = gpCenter - 0.5*gp12 + 0.5*gp13;
	gp4 = gpCenter + 0.5*gp12 + 0.5*gp13;



	gp12 = gp2 - gp1;
	gp13 = gp3 - gp1;
	gpABC = gp12.Cross(gp13);
	D = -gpABC.Dot(gp1);

	// 偏移平面
	planeOffset = D - len*gpABC.Length();

	// 中心和平面化后的点与直线
	gpCenter = (gp1 + gp2 + gp3 + gp4 ) / 4;
	gp2D[0] = GFPhysVector3(0.5, 0.5, 1.0);
	gp2D[1] = GFPhysVector3(-0.5, 0.5, 1.0);
	gp2D[2] = GFPhysVector3(0.5, -0.5, 1.0);
	gp2D[3] = GFPhysVector3(-0.5, -0.5, 1.0);
	gp2DSAT[0] = GFPhysVector3(1.0, 0.0, 0.5);
	gp2DSAT[1] = GFPhysVector3(1.0, 0.0, -0.5);
	gp2DSAT[2] = GFPhysVector3(0.0, 1.0, 0.5);
	gp2DSAT[3] = GFPhysVector3(0.0, 1.0, -0.5);

	// gp12, gp13 normalize
	gp12len = gp12.Length();
	gp13len = gp13.Length();
	gp12 = gp12 / (gp12.Dot(gp12));
	gp13 = gp13 / (gp13.Dot(gp13));


	// find the intercept points
	lineCount = 0;
	
	//printf("== G %f %f %f \n", gpCenter.GetX(), gpCenter.GetY(), gpCenter.GetZ());
	//printf("== G %f %f %f \n", gp1.GetX(), gp1.GetY(), gp1.GetZ());
	//printf("== G %f %f %f \n", gp2.GetX(), gp2.GetY(), gp2.GetZ());
	//printf("== G %f %f %f \n", gp3.GetX(), gp3.GetY(), gp3.GetZ());
	//printf("== G %f %f %f \n", gp4.GetX(), gp4.GetY(), gp4.GetZ());
	for (i = 0; i < faces->size(); i++) {
			pFace = faces->at(i);
			d1 = gpABC.Dot(pFace->m_Nodes[0]->m_CurrPosition) + planeOffset;
			d2 = gpABC.Dot(pFace->m_Nodes[1]->m_CurrPosition) + planeOffset;
			d3 = gpABC.Dot(pFace->m_Nodes[2]->m_CurrPosition) + planeOffset;
			if (d1*d2 > 0.0 && d2*d3>0.0) continue;	// 在同一侧
			
			
			sFind = 0;
			if (d1*d2 < 0.0) {
				crossVtxArr[lineCount].face = pFace;
				crossVtxArr[lineCount].organ = organ;
				crossVtxArr[lineCount].lineA = pFace->m_Nodes[0]->m_uid<pFace->m_Nodes[1]->m_uid ? pFace->m_Nodes[0] : pFace->m_Nodes[1];
				crossVtxArr[lineCount].lineB = pFace->m_Nodes[0]->m_uid<pFace->m_Nodes[1]->m_uid ? pFace->m_Nodes[1] : pFace->m_Nodes[0];
				sFind++;
				lineCount++;
			}
			if (d2*d3 < 0.0) {
				crossVtxArr[lineCount].face = pFace; 
				crossVtxArr[lineCount].organ = organ;
				crossVtxArr[lineCount].lineA = pFace->m_Nodes[1]->m_uid<pFace->m_Nodes[2]->m_uid ? pFace->m_Nodes[1] : pFace->m_Nodes[2];
				crossVtxArr[lineCount].lineB = pFace->m_Nodes[1]->m_uid<pFace->m_Nodes[2]->m_uid ? pFace->m_Nodes[2] : pFace->m_Nodes[1];
				sFind++;
				lineCount++;
			}
			if (d3*d1 < 0.0) {
				crossVtxArr[lineCount].face = pFace; 
				crossVtxArr[lineCount].organ = organ;
				crossVtxArr[lineCount].lineA = pFace->m_Nodes[2]->m_uid<pFace->m_Nodes[0]->m_uid ? pFace->m_Nodes[2] : pFace->m_Nodes[0];
				crossVtxArr[lineCount].lineB = pFace->m_Nodes[2]->m_uid<pFace->m_Nodes[0]->m_uid ? pFace->m_Nodes[0] : pFace->m_Nodes[2];
				sFind++;
				lineCount++;
			}

			if (sFind == 2) {
				for (int j = lineCount - 2; j < lineCount; j++) {
					d1 = gpABC.Dot(crossVtxArr[j].lineA->m_CurrPosition) + planeOffset;
					d2 = gpABC.Dot(crossVtxArr[j].lineB->m_CurrPosition) + planeOffset;
					crossVtxArr[j].lambda = fabs(d2) / (fabs(d1) + fabs(d2));
					crossVtxArr[j].currentPos = crossVtxArr[j].lineA->m_CurrPosition*crossVtxArr[j].lambda + crossVtxArr[j].lineB->m_CurrPosition*(1.0 - crossVtxArr[j].lambda);
					crossVtxArr[j].origPos = crossVtxArr[j].lineA->m_UnDeformedPos*crossVtxArr[j].lambda + crossVtxArr[j].lineB->m_UnDeformedPos*(1.0 - crossVtxArr[j].lambda);

					crossVtxArr[j].used = 0;
				}
				// SAT
				gpProj2D[0].SetX(gp12.Dot(crossVtxArr[lineCount - 2].currentPos - gpCenter));
				gpProj2D[0].SetY(gp13.Dot(crossVtxArr[lineCount - 2].currentPos - gpCenter));
				gpProj2D[0].SetZ(1.0);
				gpProj2D[1].SetX(gp12.Dot(crossVtxArr[lineCount - 1].currentPos - gpCenter));
				gpProj2D[1].SetY(gp13.Dot(crossVtxArr[lineCount - 1].currentPos - gpCenter));
				gpProj2D[1].SetZ(1.0);
				gpProj2DSAT[0] = GFPhysVector3(gpProj2D[1].GetY() - gpProj2D[0].GetY(), gpProj2D[0].GetX() - gpProj2D[1].GetX(), gpProj2D[1].GetX()*gpProj2D[0].GetY() - gpProj2D[0].GetX()*gpProj2D[1].GetY());
				sCross = 1;
				//printf("== ");
				if (gp2D[0].Dot(gpProj2DSAT[0])*gp2D[1].Dot(gpProj2DSAT[0])>0 && gp2D[1].Dot(gpProj2DSAT[0])*gp2D[2].Dot(gpProj2DSAT[0]) > 0 && gp2D[2].Dot(gpProj2DSAT[0])*gp2D[3].Dot(gpProj2DSAT[0]) > 0) sCross = 0;
				//if (sCross == 0) printf("A");
				if (gpProj2D[0].Dot(gp2DSAT[0])*gpProj2D[1].Dot(gp2DSAT[0]) > 0 && gpProj2D[0].Dot(gp2DSAT[0])*gp2D[0].Dot(gp2DSAT[0]) < 0.0) sCross = 0;
				//if (sCross == 0) printf("B");
				if (gpProj2D[0].Dot(gp2DSAT[1])*gpProj2D[1].Dot(gp2DSAT[1]) > 0 && gpProj2D[0].Dot(gp2DSAT[1])*gp2D[1].Dot(gp2DSAT[1]) < 0.0) sCross = 0;
				//if (sCross == 0) printf("C");
				if (gpProj2D[0].Dot(gp2DSAT[2])*gpProj2D[1].Dot(gp2DSAT[2]) > 0 && gpProj2D[0].Dot(gp2DSAT[2])*gp2D[1].Dot(gp2DSAT[2]) < 0.0) sCross = 0;
				if (sCross == 0)  {
					//printf("D %f %f %f %f ", gpProj2D[0].GetX(), gpProj2D[0].GetY(), gpProj2D[1].GetX(), gpProj2D[1].GetY());
				//	printf("D");
					//printf("== W %f %f %f \n", crossVtxArr[lineCount - 2].currentPos.GetX(), crossVtxArr[lineCount - 2].currentPos.GetY(), crossVtxArr[lineCount - 2].currentPos.GetZ());
					//printf("== W %f %f %f \n", crossVtxArr[lineCount - 1].currentPos.GetX(), crossVtxArr[lineCount - 1].currentPos.GetY(), crossVtxArr[lineCount - 1].currentPos.GetZ());
					//printf("== B %f %f %f \n", gpProj2D[0].GetX()*gp12.GetX()*gp12len*gp12len + gpProj2D[0].GetY()*gp13.GetX()*gp13len*gp13len + gpCenter.GetX(),
						//	                   gpProj2D[0].GetX()*gp12.GetY()*gp12len*gp12len + gpProj2D[0].GetY()*gp13.GetY()*gp13len*gp13len + gpCenter.GetY(),
						 //                      gpProj2D[0].GetX()*gp12.GetZ()*gp12len*gp12len + gpProj2D[0].GetY()*gp13.GetZ()*gp13len*gp13len + gpCenter.GetZ());
				}
				if (gpProj2D[0].Dot(gp2DSAT[3])*gpProj2D[1].Dot(gp2DSAT[3]) > 0 && gpProj2D[0].Dot(gp2DSAT[3])*gp2D[2].Dot(gp2DSAT[3]) < 0.0) sCross = 0;
				//if (sCross == 0) printf("E");
				//printf("\n");
				if (sCross == 0) lineCount -= 2;
			}
			else lineCount -= sFind;	// 单个交点的情况, 原因不明
			// GJK

	}
	

	if (lineCount == 0) return;

	/*
	for (i = 0; i < lineCount; i++) {
		d1 = gpABC.Dot(crossVtxArr[i].lineA->m_CurrPosition) + planeOffset;
		d2 = gpABC.Dot(crossVtxArr[i].lineB->m_CurrPosition) + planeOffset;
		crossVtxArr[i].lambda = fabs(d2) / (fabs(d1) + fabs(d2));
		crossVtxArr[i].currentPos = crossVtxArr[i].lineA->m_CurrPosition*crossVtxArr[i].lambda + crossVtxArr[i].lineB->m_CurrPosition*(1.0 - crossVtxArr[i].lambda);
		crossVtxArr[i].used = 0;
		//sprintf(name, "pt%d", i);
		//MisNewTraining_drawSingleSphere(name, p1, 0.05f);
	}*/
	//printf("AAA %d\n", lineCount);

	crossVtxCount = lineCount;

	//printf("\n");
	lastI = 0;
	crossVtxArr[lastI].used = 1;
	while (true) {
		if (lastI % 2 == 0) lastPairI = lastI + 1;
		else lastPairI = lastI - 1;
		//cw_copy3(crossVtxArr[lastPairI].currentPos, p1);
		uid1 = crossVtxArr[lastPairI].lineA->m_uid;
		uid2 = crossVtxArr[lastPairI].lineB->m_uid;
		findI = -1;
		for (i = 0; i < lineCount; i++) {
			if (i == lastPairI) continue;
			if (uid1 == crossVtxArr[i].lineA->m_uid && uid2== crossVtxArr[i].lineB->m_uid) break;
			//if (cw_ssd3(crossVtxArr[i].currentPos, p1) < 1e-6) break;
		}
		if (i == lineCount) findI = -1;
		else if (crossVtxArr[i].used==1) findI = -1;
		else findI = i;
		if (findI == -1) break;
		crossVtxArr[lastI].next = findI;
		crossVtxArr[findI].used = 1;
		lastI = findI;
		//printf("%d ", lastI); Sleep(100);
	}
	crossVtxArr[lastI].next = 0;
	//printf("\n");


	nailStep = 0.2;
	segmentRemain = offset;

	clips = organ->GetEndoGiaClips();
	
	clips->addClip(crossVtxArr[0].face, crossVtxArr[0].lineA, crossVtxArr[0].lineB, crossVtxArr[0].lambda, crossVtxArr[crossVtxArr[0].next].lineA, crossVtxArr[crossVtxArr[0].next].lineB, crossVtxArr[crossVtxArr[0].next].lambda, 1.0);

	i = 0;
	while (true) {
		nextI = crossVtxArr[i].next;
		//segmentLen = crossVtxArr[i].currentPos.Distance(crossVtxArr[nextI].currentPos);	//sqrt(cw_ssd3(p1, p2));
		segmentLen = crossVtxArr[i].origPos.Distance(crossVtxArr[nextI].origPos);	//sqrt(cw_ssd3(p1, p2));
		segmentRemain += segmentLen;
		while (true) {
			if (segmentRemain < nailStep) break;
			segmentRemain -= nailStep;
			lambda = segmentRemain / segmentLen;
			clips->addClip(crossVtxArr[i].face, crossVtxArr[i].lineA, crossVtxArr[i].lineB, crossVtxArr[i].lambda, crossVtxArr[nextI].lineA, crossVtxArr[nextI].lineB, crossVtxArr[nextI].lambda, lambda);
		}
		if (nextI == 0) break;
		i = nextI;
	}
}

void MisCTool_PluginEndoGia::showCutArea() {
	GFPhysVector3 gp1, gp2, gp3, gp4, gpMax, gpMin;
	std::vector<GFPhysSoftBodyFace*> faces;
	gp1 = m_ToolObject->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeLeft.m_LinPoints[0];
	gp2 = m_ToolObject->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeLeft.m_LinPoints[1];
	gp3 = m_ToolObject->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeRight.m_LinPoints[0];
	gp4 = m_ToolObject->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeRight.m_LinPoints[1];
	gpMax = gp1; gpMax.SetMax(gp2); gpMax.SetMax(gp3); gpMax.SetMax(gp4); gpMax = gpMax + GFPhysVector3(0.1, 0.1, 0.1);
	gpMin = gp1; gpMin.SetMin(gp2); gpMin.SetMin(gp3); gpMin.SetMin(gp4); gpMin = gpMin - GFPhysVector3(0.1, 0.1, 0.1);

	float p1[3], p2[3], p3[3];

	cw_vec3(gp1.GetX(), gp1.GetY(), gp1.GetZ(), p1);
	cw_vec3(gp2.GetX(), gp2.GetY(), gp2.GetZ(), p2);
	cw_vec3(gp3.GetX(), gp3.GetY(), gp3.GetZ(), p3);

	//printf("%f %f %f; %f %f %f; %f %f %f\n", p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], p3[0], p3[1], p3[2]);

	//MisNewTraining_drawSingleFace("face1", p1, p2, p3, 0.4f);
}

void MisCTool_PluginEndoGia::tryApplyClips(MisMedicOrgan_Ordinary **organs) {
	
		// 包围盒
		GFPhysVector3 gp1, gp2, gp3, gp4, gpMax, gpMin;
		std::vector<GFPhysSoftBodyFace*> faces;
		gp1 = m_ToolObject->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeLeft.m_LinPoints[0];
		gp2 = m_ToolObject->m_lefttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeLeft.m_LinPoints[1];
		gp3 = m_ToolObject->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeRight.m_LinPoints[0];
		gp4 = m_ToolObject->m_righttoolpartconvex.m_rigidbody->GetInterpolationWorldTransform()*m_ToolObject->m_CutBladeRight.m_LinPoints[1];
		gpMax = gp1; gpMax.SetMax(gp2); gpMax.SetMax(gp3); gpMax.SetMax(gp4); gpMax = gpMax + GFPhysVector3(0.1, 0.1, 0.1);
		gpMin = gp1; gpMin.SetMin(gp2); gpMin.SetMin(gp3); gpMin.SetMin(gp4); gpMin = gpMin - GFPhysVector3(0.1, 0.1, 0.1);

		float p1[3], p2[3], p3[3];

		cw_vec3(gp1.GetX(), gp1.GetY(), gp1.GetZ(), p1);
		cw_vec3(gp2.GetX(), gp2.GetY(), gp2.GetZ(), p2);
		cw_vec3(gp3.GetX(), gp3.GetY(), gp3.GetZ(), p3);

		//printf("%f %f %f; %f %f %f; %f %f %f\n", p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], p3[0], p3[1], p3[2]);

		//MisNewTraining_drawSingleFace("face1", p1, p2, p3, 0.1f);


		
		for (int i = 0; i < 10; i++) {
			if (organs[i] == 0) break;
			faces.clear();
			MisCTool_PluginEndoGia::getROIFaces(&faces, organs[i], gpMin, gpMax);
			if (faces.size() >= 1) {
				createNail(organs[i], &faces, 0.2, 0.0);
				createNail(organs[i], &faces, 0.25, -0.1);
				createNail(organs[i], &faces, 0.3, 0.0);

				createNail(organs[i], &faces, -0.2, 0.0);
				createNail(organs[i], &faces, -0.25, -0.1);
				createNail(organs[i], &faces, -0.3, 0.0);
			}
		}
		
}

void MisCTool_PluginEndoGia::PhysicsSimulationStart(int currStep , int TotalStep , float dt)
{
	

	
}



void MisCTool_PluginEndoGia::TryApplyEndoGia(MisMedicOrgan_Ordinary * organ , 
                                              GFPhysVectorObj<CompressNodesAndDir*> NodesAndDir,
                                              Real compressrate)
{
    if (NodesAndDir.size() == 0)
    {
        return;
    }
    GoPhysSoftBodyRestShapeModify restShapeModify;
    
    //Ogre::LogManager::getSingleton().logMessage(Ogre::String("Compressed direction is  ")+Ogre::StringConverter::toString(LeftClampNormal.m_x)+","
    //+Ogre::StringConverter::toString(LeftClampNormal.m_y)+","
    //+Ogre::StringConverter::toString(LeftClampNormal.m_z)+".");

    restShapeModify.CompressAlongLocalDirectionWithPoints( PhysicsWrapper::GetSingleTon().m_dynamicsWorld , organ->m_physbody , NodesAndDir, compressrate , true,true);
}

//==================================================================================================
void MisCTool_PluginEndoGia::PhysicsSimulationEnd(int currStep , int TotalStep , float dt)
{
}
//==================================================================================================
void MisCTool_PluginEndoGia::OneFrameUpdateStarted(float timeelapsed)
{
	showCutArea();
	
	
	

}

