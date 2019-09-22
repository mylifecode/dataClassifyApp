#include "ElectrocoagulationTrain.h"
#include "MisMedicOrganOrdinary.h"

#include "VeinConnectObject.h"
#include "TextureBloodEffect.h"
#include "MisMedicEffectRender.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "OgreAxisAlignedBox.h"
#include "Instruments/Tool.h"
#include "Inception.h"
#include "LightMgr.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "MisMedicOrganAttachment.h"
#include "CustomConstraint.h"
#include "Instruments/ElectricHook.h"
#include <stack>
#include <vector>
#include "Instruments/GraspingForceps.h"
#include "MisMedicObjectUnion.h"
#include "MXOgreGraphic.h"
#include "SYScoreTableManager.h"
const uint32 BIT_FOR_USE =       0x000000ff;
const uint32 ERROR_PIXEL_VALUE = 0x00ffffff;
const uint32 UNBURN_VALUE = 0x00000000;


Ogre::String texName[] = {
	"ElecTrain_rou_d_2.jpg",
	"ElecTrain_rou_d_3.jpg",
	"ElecTrain_rou_d_4.jpg",
	"ElecTrain_rou_d_5.jpg",
	"ElecTrain_rou_d.png",
	//"ElecTrain_rou_d_6.png",
};

void CElectroCoagulationTrain::BurnAndCutInfo::SetSegment(double upStart , double upEnd, double bottomStrat , double bottomEnd , int segment /* = 10 */)
{
	up_start_v = upStart;
	up_end_v = upEnd;
	bottom_start_v = bottomStrat;
	bottom_end_v = bottomEnd;
	
	//[0]up [1]bottom
	double range[2];
	double step[2];
	double current_height[2];

	range[0] = upEnd - upStart;
	range[1] = bottomEnd - bottomStrat;


	step[0] = range[0] / (segment - 1);
	step[1] = range[1] / (segment - 1);

	current_height[0] = upStart;
	current_height[1] = bottomStrat;

	int real_segment = segment - 1;

	for(int i = 0 ; i < real_segment ; i++)
	{
		segment_height[0].push_back(current_height[0]);
		segment_height[1].push_back(current_height[1]);

		current_height[0] += step[0];
		current_height[1] += step[1];
	}

	segment_height[0].push_back(upEnd);
	segment_height[1].push_back(bottomEnd);
	
	for(int i = 0 ; i < segment ; i++ )
	{
		is_cut[0].push_back(false);
		is_cut[1].push_back(false);
		is_burn[0].push_back(false);
		is_burn[1].push_back(false);
	}
}

void CElectroCoagulationTrain::BurnAndCutInfo::SetBurnArea(const Ogre::Vector2 & topLeft , const Ogre::Vector2 & bottomLeft , const Ogre::Vector2 & topRight , const Ogre::Vector2 & bottomRight)
{
	left = min(topLeft.x ,bottomLeft.x);
	right = max(topRight.x , bottomRight.x);
	top = min(topLeft.y , topRight.y);
	bottom = max(bottomLeft.y , bottomRight.y);
}

CElectroCoagulationTrain::BurnAndCutInfo::BurnAndCutInfo() 
{
	is_burn_start[0] = is_burn_start[1] = false;
	is_burn_end[0] = is_burn_end[1] = false ;

	is_cut_start[0] = is_cut_start[1] = false;
	is_cut_end[0] = is_cut_end[1] = false;

	is_burn_enough[0] = is_burn_enough[1] = false;
	is_cut_enough[0] = is_cut_enough[1] = false;

	is_all_burn = false;
	is_all_cut = false;
	is_finish = false;

	cut_allowable_error = 0.007;
	burn_allowable_error = 0.007;
}

bool CElectroCoagulationTrain::BurnAndCutInfo::TestForBurn(const std::vector<Ogre::Vector2> & texCoords)
{
	for(size_t t = 0; t < texCoords.size() ; t++)
	{
		const Ogre::Vector2 & uv = texCoords[t];
		//[0]--up [1]--bottom
		if(!is_burn_enough[0])
		{
			if(!is_burn_start[0])
			{
				if(abs(uv.y - up_start_v) <= burn_allowable_error)
					is_burn_start[0] = true;
			}
			if(!is_burn_end[0])
			{
// 				double distance = up_end_v - uv.y;
// 				if(distance >= 0 && distance <= 0.01)
// 					is_burn_end[0] = true;
				if(abs(up_end_v - uv.y) <= burn_allowable_error)
					is_burn_end[0] = true;
			}
			is_burn_enough[0] = is_burn_start[0] && is_burn_end[0];
				
		}
		if(!is_burn_enough[1])
		{
			if(!is_burn_start[1])
			{
				if(abs(uv.y - up_start_v) <= burn_allowable_error)
					is_burn_start[1] = true;
			}
			if(!is_burn_end[1])
			{
// 				double distance = uv.y - bottom_end_v;
// 				if(distance >= 0 && distance <= 0.01)
// 					is_burn_end[1] = true;
				if(abs(uv.y - bottom_end_v) <= burn_allowable_error)
					is_burn_end[1] = true;
			}
			is_burn_enough[1] = is_burn_start[1] && is_burn_end[1];
		}
	}
	is_all_burn = is_burn_enough[0] && is_burn_enough[1];
	return is_all_burn;
}


bool CElectroCoagulationTrain::BurnAndCutInfo::TestForCut(const std::vector<Ogre::Vector2> & texCoords)
{
	for(size_t t = 0; t < texCoords.size() ; t++)
	{
		const Ogre::Vector2 & uv = texCoords[t];
		//[0]--up [1]--bottom
		if(!is_cut_enough[0])
		{
			if(!is_cut_start[0])
			{
				if(abs(uv.y - up_start_v) <= cut_allowable_error)
					is_cut_start[0] = true;
			}
			if(!is_cut_end[0])
			{
// 				double distance = up_end_v - uv.y;
// 				if(distance >= 0 && distance <= 0.01)
// 					is_cut_end[0] = true;
				if(abs(up_end_v - uv.y) <= cut_allowable_error)
					is_cut_end[0] = true;
			}
			is_cut_enough[0] = is_cut_start[0] && is_cut_end[0];

		}
		if(!is_cut_enough[1])
		{
			if(!is_cut_start[1])
			{
				if(abs(uv.y - up_start_v) <= cut_allowable_error)
					is_cut_start[1] = true;
			}
			if(!is_cut_end[1])
			{
// 				double distance = uv.y - bottom_end_v;
// 				if(distance >= 0 && distance <= 0.01)
// 					is_cut_end[1] = true;
				if(abs(uv.y - bottom_end_v) <= cut_allowable_error)
					is_cut_end[1] = true;
			}

			is_cut_enough[1] = is_cut_start[1] && is_cut_end[1];
		}
	}
	is_all_cut = is_cut_enough[0] && is_cut_enough[1];
	return is_all_cut;
}

bool CElectroCoagulationTrain::BurnAndCutInfo::TestForBurnV2(const std::vector<Ogre::Vector2> & texCoords)
{
	for(size_t t = 0; t < texCoords.size() ; t++)
	{
		const Ogre::Vector2 & uv = texCoords[t];
		//[0]--up [1]--bottom
		if(!is_burn_enough[0])
		{
			if(!is_burn_start[0])
			{
				if(abs(uv.y - up_start_v) <= burn_allowable_error)
					is_burn_start[0] = true;
			}
			if(!is_burn_end[0])
			{
				if(uv.y <= up_end_v)
					is_burn_end[0] = true;
			}
			is_burn_enough[0] = is_burn_start[0] && is_burn_end[0];

		}
		if(!is_burn_enough[1])
		{
			if(!is_burn_start[1])
			{
				if(abs(uv.y - up_start_v) <= burn_allowable_error)
					is_burn_start[1] = true;
			}
			if(!is_burn_end[1])
			{
				if(uv.y >= bottom_end_v)
					is_burn_end[1] = true;
			}
			is_burn_enough[1] = is_burn_start[1] && is_burn_end[1];
		}
	}
	is_all_burn = is_burn_enough[0] && is_burn_enough[1];
	return is_all_burn;
}

bool CElectroCoagulationTrain::BurnAndCutInfo::TestForBurnV3(const std::vector<Ogre::Vector2> & texCoords , double partitionHeight)
{
	double partition_for_up = partitionHeight + 0.01;
	double partition_for_bottom = partitionHeight - 0.01;

	double up_miny = 2.0;
	double up_maxy = -1.0;

	double bottom_miny = 2.0;
	double bottom_maxy = -1.0;

	for(int t = 0 ; t < texCoords.size() ; t++)
	{
		const Ogre::Vector2 & uv = texCoords[t];

		if(uv.y < partition_for_up)
		{
			if(uv.y > up_maxy)
				up_maxy = uv.y;
			if(uv.y < up_miny)
				up_miny = uv.y;
		}

		if(uv.y > partition_for_bottom)
		{
			if(uv.y > bottom_maxy)
				bottom_maxy = uv.y;
			if(uv.y < bottom_miny)
				bottom_miny = uv.y;
		}
	}

	double miny[2];
	double maxy[2];

	miny[0] = up_miny;
	miny[1] = bottom_miny;

	maxy[0] = up_maxy;
	maxy[1] = bottom_maxy;

	for(int b = 0; b < 2 ; b++)
	{
		if(!is_burn_enough[b])
		{
			for(int s = 0 ; s < segment_height[b].size() ; s++ )
			{
				if(!is_burn[b][s])
				{
					if(segment_height[b][s] <= maxy[b] && segment_height[b][s] >= miny[b])
						is_burn[b][s] = true;
				}
			}
		}
	}

	for(int b = 0; b < 2 ; b++)
	{
		if(!is_burn_enough[b])
		{
			bool result = true;
			for(int s = 0 ; s < is_burn[b].size() ; s++ )
			{
				result = is_burn[b][s] && result;
				if(!result)
					break;
			}
			is_burn_enough[b] = result;
		}
	}

	is_all_burn = is_burn_enough[0] && is_burn_enough[1];
	return is_all_burn;


}

bool CElectroCoagulationTrain::BurnAndCutInfo::TestForCutV2(const std::vector<Ogre::Vector2> & texCoords)
{
	for(size_t t = 0; t < texCoords.size() ; t++)
	{
		const Ogre::Vector2 & uv = texCoords[t];
		//[0]--up [1]--bottom
		if(!is_cut_enough[0])
		{
			if(!is_cut_start[0])
			{
				if(abs(uv.y - up_start_v) <= cut_allowable_error)
					is_cut_start[0] = true;
			}
			if(!is_cut_end[0])
			{
				if(uv.y <= up_end_v)
					is_cut_end[0] = true;
			}
			is_cut_enough[0] = is_cut_start[0] && is_cut_end[0];

		}
		if(!is_cut_enough[1])
		{
			if(!is_cut_start[1])
			{
				if(abs(uv.y - up_start_v) <= cut_allowable_error)
					is_cut_start[1] = true;
			}
			if(!is_cut_end[1])
			{
				if(uv.y >= bottom_end_v)
					is_cut_end[1] = true;
			}
			is_cut_enough[1] = is_cut_start[1] && is_cut_end[1];
		}
	}
	is_all_cut = is_cut_enough[0] && is_cut_enough[1];
	return is_all_cut;
}




bool CElectroCoagulationTrain::BurnAndCutInfo::TestForCutV3(const std::vector<Ogre::Vector2> & texCoords , double partitionHeight , bool isRelaxed)
{
	double partition_for_up = partitionHeight + 0.01;
	double partition_for_bottom = partitionHeight - 0.01;

	double up_miny = 2.0;
	double up_maxy = -1.0;

	double bottom_miny = 2.0;
	double bottom_maxy = -1.0;

	for(int t = 0 ; t < texCoords.size() ; t++)
	{
		const Ogre::Vector2 & uv = texCoords[t];

		if(uv.y < partition_for_up)
		{
			if(uv.y > up_maxy)
				up_maxy = uv.y;
			if(uv.y < up_miny)
				up_miny = uv.y;
		}

		if(uv.y > partition_for_bottom)
		{
			if(uv.y > bottom_maxy)
				bottom_maxy = uv.y;
			if(uv.y < bottom_miny)
				bottom_miny = uv.y;
		}
	}

	double miny[2];
	double maxy[2];

	miny[0] = up_miny;
	miny[1] = bottom_miny;

	maxy[0] = up_maxy;
	maxy[1] = bottom_maxy;


	for(int b = 0; b < 2 ; b++)
	{
		if(!is_cut_enough[b])
		{
			for(int s = 0 ; s < segment_height[b].size() ; s++ )
			{
				if(!is_cut[b][s])
				{
					if(segment_height[b][s] <= maxy[b] && segment_height[b][s] >= miny[b])
					{
						is_cut[b][s] = true;
					}
				}
			}
		}
	}

	for(int b = 0; b < 2 ; b++)
	{
		if(!is_cut_enough[b])
		{
			bool result = true;
			for(int s = 0 ; s < is_cut[b].size() ; s++ )
			{
				result = is_cut[b][s] && result;
				if(!result)
					break;
			}
			is_cut_enough[b] = result;
		}
	}

	is_all_cut = is_cut_enough[0] && is_cut_enough[1];

	if(!is_all_cut)
	{
		if(isRelaxed)
		{
			int cut_num[2];

			for(int b = 0; b < 2 ; b++)
			{
				cut_num[b] = 0;
				for(int s = 0 ; s < is_cut[b].size() ; s++ )
				{
					if(is_cut[b][s])
						cut_num[b]++;
				}
			}
			if(cut_num[0] >= 5 && cut_num[1] >= 5)					//临时
			{
				is_all_cut = true;
			}
			else if(cut_num[0]  >= 3 && cut_num[1] >= 3)
			{
				int segment_num = is_cut[0].size();
				if(is_cut[0][segment_num - 1] == true && is_cut[1][segment_num - 1] == true)
					is_all_cut = true;
			}
		}
	}
	return is_all_cut;
}

CElectroCoagulationTrain::CElectroCoagulationTrain(void)
{
	m_pMarkDest = 0;
	m_StripConnect = 0;
}

CElectroCoagulationTrain::~CElectroCoagulationTrain(void)
{
	m_area_mark_ptr.setNull();
	
	for(int t = 0; t < 5 ; t++)
		m_textuer_ptr[t].setNull();

	
}

bool CElectroCoagulationTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	//Ogre::SceneManager * p_scene_mgr =  MXOgre_SCENEMANAGER;
	CLightMgr::Instance()->GetLight()->setPosition(Ogre::Vector3(0.0f, 0.0f, 0.40f));
	CLightMgr::Instance()->GetLight()->setDirection(Ogre::Vector3(0.0f, 0, -1));

	Init();
	
	m_isfailed = false;
	m_is_finish = false;

	return result;
}

bool CElectroCoagulationTrain::Update(float dt)
{
	if(m_isfailed)
	{
		static bool first_fail = true;
		if(first_fail)
		{
			first_fail = false;
		}
/*		return false;*/
	}
	else
	{
		if(m_burn_texcoord_buffer.size() != 0)
		{
			DealWithOperation(BURN_INSIDE_MARKAREA , m_burn_texcoord_buffer ,false);
			m_burn_texcoord_buffer.clear();
		}

		if(m_cut_texcoord_buffer.size() != 0)
		{
			DealWithOperation(CUT_INSIDE_WITH_BURN , m_cut_texcoord_buffer ,false);
			m_cut_texcoord_buffer.clear();
		}

		if(m_burnAndCutInfos.back().is_all_cut)
		{
			m_is_finish = true;
		}
		else
		{
			for(int i = 0 ; i < m_burnAndCutInfos.size() ; i++)
			{
				BurnAndCutInfo & info = m_burnAndCutInfos[i];
				if(info.is_all_burn && info.is_all_cut)
				{
					m_is_finish = true;
					if(!m_is_set_material[i])
					{
						//m_pflesh->ChangeTexture(m_textuer_ptr[i] , "BaseMap");

						m_is_set_material[i] = true;
					}
				}
				else
					m_is_finish = false;
			}
		}

		if(m_is_finish)
		{
			TrainingFinish();
		}
	}

	bool result = MisNewTraining::Update(dt);


	//for debug
	//DrawFacesClamped();

	return result;
}

void CElectroCoagulationTrain::receiveCheckPointList(MisNewTraining::OperationCheckPointType type,
													 const std::vector<Ogre::Vector2> & texCoords, 
													 const std::vector<Ogre::Vector2> & TFUV , 
													 ITool * tool , 
													 MisMedicOrganInterface * oif)
{
	
	if(texCoords.size() == 0)
		return;

	if(type == MisNewTraining::OCPT_Burn_Origin_Face && CheckOrganInBurn(oif))
	{
		//bool burn_error = false;
		std::vector<Ogre::Vector2>  texcoord_in_clamp_region;
		std::vector<Ogre::Vector2>  satisfied_texcoord;

		assert(texCoords.size() == TFUV.size());

		for(size_t t = 0 ; t < texCoords.size(); t ++)
		{
			const Ogre::Vector2 & tfuv = TFUV[t];
			//范围根据电凝钳得到
			if(tfuv.x <= 0.84 && tfuv.x >= 0.155)
				if(tfuv.y <= 1.0 && tfuv.y >= -0.01)
					texcoord_in_clamp_region.push_back(texCoords[t]);
		}
		
		int total_num = texcoord_in_clamp_region.size(); 

		if(total_num == 0)
			return;

		for(size_t t = 0 ; t < texcoord_in_clamp_region.size() ; t++)
		{
			Ogre::Vector2 & texcoord = texcoord_in_clamp_region[t];

			if(CheckIfBurnPointInRightArea(texcoord))
			{
				satisfied_texcoord.push_back(texcoord);
			}
		}

		if(( (double)satisfied_texcoord.size()  /  total_num) >= m_burn_success_percent)
			DealWithOperation(BURN_INSIDE_MARKAREA , satisfied_texcoord);
		else
			DealWithOperation(BURN_OUTSIDE_MARKAREA , satisfied_texcoord);

		m_is_burn_change = true;
	
	}
	else if (type == MisNewTraining::OCPT_Cut  && CheckOrganCut(oif))
	{
		bool is_cut_error = false;
		int total_num = texCoords.size();
		std::vector<Ogre::Vector2>  cut_satisfied_texcoord;

		for(size_t t = 0 ; t < texCoords.size(); t ++)
		{
			if(CheckIfCutInRightArea(texCoords[t]))
			{
				cut_satisfied_texcoord.push_back(texCoords[t]);
				//cut_error = true;
				//break;
			}
		}
		if(( (double)cut_satisfied_texcoord.size()  /  total_num) >= m_cut_success_percent)
		{
			//CTipMgr::Instance()->ShowTip("CutInMarkArea");
		}
		else
		{
			DealWithOperation(CUT_OUSIDE_MARKAREA , cut_satisfied_texcoord);
			is_cut_error = true;
		}

		if(!is_cut_error)
		{
			bool cut_in_unburn = false;

			std::vector<Ogre::Vector2> cut_in_burn_satisfied_texcoord;

			if(m_is_burn_change)
			{
				m_pflesh->CopyBurnTexture();
				m_is_burn_change = false;
			}

			for(size_t t = 0 ; t < cut_satisfied_texcoord.size(); t ++)
			{
				uint32 pixel_value = m_pflesh->GetBurnTexPixel(cut_satisfied_texcoord[t]);
				if((pixel_value & BIT_FOR_USE) != UNBURN_VALUE)
					cut_in_burn_satisfied_texcoord.push_back(cut_satisfied_texcoord[t]);
			}

			if( ((double)cut_in_burn_satisfied_texcoord.size() / cut_satisfied_texcoord.size()) > m_cut_in_burn_success_percent)
				DealWithOperation(CUT_INSIDE_WITH_BURN , cut_satisfied_texcoord);
			else if(((double)cut_in_burn_satisfied_texcoord.size() / cut_satisfied_texcoord.size())  < 0.2) // 在标记内，但基本不在电凝区域
				DealWithOperation(CUT_INSIDE_WITHOUT_BURN , cut_satisfied_texcoord/*cut_in_burn_satisfied_texcoord*/);
			else
				DealWithOperation(CUT_OVER_BURN_AREA , cut_satisfied_texcoord/*cut_in_burn_satisfied_texcoord*/);					//剪切时超出电凝区域
		}

	}
}

bool CElectroCoagulationTrain::CheckOrganInBurn(MisMedicOrganInterface * organ)
{
	if(organ->m_OrganID != m_pflesh->m_OrganID)
	{
		CTipMgr::Instance()->ShowTip("BurnWrongOrgan");
		return false;
	}
	return true;
}

bool CElectroCoagulationTrain::CheckOrganCut(MisMedicOrganInterface * organ)
{
	if(organ->m_OrganID != m_pflesh->m_OrganID)
	{
		CTipMgr::Instance()->ShowTip("CutWrongOrgan");
		return false;
	}
	return true;
}

void CElectroCoagulationTrain::SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ)
{
	organ->GetCreateInfo().m_distributemass = false;
}

void CElectroCoagulationTrain::CollectFacesClamped(GFPhysVectorObj<GFPhysSoftBodyFace*> & faces_clamped)
{
	//m_faces_clamped = faces_clamped;
}


void CElectroCoagulationTrain::Init()
{
	DynObjMap::iterator organ_itor = m_DynObjMap.begin();
	while(organ_itor != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * p_organ = dynamic_cast<MisMedicOrgan_Ordinary*>(organ_itor->second);
		VeinConnectObject * p_connect = dynamic_cast<VeinConnectObject*>(organ_itor->second);
		if(p_organ)
		{
			if (p_organ->m_OrganID == ELECTRAIN_FLESH)
			{
				m_pflesh = p_organ;
				std::set<GFPhysSoftBodyNode*> NodesInVessels;
				for(size_t t = 0 ; t < m_pflesh->m_PhysTetraData.size() ; t++)
				{
					if( m_pflesh->m_PhysTetraData[t].m_HasError == false)
					{
						GFPhysSoftBodyTetrahedron* PhysTetra = m_pflesh->m_PhysTetraData[t].m_PhysTetra;
						int exisNum = 0;
						for(int n = 0 ; n < 4 ; n++)
						{
							if(NodesInVessels.find(PhysTetra->m_TetraNodes[n]) != NodesInVessels.end())
							{
								exisNum++;
							}
						}
						if(exisNum <= 3)
						{	
							NodesInVessels.insert(PhysTetra->m_TetraNodes[0]);
							NodesInVessels.insert(PhysTetra->m_TetraNodes[1]);
							NodesInVessels.insert(PhysTetra->m_TetraNodes[2]);
							NodesInVessels.insert(PhysTetra->m_TetraNodes[3]);
							m_pflesh->m_PhysTetraData[t].m_ContainsVessel = true;
						}
					}

				}
			}
		}

		if (p_connect)
		{
			m_StripConnect = p_connect;
		}
		++organ_itor;
	}
	
	m_pflesh->SetBloodSystemGravityDir(Ogre::Vector3(0,-1,0));

	m_area_mark_ptr = Ogre::TextureManager::getSingleton().load("ElecTrainAreaMark.png" , 
																Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																Ogre::TEX_TYPE_2D,0,1.0f,false,Ogre::PF_A8R8G8B8);


	m_mark_tex_width = m_area_mark_ptr->getWidth();
	m_mark_tex_height = m_area_mark_ptr->getHeight();
    //copy to cpu memory 
	Ogre::HardwarePixelBufferSharedPtr pixel_buffer_ptr;
	try
	{
		pixel_buffer_ptr = m_area_mark_ptr->getBuffer();
		pixel_buffer_ptr->lock(Ogre::HardwareBuffer::HBL_NORMAL);
	}
	catch (...)
	{
		return;
	}

	Ogre::uint* pDest = static_cast<Ogre::uint*>(pixel_buffer_ptr->getCurrentLock().data);

	m_pMarkDest = new Ogre::uint[m_mark_tex_height*m_mark_tex_width];

	for (int i = 0 ; i < m_mark_tex_height*m_mark_tex_width ; i++)
	{
		m_pMarkDest[i] = pDest[i];
	}

	pixel_buffer_ptr->unlock();




	m_up_end_v = 0.1171875;
	m_center_of_markarea_v = 0.327148;						//按标记图
	m_bottom_end_v = 0.537109;
	
	m_burn_success_percent = 0.88;
	m_cut_success_percent = 0.7;
	m_cut_in_burn_success_percent = 0.8;

	m_is_burn_change = true;
	


	//分段
	m_segment_num = 5;
	double up_segment_length = (m_up_end_v - m_center_of_markarea_v) / m_segment_num;
	double bottom_segment_length = (m_bottom_end_v - m_center_of_markarea_v) / m_segment_num;

	double current_up_begin = m_center_of_markarea_v;
	double current_bottom_begin = m_center_of_markarea_v;
	

	for(int s = 0 ; s < m_segment_num; s++)
	{
		BurnAndCutInfo info;

 		info.up_start_v= current_up_begin;
 		info.up_end_v = current_up_begin + up_segment_length;
 
 		info.bottom_start_v = current_bottom_begin;
 		info.bottom_end_v = current_bottom_begin + bottom_segment_length;
 		

		if(s == (m_segment_num - 1))
		{
			info.burn_allowable_error = 0.01;
			info.cut_allowable_error = 0.008;

			//info.up_end_v +=0.01;
			//info.bottom_end_v -= 0.01;
		}


		info.SetSegment(current_up_begin , 
			info.up_end_v ,
			current_bottom_begin , 
			info.bottom_end_v, 
			6);

 		current_up_begin = info.up_end_v;
 		current_bottom_begin = info.bottom_end_v;
		
		m_burnAndCutInfos.push_back(info);

		m_is_set_material[s] = false;
	}

	for(int t = 0; t < m_segment_num; t++)
	{
		m_textuer_ptr[t] = Ogre::TextureManager::getSingleton().load(texName[t] , 
							Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
							Ogre::TEX_TYPE_2D,0,1.0f,false,Ogre::PF_R8G8B8);
	}


	InitCutAndBurnStrs();	
		

	//for debug
	//TestForUV(m_pflesh);
	m_manual =MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_manual);
	m_manual->setDynamic(true);
	
}

void CElectroCoagulationTrain::InitCutAndBurnStrs()
{
	const int segment_num = 5;
	const int random_num = 6;

	Ogre::String burn_str = "BurnFinish";
	for(int b = 1; b <= m_segment_num ; b++)
	{
		Ogre::String basic_str = burn_str + Ogre::StringConverter::toString(b);
		m_burn_strs.push_back(basic_str);
		
		if(b != m_segment_num)
		{
			for(int n = 1 ; n <= random_num ; n++)
				m_burn_strs.push_back(basic_str + "_" +  Ogre::StringConverter::toString(n));
		}
	}

	Ogre::String cut_str = "CutFinish";
	for(int c = 1 ; c <= m_segment_num ; c++)
	{
		Ogre::String basic_str = cut_str + Ogre::StringConverter::toString(c);
		m_cut_strs.push_back(basic_str);

		if(c != m_segment_num)
		{
			for(int n = 1 ; n <= random_num ; n++)
				m_cut_strs.push_back(basic_str + "_" +  Ogre::StringConverter::toString(n));
		}
	}
}


uint32 CElectroCoagulationTrain::GetPixelFromAreaTexture(float cx, float cy)
{
	if(m_pMarkDest)
	{
		int tcx = cx*(m_mark_tex_width - 1);

		int tcy = cy*(m_mark_tex_height - 1);

		uint32 pix = *(m_pMarkDest + tcy * m_mark_tex_width + tcx);

		return pix;
	}
	else
	{
		return 0 ;
	}
	
}

bool CElectroCoagulationTrain::CheckIfBurnPointInRightArea(const Ogre::Vector2 & texcoord)
{
	uint32 pixel_value = GetPixelFromAreaTexture(texcoord.x,texcoord.y);
	if((pixel_value & BIT_FOR_USE) != ERROR_PIXEL_VALUE)
		return true;
	return false;
}

bool CElectroCoagulationTrain::CheckIfCutInRightArea(const Ogre::Vector2 & texcoord)
{
	uint32 pixel_value = GetPixelFromAreaTexture(texcoord.x,texcoord.y);
	if((pixel_value & BIT_FOR_USE) != ERROR_PIXEL_VALUE)
		return true;
	return false;
}

void CElectroCoagulationTrain::DealWithOperation(OperationType type , const std::vector<Ogre::Vector2> & texCoord , bool tipFlag)
{
	if(type == BURN_OUTSIDE_MARKAREA)
	{
		CTipMgr::Instance()->ShowTip("BurnInWrongPos");
		CScoreMgr::Instance()->Grade("BurnInWrongPos");

		//m_pflesh->ChangeTexture(m_area_mark_ptr , "BaseMap");

		for(int t = 0 ; t < texCoord.size() ; t++)
			m_burn_texcoord_buffer.push_back(texCoord[t]);

		m_OnLineGradeInfo.m_iBurnOutRegCount++;
	}
	else if (type == CUT_OUSIDE_MARKAREA)
	{
		TrainingFatalError("CutInWrongArea");
		m_OnLineGradeInfo.m_iCutMiss++;
		m_OnLineGradeInfo.m_iCutOutMark++;
		m_isfailed = true;
	}
	else if(type == CUT_INSIDE_WITHOUT_BURN)
	{
		CTipMgr::Instance()->ShowTip("CutInUnBurnArea");
		CScoreMgr::Instance()->Grade("CutInUnBurnArea");
		m_OnLineGradeInfo.m_iCutMiss++;
		m_OnLineGradeInfo.m_NumCutNotInBurnReg++;

		for(int t = 0 ; t < texCoord.size() ; t++)
			m_cut_texcoord_buffer.push_back(texCoord[t]);

	}
	else if(type == CUT_OVER_BURN_AREA)
	{
		CTipMgr::Instance()->ShowTip("CutBeyondBurnArea");
		CScoreMgr::Instance()->Grade("CutInUnBurnArea");
		m_OnLineGradeInfo.m_iCutMiss++;
		m_OnLineGradeInfo.m_NumCutNotInBurnReg++;

		for(int t = 0 ; t < texCoord.size() ; t++)
			m_cut_texcoord_buffer.push_back(texCoord[t]);
	}
	else if(type == BURN_INSIDE_MARKAREA)
	{
		bool has_one_finish = false;
		bool prev_finish = true;
		for(int i = 0 ; i < m_burnAndCutInfos.size() ; i++)
		{
			BurnAndCutInfo &info  = m_burnAndCutInfos[i];
			if(!info.is_all_burn)
			{
				if(info.TestForBurnV2(texCoord))
				{
					prev_finish = true;
					//CTipMgr::Instance()->ShowTip(burnStrs[i]);
					//CScoreMgr::Instance()->Grade(burnStrs[i]);
					ShowStageTip(BURN_INSIDE_MARKAREA , i);
					DealStageScore(BURN_INSIDE_MARKAREA , i);
					has_one_finish = true;

				}
				else
				{
					prev_finish = false;
				}
			}
					
		}
		if(tipFlag)
		{
			if(!has_one_finish)
				CTipMgr::Instance()->ShowTip("BurnRight");
		}	
	}
	else if(type == CUT_INSIDE_WITH_BURN)
	{
		bool has_one_finish = false;
		bool prev_finish = true;
		for(int i = 0 ; i < m_burnAndCutInfos.size() ; i++)
		{
			BurnAndCutInfo &info  = m_burnAndCutInfos[i];
			if(!info.is_all_cut)
			{
				if(i == (m_burnAndCutInfos.size() - 1))
				{
					if(info.TestForCutV2(texCoord))
					{
						ShowStageTip(CUT_INSIDE_WITH_BURN , i);
						DealStageScore(CUT_INSIDE_WITH_BURN , i);
						has_one_finish = true;
					}
				}
				else 
				{
					if(info.TestForCut(texCoord))
					{
						ShowStageTip(CUT_INSIDE_WITH_BURN , i);
						DealStageScore(CUT_INSIDE_WITH_BURN , i);
						has_one_finish = true;
					}
				}
			}
		}
		if(tipFlag)
		{
			if(!has_one_finish)
				CTipMgr::Instance()->ShowTip("CutInBurnArea");
		}
		m_OnLineGradeInfo.m_NumCutInBurnReg++;
	}

}
 
void CElectroCoagulationTrain::ShowStageTip(OperationType type , int stage)
{
	if(stage >= m_segment_num || stage < 0)
		return;

	const int random_num = 6;

	if(type == BURN_INSIDE_MARKAREA)
	{
		if(stage != (m_segment_num -1))
		{
			int i = rand() % random_num + 1;
			CTipMgr::Instance()->ShowTip(m_burn_strs[stage * (random_num + 1) + i]);
		}
		else
			CTipMgr::Instance()->ShowTip(m_burn_strs[stage * (random_num + 1)]);
	}
	else if(type == CUT_INSIDE_WITH_BURN)
	{
		if(stage != (m_segment_num -1))
		{
			int i = rand() % random_num + 1;
			CTipMgr::Instance()->ShowTip(m_cut_strs[stage * (random_num + 1) + i]);
		}
		else
			CTipMgr::Instance()->ShowTip(m_cut_strs[stage * (random_num + 1)]);
	}
}

void CElectroCoagulationTrain::DealStageScore(OperationType type , int stage)
{
	if(stage >= m_segment_num || stage < 0)
		return;

	const int random_num = 6;

	if(type == BURN_INSIDE_MARKAREA)
	{ 
		CScoreMgr::Instance()->Grade(m_burn_strs[stage * (random_num + 1)]);
		m_OnLineGradeInfo.m_iBurnStage = stage;
	}
	else if(type == CUT_INSIDE_WITH_BURN)
	{
		CScoreMgr::Instance()->Grade(m_cut_strs[stage * (random_num + 1)]);
		m_OnLineGradeInfo.m_iCutStage = stage;
	}

	
}

void CElectroCoagulationTrain::DrawFacesClamped()
{
	assert(m_faces_tested.size() != 0);
	m_manual->clear();

	m_manual->begin("BaseWhiteNoLighting");
	for(int f = 0; f< m_faces_tested.size();f++)
	{
		GFPhysSoftBodyFace * p_face = m_faces_tested[f];
		if(p_face)
		{
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[0]->m_CurrPosition));
			m_manual->colour(1,1,0,1);
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[1]->m_CurrPosition));
			m_manual->colour(1,1,0,1);
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[2]->m_CurrPosition));
			m_manual->colour(1,1,0,1);
		}
	}
	m_manual->end();

	if(m_faces_clamped.size() == 0)
		return;
	
	m_manual->begin("BaseWhiteNoLighting");
	for(int f = 0; f< m_faces_clamped.size();f++)
	{
		GFPhysSoftBodyFace * p_face = m_faces_clamped[f];
		if(p_face)
		{
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[0]->m_CurrPosition));
			m_manual->colour(0,1,1,1);
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[1]->m_CurrPosition));
			m_manual->colour(0,1,1,1);
			m_manual->position(GPVec3ToOgre(p_face->m_Nodes[2]->m_CurrPosition));
			m_manual->colour(0,1,1,1);
		}
	}
	m_manual->end();
}

void CElectroCoagulationTrain::TestForUV(MisMedicOrgan_Ordinary *pOrgan)
{
	for(size_t f = 0 ; f < pOrgan->m_OriginFaces.size() ; f++)
	{
		MMO_Face &face = pOrgan->m_OriginFaces[f];
		int num = 0;
		for(int k = 0; k < 3; k++)
		{	
			Ogre::Vector2 texCoord = face.GetTextureCoord(k);

			if (abs(texCoord.y - m_center_of_markarea_v) < 0.005)
			{
				m_faces_tested.push_back(face.m_physface);
				break;
			}
			else if (abs(texCoord.y - m_up_end_v) < 0.005)
			{
				m_faces_tested.push_back(face.m_physface);
				break;
			}
			else if (abs(texCoord.y - m_bottom_end_v) < 0.005)
			{
				m_faces_tested.push_back(face.m_physface);
				break;
			}
		}
	}
}
//======================================================================================================================
SYScoreTable* CElectroCoagulationTrain::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01101601");
}

//======================================================================================================================
void CElectroCoagulationTrain::OnSaveTrainingReport()
{
	/*COnLineGradeMgr* onLineGradeMgr2 = COnLineGradeMgr::Instance();

	onLineGradeMgr2->SendGrade("BurnStage_100");
	onLineGradeMgr2->SendGrade("CutStage_100");
	onLineGradeMgr2->SendGrade("ElectricTime_Moderate");
	onLineGradeMgr2->SendGrade("MachineHandle_Normal");
	onLineGradeMgr2->SendGrade("CutIn_BurnArea");
	onLineGradeMgr2->SendGrade("ElectricEfficiency_High");
	onLineGradeMgr2->SendGrade("TwoHands_Cooperation");
	onLineGradeMgr2->SendGrade("Finished_In3M");*/
	

//	if (!m_bTrainingIlluminated)
	//	return;

	/*
	COnLineGradeMgr* onLineGradeMgr = COnLineGradeMgr::Instance();

	if (m_OnLineGradeInfo.m_iBurnStage == 4)
		onLineGradeMgr->SendGrade("BurnStage_100");
	else if (m_OnLineGradeInfo.m_iBurnStage == 3)
		onLineGradeMgr->SendGrade("BurnStage_80+");
	else if (m_OnLineGradeInfo.m_iBurnStage == 2)
		onLineGradeMgr->SendGrade("BurnStage_60+");
	else if (m_OnLineGradeInfo.m_iBurnStage >= 0)
		onLineGradeMgr->SendGrade("BurnStage_60-");
	else 
		onLineGradeMgr->SendGrade("BurnStage_0");

	if (m_OnLineGradeInfo.m_iCutStage == 4)
		onLineGradeMgr->SendGrade("CutStage_100");
	else if (m_OnLineGradeInfo.m_iCutStage == 3)
		onLineGradeMgr->SendGrade("CutStage_80+");
	else if (m_OnLineGradeInfo.m_iCutStage == 2)
		onLineGradeMgr->SendGrade("CutStage_60+");
	else if (m_OnLineGradeInfo.m_iCutStage >= 0)
		onLineGradeMgr->SendGrade("CutStage_60-");
	else
		onLineGradeMgr->SendGrade("CutStage_0");
	
	if (m_OnLineGradeInfo.m_iBurnStage >= 0 && m_OnLineGradeInfo.m_iCutStage >= 0)
	{
		float maxElectricTime = m_pToolsMgr->GetMaxKeeppingElectricTime();

		if (maxElectricTime > 0)
		{
			if (maxElectricTime < 5)
				onLineGradeMgr->SendGrade("ElectricTime_Moderate");
			else
				onLineGradeMgr->SendGrade("ElectricTime_TooLong");
		}

		float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
		float rightTooSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
		float ToolSpeed = std::max(leftToolSpeed, rightTooSpeed);

		if (ToolSpeed > 0.0)
		{
			if (ToolSpeed <= 5.0 && ToolSpeed > 0.0)
				onLineGradeMgr->SendGrade("MachineHandle_Normal");
			else if (ToolSpeed >  5.0 && ToolSpeed <= 10.0)
				onLineGradeMgr->SendGrade("MachineHandle_Fast");
			else
				onLineGradeMgr->SendGrade("MachineHandle_TooFast");
		}

		if (m_OnLineGradeInfo.m_iCutMiss > 0)
			onLineGradeMgr->SendGrade("CutIn_UnBurnArea");
		else
			onLineGradeMgr->SendGrade("CutIn_BurnArea");

		float ValidElectricTime = m_pToolsMgr->GetValidElectricTime();
		float ValidElectricTotal = m_pToolsMgr->GetTotalElectricTime();
		float ValidElectricRate = ValidElectricTime / ValidElectricTotal;

		if (ValidElectricTotal > 10)
		{
			if (ValidElectricRate > 0.8)
				onLineGradeMgr->SendGrade("ElectricEfficiency_High");
			else
				onLineGradeMgr->SendGrade("ElectricEfficiency_Low");
		}
	}
	
	if (m_is_finish)
	{
		float usedtime = GetElapsedTime();

		if (usedtime < 120)
			onLineGradeMgr->SendGrade("TwoHands_Cooperation");

		if (usedtime < 180)
			onLineGradeMgr->SendGrade("Finished_In3M");
		else if (usedtime < 360)
			onLineGradeMgr->SendGrade("Finished_In10M");
		else
			onLineGradeMgr->SendGrade("Finished_Out10M");
	}
	else
	{
		onLineGradeMgr->SendGrade("Finished_Out10M");
	}
	*/
     AddScoreItemDetail("0160103600", 0);
     if (m_StripConnect)
     {
	    int n0 = m_StripConnect->GetCurrConnectCount();
	    int n1 = m_StripConnect->GetInitConnectCount();
		if (n1)
		{
			float ratio = 1.f * n0 / n1;
			if (ratio > 0.5f)
			{
				AddScoreItemDetail("0160103700", 0);
			}
			else if (ratio > 0.1f)
			{
				AddScoreItemDetail("0160103701", 0);
			}
		}
    }
	if (m_OnLineGradeInfo.m_iBurnStage >= 3)
	{
		AddScoreItemDetail("0160203810", 0);
	}
	else if (m_OnLineGradeInfo.m_iBurnStage >= 0)
	{
		AddScoreItemDetail("0160203811", 0);
	}
	else
	{
		AddScoreItemDetail("0160203819", 0);
	}

	if (m_OnLineGradeInfo.m_iCutStage >= m_segment_num - 1)//沿标记剪断
	{
		AddScoreItemDetail("0160203910", 0); 
	}
	else if (m_OnLineGradeInfo.m_iCutStage >= 0)//未全部按要求完成
	{
		AddScoreItemDetail("0160203911", 0);
	}
	else//未剪切
	{
		AddScoreItemDetail("0160203919", 0);
	}

	if (m_OnLineGradeInfo.m_iCutMiss == 0)
	{
		AddScoreItemDetail("0160204010", 0);//处理范围正确
	}
	else
	{
		AddScoreItemDetail("0160204011", 0);//超出标记范围
	}

	if (m_OnLineGradeInfo.m_iCutStage >= m_segment_num - 1)
	{
		if (m_OnLineGradeInfo.m_NumCutNotInBurnReg == 0)
			AddScoreItemDetail("0160304110", 0);//处理顺序全部正确
		
		else if (m_OnLineGradeInfo.m_NumCutInBurnReg == 0)
			AddScoreItemDetail("0160304112", 0);//处理顺序全部错误
		
		else
			AddScoreItemDetail("0160304111", 0);//处理顺序全部错误
	}

	AddScoreItemDetail("0160401400", 0);//未造成出血
    

	if (m_OnLineGradeInfo.m_iBurnStage >= 0 && m_OnLineGradeInfo.m_iCutStage >= 0)
	{
		if (m_OnLineGradeInfo.m_iCutOutMark == 0)
			AddScoreItemDetail("0160503500", 0);//操作得当，没有其他损伤
		else
			AddScoreItemDetail("0160503501", 0);//剪断标记范围外的组织

		float maxElectricTime = m_pToolsMgr->GetMaxKeeppingElectricTime();

		if (maxElectricTime > 0)
		{
			if (maxElectricTime < 3)
				AddScoreItemDetail("0160601600", 0);//持续通电时间适中
			else
				AddScoreItemDetail("0160601600", 0);//持续通电时间过长
		}

		float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
		
		float rightTooSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
		
		float ToolSpeed = std::max(leftToolSpeed, rightTooSpeed);

		if (ToolSpeed > 0.0)
		{
			if (ToolSpeed <= 5.0 && ToolSpeed > 0.0)
				AddScoreItemDetail("0160800800",0);
			else if (ToolSpeed > 5.0 && ToolSpeed <= 10.0)
				AddScoreItemDetail("0160800801", 0);
			else
				AddScoreItemDetail("0160800802", 0);
		}
	}

	if (m_is_finish)
	{
		float usedtime = GetElapsedTime();

		if (usedtime < 120)
			AddScoreItemDetail("0160900500", 0);

		if (usedtime < 180)
			AddScoreItemDetail("0160900501", 0);

		else
			AddScoreItemDetail("0160900502", 0);
	}

	__super::OnSaveTrainingReport();
}

void CElectroCoagulationTrain::OnHandleEvent(MxEvent* pEvent)
{
	if (m_is_finish)
		return;

	MxToolEvent * pToolEvent = static_cast<MxToolEvent*>(pEvent);
	switch (pEvent->m_enmEventType)
	{
	case MxEvent::MXET_BurnCutFaceBegin:
		//m_burnOnceBeginTime = GetElapsedTime();
		break;

	case MxEvent::MXET_BurnCutFaceEnd:
		//m_burnOnceEndTime = 
		break;

	}

}