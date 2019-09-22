#include "MisCTool_PluginClampConnectPair.h"
#include "Tool.h"
#include "VeinConnectObject.h"
#include "CustomCollision.h"

#define DissectingFrocespActionAside 20.0f

MisCTool_PluginClampConnectPair::MisCTool_PluginClampConnectPair(CTool * tool) 
: MisMedicCToolPluginInterface(tool)
{
}

MisCTool_PluginClampConnectPair::~MisCTool_PluginClampConnectPair()
{
}

//==================================================================================================
void MisCTool_PluginClampConnectPair::OneFrameUpdateStarted(float timeelapsed)
{
	//
	float toolshaft = m_ToolObject->GetShaftAside();

	GFPhysRigidBody * leftrigid  = m_ToolObject->m_lefttoolpartconvex.m_rigidbody;

	GFPhysRigidBody * rightrigid = m_ToolObject->m_righttoolpartconvex.m_rigidbody;

	GFPhysVector3 cutLineVec = m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[1]-m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[0];
	float leftlinelensqr = cutLineVec.Dot(cutLineVec);

	cutLineVec = m_ToolObject->m_CutBladeRight.m_LinePointsWorld[1]-m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0];

	float rightlinelensqr = cutLineVec.Dot(cutLineVec);

	float aside = m_ToolObject->GetShaftAside();
	
	//check all pair state
	std::vector<VeinConnectCotactMe>::iterator itor = m_contacts.begin();

	while(itor != m_contacts.end())
	{
		const VeinConnectCotactMe & contact = (*itor);

		VeinConnectObject * veinobj = contact.m_Veinconnect;

		int cluster = contact.m_ClusterId;
		
		int pairid = contact.m_PairId;
		
		Ogre::Vector3 temp = contact.m_LocalPoint;

		GFPhysVector3  localpoint(temp.x , temp.y , temp.z);
		const VeinConnectPair & connpair = veinobj->GetConnectPair(cluster , pairid);

		bool removecontact = false;
		
		if(connpair.m_Valid)//still valid pair may be delete
		{
			if(connpair.m_BVNode)
			{
				VeinCollideData * cdata = (VeinCollideData *)connpair.m_BVNode->m_UserData;

				bool contactleft  = (cdata->m_contactRigid == leftrigid);
				
				bool contactright = (cdata->m_contactRigid == rightrigid);

				if(cdata && cdata->m_contactRigid && (cdata->m_contactRigid == leftrigid || cdata->m_contactRigid == rightrigid) )
				{
					GFPhysVector3 worldcontactpoint = cdata->m_contactRigid->GetWorldTransform()*localpoint;

					if (contact.m_toolOpenAside + DissectingFrocespActionAside < aside)
					{
                        veinobj->DestoryCluster(cluster);
						removecontact = true;
					}
					else if(toolshaft == 0)
					{
						//veinobj->SetConnectPairHookRigid(cluster, pairid , cdata->m_contactRigid  , );
						if(contactleft)
						{
							float t = (worldcontactpoint-m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[0]).Dot(cutLineVec) / leftlinelensqr;
							
							if(t < 1.0f)
								veinobj->SetConnectPairClampByRigid(cluster, pairid, cdata->m_contactRigid, worldcontactpoint);
						}
						else if(contactright)
						{
							GFPhysVector3 cutLineVec = m_ToolObject->m_CutBladeRight.m_LinePointsWorld[1]-m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0];

// 							float temp = cutLineVec.Dot(cutLineVec);

							float t = (worldcontactpoint-m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0]).Dot(cutLineVec) / rightlinelensqr;
							if(t < 1.0f)
								veinobj->SetConnectPairClampByRigid(cluster, pairid, cdata->m_contactRigid, worldcontactpoint);
						}
					}
				}
				else
				{
					removecontact = true;
				}
			}
			else
				removecontact = true;
		}
		else
		{
			removecontact = true;
		}

		if(removecontact)
			itor = m_contacts.erase(itor);
		else
			itor++;
	}
}

//==================================================================================================
void MisCTool_PluginClampConnectPair::CollideVeinConnectPair(VeinConnectObject * veinobject ,
															 GFPhysCollideObject * convexobj,
															 int cluster , 
															 int pair,
															 const GFPhysVector3 & collidepoint)
{
	
	if(convexobj == m_ToolObject->m_lefttoolpartconvex.m_rigidbody
	  ||convexobj == m_ToolObject->m_righttoolpartconvex.m_rigidbody)
	{
		bool exists = false;
		for(size_t c = 0 ; c < m_contacts.size() ; c++)
		{
			if(m_contacts[c].m_ClusterId == cluster && m_contacts[c].m_PairId == pair)
			{
				exists = true;
				break;
			}
		}
		if(exists == false)
		{
			VeinConnectCotactMe newcon;
			newcon.m_ClusterId = cluster;
			newcon.m_PairId = pair;
			newcon.m_Veinconnect = veinobject;
			newcon.m_ContactRigid = convexobj;
			GFPhysVector3 temp = convexobj->GetWorldTransform().Inverse() * collidepoint;
			newcon.m_LocalPoint = Ogre::Vector3(temp.x(), temp.y() , temp.z());
			newcon.m_toolOpenAside = m_ToolObject->GetShaftAside();
			m_contacts.push_back(newcon);
		}
	}
}
