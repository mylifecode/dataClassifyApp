#include "MisMedicFasciaTissueOperator.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
struct FasciaFaceToPress
{
	FasciaFaceToPress(GFPhysSoftBodyFace * face , const GFPhysVector3 & pt , const GFPhysVector3 & normal)
		: m_DestFace(face) , m_DestPoint(pt) , m_DestNormal(normal)
	{

	}
	GFPhysSoftBodyFace * m_DestFace;
	GFPhysVector3 m_DestPoint;
	GFPhysVector3 m_DestNormal;
};

MisMedicFasciaTissueOperator::MisMedicFasciaTissueOperator(MisMedicOrgan_Ordinary * organ)
{
	m_OrganObject = organ;
}

MisMedicFasciaTissueOperator::~MisMedicFasciaTissueOperator()
{

}

void MisMedicFasciaTissueOperator::SeperateFasciaAround(const GFPhysVector3 & centerPos)
{
	std::vector<GFPhysSoftBodyTetrahedron *> tetrasAround;

	m_OrganObject->CollectTetrasAroundPoint(centerPos , 0.01f , tetrasAround , EMMT_LayerTissue | EMMT_LayerMembrane , true);

	//
	for(size_t t = 0 ; t < tetrasAround.size() ; t++)
	{
		GFPhysSoftBodyTetrahedron * tetra = tetrasAround[t];

		bool finded = false;

		for(int f = 0 ; f < 4 ; f++)
		{
			GFPhysSoftBodyFace * surface = tetra->m_TetraFaces[f]->m_surface;
			if(surface)
			{
				std::vector<GFPhysSoftBodyFace*> pressFaces;
				pressFaces.push_back(surface);

				SeperateFasciaAround(pressFaces);
				finded = true;
				break;
			}
		}
		if(finded)
		   break;
	}
}
void MisMedicFasciaTissueOperator::SeperateFasciaAround(const std::vector<GFPhysSoftBodyFace*> & TearFaces)
{
	GFPhysAlignedVectorObj<FasciaFaceToPress> FacesToPress;
	FacesToPress.reserve(TearFaces.size());

	std::vector<GFPhysSoftBodyTetrahedron *> TetraToRemove;

	for(size_t c = 0 ; c < TearFaces.size() ; c++)
	{
	    GFPhysSoftBodyFace * face = TearFaces[c];

		if(face->m_GenFace && face->m_GenFace->m_ShareTetrahedrons.size() > 0)
		{
			GFPhysSoftBodyTetrahedron * tetra = face->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;

			GFPhysVector3 crossVec = (face->m_Nodes[1]->m_UnDeformedPos - face->m_Nodes[0]->m_UnDeformedPos).Cross(face->m_Nodes[2]->m_UnDeformedPos - face->m_Nodes[0]->m_UnDeformedPos);

			float crossVecLen = crossVec.Length();

			if(crossVecLen < 0.00001f)
			{
			   TetraToRemove.push_back(tetra);
			   continue;
			}
			else
			{
				GFPhysVector3 faceNormal = crossVec / crossVecLen;

				//find couple point dist to this face
				float currThickness = 6.0f * tetra->m_RestSignedVolume / crossVecLen;

				//test only
#if(1)		  
				for(size_t n = 0 ; n < 4 ; n++)
				{
					if(tetra->m_TetraNodes[n] != face->m_Nodes[0] && tetra->m_TetraNodes[n] != face->m_Nodes[1] && tetra->m_TetraNodes[n] != face->m_Nodes[2])
					{
						float testdist = -(tetra->m_TetraNodes[n]->m_UnDeformedPos-face->m_Nodes[0]->m_UnDeformedPos).Dot(faceNormal);

						int i = 0 ;
						int j = i+1;
						break;
					}
				}
#endif
				//


				float removedThickness = currThickness * 0.5f;

				GFPhysVector3 pPoint = face->m_Nodes[1]->m_UnDeformedPos - faceNormal * removedThickness;

				FacesToPress.push_back(FasciaFaceToPress(face , pPoint , faceNormal));
			}
		}
		
	}

	//move press faces position to the plane
	for(size_t c = 0 ; c < FacesToPress.size() ; c++)
	{
		GFPhysSoftBodyFace * face = FacesToPress[c].m_DestFace;

		GFPhysVector3 destPoint  = FacesToPress[c].m_DestPoint;

		GFPhysVector3 destNormal = FacesToPress[c].m_DestNormal;

		for(int n = 0 ; n < 3 ; n++)
		{
			float dist = (face->m_Nodes[n]->m_UnDeformedPos-destPoint).Dot(destNormal);
			if(dist > 0)
			{
			   face->m_Nodes[n]->m_UnDeformedPos -= destNormal * dist;
			}
		}
	}

	//update element influenced by this compress
	GoPhysSoftBodyRestShapeModify shapemodify;//need optimize only the element influenced should be rebuild
	shapemodify.PostModifyUndeformedPos(PhysicsWrapper::GetSingleTon().m_dynamicsWorld,
										m_OrganObject->m_physbody,
										true,
										true);
}