#include "CustomCollision.h"
#include "MisMedicOrganOrdinary.h"
#include "VeinConnectObject.h"


#if SEGMENT_COLLIDE
VeinCollideData::VeinCollideData(const GFPhysVector3 & pA, const GFPhysVector3 & pB, int ClusterIndex, int PairIndex)
{
    m_PointA = pA;
    m_PointB = pB;
    m_ClusterIndex = ClusterIndex;
    m_PairIndex = PairIndex;
    m_InContact = false;
    //m_WillBeDissected = false;
    m_CanContact = true;
    m_InHookState = false;
    m_contactRigid = 0;
    m_ContactRecovredTime = 0;
    m_HostObject = 0;
    m_collisionlistener = 0;
    m_LocalHookOffset = GFPhysVector3(1, 0, 0);
}
#else
VeinCollideData::VeinCollideData(const GFPhysVector3 & pA0, const GFPhysVector3 & pB0, const GFPhysVector3 & pA1, const GFPhysVector3 & pB1, int ClusterIndex, int PairIndex)
{
    m_PointA0 = pA0;
    m_PointB0 = pB0;
    m_PointA1 = pA1;
    m_PointB1 = pB1;
    m_ClusterIndex = ClusterIndex;
    m_PairIndex = PairIndex;
    m_InContact = false;
    //m_WillBeDissected = false;
    m_CanContact = true;
    m_InHookState = false;
    m_contactRigid = 0;
    m_ContactRecovredTime = 0;
    m_HostObject = 0;
    m_collisionlistener = 0;
    m_LocalHookOffset = GFPhysVector3(1, 0, 0);
}
#endif
//----------------------------------------------------------------------------------------------------------------------
void VeinCollideData::AddContactPoint(const GFPhysVector3& normalOnBInWorld,const GFPhysVector3& pointInWorld,Real depth)
{    
    if ((depth < 0.01f && depth > -0.05f))//|| depth + GP_EPSILON < 0.0f)
    {
        m_InContact = true;
        m_CanContact = false;
		m_InHookState = false;
		m_contactinRigidLocal = m_contactRigid->GetWorldTransform().Inverse() * (pointInWorld + normalOnBInWorld*depth);//record contact point to move with
        m_LocalHookOffset = m_contactRigid->GetWorldTransform().Inverse()*normalOnBInWorld;

        if (m_collisionlistener)
            m_collisionlistener->OnVeinConnectPairCollide(m_HostObject, m_contactRigid, m_ClusterIndex, m_PairIndex, pointInWorld);        
    }
}
//----------------------------------------------------------------------------------------------------------------------
void VeinCollideData::AddHookPoint(const GFPhysVector3& pointInWorld, const GFPhysVector3 & localHookOffset, const GFPhysVector3 & localHookDir)
{
    m_InHookState = true;
    m_CanContact = false;//disable contact
    m_InContact = false;//mask contact state
    m_contactinRigidLocal = m_contactRigid->GetWorldTransform().Inverse()*pointInWorld;//record hook point to move with
    m_contactinRigidLocal += localHookOffset;

    m_LocalHookOffset = localHookOffset;
    m_WorldHookOffset = m_contactRigid->GetWorldTransform().GetBasis()*m_LocalHookOffset;

    m_LocalHookDir = localHookDir;
    m_WorldHookDir = m_contactRigid->GetWorldTransform().GetBasis()*m_LocalHookDir;
}
//----------------------------------------------------------------------------------------------------------------------
/*void VeinCollideData::SetContactOff()
{
	m_contactRigid = 0;
	m_InContact = false;
	m_CanContact = false;
	//m_WillBeDissected = false;
	m_ContactRecovredTime = 0;
}*/
//----------------------------------------------------------------------------------------------------------------------
void VeinCollideData::SetHookAndContactOff()
{
	m_contactRigid = 0;
	m_InHookState = false;
	m_InContact = false;
	m_CanContact = false;
	m_ContactRecovredTime = 0;
}
//----------------------------------------------------------------------------------------------------------------------
GFPhysVector3 VeinCollideData::CalculatStickPointInWorld()
{
	assert(m_contactRigid != NULL);
	GFPhysVector3 contactworld = m_contactRigid->GetWorldTransform()*m_contactinRigidLocal;
	return contactworld;
}
//----------------------------------------------------------------------------------------------------------------------
void VeinCollideData::Update(float dt)
{	
	if (m_contactRigid && (m_InHookState || m_InContact))
	{
		m_contactinWorld = CalculatStickPointInWorld();
		m_WorldHookOffset = m_contactRigid->GetWorldTransform().GetBasis() * m_LocalHookOffset;
		m_WorldHookDir = m_contactRigid->GetWorldTransform().GetBasis() * m_LocalHookDir;

		/*if (m_LocalHookDir.Length2() > GP_EPSILON)//if we have valid hook dir
		{
			GFPhysVector3 dir0 = (m_PointB0 - m_PointA0).Normalized();
			GFPhysVector3 Expand0 = (m_contactinWorld - m_PointA0) - dir0 * (m_contactinWorld - m_PointA0).Dot(dir0);

			GFPhysVector3 dir1 = (m_PointB1 - m_PointA1).Normalized();
			GFPhysVector3 Expand1 = (m_contactinWorld - m_PointA1) - dir1 * (m_contactinWorld - m_PointA1).Dot(dir1);

			if (Expand0.Normalized().Dot(m_WorldHookDir) < -0.3f && Expand0.Length() > 0.2f && Expand1.Normalized().Dot(m_WorldHookDir) < -0.3f && Expand1.Length() > 0.2f)
			{
				SetHookOff();
			}
		}*/
    }
	else if(!m_CanContact)
    {
        m_ContactRecovredTime += dt;
        if (m_ContactRecovredTime > 0.5f)
        {
            m_ContactRecovredTime = 0;
            m_CanContact = true;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------
VeinTreeCollideConvexCallBack::VeinTreeCollideConvexCallBack()
{
	m_Convex = NULL;
	m_collisionlistener = 0;
}
//----------------------------------------------------------------------------------------------------------------------
void VeinTreeCollideConvexCallBack::ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
{
#if 1
    VeinCollideData * collidedata = (VeinCollideData*)UserData;

    if (collidedata == 0)
        return;

    GFPhysConvexCDShape * convexToCheck[100];
    GFPhysTransform childTrans[20];

    int convexNum = 0;

    GFPhysConvexCDShape * SingleConvexshape = dynamic_cast<GFPhysConvexCDShape*>(m_Convex->GetCollisionShape());
    GFPhysCompoundShape * CombineHullShape = dynamic_cast<GFPhysCompoundShape*>(m_Convex->GetCollisionShape());
    if (SingleConvexshape)
    {
        childTrans[convexNum].SetIdentity();
        convexToCheck[convexNum++] = SingleConvexshape;
    }
    else if (CombineHullShape)
    {
        for (int c = 0; c < CombineHullShape->GetNumComponent(); c++)
        {
            childTrans[convexNum] = CombineHullShape->GetComponentTransform(c);
            convexToCheck[convexNum++] = (GFPhysConvexCDShape*)CombineHullShape->GetComponentShape(c);
        }
    }
#if SEGMENT_COLLIDE
    bool instersectBox = IsLineSegAABBOverLap(m_Convexaabbmin , m_Convexaabbmax , collidedata->m_PointA , collidedata->m_PointB);
#else

    GFPhysVector3 veinaabbmin = collidedata->m_PointA0;
    veinaabbmin.SetMin(collidedata->m_PointA1);
    veinaabbmin.SetMin(collidedata->m_PointB0);
    veinaabbmin.SetMin(collidedata->m_PointB1);

    GFPhysVector3 veinaabbmax = collidedata->m_PointA0;
    veinaabbmax.SetMax(collidedata->m_PointA1);
    veinaabbmax.SetMax(collidedata->m_PointB0);
    veinaabbmax.SetMax(collidedata->m_PointB1);

    bool instersectBox = TestAabbAgainstAabb2(m_Convexaabbmin, m_Convexaabbmax, veinaabbmin, veinaabbmax);
#endif

    if (instersectBox && collidedata->m_CanContact) 
    {
        for (int c = 0; c < convexNum; c++)
        {
            if (!collidedata->m_InContact)
            {
                GFPhysConvexCDShape * SingleConvexshape = convexToCheck[c];

                //first test line segment is overlap aabb
                //further test line segment to convex
                GFPhysConvexCDShape* min0 = SingleConvexshape;
#if SEGMENT_COLLIDE
                GFPhysLineSegmentShape LineShape(collidedata->m_PointA, collidedata->m_PointB);

                LineShape.SetMargin(0);

                GFPhysConvexCDShape* min1 = &LineShape;
#else
                GFPhysTetrahedronShape TetrahedronShape(collidedata->m_PointA0, collidedata->m_PointA1, collidedata->m_PointB0, collidedata->m_PointB1);

                TetrahedronShape.SetMargin(0);

                GFPhysConvexCDShape* min1 = &TetrahedronShape;
#endif
                GFPhysGJKCollideDetector::ClosestPointInput input;

                GFPhysVSimplexCloseCalculator VSimplexSolver;

                GFPhysGjkEpaPDCalculor PDSolver;

                GFPhysGJKCollideDetector gjkPairDetector(min0, min1, &VSimplexSolver, &PDSolver);

                gjkPairDetector.SetMinkowskiA(min0);

                gjkPairDetector.SetMinkowskiB(min1);

                input.m_maximumDistanceSquared = min0->GetMargin() + min1->GetMargin();

                input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;

                input.m_transformA = m_Convex->GetWorldTransform()  * childTrans[c];

                input.m_transformB.SetIdentity();

                collidedata->m_contactRigid = m_Convex;

                collidedata->m_collisionlistener = m_collisionlistener;

                gjkPairDetector.GetClosestPoints(false, input, *collidedata);
            }
        }
    }
#else
    VeinCollideData * collidedata = (VeinCollideData*)UserData;

    if (collidedata == 0)
        return;

    GFPhysConvexCDShape * convexToCheck[100];
    GFPhysTransform childTrans[20];

    int convexNum = 0;

    GFPhysConvexCDShape * SingleConvexshape = dynamic_cast<GFPhysConvexCDShape*>(m_Convex->GetCollisionShape());
    GFPhysCompoundShape * CombineHullShape = dynamic_cast<GFPhysCompoundShape*>(m_Convex->GetCollisionShape());
    if (SingleConvexshape)
    {
        childTrans[convexNum].SetIdentity();
        convexToCheck[convexNum++] = SingleConvexshape;

    }
    else if (CombineHullShape)
    {
        for (int c = 0; c < CombineHullShape->GetNumComponent(); c++)
        {
            childTrans[convexNum] = CombineHullShape->GetComponentTransform(c);
            convexToCheck[convexNum++] = (GFPhysConvexCDShape*)CombineHullShape->GetComponentShape(c);
        }
    }


    GFPhysVector3 veinaabbmin = collidedata->m_PointA0;
    veinaabbmin.SetMin(collidedata->m_PointA1);
    veinaabbmin.SetMin(collidedata->m_PointB0);
    veinaabbmin.SetMin(collidedata->m_PointB1);

    GFPhysVector3 veinaabbmax = collidedata->m_PointA0;
    veinaabbmax.SetMax(collidedata->m_PointA1);
    veinaabbmax.SetMax(collidedata->m_PointB0);
    veinaabbmax.SetMax(collidedata->m_PointB1);

    bool instersectBox = TestAabbAgainstAabb2(m_Convexaabbmin, m_Convexaabbmax, veinaabbmin, veinaabbmax);


    if (instersectBox && collidedata->m_CanContact)
    {
        for (int c = 0; c < convexNum; c++)
        {
            GFPhysConvexCDShape * SingleConvexshape = convexToCheck[c];

            //first test line segment is overlap aabb
            //further test line segment to convex
            GFPhysConvexCDShape* min0 = SingleConvexshape;

            GFPhysTriangleShape* TriangleShape[2];
            GFPhysTriangleShape temp0 = GFPhysTriangleShape(collidedata->m_PointA0, collidedata->m_PointA1, collidedata->m_PointB1);
            GFPhysTriangleShape temp1 = GFPhysTriangleShape(collidedata->m_PointA0, collidedata->m_PointB0, collidedata->m_PointB1);

            TriangleShape[0] = &temp0;
            TriangleShape[1] = &temp1;

            for (int i = 0; i < 2;i++)
            {
                if (collidedata->m_InContact == false)
                {
                    TriangleShape[i]->SetMargin(0);

                    GFPhysConvexCDShape* min1 = TriangleShape[i];

                    GFPhysGJKCollideDetector::ClosestPointInput input;

                    GFPhysVSimplexCloseCalculator VSimplexSolver;

                    GFPhysGjkEpaPDCalculor PDSolver;

                    GFPhysGJKCollideDetector gjkPairDetector(min0, min1, &VSimplexSolver, &PDSolver);

                    gjkPairDetector.SetMinkowskiA(min0);

                    gjkPairDetector.SetMinkowskiB(min1);

                    input.m_maximumDistanceSquared = min0->GetMargin() + min1->GetMargin();

                    input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;

                    input.m_transformA = m_Convex->GetWorldTransform()  * childTrans[c];

                    input.m_transformB.SetIdentity();

                    collidedata->m_contactRigid = m_Convex;

                    collidedata->m_collisionlistener = m_collisionlistener;

                    gjkPairDetector.GetClosestPoints(false, input, *collidedata);
                }
            }  
        }
    }
#endif
}

//----------------------------------------------------------------------------------------------------------------------
VeinTreeCollideSphereCallBack::VeinTreeCollideSphereCallBack()
{
	m_Convex = NULL;
}

//----------------------------------------------------------------------------------------------------------------------
void VeinTreeCollideSphereCallBack::ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
{

	VeinCollideData * collidedata = (VeinCollideData*)UserData;
	
	if(collidedata == 0)
		return;

	//const VeinConnectPair & connpair =  m_VeinConnectObj->GetConnectPair(collidedata->m_ClusterIndex , collidedata->m_PairIndex);

	/*
	if(!m_VeinConnectObj->m_CanBeHooked)
	{
		collidedata->SetHookOff();
		return;
	}
	*/

	//first test line segment is overlap aabb
	
#if SEGMENT_COLLIDE
    bool instersectBox = IsLineSegAABBOverLap(m_Radiusaabbmin , m_Radiusaabbmax , collidedata->m_PointA , collidedata->m_PointB);
#else

    //GFPhysVector3 veinaabbmin = collidedata->m_PointA0;
    //veinaabbmin.SetMin(collidedata->m_PointA1);
   // veinaabbmin.SetMin(collidedata->m_PointB0);
    //veinaabbmin.SetMin(collidedata->m_PointB1);

   // GFPhysVector3 veinaabbmax = collidedata->m_PointA0;
    //veinaabbmax.SetMax(collidedata->m_PointA1);
    //veinaabbmax.SetMax(collidedata->m_PointB0);
    //veinaabbmax.SetMax(collidedata->m_PointB1);

	bool instersectBox = true;// TestAabbAgainstAabb2(m_Radiusaabbmin, m_Radiusaabbmax, veinaabbmin, veinaabbmax);
#endif

	if(instersectBox)//further test line segment to sphere
	{
#if SEGMENT_COLLIDE

		GFPhysVector3 P1 = collidedata->m_PointA;

		GFPhysVector3 Q1 = collidedata->m_PointB;

		float s , t;

		GFPhysVector3 C1 , C2;

		float Dist = GPClosestPtSegmentSegment(P1 , Q1 , m_WorldSegmentPointA , m_WorldSegmentPointB , s , t , C1, C2);

		//m_WorldHookDir;
		//GFPhysVector3 direction = (C1-C2).Normalized();

		float dotV = /*direction*/(C1-C2).Dot(m_WorldHookDir);

		if(Dist < m_radius && t > 0.1f && t < 0.9f && dotV > 0)
		{
            if (m_VeinConnectObj->SetHookInfo(collidedata->m_ClusterIndex, collidedata->m_PairIndex))
            {
                if (m_VeinConnectObj->m_IsNewMode)
                    m_VeinConnectObj->SetClusterHookedOn(collidedata->m_ClusterIndex);
                collidedata->m_contactRigid = m_Convex;
                collidedata->AddHookPoint(C2, m_HookLocalOffset, m_HookLocalDir);
                m_VeinConnectObj->AddHookedCount();
                m_currHookCount++;
            }
        }
#else
        if (true)
        {
            Real Rayweight[4];
            GFPhysVector3 intersectpt;
            GFPhysVector3 hookpoint;
            Real triangleWeight[3];
            bool intersect[4];
            //permutation 0 1 2,1 2 3, 0 2 3, 0 1 3
            
            GFPhysVector3 collidedatapoints[4];
			//become slim
#if 1       
			
			GFPhysVector3 center;
			center = 0.5f*(collidedata->m_PointA0 + collidedata->m_PointA1);
			collidedatapoints[0] = center + (collidedata->m_PointA0 - center)*0.9f;
			collidedatapoints[1] = center + (collidedata->m_PointA1 - center)*0.9f;

			center = 0.5f*(collidedata->m_PointB0 + collidedata->m_PointB1);
			collidedatapoints[2] = center + (collidedata->m_PointB0 - center)*0.9f;
			collidedatapoints[3] = center + (collidedata->m_PointB1 - center)*0.9f;
			
			center = (collidedatapoints[0] + collidedatapoints[2])*0.5f;
			collidedatapoints[0] = center + (collidedatapoints[0] - center)*0.9f;
			collidedatapoints[2] = center + (collidedatapoints[2] - center)*0.9f;

			center = (collidedatapoints[1] + collidedatapoints[3]) * 0.5f;
			collidedatapoints[1] = center + (collidedatapoints[1] - center)*0.9f;
			collidedatapoints[3] = center + (collidedatapoints[3] - center)*0.9f;

			/*
			GFPhysVector3 dis0 = collidedata->m_PointA0 - collidedata->m_PointB0;
			GFPhysVector3 dis1 = collidedata->m_PointA1 - collidedata->m_PointB1;

			GFPhysVector3 disA = collidedata->m_PointA0 - collidedata->m_PointA1;
			GFPhysVector3 disB = collidedata->m_PointB0 - collidedata->m_PointB1;

			if (disA.Length2() + disB.Length2() < dis0.Length2() + dis1.Length2())
			{
				if (disA.Length2() < 0.1f || disB.Length2() < 0.1f)
				{
					GFPhysVector3 centerA = 0.5f*(collidedata->m_PointA0 + collidedata->m_PointA1);
					collidedatapoints[0] = centerA + 0.45f*disA;
					collidedatapoints[1] = centerA - 0.45f*disA;
					GFPhysVector3 centerB = 0.5f*(collidedata->m_PointB0 + collidedata->m_PointB1);
					collidedatapoints[2] = centerB + 0.45f*disB;
					collidedatapoints[3] = centerB - 0.45f*disB;
				}
				else
				{
					GFPhysVector3 dirA = disA.Normalize();
					GFPhysVector3 dirB = disB.Normalize();
					collidedatapoints[0] = collidedata->m_PointA0 - 0.01f*dirA;
					collidedatapoints[1] = collidedata->m_PointA1 + 0.01f*dirA;
					collidedatapoints[2] = collidedata->m_PointB0 - 0.01f*dirB;
					collidedatapoints[3] = collidedata->m_PointB1 + 0.01f*dirB;
				}
			}
			else
			{
				if (dis0.Length2() < 0.1f || dis1.Length2() < 0.1f)
				{
					GFPhysVector3 center0 = 0.5f*(collidedata->m_PointA0 + collidedata->m_PointB0);
					collidedatapoints[0] = center0 + 0.45f*dis0;
					collidedatapoints[1] = center0 - 0.45f*dis0;

					GFPhysVector3 center1 = 0.5f*(collidedata->m_PointA1 + collidedata->m_PointB1);
					collidedatapoints[2] = center1 + 0.45f*dis1;
					collidedatapoints[3] = center1 - 0.45f*dis1;
					collidedatapoints[3] = center1 - 0.45f*dis1;
				}
				else
				{
					GFPhysVector3 dir0 = dis0.Normalize();
					GFPhysVector3 dir1 = dis1.Normalize();
					collidedatapoints[0] = collidedata->m_PointA0 - 0.01f*dir0;
					collidedatapoints[1] = collidedata->m_PointA1 - 0.01f*dir1;
					collidedatapoints[2] = collidedata->m_PointB0 + 0.01f*dir0;
					collidedatapoints[3] = collidedata->m_PointB1 + 0.01f*dir1;
				}
			}
			*/
#else
			collidedatapoints[0] = collidedata->m_PointA0;
			collidedatapoints[1] = collidedata->m_PointA1;
			collidedatapoints[2] = collidedata->m_PointB0;
			collidedatapoints[3] = collidedata->m_PointB1;
#endif
			
			if (collidedata->m_InHookState == false)
			{
				GFPhysVector3 HooksegMents[2];
				GFPhysVector3 StripQuads[4];

				HooksegMents[0] = m_WorldSegmentPointA;
				HooksegMents[1] = m_WorldSegmentPointB;
				StripQuads[0] = collidedatapoints[0];
				StripQuads[1] = collidedatapoints[1];
				StripQuads[2] = collidedatapoints[2];
				StripQuads[3] = collidedatapoints[3];

				GFPhysTransform IdentTrans;
				IdentTrans.SetIdentity();
				GFPhysVector3 closetPointA, closetPointB;
				Real dist = GetConvexsClosetPoint(HooksegMents, 2, 0,
					StripQuads, 4, 0.0f,
					IdentTrans, IdentTrans,
					closetPointA, closetPointB,
					m_radius + 0.01f);

				if (dist <= m_radius
					&& dist >= m_radius*0.25f
					&& m_WorldHookDir.Dot((closetPointA - closetPointB).Normalized()) < 0
					&& m_VeinConnectObj->m_HookedCount < 2)
				{
					if (m_VeinConnectObj->SetHookInfo(collidedata->m_ClusterIndex, collidedata->m_PairIndex))
					{
						hookpoint = closetPointA;// +(closetPointB - closetPointA).Normalized() * m_radius;

						m_VeinConnectObj->m_clusters[collidedata->m_ClusterIndex].SetHookOn(m_Convex, hookpoint, m_HookLocalOffset, m_HookLocalDir);

						m_VeinConnectObj->AddHookedCount();

						m_currHookCount++;

						return;
					}
				}
			}
			else if (collidedata->m_contactRigid != m_Convex)
			{
				GFPhysVector3 P1 = (collidedata->m_PointA0 + collidedata->m_PointA1)*0.5f;

				GFPhysVector3 Q1 = (collidedata->m_PointB0 + collidedata->m_PointB1)*0.5f;

				float s, t;

				GFPhysVector3 C1, C2;

				float Dist = GPClosestPtSegmentSegment(P1, collidedata->m_contactinWorld, m_WorldSegmentPointA, m_WorldSegmentPointB, s, t, C1, C2);

				float dotV = (C1 - C2).Dot(m_WorldHookDir);

				if (Dist < m_radius && t > 0.1f && t < 0.9f && dotV > 0)
				{
					if (m_VeinConnectObj->SetHookInfo(collidedata->m_ClusterIndex, collidedata->m_PairIndex))
					{
						m_VeinConnectObj->m_clusters[collidedata->m_ClusterIndex].SetHookOn(m_Convex, C2, m_HookLocalOffset, m_HookLocalDir);

						m_VeinConnectObj->AddHookedCount();
						
						m_currHookCount++;
					}
				}
				else
				{
					float Dist = GPClosestPtSegmentSegment(Q1, collidedata->m_contactinWorld, m_WorldSegmentPointA, m_WorldSegmentPointB, s, t, C1, C2);

					float dotV = (C1 - C2).Dot(m_WorldHookDir);

					if (Dist < m_radius && t > 0.1f && t < 0.9f && dotV > 0)
					{
						if (m_VeinConnectObj->SetHookInfo(collidedata->m_ClusterIndex, collidedata->m_PairIndex))
						{
							m_VeinConnectObj->m_clusters[collidedata->m_ClusterIndex].SetHookOn(m_Convex, C2, m_HookLocalOffset, m_HookLocalDir);

							m_VeinConnectObj->AddHookedCount();

							m_currHookCount++;
						}
					}
				}
			}

        }
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool IsSeperateAxis(const GFPhysVector3 & axis , GFPhysVector3 triangleA[3] , GFPhysVector3 triangleB[3] , float Margin)
{
	Real mintA = GP_INFINITY;
	Real maxtA = -GP_INFINITY;
	for(int va = 0 ; va < 3 ; va++)
	{
		Real t = triangleA[va].Dot(axis);
		if(t < mintA)
		   mintA = t;
		if(t > maxtA)
		   maxtA = t;
	}

	Real mintB = GP_INFINITY;
	Real maxtB = -GP_INFINITY;
	for(int vb = 0 ; vb < 3 ; vb++)
	{
		Real t = triangleB[vb].Dot(axis);
		if(t < mintB)
		   mintB = t;
		if(t > maxtB)
		   maxtB = t;
	}

	mintA -= Margin;
	maxtA += Margin;

	if(mintB > maxtA || mintA > maxtB)
		return true;
	else
		return false;
}
//----------------------------------------------------------------------------------------------------------------------
bool TriangleIntersectWithMargin(GFPhysVector3 triangleA[3], GFPhysVector3 triangleB[3], float margin)
{
	//simple SAT test
	for(int ea = 0 ; ea < 3 ; ea++)
	{
		GFPhysVector3 nodeA0 = triangleA[ea];
		
		GFPhysVector3 nodeA1 = triangleA[(ea+1)%3];

		GFPhysVector3 nA = (nodeA0-nodeA1);

		for(int eb = 0 ; eb < 3 ; eb++)
		{
			GFPhysVector3 nodeB0 = triangleB[eb];
			
			GFPhysVector3 nodeB1 = triangleB[(eb+1)%3];

			GFPhysVector3 nB = (nodeB0-nodeB1);

			GFPhysVector3 Axis = nA.Cross(nB).Normalized();

			//separate axis find !!
			if(IsSeperateAxis(Axis,triangleA,triangleB,margin) )
			   return false;
		}
	}

	GFPhysVector3 normalA = (triangleA[1]-triangleA[0]).Cross(triangleA[2]-triangleA[0]).Normalized();
	GFPhysVector3 normalB = (triangleB[1]-triangleB[0]).Cross(triangleB[2]-triangleB[0]).Normalized();

	if(IsSeperateAxis(normalA,triangleA,triangleB,margin) )
		return false;

	if(IsSeperateAxis(normalB,triangleA,triangleB,margin) )
		return false;

	return true;
}

VeinCollideDataV2::VeinCollideDataV2(int clusterIndex , int pairIndex)
: m_ClusterIndex(clusterIndex) ,
m_PairIndex(pairIndex) ,
m_IsInContact(false)
{

}

VeinV2TreeCollideConvexCallBack::VeinV2TreeCollideConvexCallBack(){}

void VeinV2TreeCollideConvexCallBack::ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
{
	VeinCollideDataV2 * pData = (VeinCollideDataV2*)UserData;

	if(!pData)
		return;

	pData->m_IsInContact = true;

}

ViewDetection::ViewDetection(const Ogre::Vector3 & pos /* = Ogre::Vector3::ZERO */, float minCos /* = 0.95  */, bool isDebug /* = false */)
: m_IsDebug(isDebug) ,
m_AccumulatedTime(0.f) , 
m_LastResult(false) , 
m_IsDetectDir(false) , 
m_IsDetectDist(false) ,
m_Position(pos) , 
m_FaceTo(Ogre::Vector3::ZERO) , 
m_DetectDist(0.f) ,
m_MinCos(minCos)
{
	//debug
	if(m_IsDebug)
	{
		m_pManual = NULL;
		m_pManual = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pManual);
		m_pManual->setDynamic(true);
	}
}

ViewDetection::~ViewDetection()
{
	if(m_IsDebug)
	{
		if(m_pManual)
		{
			m_pManual->detachFromParent();
			MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pManual);
			m_pManual = NULL;
		}
	}
}

bool ViewDetection::Update(float dt , Ogre::Camera * pCamera)
{
	if(m_LastResult)
		m_AccumulatedTime += dt;

	m_LastResult = false;

	if(m_IsDetectDist)
	{
		Ogre::Vector3 camPos = pCamera->getRealPosition();
		if(camPos.squaredDistance(m_Position) > (m_DetectDist * m_DetectDist))
			return false;
	}

	if(m_IsDetectDir)
	{
		
	}
	else
	{
		Ogre::Vector3 viewDir = m_Position - pCamera->getRealPosition();
		Ogre::Vector3 cameraDir = pCamera->getRealDirection();
		viewDir.normalise();
		float dot = viewDir.dotProduct(cameraDir);
		if(dot > m_MinCos)
			m_LastResult = true;
	}
	return m_LastResult;
}

void ViewDetection::Draw(Ogre::Camera * pCamera , int colorIndex)
{
	static Ogre::ColourValue color[2][3] = { { Ogre::ColourValue::Red , Ogre::ColourValue::Green , Ogre::ColourValue::Blue} , 
																	 {Ogre::ColourValue(0,1,1,1) , Ogre::ColourValue(0,1,1,1) ,Ogre::ColourValue(0,1,1,1) } };

	float size = 1.0f;
	if(m_IsDetectDist)
		size = m_DetectDist;

	if(m_pManual)
	{
		Ogre::Vector3 conner[8];
		static Ogre::Vector3 halfX = 0.5 * Ogre::Vector3::UNIT_X;
		static Ogre::Vector3 halfY = 0.5 * Ogre::Vector3::UNIT_Y;
		static Ogre::Vector3 halfZ = 0.5 * Ogre::Vector3::UNIT_Z;
		
		float boxsize = 0.1;

		conner[0] = m_Position +  (- halfX + halfY + halfZ) * boxsize;
		conner[1] = m_Position + (halfX + halfY + halfZ) * boxsize;
		conner[2] = m_Position + (halfX - halfY + halfZ) * boxsize;
		conner[3] = m_Position + (-halfX  - halfY + halfZ) * boxsize;

		conner[4] = m_Position +  (-halfX + halfY - halfZ) * boxsize;
		conner[5] = m_Position + (halfX + halfY - halfZ) * boxsize;
		conner[6] = m_Position + (halfX - halfY - halfZ) * boxsize;
		conner[7] = m_Position + (-halfX  - halfY - halfZ) * boxsize;


		m_pManual->clear();
		m_pManual->begin("BaseWhiteNoLighting" , Ogre::RenderOperation::OT_LINE_LIST);

		m_pManual->position(m_Position);
		m_pManual->colour(Ogre::ColourValue::Red);
		m_pManual->position(m_Position + Ogre::Vector3::UNIT_X * size);
		m_pManual->colour(Ogre::ColourValue::Red);

		m_pManual->position(m_Position);
		m_pManual->colour(Ogre::ColourValue::Green);
		m_pManual->position(m_Position + Ogre::Vector3::UNIT_Y * size);
		m_pManual->colour(Ogre::ColourValue::Green);

		m_pManual->position(m_Position);
		m_pManual->colour(Ogre::ColourValue::Blue);
		m_pManual->position(m_Position + Ogre::Vector3::UNIT_Z * size);
		m_pManual->colour(Ogre::ColourValue::Blue);

//==================================== 1
		m_pManual->position(conner[0]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[1]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[1]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[2]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[2]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[3]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[3]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[0]);
		m_pManual->colour(color[colorIndex][1]);

//========================================2
		m_pManual->position(conner[4]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[5]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[5]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[6]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[6]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[7]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[7]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[4]);
		m_pManual->colour(color[colorIndex][1]);

//========================================3
		m_pManual->position(conner[0]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[4]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[1]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[5]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[3]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[7]);
		m_pManual->colour(color[colorIndex][1]);

		m_pManual->position(conner[2]);
		m_pManual->colour(color[colorIndex][1]);
		m_pManual->position(conner[6]);
		m_pManual->colour(color[colorIndex][1]);

		if(m_IsDetectDir)
		{
			m_pManual->position(m_Position);
			m_pManual->colour(Ogre::ColourValue(0,1,1,1));
			m_pManual->position(m_Position + m_IsDetectDir);
			m_pManual->colour(Ogre::ColourValue(0,1,1,1));
		}

		m_pManual->end();
	}
}