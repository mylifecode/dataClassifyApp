#include "PathputTool.h"
#include "PhysicsWrapper.h"
#include "BasicTraining.h"
#include "stdafx.h"
#include <direct.h> 
#include "ogrewidget.h"
//#include "dtkidtypes.h"
#include "math/GoPhysTransformUtil.h"
#include "EditCamera.h"
#include "CallBackForUnion.h"
#include "Nephrectomy.h"

extern EditorCamera* gEditCamera;

//using namespace dtk;
//note that the dtkID must be a 32-bit unsigned-int.
//The Array-Based Mesh Data Structure depends on this feacture.
typedef unsigned int 		dtkWORD;
typedef unsigned long long 	dtkDWORD;

typedef dtkWORD dtkID;

const dtkID dtkErrorID = 0xFFFFFFFF;

struct dtkID2
{
	dtkID a, b;

	dtkID2(const dtkID &x=0, const dtkID &y=0)
		:a(x), b(y)
	{}

	dtkID2(const dtkID2 &rhs)
		:a(rhs.a), b(rhs.b)
	{}

	const dtkID& operator[](const int &n) const
	{
		switch (n)
		{
		case 0: return a;
		case 1: return b;
		default:
			//dtkAssert(false, OUT_OF_RANGE);
			return b;
		}
	}

	dtkID& operator[](const int &n)
	{
		return const_cast<dtkID&>(
			static_cast<const dtkID2&>(*this)[n]
		);
	}

	bool operator==(const dtkID2 &rhs) const
	{
		return (a == rhs.a && b == rhs.b);
	}

	bool operator<(const dtkID2 &rhs) const
	{
		if (a < rhs.a) return true;
		if (a > rhs.a) return false;

		if (b < rhs.b) return true;
		if (b > rhs.b) return false;

		return false;
	}
};

inline std::ostream& operator<<(std::ostream &out, const dtkID2 &id)
{
	out<<"{"<<id.a<<","<<id.b<<"}";
	return out;
}

inline dtkID2 sort(const dtkID &id0, const dtkID &id1)
{
	return (id0 < id1? dtkID2(id0, id1): dtkID2(id1, id0));
}

inline dtkID2 sort(const dtkID2 &id)
{
	return sort(id.a, id.b);
}

struct dtkID3
{
	dtkID a, b, c;

	dtkID3(const dtkID &x=0, const dtkID &y=0, const dtkID &z=0)
		:a(x), b(y), c(z)
	{}

	dtkID3(const dtkID3 &rhs)
		:a(rhs.a), b(rhs.b), c(rhs.c)
	{}

	const dtkID& operator[](const int &n) const
	{
		switch (n)
		{
		case 0: return a;
		case 1:	return b;
		case 2:	return c;
		default:
			//dtkAssert(false, OUT_OF_RANGE);
			return b;
		}
	}

	dtkID& operator[](const int &n)
	{
		return const_cast<dtkID&>(
			static_cast<const dtkID3&>(*this)[n]
		);
	}

	bool operator==(const dtkID3 &rhs) const
	{
		return (a == rhs.a && b == rhs.b && c == rhs.c);
	}

	bool operator<(const dtkID3 &rhs) const
	{
		if (a < rhs.a) return true;
		if (a > rhs.a) return false;

		if (b < rhs.b) return true;
		if (b > rhs.b) return false;

		if (c < rhs.c) return true;
		if (c > rhs.c) return false;

		return false;
	}
};

inline dtkID3 sort(const dtkID &id0, const dtkID &id1, const dtkID &id2)
{
	dtkID3 rtn(id0, id1, id2);
	if (rtn.a > rtn.b) std::swap(rtn.a, rtn.b);
	if (rtn.a > rtn.c) std::swap(rtn.a, rtn.c);
	if (rtn.b > rtn.c) std::swap(rtn.b, rtn.c);

	return rtn;
}

inline dtkID3 sort(const dtkID3 &id)
{
	return sort(id.a, id.b, id.c);
}

inline std::ostream& operator<<(std::ostream &out, const dtkID3 &id)
{
	out<<"{"<<id.a<<","<<id.b<<","<<id.c<<"}";
	return out;
}
struct dtkID4
{
	dtkID a, b, c, d;

	dtkID4(const dtkID &ta=0, const dtkID &tb=0, 
		const dtkID &tc=0, const dtkID &td=0)
		:a(ta), b(tb), c(tc), d(td)
	{}

	dtkID4(const dtkID4 &rhs)
		:a(rhs.a), b(rhs.b), c(rhs.c), d(rhs.d)
	{}

	const dtkID& operator[](const int &n) const
	{
		switch (n)
		{
		case 0:	return a;
		case 1:	return b;
		case 2:	return c;
		case 3:	return d;
		default:
			//dtkAssert(false, OUT_OF_RANGE);
			return d;
		}
	}

	dtkID& operator[](const int &n)
	{
		return const_cast<dtkID&>(
			static_cast<const dtkID4&>(*this)[n]
		);
	}

	bool operator==(const dtkID4 &rhs) const
	{
		return (a==rhs.a && b==rhs.b && c==rhs.c && d==rhs.d);
	}

	bool operator<(const dtkID4 &rhs) const
	{
		if (a < rhs.a) return true;
		if (a > rhs.a) return false;

		if (b < rhs.b) return true;
		if (b > rhs.b) return false;

		if (c < rhs.c) return true;
		if (c > rhs.c) return false;

		if (d < rhs.d) return true;
		if (d > rhs.d) return false;


		return false;
	}
};

inline dtkID4 sort(const dtkID &a, const dtkID &b, const dtkID &c, const dtkID &d)
{
	dtkID4 rtn(a, b, c, d);

	if (rtn.a > rtn.b) std::swap(rtn.a, rtn.b);
	if (rtn.a > rtn.c) std::swap(rtn.a, rtn.c);
	if (rtn.a > rtn.d) std::swap(rtn.a, rtn.d);
	if (rtn.b > rtn.c) std::swap(rtn.b, rtn.c);
	if (rtn.b > rtn.d) std::swap(rtn.b, rtn.d);
	if (rtn.c > rtn.d) std::swap(rtn.d, rtn.c);

	return rtn;
}

inline dtkID4 sort(const dtkID4 &id)
{
	return sort(id.a, id.b, id.c, id.d);
}

inline std::ostream& operator<<(std::ostream &out, const dtkID4 &id)
{
	out<<"{"<<id.a<<","<<id.b<<","<<id.c<<","<<id.d<<"}";
	return out;
}

//int ConnectPair::s_gpairid = 0;
#define STRIPSIZESCALE 1.8f
static void ExtractPointWeights(Ogre::Vector3 facevert[3] , Ogre::Vector3 p , float weights[3])
{
	Ogre::Vector3 a = facevert[0];

	Ogre::Vector3 b = facevert[1];

	Ogre::Vector3 c = facevert[2];

	Ogre::Vector3 v0 = b-a;

	Ogre::Vector3 v1 = c-a;

	Ogre::Vector3 v2 = p-a;

	double d00 = v0.dotProduct(v0);
	double d01 = v0.dotProduct(v1);
	double d11 = v1.dotProduct(v1);
	double d20 = v2.dotProduct(v0);

	double d21 = v2.dotProduct(v1);
	double denom = d00*d11-d01*d01;

	float v = 0;
	float w = 0;
	if(fabs(denom) > 1e-10F)
	{
		v = (float)((d11 * d20 - d01 * d21) / denom);
		w =  (float)((d00 * d21 - d01 * d20) / denom);
	}

	float u = 1.0f-v-w;

	if(u < 0) u = 0;
	if(u > 1) u = 1;

	if(v < 0) v = 0;
	if(v > 1) v = 1;

	if(w < 0) w = 0;
	if(w > 1) w = 1;

	float sum = u+v+w;
	weights[0] = u / sum;
	weights[1] = v / sum;
	weights[2] = w / sum;
}


bool isTooClose(const std::vector<EditPointInOrgan> & organpoints , EditPointInOrgan current)
{
	for (size_t i = 0 ; i < organpoints.size() ; i++)
	{
		const EditPointInOrgan & orth = organpoints[i];
		
		float delta = (orth.m_position-current.m_position).squaredLength();
		
        if (delta < PathPutTool::s_putdensity * PathPutTool::s_putdensity)
		   return true;
	}
	return false;
}

/*PathPointsInOrgan::PathPointsInOrgan()
{
		m_belongobject = 0;
		m_pathid = -1;
		m_dynobjtype = -1;
}
PathPointsInOrgan::~PathPointsInOrgan()
{

}
*/
static PathPutTool * s_instance = 0;

float PathPutTool::s_putdensity = 0.2f;

PathPutTool * PathPutTool::GetCurrentTool()
{
		return s_instance;
}

PathPutTool::PathPutTool()
{
	//m_currentselorgn = 0;
	
	s_instance = this;
	
	m_edittype = ET_None;

	m_operatepathid = -1;

	m_lastputmouth = Ogre::Vector2::ZERO;

	m_puttedcount = 0;
	
	//m_ingroupeditmode = true;

	//m_pairinedit = 0;
	
	m_selectedconnect = -1;

	s_putdensity = 0.2f;  

	m_hiddenselected = false;
	
	m_hiddenunselected= false;

	m_IsCtrlPressed = false;

	m_EditedObject = 0;

	OgreWidget *  widget = MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);

	widget->AddListener(this);

    m_pDest = 0;

    m_SelectedClusterId.clear();

    m_SendDebuginfo = false;
}
PathPutTool::~PathPutTool()
{
	OgreWidget * widget = MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);
	if (widget)
	    widget->RemoveListener(this);

    if (m_pDest)
    {
        delete[]m_pDest;
        m_pDest = 0;
    }
}
//===========================================================================================================
void PathPutTool::Construct(Ogre::SceneManager * scenemgr , CBasicTraining * hosttrain)
{
	//init Draw Point Object
	m_renderobj = scenemgr->createManualObject();
	Ogre::SceneNode * pDrawPointNode = scenemgr->getRootSceneNode()->createChildSceneNode("PathputTool");
	pDrawPointNode->attachObject(m_renderobj);
	pDrawPointNode->setPosition(0,0,0);

	m_hosttrain = hosttrain;
	m_hosttrain->m_pathtool = this;
}
//===========================================================================================================
void PathPutTool::AddConnectionFromVeinObject(VeinConnectObject * veinobj)
{
	m_EditedObject = veinobj;
    m_ConnectPairs.clear();
    for (size_t v = 0, ni = veinobj->m_clusters.size(); v < ni; v++)
	{
		VeinConnectCluster & cluster = veinobj->m_clusters[v];
		if(cluster.m_Valid)
		   m_ConnectPairs.push_back(v);
	}

    for (size_t i = 0, ni = m_editorlistener.size(); i < ni; i++)
	{
		if(m_editorlistener[i])
		   m_editorlistener[i]->OnConnectLoaded();
	}
}
//=================================================================================================
void PathPutTool::RemoveSelectedCluster()
{
	if(m_selectedconnect >= 0)
	{
		m_EditedObject->DestoryCluster(m_selectedconnect);
		m_selectedconnect = -1;
		RefreshClusterLists();
	}
}
//===================================================================================================
void PathPutTool::RefreshClusterLists()
{
	m_ConnectPairs.clear();//reload connect list
	for(size_t v = 0 ; v < m_EditedObject->m_clusters.size() ; v++)
	{
		VeinConnectCluster & cluster = m_EditedObject->m_clusters[v];
		if(cluster.m_Valid)
		   m_ConnectPairs.push_back(v);
	}

	for (size_t j = 0 ; j < m_editorlistener.size() ; j++)
	{
		if(m_editorlistener[j])
			m_editorlistener[j]->OnConnectPairChanged();
	}
}
//=================================================================================================
void PathPutTool::SetCurrentSelectedConnect(int selectedid)
{
	if(m_selectedconnect >= 0)
	{
	   m_EditedObject->SetClusterColor(m_selectedconnect , Ogre::ColourValue::White);//reset old color
	}
	m_selectedconnect = selectedid;
	m_EditedObject->SetClusterColor(selectedid , Ogre::ColourValue::Red);
}

//===========================================================================================================
void PathPutTool::OnMousePressed(char button ,int mx , int my)//, Ogre::Camera * camera ,  CBasicTraining * pTraining)
{
	if (gEditCamera && gEditCamera->m_IsFreezed == false)
		return;

	if((button & Qt::LeftButton) == 0)
	   return;

	if(m_EditedObject )
	{
		Ogre::RenderWindow * rw =  MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE);
		// Create the ray to test
		Ogre::Real tx = (Ogre::Real)mx / (Ogre::Real) rw->getWidth();
		Ogre::Real ty = (Ogre::Real)my / (Ogre::Real) rw->getHeight();
		Ogre::Camera * camera = m_hosttrain->m_pLargeCamera;

			
		if(m_ClusterInEdit.m_IsInEditState == false)
		{
		   EditPointInOrgan editpoint = PickNearestEditPointInScene(camera , tx , ty);

		   if(editpoint.m_Organ != 0)
		   {
			  m_ClusterInEdit.Reset();
			  m_ClusterInEdit.m_IsInEditState = true;
			  m_ClusterInEdit.m_pairobjA = editpoint.m_Organ;
			  m_ClusterInEdit.m_pairpointA.push_back(editpoint);
		   }
		}
		else if(m_ClusterInEdit.m_finisha == false)
		{
			EditPointInOrgan editpoint = PickNearestEditPointInScene(camera , tx , ty);
			if(editpoint.m_Organ == m_ClusterInEdit.m_pairobjA)
			   m_ClusterInEdit.m_pairpointA.push_back(editpoint);
		}
		else if(m_ClusterInEdit.m_pairobjB == 0)
		{
			EditPointInOrgan editpoint = PickNearestEditPointInScene(camera , tx , ty);
			m_ClusterInEdit.m_pairobjB = editpoint.m_Organ;
			m_ClusterInEdit.m_pairpointB.push_back(editpoint);
		}
		else if(m_ClusterInEdit.m_finishb == false)
		{
			EditPointInOrgan editpoint = PickNearestEditPointInScene(camera , tx , ty);
			if(editpoint.m_Organ == m_ClusterInEdit.m_pairobjB)
			   m_ClusterInEdit.m_pairpointB.push_back(editpoint);
		}
	}
}

//=========================================================================================================================
void PathPutTool::OnMouseMoved(char button , int mx , int my)// , Ogre::Camera * camera ,  CBasicTraining * pTraining) 
{
	m_vecPointPreview.clear();

	if (gEditCamera && gEditCamera->m_IsFreezed == false)
		return;

	Ogre::RenderWindow * rw =  MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE);
	// Create the ray to test
	Ogre::Real tx = (Ogre::Real)mx / (Ogre::Real) rw->getWidth();
	Ogre::Real ty = (Ogre::Real)my / (Ogre::Real) rw->getHeight();
	Ogre::Camera * camera = m_hosttrain->m_pLargeCamera;
	
    if ((button & Qt::LeftButton) != 0)
    {
        if (m_ClusterInEdit.m_IsInEditState)
        {
            if (m_ClusterInEdit.m_finisha == false)
            {
                EditPointInOrgan editpoint = PickNearestEditPointInScene(camera, tx, ty);

                bool tooclose = isTooClose(m_ClusterInEdit.m_pairpointA, editpoint);

                if (editpoint.m_Organ == m_ClusterInEdit.m_pairobjA && tooclose == false)
                    m_ClusterInEdit.m_pairpointA.push_back(editpoint);
            }
            else if (m_ClusterInEdit.m_finishb == false)
            {
                EditPointInOrgan editpoint = PickNearestEditPointInScene(camera, tx, ty);

                bool tooclose = isTooClose(m_ClusterInEdit.m_pairpointB, editpoint);

                if (editpoint.m_Organ == m_ClusterInEdit.m_pairobjB && tooclose == false)
                    m_ClusterInEdit.m_pairpointB.push_back(editpoint);
            }
        }
    }
    else if ((button & Qt::RightButton) != 0)
    {
        if (m_EditedObject == NULL)
        {
            return;
        }
        EditPointInOrgan editpoint = PickNearestEditPointInScene(camera, tx, ty);

        for (int clusterid = 0; clusterid < (int)(m_EditedObject->m_clusters.size()); clusterid++)
        {
            VeinConnectCluster & cluster = m_EditedObject->m_clusters[clusterid];

            for (size_t p = 0; p < 2; p++)
            {
                if (cluster.m_pair[p].m_faceA == editpoint.m_Face || cluster.m_pair[p].m_faceB == editpoint.m_Face)
                {
                    if (m_SelectedClusterId.find(clusterid) == m_SelectedClusterId.end())
                    {
                        m_SelectedClusterId.insert(clusterid);
                        m_EditedObject->SetClusterColor(clusterid, Ogre::ColourValue::Red);
                    }
                }
            }
        }
        /*        for (set<int>::iterator set_iter = m_SelectedClusterid.begin(); set_iter != m_SelectedClusterid.end(); set_iter++)
        {
        std::cout <<"m_SelectedClusterid:"<< *set_iter << std::endl;
        Ogre::LogManager::getSingleton().logMessage(Ogre::String("m_SelectedClusterid is  ") + Ogre::StringConverter::toString(*set_iter));
        } */
    }
    else
    {
        EditPointInOrgan editpoint = PickNearestEditPointInScene(camera, tx, ty);

        if (editpoint.m_Face)
        {
            if (m_vecPointPreview.size() == 0)
            {
                m_vecPointPreview.push_back(editpoint);
            }
            else
            {
                m_vecPointPreview[0] = editpoint;
            }
        }
    }

      
}
//=========================================================================================================================
void PathPutTool::checkClusterInEdit()
{
	if (gEditCamera && gEditCamera->m_IsFreezed == false)
		return;

	if (m_ClusterInEdit.m_IsInEditState)
	{
		if (m_ClusterInEdit.m_finisha == false && m_ClusterInEdit.m_pairobjA != 0)
		{
			if (!m_IsCtrlPressed)
			{
				m_ClusterInEdit.m_finisha = true;
			}
		}

		else if (m_ClusterInEdit.m_finishb == false && m_ClusterInEdit.m_pairobjB != 0)
		{
			if (m_IsCtrlPressed)
			{
				return;
			}


			m_ClusterInEdit.m_finishb = true;

			std::vector<EditPointInOrgan> & PointInA = m_ClusterInEdit.m_pairpointA;

			std::vector<EditPointInOrgan> & PointInB = m_ClusterInEdit.m_pairpointB;

			size_t PointCount = (PointInA.size() < PointInB.size() ? PointInA.size() : PointInB.size());

			if (PointInA.size() == 1 && PointInB.size() == 1)
			{
				EditPointInOrgan tt0 = PointInA[0];
				EditPointInOrgan tt1 = PointInB[0];
				PointInA.clear();
				PointInB.clear();

				PointInA.push_back(tt0);
				PointInA.push_back(tt1);

				PointInB.push_back(tt0);
				PointInB.push_back(tt1);
				PointCount = 2;
			}
			for (size_t c = 0; c < PointCount - 1; c++)//every 2 points create a new cluster
			{
				VeinConnectCluster clusterNew;
				clusterNew.m_ObjAID = m_ClusterInEdit.m_pairobjA->m_OrganID;
				clusterNew.m_ObjBID = m_ClusterInEdit.m_pairobjB->m_OrganID;
				for (size_t p = 0; p < 2; p++)
				{
					size_t t = c + p;

					VeinConnectPair pair;

					pair.m_ObjAType = DOT_VOLMESH;//m_ClusterInEdit.m_pairobjA->m_OrganID;
					pair.m_ObjBType = DOT_VOLMESH;//m_ClusterInEdit.m_pairobjB->m_OrganID;

					pair.m_SuspendDistInFaceB = 0;
					pair.m_SuspendDistInFaceA = 0;

					pair.m_faceA = PointInA[t].m_Face;
					pair.m_faceB = PointInB[t].m_Face;

					pair.m_weightsA[0] = PointInA[t].m_weights[0];
					pair.m_weightsA[1] = PointInA[t].m_weights[1];
					pair.m_weightsA[2] = PointInA[t].m_weights[2];

					pair.m_weightsB[0] = PointInB[t].m_weights[0];
					pair.m_weightsB[1] = PointInB[t].m_weights[1];
					pair.m_weightsB[2] = PointInB[t].m_weights[2];

                    clusterNew.m_pair[p] = pair;
				}

				m_EditedObject->m_clusters.push_back(clusterNew);
			}
			m_ClusterInEdit.m_IsInEditState = false;

			RefreshClusterLists();
		}
	}

}
//=========================================================================================================================
void PathPutTool::OnMouseReleased(char button , int mx , int my)
{
	checkClusterInEdit();

    if (true || (button & Qt::RightButton) != 0)
    {
        for (set<int>::iterator set_iter = m_SelectedClusterId.begin(); set_iter != m_SelectedClusterId.end(); set_iter++)
        {
            m_EditedObject->SetClusterColor(*set_iter, Ogre::ColourValue::White);
        }
        m_SendDebuginfo = true;
        //m_SelectedClusterId.clear();
    }
}

void PathPutTool::OnKeyPress(int whichButton)
{
	if (whichButton == Qt::Key_Control)
	{
		m_IsCtrlPressed = true;
	}

}
void PathPutTool::OnKeyRelease(int whichButton)
{
	if (whichButton == Qt::Key_Control)
	{
		m_IsCtrlPressed = false;
		checkClusterInEdit();
	}

}

//================================================================================================================================
EditPointInOrgan PathPutTool::PickNearestEditPointInScene(Ogre::Camera * camera ,float mousex , float mousey)
{
	MisMedicOrgan_Ordinary * firsthittedobj = 0;

	EditPointInOrgan firsthittedpoint;

	float mint = FLT_MAX;

	std::vector<MisMedicOrganInterface*> oifs;

	m_hosttrain->GetAllOrgan(oifs);

	for(size_t c = 0 ; c < oifs.size() ; c++)
	{
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(oifs[c]);
		if(organ)
		{
			if (organ->m_Transparent)
				continue;

			EditPointInOrgan pickedpoint = PickEditPointOnObject(camera , mousex , mousey , organ);

			if(pickedpoint.m_Face)
			{
				float t = (pickedpoint.m_position-camera->getDerivedPosition()).dotProduct(camera->getDerivedDirection());

				if(t < mint)
				{
					mint = t;
					firsthittedobj = organ;
					firsthittedpoint = pickedpoint;
					firsthittedpoint.m_Organ = organ;
				}
			}
		}
	}

	return firsthittedpoint;
}
//================================================================================================================================
EditPointInOrgan PathPutTool::PickEditPointOnObject(Ogre::Camera * camera ,float mousex , float mousey , MisMedicOrgan_Ordinary * dynobj)
{
	EditPointInOrgan resultpoint;
	
	resultpoint.m_Face = 0;

	Ogre::Ray mouseRay;

	camera->getCameraToViewportRay(mousex , mousey ,&mouseRay);  

	float closest_distance = -1.0f;

	Ogre::Vector3 closest_result;

	bool new_closest_found = false;

	GFPhysSoftBodyFace * hittedface = 0;

	GFPhysSoftBody * sb = dynobj->m_physbody;

	//GFPhysSoftBodyFace * face = sb->GetFaceList();
	
	//while(face)
	for(size_t f = 0 ; f < sb->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * face = sb->GetFaceAtIndex(f);

		Ogre::Vector3 trianglevert[3];

		for(int v = 0 ; v < 3 ; v++)
		{
			GFPhysVector3 temp = face->m_Nodes[v]->m_CurrPosition;
			
			trianglevert[v] = Ogre::Vector3(temp.x(), temp.y(), temp.z());
		}

		std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(mouseRay, trianglevert[0], trianglevert[1], trianglevert[2], true, false);

		if (hit.first)
		{
			if ((closest_distance < 0.0f) || (hit.second < closest_distance))
			{
				// this is the closest so far, save it off
				closest_distance = hit.second;
				hittedface = face;
				new_closest_found = true;
			}
		}
		//face = face->m_Next;
	}
	
	//
	if(hittedface)
	{
		Ogre::Vector3 facevert[3];

		for(int v = 0 ; v <3 ; v++)
		{
			GFPhysVector3 temp = hittedface->m_Nodes[v]->m_CurrPosition;
			facevert[v] = Ogre::Vector3(temp.x(), temp.y(), temp.z());
		}

		closest_result = mouseRay.getPoint(closest_distance);   

		ExtractPointWeights(facevert , closest_result , resultpoint.m_weights);

		resultpoint.m_Face = hittedface;
		resultpoint.m_position = closest_result;
	}

	return resultpoint;
}

void PathPutTool::DrawOneSelectedCluster(const VeinConnectCluster & cluster, Ogre::ColourValue boxcolor , int & startoffset)
{

    for (size_t p = 0; p < 2; p++)
    {
        const VeinConnectPair & pair = cluster.m_pair[p];

		int numvert = drawonepoint(pair.m_CurrPointPosA ,  startoffset , boxcolor);
		startoffset += numvert;

		numvert = drawonepoint(pair.m_CurrPointPosB ,  startoffset , boxcolor);
		startoffset += numvert;
	}
}
//==================================================================================================================
void PathPutTool::DrawOneEditPair(const EditClusterInOrgan & editcluster , Ogre::ColourValue boxcolor , int & startoffset)
{
	if(editcluster.m_pairobjA != 0)
	{
	    for (size_t i = 0 ; i < editcluster.m_pairpointA.size() ; i++)
	    {
			const EditPointInOrgan & pointone = editcluster.m_pairpointA[i];

			GFPhysVector3 temp = pointone.m_Face->m_Nodes[0]->m_CurrPosition*pointone.m_weights[0]
								+pointone.m_Face->m_Nodes[1]->m_CurrPosition*pointone.m_weights[1]
								+pointone.m_Face->m_Nodes[2]->m_CurrPosition*pointone.m_weights[2];

			Ogre::Vector3 location(temp.x() , temp.y() , temp.z());

			int numvert = drawonepoint(location ,  startoffset , boxcolor);

			startoffset += numvert;
	   }
	}

	if(editcluster.m_pairobjB != 0)
	{
		for (size_t i = 0 ; i < editcluster.m_pairpointB.size() ; i++)
		{
			const EditPointInOrgan & pointone = editcluster.m_pairpointB[i];
			
			GFPhysVector3 temp = pointone.m_Face->m_Nodes[0]->m_CurrPosition*pointone.m_weights[0]
								+pointone.m_Face->m_Nodes[1]->m_CurrPosition*pointone.m_weights[1]
								+pointone.m_Face->m_Nodes[2]->m_CurrPosition*pointone.m_weights[2];

			Ogre::Vector3 location(temp.x() , temp.y() , temp.z());
			
			int numvert = drawonepoint(location ,  startoffset , boxcolor);

			startoffset += numvert;
		}
	}

}

void PathPutTool::DrawPreviewPoint(const std::vector<EditPointInOrgan> & vecEditPoint , Ogre::ColourValue boxcolor, int & startoffset)
{
	for (int i = 0; i < (int)vecEditPoint.size(); i++)
	{
		const EditPointInOrgan & pointone = m_vecPointPreview[i];

		GFPhysVector3 temp = pointone.m_Face->m_Nodes[0]->m_CurrPosition*pointone.m_weights[0]
			+ pointone.m_Face->m_Nodes[1]->m_CurrPosition*pointone.m_weights[1]
			+ pointone.m_Face->m_Nodes[2]->m_CurrPosition*pointone.m_weights[2];

		Ogre::Vector3 location(temp.x(), temp.y(), temp.z());

		int numvert = drawonepoint(location, startoffset, boxcolor);
		startoffset += numvert;
	}
	
}

void PathPutTool::HideSelectedOgran()
{
	if (NULL == gEditCamera)
		return;

	if (NULL == gEditCamera->m_SelectOrgan)
		return;

	MisMedicOrgan_Ordinary *pOrgan = gEditCamera->m_SelectOrgan;
	pOrgan->m_Transparent = !pOrgan->m_Transparent;
	if (pOrgan->m_Transparent)
	{
		pOrgan->SetOrdinaryMatrial("MisMedical/StaticTemplateDX11_Transparent");
	}

	else
	{
		Ogre::String nameMaterial = pOrgan->getMaterialName();
		pOrgan->SetOrdinaryMatrial(nameMaterial);
	}
	
}

//==================================================================================================================
int PathPutTool::drawonepoint(Ogre::Vector3 position , int startoffset , Ogre::ColourValue color)
{
	float size = 0.05f;

	Ogre::ColourValue boxcolor = color;

	m_renderobj->position( position+Ogre::Vector3(-size , -size , -size) );   //0
	m_renderobj->colour(boxcolor);

	m_renderobj->position( position+Ogre::Vector3(size , -size , -size) );    //1
	m_renderobj->colour(boxcolor);

	m_renderobj->position( position+Ogre::Vector3(size , -size , size) );    //2
	m_renderobj->colour(boxcolor);

	m_renderobj->position( position+Ogre::Vector3(-size , -size , size) );    //3
	m_renderobj->colour(boxcolor);

	m_renderobj->position( position+Ogre::Vector3(-size , size , -size) );    //4
	m_renderobj->colour(boxcolor);

	m_renderobj->position( position+Ogre::Vector3(size , size , -size) );    //5
	m_renderobj->colour(boxcolor);

	m_renderobj->position( position+Ogre::Vector3(size , size , size) );    //6
	m_renderobj->colour(boxcolor);

	m_renderobj->position( position+Ogre::Vector3(-size , size , size) );    //7
	m_renderobj->colour(boxcolor);

	//index
	m_renderobj->triangle(0+startoffset, 2+startoffset, 1+startoffset);
	m_renderobj->triangle(0+startoffset, 2+startoffset, 3+startoffset);
	m_renderobj->triangle(3+startoffset, 4+startoffset, 0+startoffset);
	m_renderobj->triangle(3+startoffset, 7+startoffset, 4+startoffset);
	m_renderobj->triangle(4+startoffset, 7+startoffset, 6+startoffset);
	m_renderobj->triangle(4+startoffset, 6+startoffset, 5+startoffset);
	m_renderobj->triangle(5+startoffset, 2+startoffset, 1+startoffset);
	m_renderobj->triangle(5+startoffset, 6+startoffset, 2+startoffset);
	m_renderobj->triangle(0+startoffset, 4+startoffset, 1+startoffset);
	m_renderobj->triangle(5+startoffset, 1+startoffset, 4+startoffset);
	m_renderobj->triangle(3+startoffset, 6+startoffset, 7+startoffset);
	m_renderobj->triangle(3+startoffset, 2+startoffset, 6+startoffset);
	
	return 8;
}
//=================================================================================================
void  PathPutTool::AddEventListener(PathPutToolEventListener * listener)
{
		for(size_t i = 0 ; i < m_editorlistener.size() ; i++)
		{
			if(m_editorlistener[i] == listener)
				return;
		}

		m_editorlistener.push_back(listener);
}
//=================================================================================================
void  PathPutTool::RemoveEventListener(PathPutToolEventListener * listener)
{
	for(size_t i = 0 ; i < m_editorlistener.size() ; i++)
	{
		if(m_editorlistener[i] == listener)
		{
			m_editorlistener.erase(m_editorlistener.begin()+i);
				return;
		}	
	}
}

//=================================================================================================
void PathPutTool::Reset()
{
	if(m_ClusterInEdit.m_IsInEditState)
	{
		m_ClusterInEdit.Reset();
	}
}
//=================================================================================================
void  PathPutTool::ChangeSelectedPairSuspendDistInPartA(float heighta)
{
	if(m_selectedconnect >= 0 && m_EditedObject)
	{
		VeinConnectCluster & cluster = m_EditedObject->m_clusters[m_selectedconnect];


        for (size_t p = 0; p < 2; p++)
        {
            VeinConnectPair & pair = cluster.m_pair[p];
			pair.m_SuspendDistInFaceA = heighta;
		}
	}
}

//=================================================================================================
void  PathPutTool::ChangeSelectedPairSuspendDistInPartB(float heighta)
{
	if(m_selectedconnect >= 0 && m_EditedObject)
	{
		VeinConnectCluster & cluster = m_EditedObject->m_clusters[m_selectedconnect];


        for (size_t p = 0; p < 2; p++)
        {
            VeinConnectPair & pair = cluster.m_pair[p];
			pair.m_SuspendDistInFaceB = heighta;
		}
	}
}
//=================================================================================================
void PathPutTool::ChangeSelectedPairSuspendDist(float heighta , float heightb)
{
	ChangeSelectedPairSuspendDistInPartA(heighta);
	ChangeSelectedPairSuspendDistInPartA(heightb);
}

//void PathPutTool::SetObjectInEdit(CDynamicObject * objinedit)
//{
		//m_currentselorgn = objinedit;
	//	m_operatepathid = -1;
//}
//=================================================================================================
void PathPutTool::SetPathToEdit(int pathid)
{
	m_operatepathid = pathid;
	//m_currentselorgn = 0; 
}
//=================================================================================================
void PathPutTool::update(float dt, Ogre::Camera * camera)
{
	Ogre::ColourValue boxcolor = Ogre::ColourValue::Green;

	Ogre::ColourValue selcolor = Ogre::ColourValue::Blue;

	m_renderobj->setDynamic(true);

	m_renderobj->clear();
	
	m_renderobj->begin("Editor/Cube", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	int startoffset = 0;

	if(m_ClusterInEdit.m_IsInEditState)
	   DrawOneEditPair(m_ClusterInEdit , boxcolor , startoffset);

	if (m_ClusterInEdit.m_finisha == false)
		DrawPreviewPoint(m_vecPointPreview, Ogre::ColourValue::Blue, startoffset);
	else if (m_ClusterInEdit.m_finishb == false)
		DrawPreviewPoint(m_vecPointPreview, Ogre::ColourValue::Red, startoffset);
	
	//see if selected any pair
	if(m_selectedconnect >= 0 && m_selectedconnect < (int)m_EditedObject->m_clusters.size())
	{
		const VeinConnectCluster & cluster = m_EditedObject->m_clusters[m_selectedconnect];
		
		DrawOneSelectedCluster(cluster, boxcolor , startoffset);
	}

	m_renderobj->end();

}
//=================================================================================================
void PathPutTool::ExportToObjFile(char * objfilename , char * mapfilename)
{	
	FILE  * fp = fopen(objfilename , "w");

	FILE * fpmap = fopen(mapfilename , "w");

	const std::vector<VeinConnectCluster> & clusters = m_EditedObject->m_clusters;

	int stripcount = 0;

	for(size_t c = 0 ; c < clusters.size() ; c++)
	{

        int paircount = 2;

        fprintf(fpmap, "c  %d %d\n", c, 1);

        //for (int p = 0; p < 1; p++)
        {
            const VeinConnectPair & pair0 = clusters[c].m_pair[0];
            const VeinConnectPair & pair1 = clusters[c].m_pair[1];

			if(pair0.m_Valid && pair1.m_Valid)
			{
				float resizetime = STRIPSIZESCALE;

				Ogre::Vector3 centeradherA = (pair0.m_CurrPointPosA+pair1.m_CurrPointPosA)*0.5f;
				Ogre::Vector3 centeradherB = (pair0.m_CurrPointPosB+pair1.m_CurrPointPosB)*0.5f;

				Ogre::Vector3 adhereA[2];
				Ogre::Vector3 adhereB[2];

				adhereA[0] = centeradherA+(pair0.m_CurrPointPosA-centeradherA)*resizetime;
				adhereA[1] = centeradherA+(pair1.m_CurrPointPosA-centeradherA)*resizetime;

				adhereB[0] = centeradherB+(pair0.m_CurrPointPosB-centeradherB)*resizetime;
				adhereB[1] = centeradherB+(pair1.m_CurrPointPosB-centeradherB)*resizetime;

				//write to map file
				fprintf(fpmap , "v  %f %f %f\n" , adhereA[0].x , adhereA[0].y , adhereA[0].z);
				fprintf(fpmap , "v  %f %f %f\n" , adhereA[1].x , adhereA[1].y , adhereA[1].z);
				fprintf(fpmap , "v  %f %f %f\n" , adhereB[0].x , adhereB[0].y , adhereB[0].z);

				fprintf(fpmap , "v  %f %f %f\n" , adhereB[0].x , adhereB[0].y, adhereB[0].z);
				fprintf(fpmap , "v  %f %f %f\n" , adhereA[1].x , adhereA[1].y, adhereA[1].z);
				fprintf(fpmap , "v  %f %f %f\n" , adhereB[1].x , adhereB[1].y, adhereB[1].z);
				
				//write to object file
				fprintf(fp , "v  %f %f %f\n" , adhereA[0].x , adhereA[0].y, adhereA[0].z);
				fprintf(fp , "v  %f %f %f\n" , adhereA[1].x , adhereA[1].y, adhereA[1].z);
				fprintf(fp , "v  %f %f %f\n" , adhereB[0].x , adhereB[0].y, adhereB[0].z);

				fprintf(fp , "v  %f %f %f\n" , adhereB[0].x , adhereB[0].y, adhereB[0].z);
				fprintf(fp , "v  %f %f %f\n" , adhereA[1].x , adhereA[1].y, adhereA[1].z);
				fprintf(fp , "v  %f %f %f\n" , adhereB[1].x , adhereB[1].y, adhereB[1].z);

				stripcount++;
			}
		}
	}

	for (int s = 0 ; s < stripcount ; s++)
	{
		int offset = 6*s;
		fprintf(fp , "f  %d %d %d %d\n" , offset+1 , offset+2, offset+6, offset+3);
	}

	fclose(fp);
	fclose(fpmap);
}
void PathPutTool::AutoDetectConnectAdhersion(Ogre::String filename , DynObjMap & dynmap)
{
	FILE * fp_forwrite = fopen("c:\\veinconnect\\teststrip.sdf" , "wb");

	Ogre::DataStreamPtr datastream;

	datastream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);

	int paircount = 0;

	datastream->read(&paircount , sizeof(paircount));

	fwrite(&paircount , sizeof(paircount) , 1 , fp_forwrite);

	int autoincpathid = 0;

	for (int i = 0 ; i < paircount ; i++)
	{
		int objAid;

		int objBid;

		int pointcount;

		//read 
		datastream->read(&objAid , sizeof(objAid) );
		datastream->read(&objBid , sizeof(objBid));
		datastream->read(&pointcount , sizeof(pointcount));

		MisMedicOrganInterface *dynObjA , *dynObjB;

		dynObjA = dynmap.find(objAid)->second;

		dynObjB = dynmap.find(objBid)->second;

		int objTypeA = dynObjA->GetCreateInfo().m_objTopologyType;

		int objTypeB = dynObjB->GetCreateInfo().m_objTopologyType;

		//write to file
		fwrite(&objAid , sizeof(objAid) , 1 , fp_forwrite);
		fwrite(&objBid , sizeof(objBid) , 1 , fp_forwrite);
		fwrite(&pointcount , sizeof(pointcount) , 1 , fp_forwrite);

		//read point 
		for (int p = 0 ; p < pointcount ; p++)
		{
			VeinConnectPair pair;

			float suspx , suspy , suspz, susdist;

			int	  Afaceid , Bfaceid;

			float AWeighs[3];

			float BWeighs[3];

			float OriginPosA_X;
			float OriginPosA_Y;
			float OriginPosA_Z;
			datastream->read(&Afaceid ,	sizeof(Afaceid));
			datastream->read(&(AWeighs[0]) , sizeof(AWeighs));
			datastream->read(&suspx , sizeof(float));
			datastream->read(&suspy , sizeof(float));
			datastream->read(&suspz , sizeof(float));
			datastream->read(&susdist , sizeof(float));
			datastream->read(&OriginPosA_X , sizeof(float));
			datastream->read(&OriginPosA_Y , sizeof(float));
			datastream->read(&OriginPosA_Z , sizeof(float));
			//pair.m_tubOffsetA = Ogre::Vector3(suspx , suspy , suspz);
			//pair.m_tubOffsetA.normalise();
			pair.m_SuspendDistInFaceA = susdist;


			float OriginPosB_X;
			float OriginPosB_Y;
			float OriginPosB_Z;
			datastream->read(&Bfaceid ,	sizeof(Bfaceid));
			datastream->read(&(BWeighs[0]) , sizeof(BWeighs));
			datastream->read(&suspx , sizeof(float));
			datastream->read(&suspy , sizeof(float));
			datastream->read(&suspz , sizeof(float));
			datastream->read(&susdist , sizeof(float));

			datastream->read(&OriginPosB_X , sizeof(float));
			datastream->read(&OriginPosB_Y , sizeof(float));
			datastream->read(&OriginPosB_Z , sizeof(float));

			//pair.m_tubOffsetB = Ogre::Vector3(suspx , suspy , suspz);
			//pair.m_tubOffsetB.normalise();
			pair.m_SuspendDistInFaceB = susdist;

			//add constraint
			pair.m_ObjAType = objTypeA;
			pair.m_ObjBType = objTypeB;

			//
			MisMedicOrgan_Ordinary * dynMeshA = 0;
			MisMedicOrgan_Ordinary * dynMeshB = 0;

			GFPhysSoftBodyFace * faceA = 0;
			GFPhysSoftBodyFace * faceB = 0;

			if(objTypeA == DOT_VOLMESH)
			{
				dynMeshA = (MisMedicOrgan_Ordinary*)dynObjA;
				//pair.m_faceA = dynMeshA->m_OriginFaces[Afaceid].m_physface;
				//faceA = pair.m_faceA;
			}
			
			if(objTypeB == DOT_VOLMESH)
			{
				dynMeshB = (MisMedicOrgan_Ordinary*)dynObjB;
				//pair.m_faceB = dynMeshB->m_OriginFaces[Bfaceid].m_physface;
				//faceB = pair.m_faceB;
			}
			//find closet point in objectA
			float closetDist;
			GFPhysSoftBodyFace * closetFace;
			GFPhysVector3 closetPointA , closetPointB;
			int faceAid , faceBid ,materialid;
			ClosetFaceToPoint(dynMeshA,
										GFPhysVector3(OriginPosA_X , OriginPosA_Y , OriginPosA_Z),
										closetDist,
										closetFace,
										closetPointA);
			
			dynMeshA->ExtractFaceIdAndMaterialIdFromUsrData(closetFace , materialid , faceAid);
			//recalculate face id and weights
			Afaceid = faceAid;
			CalcBaryCentric(closetFace->m_Nodes[0]->m_CurrPosition,
				closetFace->m_Nodes[1]->m_CurrPosition,
				closetFace->m_Nodes[2]->m_CurrPosition,
							closetPointA,
							AWeighs[0],
							AWeighs[1],
							AWeighs[2]);

			ClosetFaceToPoint(dynMeshB,
										GFPhysVector3(OriginPosB_X , OriginPosB_Y , OriginPosB_Z),
										closetDist,
										closetFace,
										closetPointB);
			
			dynMeshB->ExtractFaceIdAndMaterialIdFromUsrData(closetFace , materialid , faceBid);
			//recalculate face id and weights
			Bfaceid = faceBid;
			CalcBaryCentric(closetFace->m_Nodes[0]->m_CurrPosition,
				closetFace->m_Nodes[1]->m_CurrPosition,
				closetFace->m_Nodes[2]->m_CurrPosition,
							closetPointB,
							BWeighs[0],
							BWeighs[1],
							BWeighs[2]);

			
			//
			Ogre::Vector3 tubOffsetA;
			Ogre::Vector3 tubOffsetB;

			fwrite(&Afaceid ,	sizeof(Afaceid) , 1 , fp_forwrite);
			fwrite(AWeighs , sizeof(AWeighs) , 1 , fp_forwrite);
			fwrite(&tubOffsetA.x, sizeof(float), 1, fp_forwrite);
			fwrite(&tubOffsetA.y, sizeof(float), 1, fp_forwrite);
			fwrite(&tubOffsetA.z, sizeof(float), 1, fp_forwrite);
			fwrite(&pair.m_SuspendDistInFaceA , sizeof(float) , 1 , fp_forwrite);
			fwrite(&closetPointA.m_x , sizeof(float) , 1 , fp_forwrite);
			fwrite(&closetPointA.m_y , sizeof(float) , 1 , fp_forwrite);
			fwrite(&closetPointA.m_z , sizeof(float) , 1 , fp_forwrite);

			fwrite(&Bfaceid ,	sizeof(Bfaceid) , 1 , fp_forwrite);
			fwrite(BWeighs , sizeof(BWeighs) , 1 , fp_forwrite);
			fwrite(&tubOffsetB.x, sizeof(float), 1, fp_forwrite);
			fwrite(&tubOffsetB.y, sizeof(float), 1, fp_forwrite);
			fwrite(&tubOffsetB.z, sizeof(float), 1, fp_forwrite);
			fwrite(&pair.m_SuspendDistInFaceB , sizeof(float) , 1 , fp_forwrite);
			fwrite(&closetPointB.m_x , sizeof(float) , 1 , fp_forwrite);
			fwrite(&closetPointB.m_y , sizeof(float) , 1 , fp_forwrite);
			fwrite(&closetPointB.m_z , sizeof(float) , 1 , fp_forwrite);


		}
		
	}
	datastream->close();
	fclose(fp_forwrite);
}
void PathPutTool::serialize(Ogre::String filename)
{
	mkdir("c:\\veinconnect");

	//Ogre::StringUtil::splitBaseFilename(filename,xx,xx);
	Ogre::String new_file_name("c:\\veinconnect\\teststrip.vein");

	FILE * fp = fopen(filename.c_str() , "wb");

	FILE * new_fp = fopen(new_file_name.c_str(),"wb");
	
    if (m_EditedObject == NULL)
    {
		MessageBoxA(0, "请选择要导出的筋膜名" , "导出失败" , 0);
        return;
    }


	//eliminate no valid clusters
	std::vector<VeinConnectCluster>::iterator itor = m_EditedObject->m_clusters.begin();
	while(itor != m_EditedObject->m_clusters.end())
	{
		const VeinConnectCluster & clusters = (*itor);


        bool isvalidcluster = false;

        for (int p = 0; p < 2; p++)
        {
            VeinConnectPair pair = clusters.m_pair[p];

            if( (pair.m_CurrPointPosA - pair.m_CurrPointPosB).squaredLength() < GP_EPSILON)
            {
                pair.m_Valid = false;
            }

			if(pair.m_Valid)
			{
				isvalidcluster = true;
				break;
			}
		}

		if(isvalidcluster == false)
		   itor = m_EditedObject->m_clusters.erase(itor);
		else
		   itor++;
	}
	RefreshClusterLists();

	int clustercount = m_EditedObject->m_clusters.size();

	fwrite(&clustercount , sizeof(clustercount) , 1 , fp);

	//write to new vein obj for the count of clusters 
	fwrite(&clustercount , sizeof(clustercount) , 1 , new_fp);

	const std::vector<VeinConnectCluster> & clusters = m_EditedObject->m_clusters;

	for (int c = 0 ; c < clustercount ; c++)
	{

        int objAid = clusters[c].m_ObjAID;

        int objBid = clusters[c].m_ObjBID;

        int ValidPairCount = 0;

        for (int p = 0; p < 2; p++)
        {
            VeinConnectPair pair = clusters[c].m_pair[p];

			 if(pair.m_Valid == true)
			 {
				ValidPairCount++;
			 }
		 }

		 fwrite(&objAid , sizeof(objAid) , 1 , fp);
		
		 fwrite(&objBid , sizeof(objBid) , 1 , fp);

		 fwrite(&ValidPairCount , sizeof(ValidPairCount) , 1 , fp);
		 
		//write to new veinobj for organ organ id
		fwrite(&objAid , sizeof(objAid) , 1 , new_fp);
		
		fwrite(&objBid , sizeof(objBid) , 1 , new_fp);

		int num_vertices = 2;
		fwrite(&num_vertices, sizeof(num_vertices) , 1 , new_fp);
		

		 //write point

        for (int p = 0; p < 2; p++)
        {
            VeinConnectPair pair = clusters[c].m_pair[p];
 
			  if(pair.m_Valid)
			  {
				  int FaceAID = MisMedicOrgan_Ordinary::GetOriginFaceIndexFromUsrData(pair.m_faceA);

				  int FaceBID = MisMedicOrgan_Ordinary::GetOriginFaceIndexFromUsrData(pair.m_faceB);

				  Ogre::Vector3 tubeOffset;

				  fwrite(&FaceAID ,	sizeof(FaceAID) , 1 , fp);
				  fwrite(pair.m_weightsA , sizeof(pair.m_weightsA) , 1 , fp);
				  fwrite(&tubeOffset.x, sizeof(float), 1, fp);
				  fwrite(&tubeOffset.y, sizeof(float), 1, fp);
				  fwrite(&tubeOffset.z, sizeof(float), 1, fp);
				  fwrite(&pair.m_SuspendDistInFaceA , sizeof(float) , 1 , fp);
				  fwrite(&pair.m_CurrPointPosA.x , sizeof(float) , 1 , fp);
				  fwrite(&pair.m_CurrPointPosA.y , sizeof(float) , 1 , fp);
				  fwrite(&pair.m_CurrPointPosA.z , sizeof(float) , 1 , fp);

				  fwrite(&FaceBID ,	sizeof(FaceBID) , 1 , fp);
				  fwrite(pair.m_weightsB , sizeof(pair.m_weightsB) , 1 , fp);
				  fwrite(&tubeOffset.x, sizeof(float), 1, fp);
				  fwrite(&tubeOffset.y, sizeof(float), 1, fp);
				  fwrite(&tubeOffset.z, sizeof(float), 1, fp);
				  fwrite(&pair.m_SuspendDistInFaceB , sizeof(float) , 1 , fp);
				  fwrite(&pair.m_CurrPointPosB.x , sizeof(float) , 1 , fp);
				  fwrite(&pair.m_CurrPointPosB.y , sizeof(float) , 1 , fp);
				  fwrite(&pair.m_CurrPointPosB.z , sizeof(float) , 1 , fp);

				  //write to new 

				  float u = p / 1 ;
				  float v = 0;

				  fwrite(&FaceAID ,	sizeof(FaceAID) , 1 , new_fp);
				  fwrite(pair.m_weightsA , sizeof(pair.m_weightsA) , 1 , new_fp);
				  fwrite(&FaceBID ,	sizeof(FaceBID) , 1 , new_fp);
				  fwrite(pair.m_weightsB , sizeof(pair.m_weightsB) , 1 , new_fp);

				 
				  fwrite(&pair.m_CurrPointPosA.x , sizeof(float) , 1 , new_fp);
				  fwrite(&pair.m_CurrPointPosA.y , sizeof(float) , 1 , new_fp);
				  fwrite(&pair.m_CurrPointPosA.z , sizeof(float) , 1 , new_fp);

				  fwrite(&u , sizeof(float) , 1 , new_fp);
				  fwrite(&v , sizeof(float) , 1 , new_fp);
				  v = 1.0f;

				  fwrite(&pair.m_CurrPointPosB.x , sizeof(float) , 1 , new_fp);
				  fwrite(&pair.m_CurrPointPosB.y , sizeof(float) , 1 , new_fp);
				  fwrite(&pair.m_CurrPointPosB.z , sizeof(float) , 1 , new_fp);

				  fwrite(&u , sizeof(float) , 1 , new_fp);
				  fwrite(&v , sizeof(float) , 1 , new_fp);

			  }
		 }
	}

	fclose(fp);
	fclose(new_fp);

	this->ExportToObjFile("c:\\veinconnect\\veinconnect.obj" , "c:\\veinconnect\\veinconnect.map");
}

static std::vector<std::string> split(std::string str,std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str+=pattern;//扩展字符串以方便操作
    std::string::size_type size = str.size();

    for (std::string::size_type i = 0; i<size; i++)
	{
		pos=str.find(pattern,i);
		if(pos<size)
		{
			std::string s=str.substr(i,pos-i);
			result.push_back(s);
			i=pos+pattern.size()-1;
		}
	}
	return result;
}

class StripRemapData
{
public:
	 int m_belongconnect;
	 int m_index;
	 Ogre::Vector3 m_trianglevert[6];
	 Ogre::Vector2 m_texcoord[4];
};

struct TQuadTexCoord
{
public:
	Ogre::Vector2 texcoor[4];
};
class ConnectionData
{
public:
		std::vector<TQuadTexCoord> quadcoord;
};
static bool isSameTriangle(Ogre::Vector3 srctrivert[3] , Ogre::Vector3 dsttrivert[3] , int resultorder[3])
{
		int order[6][3] = {{0,1,2},{0,2,1},{1,2,0},{1,0,2},{2,1,0},{2,0,1}};
		for (int i = 0 ; i < 6; i++)
		{
            float d0 = (dsttrivert[order[i][0]] - srctrivert[0]).squaredLength();
			 if(d0 < 0.01f)
			 {
                 float d1 = (dsttrivert[order[i][1]] - srctrivert[1]).squaredLength();
				   if(d1 < 0.01f)
				   {
                       float d2 = (dsttrivert[order[i][2]] - srctrivert[2]).squaredLength();
						 if (d2 < 0.01f)
						 {
								resultorder[0] = order[i][0];
								resultorder[1] = order[i][1];
								resultorder[2] = order[i][2];
								return true;
						 }
				   }
			 }
		}
		return false;
}
//======================================================================================================================
void PathPutTool::mapobjfiletoConnection(char * filemap, char * fileobj, char * outputfilename)
{
	FILE  * fpobj = fopen(fileobj , "r");
	FILE  * fpmap = fopen(filemap , "r");

	std::vector<Ogre::Vector3> objvertexpos;
	std::vector<Ogre::Vector2> objvertextexcoord;
	std::vector<dtkID3> faces;
	std::vector<dtkID3> facestex;
	std::vector<StripRemapData> connectstrip;
	char buffer[1024];
	while(1)
	{
		char * p = fgets(buffer, 1024, fpobj);
		if (p == 0)
			break;
		else if(p[0] != 0)
		{
			if (p[0] == 'v')
			{
				if( p[1] ==' ')
				{
					int c = 1;
					while(p[c] == ' ') c++;

					std::string linestr = p+c;
					std::vector<std::string> substring = split(linestr , " ");

					Ogre::Vector3 vertex;
					vertex.x = Ogre::StringConverter::parseReal(substring[0]);
					vertex.y = Ogre::StringConverter::parseReal(substring[1]);
					vertex.z = Ogre::StringConverter::parseReal(substring[2]);

					objvertexpos.push_back(vertex);
				}
				else if(p[1] == 't' )
				{
					int c = 2;
					while(p[c] == ' ') c++;
					std::string linestr = p+c;
					std::vector<std::string> substring = split(linestr , " ");

					Ogre::Vector2 textcoord;
					textcoord.x = Ogre::StringConverter::parseReal(substring[0]);
					textcoord.y = 1-Ogre::StringConverter::parseReal(substring[1]);

					objvertextexcoord.push_back(textcoord);
				}
			}
			else if (p[0] == 'f')
			{
				int c = 1;
				while(p[c] == ' ') c++;
				std::string linestr = p+c;

				dtkID4 quadid;
				dtkID4 quadtex;

				std::vector<std::string> substring = split(linestr , " ");
				std::vector<std::string> facestr = split(substring[0] , "/");
				quadid.a = Ogre::StringConverter::parseInt(facestr[0]);
				quadtex.a = Ogre::StringConverter::parseInt(facestr[1]);

				facestr = split(substring[1] , "/");
				quadid.b = Ogre::StringConverter::parseInt(facestr[0]);
				quadtex.b = Ogre::StringConverter::parseInt(facestr[1]);

				facestr = split(substring[2] , "/");
				quadid.c = Ogre::StringConverter::parseInt(facestr[0]);
				quadtex.c = Ogre::StringConverter::parseInt(facestr[1]);

				facestr = split(substring[3] , "/");
				quadid.d = Ogre::StringConverter::parseInt(facestr[0]);
				quadtex.d = Ogre::StringConverter::parseInt(facestr[1]);

				//2triangle per quad
				faces.push_back(dtkID3(quadid.a , quadid.b , quadid.d));
				faces.push_back(dtkID3(quadid.d , quadid.b , quadid.c));

				facestex.push_back(dtkID3(quadtex.a , quadtex.b , quadtex.d));
				facestex.push_back(dtkID3(quadtex.d , quadtex.b , quadtex.c));

			}
		}
	}
	
	std::vector<ConnectionData> conntects;
	//read map info
	int connectcount = 0;
	while(1)
	{
			char * p = fgets(buffer, 1024, fpmap);
			if (p == 0)
				break;
			
			else if(p[0] != 0)
			{
				if (p[0] == 'c')
				{
						int c = 1;
						
						while(p[c] == ' ') c++;
						
						std::string linestr = p+c;
						
						std::vector<std::string> substring = split(linestr , " ");	
						
						int substripcount = Ogre::StringConverter::parseInt(substring[1]);
						
						//push connect
						ConnectionData conndata;
						conndata.quadcoord.resize(substripcount);
						conntects.push_back(conndata);

						for(int j = 0 ; j < substripcount ; j++)
						{
								StripRemapData sdata;

								Ogre::Vector3 trivertpos[6];
								for (size_t i = 0 ; i < 6 ; i++)
								{
										char * q = fgets(buffer, 1024, fpmap);
										int c = 1;
										while(p[c] == ' ') c++;
										std::string linestr = p+c;
										std::vector<std::string> substring = split(linestr , " ");

										Ogre::Vector3 vertex;
										vertex.x = Ogre::StringConverter::parseReal(substring[0]);
										vertex.y = Ogre::StringConverter::parseReal(substring[1]);
										vertex.z = Ogre::StringConverter::parseReal(substring[2]);
										sdata.m_trianglevert[i] = vertex;
								}
								sdata.m_belongconnect = connectcount;
								sdata.m_index = j;
								connectstrip.push_back(sdata);
						}
						connectcount++;
				}
			}
	}
	//remap
	for (size_t f = 0 ; f < faces.size() ; f++)
	{
		 Ogre::Vector3 vertpos[3];
		 Ogre::Vector2 verttexcoord[3];
		 dtkID3 fid = faces[f];
		
		 dtkID3 tid = facestex[f];
		
		 vertpos[0] = objvertexpos[fid.a-1];
		 verttexcoord[0] = objvertextexcoord[tid.a-1];

		 vertpos[1] = objvertexpos[fid.b-1];
		 verttexcoord[1] = objvertextexcoord[tid.b-1];

		 vertpos[2] = objvertexpos[fid.c-1];
		 verttexcoord[2] = objvertextexcoord[tid.c-1];

		 bool samefinded = false;

		 for (size_t c = 0 ; c < connectstrip.size() ; c++)
		 {
				StripRemapData & cdata = connectstrip[c];

				int sameorder[3];

				bool issame = isSameTriangle(&cdata.m_trianglevert[0] , vertpos , sameorder);
				if (issame == true)
				{
						cdata.m_texcoord[0] = verttexcoord[sameorder[0]];
						cdata.m_texcoord[1] = verttexcoord[sameorder[1]];
						cdata.m_texcoord[2] = verttexcoord[sameorder[2]];
						samefinded = true;
						break;
				}
			
				issame = isSameTriangle(&cdata.m_trianglevert[3] , vertpos , sameorder);
				if (issame == true)
				{
						cdata.m_texcoord[2] = verttexcoord[sameorder[0]];
						cdata.m_texcoord[1] = verttexcoord[sameorder[1]];
						cdata.m_texcoord[3] = verttexcoord[sameorder[2]];
						samefinded = true;
						break;
				}
		 }
		 if(samefinded == false)
		 {
			 MessageBoxA(0, "纹理坐标生成失败" , "纹理坐标生成失败" , 0);
		 }
	}

	//write texcoord to strip object
	for (size_t c = 0 ; c < connectstrip.size() ; c++)
	{
			int connectid = connectstrip[c].m_belongconnect;

			int stripid = connectstrip[c].m_index;

			ConnectionData & connect = conntects[connectid];

			SY_ASSERT(stripid < (int)connect.quadcoord.size());
			TQuadTexCoord & stripcoordtex = connect.quadcoord[stripid];
			stripcoordtex.texcoor[0] = connectstrip[c].m_texcoord[0];
			stripcoordtex.texcoor[1] = connectstrip[c].m_texcoord[1];
			stripcoordtex.texcoor[2] = connectstrip[c].m_texcoord[2];
			stripcoordtex.texcoor[3] = connectstrip[c].m_texcoord[3];
	}
	FILE * ouputfp = fopen(outputfilename , "wb");

	int  connectnum = conntects.size();

	fwrite(&connectnum , sizeof(int) , 1 , ouputfp);

	for (size_t c = 0 ;c < conntects.size(); c++)
	{
			ConnectionData & connect = conntects[c];
			
			int stripnum = connect.quadcoord.size();

			SY_ASSERT(stripnum >=0 && stripnum < 1000);
			
			fwrite(&stripnum , sizeof(int) , 1 , ouputfp);
			
			for (int s = 0 ; s < stripnum ; s++)
			{
				 TQuadTexCoord & stripcoordtex = connect.quadcoord[s];
				 //write 4 texture coordinate
				 for (int t = 0 ; t < 4 ; t++)
				 {
					 Ogre::Vector2 texcoord = stripcoordtex.texcoor[t];
					 fwrite(&texcoord.x , sizeof(float) , 1 , ouputfp);
					 fwrite(&texcoord.y , sizeof(float) , 1 , ouputfp);
				 }
			}
	}

	fclose(fpobj);
	fclose(fpmap);
	fclose(ouputfp);
}
//======================================================================================================================
int PathPutTool::GetRGBFromAreaTesture(float cx, float cy, uint8& red, uint8& green, uint8& blue)
{
    if (m_pDest == NULL)
    {
        Ogre::HardwarePixelBufferSharedPtr pixelBuffer;
        if (m_area.isNull())
        {
            return 0;
        }
        try
        {
            pixelBuffer = m_area->getBuffer();
            pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
        }
        catch (...)
        {
            return 0;
        }

        Ogre::uint* pDest = static_cast<Ogre::uint*>(pixelBuffer->getCurrentLock().data);

        m_pDest = new Ogre::uint[m_areaHeight*m_areaWidth];

        for (int i = 0; i < m_areaHeight*m_areaWidth; i++)
        {
            m_pDest[i] = pDest[i];
        }

        pixelBuffer->unlock();
    }

    int tcx = cx*(m_areaWidth - 1);

    int tcy = cy*(m_areaHeight - 1);

    uint32 pix = *(m_pDest + tcy * m_areaWidth + tcx);

    if (pix == 0)
    {
        return 0;
    }

    uint32 markValue = pix & 0x00ffffff;

    //GetRGBComponent(markValue, red, green, blue);

    blue = markValue & 0x000000ff;
    markValue = markValue >> 8;
    green = markValue & 0x000000ff;
    markValue = markValue >> 8;
    red = markValue & 0x000000ff;
    /*markValue = markValue >> 8;
    uint8 alpha = markValue & 0x000000ff;*/
    return 0;
}
//======================================================================================================================
void PathPutTool::AutoGenAdhersion2()
{    
    CNephrectomy * train = dynamic_cast<CNephrectomy*>(m_hosttrain);
    if (train == NULL)
    {
        return;
    }
    m_area = train->m_Area;
    m_areaHeight = train->m_AreaHeight;
    m_areaWidth = train->m_AreaWidth;

    MisMedicOrgan_Ordinary* Organ_GEROTAS = train->m_Gerotas;
    MisMedicOrgan_Ordinary* Organ_KIDNEY_VESSELS = train->m_KidneyVessels;
    
    //////////////////////////////////////////////////////////////////////////

    Real StickThresHold = 0.30f;

    const GFPhysDBVTree & FaceBTrees = Organ_KIDNEY_VESSELS->m_physbody->GetSoftBodyShape().GetFaceBVTree(false);

    GFPhysDBVTree NodeTreeA;

    GFPhysVector3 Extend(StickThresHold, StickThresHold, StickThresHold);

    for (size_t f = 0; f < Organ_GEROTAS->m_physbody->GetNumFace(); f++)
    {
        GFPhysSoftBodyFace * FaceA = Organ_GEROTAS->m_physbody->GetFaceAtIndex(f);
        GFPhysVector3 CenterPos = (FaceA->m_Nodes[0]->m_UnDeformedPos
            + FaceA->m_Nodes[1]->m_UnDeformedPos
            + FaceA->m_Nodes[2]->m_UnDeformedPos)*0.333f;
        GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(CenterPos - Extend, CenterPos + Extend);
        dbvn->m_UserData = FaceA;        
    }

    //check those node lie in the threshold of A' s tetrahedron
    FaceFaceCenterDistCallBack NodesFaceRangeCB(StickThresHold);

    NodeTreeA.CollideWithDBVTree(FaceBTrees, &NodesFaceRangeCB);

    std::vector<FaceClosetFace> facesVector;

    std::map<GFPhysSoftBodyFace *, FaceClosetFace>::iterator nitor = NodesFaceRangeCB.m_ClostPairs.begin();

    //重新分组得到路径    
    Real weights1[3] = { 1.0f, 0.0f, 0.0f };// { 0.333f, 0.333f, 0.333f };
    Real weights2[3] = { 0.0f, 1.0f, 0.0f };
    Real weights3[3] = { 0.0f, 0.0f, 1.0f };
    
    while (nitor != NodesFaceRangeCB.m_ClostPairs.end())
    {
        Ogre::Vector2 textcoord1 = Organ_KIDNEY_VESSELS->GetTextureCoord(nitor->second.m_FaceB, weights1);
        Ogre::Vector2 textcoord2 = Organ_KIDNEY_VESSELS->GetTextureCoord(nitor->second.m_FaceB, weights2);
        Ogre::Vector2 textcoord3 = Organ_KIDNEY_VESSELS->GetTextureCoord(nitor->second.m_FaceB, weights3);
                             
#if 0
        uint8 red = 0, green = 0, blue = 0;
        GetRGBFromAreaTesture(textcoord1.x, textcoord1.y,red,green,blue);
        if (red == 0 && green == 0 && blue == 255)
        {
            facesVector.push_back(nitor->second);
        }
        else
        {
            GetRGBFromAreaTesture(textcoord2.x, textcoord2.y, red, green, blue);
            if (red == 0 && green == 0 && blue == 255)
            {
                facesVector.push_back(nitor->second);
            }
            else
            {
                GetRGBFromAreaTesture(textcoord3.x, textcoord3.y, red, green, blue);
                if (red == 0 && green == 0 && blue == 255)
                {
                    facesVector.push_back(nitor->second);
                }
            }
        }
#else
        uint8 red0 = 0, red1 = 0, red2 = 0;
        uint8 green0 = 0, green1 = 0, green2 = 0;
        uint8 blue0 = 0, blue1 = 0, blue2 = 0;
        GetRGBFromAreaTesture(textcoord1.x, textcoord1.y, red0, green0, blue0);
        GetRGBFromAreaTesture(textcoord2.x, textcoord2.y, red1, green1, blue1);
        GetRGBFromAreaTesture(textcoord3.x, textcoord3.y, red2, green2, blue2);

        /*if (((red0 == 0 && green0 == 0 && blue0 == 255) && (red1 == 0 && green1 == 0 && blue1 == 255)) ||
            ((red0 == 0 && green0 == 0 && blue0 == 255) && (red2 == 0 && green2 == 0 && blue2 == 255)) ||
            ((red1 == 0 && green1 == 0 && blue1 == 255) && (red2 == 0 && green2 == 0 && blue2 == 255)))*/
        //if ((red0 == 0 && green0 == 0 && blue0 == 255) && (red1 == 0 && green1 == 0 && blue1 == 255) && (red2 == 0 && green2 == 0 && blue2 == 255))
        if (red0 == 0 && green0 == 0 && blue0 == 255)
        {
            facesVector.push_back(nitor->second);
        }

#endif
        nitor++;
    }

    //按照面片中心得到纹理坐标，再由颜色判断得到

    if (m_EditedObject == NULL || facesVector.size() == 0)
    {
        return;
    }
    m_EditedObject->m_clusters.clear();

    for (int c = 0; c < (int)facesVector.size() - 1; c++)
    {
        VeinConnectCluster clusterNew;
        clusterNew.m_ObjAID = 50;
        clusterNew.m_ObjBID = 51;
        //////////////////////////////////////////////////////////////////////////        

        for (int p = 0; p < 2; p++)
        {
            int t = c + p;
            VeinConnectPair pair;

            pair.m_ObjAType = DOT_VOLMESH;//m_ClusterInEdit.m_pairobjA->m_OrganID;
            pair.m_ObjBType = DOT_VOLMESH;//m_ClusterInEdit.m_pairobjB->m_OrganID;

            pair.m_SuspendDistInFaceB = 0;
            pair.m_SuspendDistInFaceA = 0;

            pair.m_faceA = facesVector[t].m_FaceA;
            pair.m_faceB = facesVector[t].m_FaceB;

            pair.m_weightsA[0] = 0.33333f;
            pair.m_weightsA[1] = 0.33333f;
            pair.m_weightsA[2] = 0.33333f;

            pair.m_weightsB[0] = 0.33333f;
            pair.m_weightsB[1] = 0.33333f;
            pair.m_weightsB[2] = 0.33333f;


            clusterNew.m_pair[p]=pair;
        }


        GFPhysVector3 CenterPosA0 = (clusterNew.m_pair[0].m_faceA->m_Nodes[0]->m_UnDeformedPos
            + clusterNew.m_pair[0].m_faceA->m_Nodes[1]->m_UnDeformedPos
            + clusterNew.m_pair[0].m_faceA->m_Nodes[2]->m_UnDeformedPos)*0.333f;
        GFPhysVector3 CenterPosB0 = (clusterNew.m_pair[0].m_faceB->m_Nodes[0]->m_UnDeformedPos
            + clusterNew.m_pair[0].m_faceB->m_Nodes[1]->m_UnDeformedPos
            + clusterNew.m_pair[0].m_faceB->m_Nodes[2]->m_UnDeformedPos)*0.333f;

        GFPhysVector3 CenterPosA1 = (clusterNew.m_pair[1].m_faceA->m_Nodes[0]->m_UnDeformedPos
            + clusterNew.m_pair[1].m_faceA->m_Nodes[1]->m_UnDeformedPos
            + clusterNew.m_pair[1].m_faceA->m_Nodes[2]->m_UnDeformedPos)*0.333f;
        GFPhysVector3 CenterPosB1 = (clusterNew.m_pair[1].m_faceB->m_Nodes[0]->m_UnDeformedPos
            + clusterNew.m_pair[1].m_faceB->m_Nodes[1]->m_UnDeformedPos
            + clusterNew.m_pair[1].m_faceB->m_Nodes[2]->m_UnDeformedPos)*0.333f;

        Real dB = CenterPosB0.Distance(CenterPosB1);
        Real dA = CenterPosA0.Distance(CenterPosA1);
        Real dAB0 = CenterPosA0.Distance(CenterPosB0);
        Real dAB1 = CenterPosA1.Distance(CenterPosB1);
        Real LowerLimit = 0.5f;
        Real UpperLimit = 3.0f;

        if (dA > LowerLimit * StickThresHold && dA < UpperLimit * StickThresHold
            && dB > LowerLimit * StickThresHold && dB < UpperLimit * StickThresHold
            && dAB0 > LowerLimit * StickThresHold && dAB0 < UpperLimit * StickThresHold
            && dAB1 > LowerLimit * StickThresHold && dAB1 < UpperLimit * StickThresHold)
        {
            m_EditedObject->m_clusters.push_back(clusterNew);
        }
    }
    m_ClusterInEdit.m_IsInEditState = false;

    RefreshClusterLists();
}
//======================================================================================================================
void PathPutTool::AutoGenAdhersion1()
{
    CNephrectomy * train = dynamic_cast<CNephrectomy*>(m_hosttrain);
    if (train == NULL)
    {
        return;
    }
    MisMedicOrgan_Ordinary* Organ_GEROTAS = train->m_Gerotas;
    MisMedicOrgan_Ordinary* Organ_KIDNEY_VESSELS = train->m_KidneyVessels;
    std::vector<FaceClosetFace> facesVector;
    //////////////////////////////////////////////////////////////////////////
    std::vector<int> index;
    //细管子上
    /*index.push_back(318);
    index.push_back(319);
    index.push_back(323);
    index.push_back(355);
    index.push_back(331);
    index.push_back(354);
    index.push_back(327);
    index.push_back(364);
    index.push_back(1984);
    index.push_back(1611);
    index.push_back(1623);
    index.push_back(1635);
    index.push_back(1647);
    index.push_back(1659);
    index.push_back(1671);
    index.push_back(1683);
    index.push_back(1695);
    index.push_back(1707);
    index.push_back(168);
    index.push_back(375);
    index.push_back(377);
    index.push_back(385);
    index.push_back(380);
    index.push_back(391);
    index.push_back(390);
    index.push_back(398);
    index.push_back(1987);
    index.push_back(1604);
    index.push_back(1615);
    index.push_back(1627);
    index.push_back(1639);
    index.push_back(1651);
    index.push_back(1663);
    index.push_back(1675);
    index.push_back(1687);
    index.push_back(406);
    index.push_back(405);
    index.push_back(412);
    index.push_back(402);
    index.push_back(410);
    index.push_back(416);
    index.push_back(1608);
    index.push_back(1620);
    index.push_back(1632);
    index.push_back(1644);
    index.push_back(1656);
    index.push_back(1668);
    index.push_back(1680);
    index.push_back(1692);
    index.push_back(1704);*/


    //粗管子背以及出口        
    /*index.push_back(116);
    index.push_back(118);
    index.push_back(120);
    index.push_back(1069);
    index.push_back(1081);
    index.push_back(1093);
    index.push_back(1105);
    index.push_back(1117);
    index.push_back(1129);
    index.push_back(1141);
    index.push_back(1153);
    index.push_back(1165);
    index.push_back(1177);
    index.push_back(1189);
    index.push_back(1201);
    index.push_back(1187);
    index.push_back(1199);
    index.push_back(1185);
    index.push_back(1197);
    index.push_back(1193);
    index.push_back(1205);
    index.push_back(1191);
    index.push_back(1203);*/





    //大管子
    /*index.push_back(1042);
    index.push_back(225);
    index.push_back(228);
    index.push_back(226);
    index.push_back(227);
    index.push_back(230);
    index.push_back(231);
    index.push_back(229);
    index.push_back(107);
    index.push_back(98);
    index.push_back(97);
    index.push_back(93);
    index.push_back(103);
    index.push_back(1038);
    index.push_back(1039);
    index.push_back(1025);
    index.push_back(1024);
    index.push_back(211);
    index.push_back(212);
    index.push_back(210);
    index.push_back(213);
    index.push_back(214);
    index.push_back(221);
    index.push_back(218);
    index.push_back(217);
    index.push_back(224);
    index.push_back(222);*/



    //肾上腺
    index.push_back(289);
    index.push_back(275);
    index.push_back(262);
    index.push_back(265);
    index.push_back(277);

    //肾 
    index.push_back(601);
    index.push_back(607);
    index.push_back(479);
    index.push_back(818);
    index.push_back(864);
    index.push_back(862);
    index.push_back(599);
    index.push_back(605);
    index.push_back(611);
    index.push_back(606);
    index.push_back(483);
    index.push_back(490);
    index.push_back(1042);
    index.push_back(1045);
    index.push_back(816);
    index.push_back(860);
    index.push_back(861);
    index.push_back(596);
    index.push_back(603);
    index.push_back(604);
    index.push_back(610);
    index.push_back(484);
    index.push_back(488);
    index.push_back(480);
    index.push_back(1041);
    index.push_back(225);
    index.push_back(1051);
    index.push_back(1046);
    index.push_back(815);
    index.push_back(866);
    index.push_back(867);
    index.push_back(597);
    index.push_back(602);
    index.push_back(577);
    index.push_back(585);
    index.push_back(609);
    index.push_back(482);
    index.push_back(487);
    index.push_back(489);
    index.push_back(83);



    //remove NO15,22,38,42,51
    Ogre::Real mindist = 1000.0f;
    Ogre::Real maxdist = -1000.0f;

    for (size_type i = 0; i < index.size(); i++)
    {
        GFPhysSoftBodyFace * FaceA = Organ_KIDNEY_VESSELS->m_physbody->GetFaceAtIndex(index[i]);
        GFPhysVector3 CenterPos = (
            FaceA->m_Nodes[0]->m_CurrPosition
            + FaceA->m_Nodes[1]->m_CurrPosition
            + FaceA->m_Nodes[2]->m_CurrPosition) * 0.333f;

        GFPhysVector3 Normal = FaceA->m_FaceNormal;

        Ogre::Vector3 lineSeg0 = GPVec3ToOgre(CenterPos);
        Ogre::Vector3 lineSeg1 = GPVec3ToOgre(CenterPos + Normal * 0.50f);
        Real dist;

        GoPhys::GFPhysSoftBodyFace * bodyface = Organ_GEROTAS->GetRayIntersectFace(lineSeg0, lineSeg1, dist);



        if (bodyface)
        {

            GFPhysVector3 bodyfacebnode = OgreToGPVec3(lineSeg0 + (lineSeg1 - lineSeg0)*dist);
            Real facePtWeights[3];
            CalcBaryCentric(bodyface->m_Nodes[0]->m_UnDeformedPos,
                bodyface->m_Nodes[1]->m_UnDeformedPos,
                bodyface->m_Nodes[2]->m_UnDeformedPos, bodyfacebnode,
                facePtWeights[0],
                facePtWeights[1],
                facePtWeights[2]);

            if (bodyface->m_FaceNormal.Dot(Normal) > 0)
            {
                int k = 0;
            }

            FaceClosetFace nNode(FaceA, dist);
            nNode.m_FaceB = bodyface;
            facesVector.push_back(nNode);

            if (dist < mindist)
            {
                mindist = dist;
            }

            if (dist > maxdist)
            {
                maxdist = dist;
            }
        }
        else
        {
            int k = 1;
        }
    }

    if (m_EditedObject == NULL || facesVector.size() == 0)
    {
        return;
    }
    m_EditedObject->m_clusters.clear();

    for (int c = 0; c < (int)facesVector.size() - 1; c++)
    {
        VeinConnectCluster clusterNew;
        clusterNew.m_ObjAID = 51;
        clusterNew.m_ObjBID = 50;
        //////////////////////////////////////////////////////////////////////////        

        for (int p = 0; p < 2; p++)
        {
            int t = c + p;
            VeinConnectPair pair;

            pair.m_ObjAType = DOT_VOLMESH;//m_ClusterInEdit.m_pairobjA->m_OrganID;
            pair.m_ObjBType = DOT_VOLMESH;//m_ClusterInEdit.m_pairobjB->m_OrganID;

            pair.m_SuspendDistInFaceB = 0;
            pair.m_SuspendDistInFaceA = 0;

            pair.m_faceA = facesVector[t].m_FaceA;
            pair.m_faceB = facesVector[t].m_FaceB;

            pair.m_weightsA[0] = 0.33333f;
            pair.m_weightsA[1] = 0.33333f;
            pair.m_weightsA[2] = 0.33333f;

            pair.m_weightsB[0] = 0.33333f;
            pair.m_weightsB[1] = 0.33333f;
            pair.m_weightsB[2] = 0.33333f;

            clusterNew.m_pair[p] = pair;

        }
        m_EditedObject->m_clusters.push_back(clusterNew);
    }
    m_ClusterInEdit.m_IsInEditState = false;

    RefreshClusterLists();
}
//======================================================================================================================
bool CurrLen2RelativePos(std::vector<Ogre::Vector3>& points, Real totalLen, Real currlen, int& index, Real& weight)
{
    currlen = GPClamped(currlen, 0.0f, totalLen);

    if (fabsf(currlen) < GP_EPSILON)
    {
        index = 0;
        weight = 1.0f;
        return true;
    }

    Real partsum = 0.0f;
    for (int n = 0,ni = (int)points.size() - 1; n <ni; n++)
    {
        Ogre::Vector3 p0 = points[n];
        Ogre::Vector3 p1 = points[n + 1];

        float segLen = (p0 - p1).length();

        partsum += segLen;

        if (segLen > GP_EPSILON && partsum > currlen)
        {
            index = n;
            weight = GPClamped((partsum - currlen) / segLen, 0.0f, 1.0f);
            return true;

        }

        if (fabsf(partsum - currlen) < GP_EPSILON)
        {
            index = n;
            weight = 0.0f;
            return true;
        }
    }
    return false;
}
void PathPutTool::AutoGenAdhersion()
{
    CNephrectomy * train = dynamic_cast<CNephrectomy*>(m_hosttrain);
    if (train == NULL)
    {
        return;
    }
    MisMedicOrgan_Ordinary* Organ_GEROTAS = train->m_Gerotas;
    MisMedicOrgan_Ordinary* Organ_KIDNEY_VESSELS = train->m_KidneyVessels;
    MisMedicOrgan_Ordinary* Organ_MesoColon = train->m_MesoColon;
    std::vector<PointOnFaceClosetPointOnFace> facesVector;
    //大网膜和肾周间筋膜
#if 1
    std::vector<int> Pointindex;

    //大网膜
	Pointindex.push_back(730);
	Pointindex.push_back(1024);
	Pointindex.push_back(507);
	Pointindex.push_back(727);
	Pointindex.push_back(594);
	Pointindex.push_back(704);
	Pointindex.push_back(527);
	Pointindex.push_back(554);
	Pointindex.push_back(711);
	Pointindex.push_back(616);
	Pointindex.push_back(610);
	Pointindex.push_back(745);
	Pointindex.push_back(735);
	Pointindex.push_back(713);
	Pointindex.push_back(595);
	Pointindex.push_back(982);
	Pointindex.push_back(921);
	Pointindex.push_back(533);
	Pointindex.push_back(916);

    for (size_type i = 0, ni = Pointindex.size(); i <ni; i++)
    {
        GFPhysSoftBodyFace * FaceA = Organ_MesoColon->m_physbody->GetFaceAtIndex(Pointindex[i]);
        GFPhysVector3 CenterPos = (
            FaceA->m_Nodes[0]->m_UnDeformedPos
            + FaceA->m_Nodes[1]->m_UnDeformedPos
            + FaceA->m_Nodes[2]->m_UnDeformedPos) * 0.333f;

        Ogre::Vector3 Normal = GPVec3ToOgre(FaceA->m_FaceNormal);
        Ogre::Vector3 lineSeg0 = GPVec3ToOgre(CenterPos) - Normal * 0.05f;

        Ogre::Vector3 lineSeg1 = lineSeg0 + Normal * 1.5f;
        Real distA = 0.0f;
        Real distB = 0.0f;
        GoPhys::GFPhysSoftBodyFace * bodyfaceA = Organ_MesoColon->GetRayIntersectFace(lineSeg0, lineSeg1, distA);
        if (bodyfaceA)
        {
            GFPhysVector3 bodyfacebnodeA = OgreToGPVec3(lineSeg0 + Normal * distA);
            Real facePtWeightsA[3];
            CalcBaryCentric(bodyfaceA->m_Nodes[0]->m_UnDeformedPos,
                bodyfaceA->m_Nodes[1]->m_UnDeformedPos,
                bodyfaceA->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeA,
                facePtWeightsA[0],
                facePtWeightsA[1],
                facePtWeightsA[2]);
            if (facePtWeightsA[0] < 0 || facePtWeightsA[0]>1 ||
                facePtWeightsA[1] < 0 || facePtWeightsA[1]>1 ||
                facePtWeightsA[2] < 0 || facePtWeightsA[2]>1)
            {
                MessageBoxA(0, "Wrong weights!!", "", 0);
            }
            GoPhys::GFPhysSoftBodyFace * bodyfaceB = Organ_GEROTAS->GetRayIntersectFace(lineSeg0, lineSeg1, distB);
            if (bodyfaceB)
            {
                GFPhysVector3 bodyfacebnodeB = OgreToGPVec3(lineSeg0 + Normal * distB); //Normal[j]
                Real facePtWeightsB[3];
                CalcBaryCentric(bodyfaceB->m_Nodes[0]->m_UnDeformedPos,
                    bodyfaceB->m_Nodes[1]->m_UnDeformedPos,
                    bodyfaceB->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeB,
                    facePtWeightsB[0],
                    facePtWeightsB[1],
                    facePtWeightsB[2]);
                if (facePtWeightsB[0] < 0 || facePtWeightsB[0]>1 ||
                    facePtWeightsB[1] < 0 || facePtWeightsB[1]>1 ||
                    facePtWeightsB[2] < 0 || facePtWeightsB[2]>1)
                {
                    MessageBoxA(0, "Wrong weights!!", "", 0);
                }
                PointOnFaceClosetPointOnFace pointpair = PointOnFaceClosetPointOnFace(bodyfaceA, facePtWeightsA, bodyfaceB, facePtWeightsB, distB - distA);
                facesVector.push_back(pointpair);
            }
            else
            {
                MessageBoxA(0, "outer layer return false!", "", 0);
            }
        }
        else
        {
            MessageBoxA(0, "inner layer return false!", "", 0);
        }
    }

#endif
//肾周 和 肾上腺
#if 0
    std::vector<int> Pointindex;
    //肾上腺
    Pointindex.push_back(289);
    Pointindex.push_back(275);
    Pointindex.push_back(262);
    Pointindex.push_back(265);
    Pointindex.push_back(277);

    //肾
    Pointindex.push_back(601);
    Pointindex.push_back(607);
    Pointindex.push_back(479);
    Pointindex.push_back(818);
    Pointindex.push_back(864);
    Pointindex.push_back(862);
    /*Pointindex.push_back(599);*/
    Pointindex.push_back(605);
    Pointindex.push_back(611);
    Pointindex.push_back(606);
    Pointindex.push_back(483);
    Pointindex.push_back(490);
    Pointindex.push_back(1042);
    /*Pointindex.push_back(1045);*/
    Pointindex.push_back(816);
    Pointindex.push_back(860);
    Pointindex.push_back(861);
    Pointindex.push_back(596);
    /*Pointindex.push_back(603);*/
    Pointindex.push_back(604);
    Pointindex.push_back(610);
    Pointindex.push_back(484);
    Pointindex.push_back(488);
    /*Pointindex.push_back(480);*/
    Pointindex.push_back(1041);
    Pointindex.push_back(225);
    Pointindex.push_back(1051);
    Pointindex.push_back(1046);
    Pointindex.push_back(815);
    /*Pointindex.push_back(866);*/
    Pointindex.push_back(867);
    Pointindex.push_back(597);
    Pointindex.push_back(602);
    Pointindex.push_back(577);
    Pointindex.push_back(585);
    Pointindex.push_back(609);
    Pointindex.push_back(482);
    /*Pointindex.push_back(487);*/
    Pointindex.push_back(489);
    Pointindex.push_back(83);


    

    for (size_type i = 0,ni = Pointindex.size(); i < ni ; i++)
    {
        GFPhysSoftBodyFace * FaceA = Organ_KIDNEY_VESSELS->m_physbody->GetFaceAtIndex(Pointindex[i]);
        GFPhysVector3 CenterPos = (
            FaceA->m_Nodes[0]->m_UnDeformedPos
            + FaceA->m_Nodes[1]->m_UnDeformedPos
            + FaceA->m_Nodes[2]->m_UnDeformedPos) * 0.333f;

        Ogre::Vector3 Normal = GPVec3ToOgre(FaceA->m_FaceNormal);
        Ogre::Vector3 lineSeg0 = GPVec3ToOgre(CenterPos) - Normal * 0.01f;

        Ogre::Vector3 lineSeg1 = lineSeg0 + Normal * 0.5f;
        Real distA = 0.0f;
        Real distB = 0.0f;
        GoPhys::GFPhysSoftBodyFace * bodyfaceA = Organ_KIDNEY_VESSELS->GetRayIntersectFace(lineSeg0, lineSeg1, distA);
        if (bodyfaceA)
        {
            GFPhysVector3 bodyfacebnodeA = OgreToGPVec3(lineSeg0 + Normal * distA);
            Real facePtWeightsA[3];
            CalcBaryCentric(bodyfaceA->m_Nodes[0]->m_UnDeformedPos,
                bodyfaceA->m_Nodes[1]->m_UnDeformedPos,
                bodyfaceA->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeA,
                facePtWeightsA[0],
                facePtWeightsA[1],
                facePtWeightsA[2]);
            if (facePtWeightsA[0] < 0 || facePtWeightsA[0]>1 ||
                facePtWeightsA[1] < 0 || facePtWeightsA[1]>1 ||
                facePtWeightsA[2] < 0 || facePtWeightsA[2]>1)
            {
                MessageBoxA(0, "Wrong weights!!", "", 0);
            }
            GoPhys::GFPhysSoftBodyFace * bodyfaceB = Organ_GEROTAS->GetRayIntersectFace(lineSeg0, lineSeg1, distB);
            if (bodyfaceB)
            {
                GFPhysVector3 bodyfacebnodeB = OgreToGPVec3(lineSeg0 + Normal * distB); //Normal[j]
                Real facePtWeightsB[3];
                CalcBaryCentric(bodyfaceB->m_Nodes[0]->m_UnDeformedPos,
                    bodyfaceB->m_Nodes[1]->m_UnDeformedPos,
                    bodyfaceB->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeB,
                    facePtWeightsB[0],
                    facePtWeightsB[1],
                    facePtWeightsB[2]);
                if (facePtWeightsB[0] < 0 || facePtWeightsB[0]>1 ||
                    facePtWeightsB[1] < 0 || facePtWeightsB[1]>1 ||
                    facePtWeightsB[2] < 0 || facePtWeightsB[2]>1)
                {
                    MessageBoxA(0, "Wrong weights!!", "", 0);
                }
                PointOnFaceClosetPointOnFace pointpair = PointOnFaceClosetPointOnFace(bodyfaceA, facePtWeightsA, bodyfaceB, facePtWeightsB, distB - distA);
                facesVector.push_back(pointpair);
            }
            else
            {
                MessageBoxA(0, "outer layer return false!", "", 0);
            }
        }
        else
        {
            MessageBoxA(0, "inner layer return false!", "", 0);
        }
    }

#endif
    //血管和输尿管
//////////////////////////////////////////////////////////////////////////
#if 0
    //细血管
#if 1
    std::vector<Ogre::Vector3> PointsInTube;
    PointsInTube.push_back(Ogre::Vector3(-2.104, 5.286, 6.976));
    PointsInTube.push_back(Ogre::Vector3(-2.173, 5.375, 6.602));
    PointsInTube.push_back(Ogre::Vector3(-2.253, 5.444, 6.109));
    PointsInTube.push_back(Ogre::Vector3(-2.337, 5.518, 5.601));
    PointsInTube.push_back(Ogre::Vector3(-2.472, 5.571, 5.108));
    PointsInTube.push_back(Ogre::Vector3(-2.689, 5.624, 4.520));
    PointsInTube.push_back(Ogre::Vector3(-2.844, 5.645, 4.154));
    PointsInTube.push_back(Ogre::Vector3(-2.973, 5.650, 3.841));
    PointsInTube.push_back(Ogre::Vector3(-3.147, 5.688, 3.412));
    PointsInTube.push_back(Ogre::Vector3(-3.266, 5.719, 3.116));
    PointsInTube.push_back(Ogre::Vector3(-3.416, 5.778, 2.782));
    PointsInTube.push_back(Ogre::Vector3(-3.542, 5.815, 2.522));
    PointsInTube.push_back(Ogre::Vector3(-3.696, 5.889, 2.231));
    PointsInTube.push_back(Ogre::Vector3(-3.807, 5.947, 2.013));
    PointsInTube.push_back(Ogre::Vector3(-4.091, 6.080, 1.484));
    PointsInTube.push_back(Ogre::Vector3(-4.338, 6.170, 1.044));
    PointsInTube.push_back(Ogre::Vector3(-4.505, 6.244, 0.662));
    PointsInTube.push_back(Ogre::Vector3(-4.642, 6.270, 0.238));
    PointsInTube.push_back(Ogre::Vector3(-4.743, 6.281, -0.201));
    PointsInTube.push_back(Ogre::Vector3(-4.809, 6.281, -0.572));
    PointsInTube.push_back(Ogre::Vector3(-4.857, 6.276, -1.055));
    PointsInTube.push_back(Ogre::Vector3(-4.845, 6.307, -1.531));
    PointsInTube.push_back(Ogre::Vector3(-4.811, 6.329, -1.923));
    PointsInTube.push_back(Ogre::Vector3(-4.783, 6.398, -2.310));
    PointsInTube.push_back(Ogre::Vector3(-4.745, 6.451, -2.671));
    PointsInTube.push_back(Ogre::Vector3(-4.718, 6.525, -3.084));
    PointsInTube.push_back(Ogre::Vector3(-4.703, 6.599, -3.349));
    PointsInTube.push_back(Ogre::Vector3(-4.677, 6.742, -3.720));

    std::vector<Ogre::Vector3> Normal;
    Normal.push_back(Ogre::Vector3(-0.172,-0.321,0).normalisedCopy());
    Normal.push_back(Ogre::Vector3(0.54,0.152,0).normalisedCopy());
  
#if 1

    float totalLen = 0;
    for (int n = 0,ni = PointsInTube.size() - 1; n < ni; n++)
    {
        Ogre::Vector3 p0 = PointsInTube[n];
        Ogre::Vector3 p1 = PointsInTube[n + 1];

        float t = (p0 - p1).length();
        totalLen += t;
    }
    int k = 11;
    int index;
    Real weight;
    for (size_type j = 0,nj=Normal.size(); j <nj ; j++)
    {        
        for (int i = 0; i < k; i++)
        {            
            Real currentlength = i * totalLen / k;
            if (CurrLen2RelativePos(PointsInTube, totalLen, currentlength, index, weight) == false)
            {
                return;
            }
            Ogre::Vector3 lineSeg0 = PointsInTube[index] * weight + PointsInTube[index + 1] * (1 - weight);           
#else     
    for (size_type j = 0,nj = Normal.size(); j <nj ; j++)
    {
        for (size_type i = 0,ni=PointsInTube.size(); i < ni; i++)
        {
            Ogre::Vector3 lineSeg0 = PointsInTube[i];
#endif
            Ogre::Vector3 lineSeg1 = lineSeg0 + Normal[j] * 1.0f;
            Real distA = 0.0f;
            Real distB = 0.0f;
            GoPhys::GFPhysSoftBodyFace * bodyfaceA = Organ_KIDNEY_VESSELS->GetRayIntersectFace(lineSeg0, lineSeg1, distA);
            if (bodyfaceA)
            {
                GFPhysVector3 bodyfacebnodeA = OgreToGPVec3(lineSeg0 + Normal[j] * distA);//Normal[j]
                Real facePtWeightsA[3];
                CalcBaryCentric(bodyfaceA->m_Nodes[0]->m_UnDeformedPos,
                    bodyfaceA->m_Nodes[1]->m_UnDeformedPos,
                    bodyfaceA->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeA,
                    facePtWeightsA[0],
                    facePtWeightsA[1],
                    facePtWeightsA[2]);
                if (facePtWeightsA[0] < 0 || facePtWeightsA[0]>1 ||
                    facePtWeightsA[1] < 0 || facePtWeightsA[1]>1 ||
                    facePtWeightsA[2] < 0 || facePtWeightsA[2]>1)
                {
                    MessageBoxA(0, "Wrong weights!!", "", 0);
                }
                GoPhys::GFPhysSoftBodyFace * bodyfaceB = Organ_GEROTAS->GetRayIntersectFace(lineSeg0, lineSeg1, distB);
                if (bodyfaceB)
                {
                    GFPhysVector3 bodyfacebnodeB = OgreToGPVec3(lineSeg0 + Normal[j] * distB); //Normal[j]
                    Real facePtWeightsB[3];
                    CalcBaryCentric(bodyfaceB->m_Nodes[0]->m_UnDeformedPos,
                        bodyfaceB->m_Nodes[1]->m_UnDeformedPos,
                        bodyfaceB->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeB,
                        facePtWeightsB[0],
                        facePtWeightsB[1],
                        facePtWeightsB[2]);
                    if (facePtWeightsB[0]<0 || facePtWeightsB[0]>1 ||
                        facePtWeightsB[1]<0 || facePtWeightsB[1]>1 || 
                        facePtWeightsB[2]<0 || facePtWeightsB[2]>1 )
                    {
                        MessageBoxA(0, "Wrong weights!!", "", 0);
                    }
                    PointOnFaceClosetPointOnFace pointpair = PointOnFaceClosetPointOnFace(bodyfaceA, facePtWeightsA, bodyfaceB, facePtWeightsB, distB - distA);
                    facesVector.push_back(pointpair);
                }
                else
                {
                    MessageBoxA(0, "outer layer return false!", "", 0);
                }
            }
            else
            {
                MessageBoxA(0, "inner layer return false!", "", 0);
            }
        }                  
    }
#endif


    //大管子合并部分
#if 1
    //////////////////////////////////////////////////////////////////////////
    std::vector<Ogre::Vector3> PointsInTube2;
    PointsInTube2.push_back(Ogre::Vector3(-4.785, 6.670, -4.651));
    PointsInTube2.push_back(Ogre::Vector3(-4.052, 6.385, -4.467));
    PointsInTube2.push_back(Ogre::Vector3(-3.449, 6.057, -4.267));
    PointsInTube2.push_back(Ogre::Vector3(-2.903, 5.782, -4.105));
    PointsInTube2.push_back(Ogre::Vector3(-2.357, 5.496, -3.874));
    PointsInTube2.push_back(Ogre::Vector3(-1.847, 5.189, -3.653));

    std::vector<Ogre::Vector3> Normal2;
    Normal2.push_back(Ogre::Vector3(0, 0.605, 0.645).normalisedCopy());
    Normal2.push_back(Ogre::Vector3(0, 0.605, -0.961).normalisedCopy());

#if 0

    totalLen = 0;
    for (int n = 0,ni =PointsInTube2.size() - 1; n <ni ; n++)
    {
        Ogre::Vector3 p0 = PointsInTube2[n];
        Ogre::Vector3 p1 = PointsInTube2[n + 1];

        float t = (p0 - p1).length();
        totalLen += t;
    }
    k = 5;    
    for (size_type j = 0,nj=Normal2.size(); j < nj; j++)
    {        
        for (int i = 0; i < k; i++)
        {            
            Real currentlength = i * totalLen / k;
            if (CurrLen2RelativePos(PointsInTube2, totalLen, currentlength, index, weight) == false)
            {
                return;
            }
            Ogre::Vector3 lineSeg0 = PointsInTube2[index] * weight + PointsInTube2[index + 1] * (1 - weight);           
#else
    for (size_type j = 0,nj=Normal2.size(); j < nj; j++)
    {
        for (size_type i = 0,ni=PointsInTube2.size(); i <ni ; i++)
        {
            Ogre::Vector3 lineSeg0 = PointsInTube2[i];
#endif            
            Ogre::Vector3 lineSeg1 = lineSeg0 + Normal2[j] * 2.50f;
            Real distA = 0.0f;
            Real distB = 0.0f;

            GoPhys::GFPhysSoftBodyFace * bodyfaceA = Organ_KIDNEY_VESSELS->GetRayIntersectFace(lineSeg0, lineSeg1, distA);

            if (bodyfaceA)
            {
                GFPhysVector3 bodyfacebnodeA = OgreToGPVec3(lineSeg0 + Normal2[j] * distA);//Normal[j]
                Real facePtWeightsA[3];
                CalcBaryCentric(bodyfaceA->m_Nodes[0]->m_UnDeformedPos,
                    bodyfaceA->m_Nodes[1]->m_UnDeformedPos,
                    bodyfaceA->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeA,
                    facePtWeightsA[0],
                    facePtWeightsA[1],
                    facePtWeightsA[2]);
                if (facePtWeightsA[0] < 0 || facePtWeightsA[0]>1 ||
                    facePtWeightsA[1] < 0 || facePtWeightsA[1]>1 ||
                    facePtWeightsA[2] < 0 || facePtWeightsA[2]>1)
                {
                    MessageBoxA(0, "Wrong weights!!", "", 0);
                }

                GoPhys::GFPhysSoftBodyFace * bodyfaceB = Organ_GEROTAS->GetRayIntersectFace(lineSeg0, lineSeg1, distB);
                if (bodyfaceB)
                {
                    GFPhysVector3 bodyfacebnodeB = OgreToGPVec3(lineSeg0 + Normal2[j] * distB); //Normal[j]
                    Real facePtWeightsB[3];
                    CalcBaryCentric(bodyfaceB->m_Nodes[0]->m_UnDeformedPos,
                        bodyfaceB->m_Nodes[1]->m_UnDeformedPos,
                        bodyfaceB->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeB,
                        facePtWeightsB[0],
                        facePtWeightsB[1],
                        facePtWeightsB[2]);
                    if (facePtWeightsB[0] < 0 || facePtWeightsB[0]>1 ||
                        facePtWeightsB[1] < 0 || facePtWeightsB[1]>1 ||
                        facePtWeightsB[2] < 0 || facePtWeightsB[2]>1)
                    {
                        MessageBoxA(0, "Wrong weights!!", "", 0);
                    }

                    PointOnFaceClosetPointOnFace pointpair = PointOnFaceClosetPointOnFace(bodyfaceA, facePtWeightsA, bodyfaceB, facePtWeightsB, distB - distA);

                    facesVector.push_back(pointpair);
                }
                else
                {
                    MessageBoxA(0, "outer layer return false!", "", 0);
                }
            }
            else
            {
                MessageBoxA(0, "inner layer return false!", "", 0);
            }
        }
    }
#endif

    //输尿管
#if 1
    std::vector<Ogre::Vector3> PointsInTube3;


    PointsInTube3.push_back(Ogre::Vector3(-3.149, 5.250, 7.018));
    PointsInTube3.push_back(Ogre::Vector3(-3.178, 5.250, 6.677));
    PointsInTube3.push_back(Ogre::Vector3(-3.208, 5.253, 6.339));
    PointsInTube3.push_back(Ogre::Vector3(-3.225, 5.253, 5.988));
    PointsInTube3.push_back(Ogre::Vector3(-3.257, 5.253, 5.651));
    PointsInTube3.push_back(Ogre::Vector3(-3.288, 5.253, 5.310));
    PointsInTube3.push_back(Ogre::Vector3(-3.323, 5.253, 4.972));
    PointsInTube3.push_back(Ogre::Vector3(-3.371, 5.253, 4.661));
    PointsInTube3.push_back(Ogre::Vector3(-3.423, 5.230, 4.324));
    PointsInTube3.push_back(Ogre::Vector3(-3.492, 5.223, 4.027));
    PointsInTube3.push_back(Ogre::Vector3(-3.563, 5.209, 3.666));
    PointsInTube3.push_back(Ogre::Vector3(-3.625, 5.193, 3.382));
    PointsInTube3.push_back(Ogre::Vector3(-3.694, 5.155, 3.044));
    PointsInTube3.push_back(Ogre::Vector3(-3.727, 5.095, 2.737));
    PointsInTube3.push_back(Ogre::Vector3(-3.747, 5.046, 2.477));
    PointsInTube3.push_back(Ogre::Vector3(-3.760, 4.986, 2.204));
    PointsInTube3.push_back(Ogre::Vector3(-3.755, 4.922, 1.968));
    PointsInTube3.push_back(Ogre::Vector3(-3.734, 4.862, 1.745));
    PointsInTube3.push_back(Ogre::Vector3(-3.702, 4.770, 1.498));
    PointsInTube3.push_back(Ogre::Vector3(-3.663, 4.700, 1.223));
    PointsInTube3.push_back(Ogre::Vector3(-3.616, 4.612, 0.945));
    PointsInTube3.push_back(Ogre::Vector3(-3.552, 4.527, 0.644));
    PointsInTube3.push_back(Ogre::Vector3(-3.503, 4.436, 0.213));

    std::vector<Ogre::Vector3> Normal3;

    Normal3.push_back(Ogre::Vector3(-1, 0, 0).normalisedCopy());
    Normal3.push_back(Ogre::Vector3(1, 0, 0).normalisedCopy());
#if 1

    totalLen = 0;
    for (int n = 0,ni=PointsInTube3.size() - 1; n < ni; n++)
    {
        Ogre::Vector3 p0 = PointsInTube3[n];
        Ogre::Vector3 p1 = PointsInTube3[n + 1];

        float t = (p0 - p1).length();
        totalLen += t;
    }
    k = 9;
    for (size_type j = 0; j < Normal3.size(); j++)
    {        
        for (int i = 0; i < k; i++)
        {            
            Real currentlength = i * totalLen / k;
            if (CurrLen2RelativePos(PointsInTube3, totalLen, currentlength, index, weight) == false)
            {
                return;
            }
            Ogre::Vector3 lineSeg0 = PointsInTube3[index] * weight + PointsInTube3[index + 1] * (1 - weight);           
#else
    for (size_type j = 0; j < Normal3.size(); j++)
    {
        for (size_type i = 0,ni=PointsInTube3.size(); i <ni ; i++)
        {
            Ogre::Vector3 lineSeg0 = PointsInTube3[i];
#endif
            
            Ogre::Vector3 lineSeg1 = lineSeg0 + Normal3[j] * 1.50f;
            Real distA = 0.0f;
            Real distB = 0.0f;

            GoPhys::GFPhysSoftBodyFace * bodyfaceA = Organ_KIDNEY_VESSELS->GetRayIntersectFace(lineSeg0, lineSeg1, distA);

            if (bodyfaceA)
            {
                GFPhysVector3 bodyfacebnodeA = OgreToGPVec3(lineSeg0 + Normal3[j] * distA);//Normal[j]
                Real facePtWeightsA[3];
                CalcBaryCentric(bodyfaceA->m_Nodes[0]->m_UnDeformedPos,
                    bodyfaceA->m_Nodes[1]->m_UnDeformedPos,
                    bodyfaceA->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeA,
                    facePtWeightsA[0],
                    facePtWeightsA[1],
                    facePtWeightsA[2]);
                if (facePtWeightsA[0] < 0 || facePtWeightsA[0]>1 ||
                    facePtWeightsA[1] < 0 || facePtWeightsA[1]>1 ||
                    facePtWeightsA[2] < 0 || facePtWeightsA[2]>1)
                {
                    MessageBoxA(0, "Wrong weights!!", "", 0);
                }

                GoPhys::GFPhysSoftBodyFace * bodyfaceB = Organ_GEROTAS->GetRayIntersectFace(lineSeg0, lineSeg1, distB);
                if (bodyfaceB)
                {
                    GFPhysVector3 bodyfacebnodeB = OgreToGPVec3(lineSeg0 + Normal3[j] * distB); //Normal[j]
                    Real facePtWeightsB[3];
                    CalcBaryCentric(bodyfaceB->m_Nodes[0]->m_UnDeformedPos,
                        bodyfaceB->m_Nodes[1]->m_UnDeformedPos,
                        bodyfaceB->m_Nodes[2]->m_UnDeformedPos, bodyfacebnodeB,
                        facePtWeightsB[0],
                        facePtWeightsB[1],
                        facePtWeightsB[2]);
                    if (facePtWeightsB[0] < 0 || facePtWeightsB[0]>1 ||
                        facePtWeightsB[1] < 0 || facePtWeightsB[1]>1 ||
                        facePtWeightsB[2] < 0 || facePtWeightsB[2]>1)
                    {
                        MessageBoxA(0, "Wrong weights!!", "", 0);
                    }

                    PointOnFaceClosetPointOnFace pointpair = PointOnFaceClosetPointOnFace(bodyfaceA, facePtWeightsA, bodyfaceB, facePtWeightsB, distB - distA);

                    facesVector.push_back(pointpair);
                }
                else
                {                    
                    MessageBoxA(0, "outer layer return false!", "", 0);
                }
            }
            else
            {
                MessageBoxA(0, "inner layer return false!", "", 0);
            }
        }
    }

#endif



#endif
    //////////////////////////////////////////////////////////////////////////
    if (m_EditedObject == NULL || facesVector.size() == 0)
    {
        return;
    }
    m_EditedObject->m_clusters.clear();
    for (int c = 0; c < (int)facesVector.size() - 1; c++)
    {
        VeinConnectCluster clusterNew;
        clusterNew.m_ObjAID = 52;
        clusterNew.m_ObjBID = 50;

        //clusterNew.m_ObjAID = 51;
        //clusterNew.m_ObjBID = 50;
        //////////////////////////////////////////////////////////////////////////        

        for (int p = 0; p < 2; p++)
        {
            int t = c + p;
            VeinConnectPair pair;

            pair.m_ObjAType = DOT_VOLMESH;//m_ClusterInEdit.m_pairobjA->m_OrganID;
            pair.m_ObjBType = DOT_VOLMESH;//m_ClusterInEdit.m_pairobjB->m_OrganID;

            pair.m_SuspendDistInFaceB = 0;
            pair.m_SuspendDistInFaceA = 0;

            pair.m_faceA = facesVector[t].m_FaceA;
            pair.m_faceB = facesVector[t].m_FaceB;

            pair.m_weightsA[0] = facesVector[t].m_weightA[0];
            pair.m_weightsA[1] = facesVector[t].m_weightA[1];
            pair.m_weightsA[2] = facesVector[t].m_weightA[2];

            pair.m_weightsB[0] = facesVector[t].m_weightB[0];
            pair.m_weightsB[1] = facesVector[t].m_weightB[1];
            pair.m_weightsB[2] = facesVector[t].m_weightB[2];

            clusterNew.m_pair[p] = pair;

        }
        GFPhysVector3 CenterPosA0 = (clusterNew.m_pair[0].m_faceA->m_Nodes[0]->m_UnDeformedPos
            + clusterNew.m_pair[0].m_faceA->m_Nodes[1]->m_UnDeformedPos
            + clusterNew.m_pair[0].m_faceA->m_Nodes[2]->m_UnDeformedPos)*0.333f;
        GFPhysVector3 CenterPosB0 = (clusterNew.m_pair[0].m_faceB->m_Nodes[0]->m_UnDeformedPos
            + clusterNew.m_pair[0].m_faceB->m_Nodes[1]->m_UnDeformedPos
            + clusterNew.m_pair[0].m_faceB->m_Nodes[2]->m_UnDeformedPos)*0.333f;

        GFPhysVector3 CenterPosA1 = (clusterNew.m_pair[1].m_faceA->m_Nodes[0]->m_UnDeformedPos
            + clusterNew.m_pair[1].m_faceA->m_Nodes[1]->m_UnDeformedPos
            + clusterNew.m_pair[1].m_faceA->m_Nodes[2]->m_UnDeformedPos)*0.333f;
        GFPhysVector3 CenterPosB1 = (clusterNew.m_pair[1].m_faceB->m_Nodes[0]->m_UnDeformedPos
            + clusterNew.m_pair[1].m_faceB->m_Nodes[1]->m_UnDeformedPos
            + clusterNew.m_pair[1].m_faceB->m_Nodes[2]->m_UnDeformedPos)*0.333f;

        Real dB = CenterPosB0.Distance(CenterPosB1);
        Real dA = CenterPosA0.Distance(CenterPosA1);
        Real dAB0 = CenterPosA0.Distance(CenterPosB0);
        Real dAB1 = CenterPosA1.Distance(CenterPosB1);
        Real UpperLimit = 6.5f;

        if (dA < UpperLimit && dB < UpperLimit && dAB0 < UpperLimit && dAB1 < UpperLimit)
        {
            m_EditedObject->m_clusters.push_back(clusterNew);
        }
        else
        {
            int kkk = 1;
        }
        //m_EditedObject->m_clusters.push_back(clusterNew);
    }
    m_ClusterInEdit.m_IsInEditState = false;

    RefreshClusterLists();
}
