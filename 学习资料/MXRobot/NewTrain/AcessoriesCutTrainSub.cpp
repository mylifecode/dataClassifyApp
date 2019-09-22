#include "AcessoriesCutTrainSub.h"
#include "CustomConstraint.h"
#include <basetsd.h>
#include"Instruments/MisCTool_PluginSuction.h"
#include "Inception.h"
#include "WaterPool.h"
#include "EffectManager.h"
#include "WaterManager.h"
#include <algorithm>
#include "Instruments/Tool.h"
#include "QKeyEvent"

extern UINT32 AreaMark[];// = 
// {
// 	0x00001,		//	子宫体
// 	0x00004,		//	左输卵管
// 	0x00008,		//	右输卵管
// 	0x00010,		//	左卵巢
// 	0x00020,		//	右卵巢
// 	0x00040,		//	左悬韧带固有韧带
// 	0x00080,		//	右悬韧带固有韧带
// 	0x00100,		//	可烧白区域1
// 	0x00200,		//	可烧白区域2
// 	0x01000,		//	判定区域1
// 	0x02000,		//	判定区域2
// 	0x10000,		//	病灶区3
// 	0x20000,		//	病灶区4
// 	0x40000,		//	病灶区5
// };

void GetRGBComponent(uint32 value , uint8 & red , uint8 & green , uint8 & blue)
{
	blue = value & 0x000000ff;
	value = value >> 8;
	green = value & 0x000000ff;
	value = value >> 8;
	red = value & 0x000000ff;
}

CAcessoriesCutTraining_1st::CAcessoriesCutTraining_1st( const Ogre::String & strName )
:	CAcessoriesCutTraining(strName)
,	cutLevel(0)
,	currentLevel(0)
,	generateBleedPointForFinish(false)
{
	memset(nodeLineCuted, false,sizeof(nodeLineCuted));

}


bool CAcessoriesCutTraining_1st::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = CAcessoriesCutTraining::Initialize(pTrainingConfig , pToolConfig);
	searchRoad(0);
	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);


	

	{

// 		Ogre::ColourValue col;
// 		m_tex = Ogre::TextureManager::getSingleton().load("Adnexa_Uteri.tga" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 0);
// 		Ogre::Image img;
// 		m_tex->convertToImage(img);
// 		for(size_t f = 0 ; f < pOO->m_OriginFaces.size() ; f++)
// 		{
// 			MMO_Face & organface = pOO->m_OriginFaces[f];
// 
// 			for(int i = 0; i < 3; i ++)
// 			{
// 				int tcx = organface.m_TextureCoord[i].x *(m_AreaWidth - 1);
// 
// 				int tcy = organface.m_TextureCoord[i].y*(m_AreaHeight - 1);
// 				int index = 0xfff00000;
// 				if (organface.vi[i] > 0 && organface.vi[i] < 0x00100000)
// 				{
// 					index += organface.vi[i];
// 				}
// 				col.setAsABGR(index);
// 				img.setColourAt(col, tcx, tcy, 0);
// 			}
// 		}
// 		img.save("c://CAcessoriesCutTraining_1st.bmp");
	}

	for(size_t f = 0 ; f < pOO->m_OriginFaces.size() ; f++)
	{
		MMO_Face & organface = pOO->m_OriginFaces[f];

		GFPhysSoftBodyFace * physface = organface.m_physface;

		if(physface == 0)
			continue;
		for(int v = 0 ; v < 3 ; v++)
		{
			//GFPhysSoftBodyNode * FaceNode = physface->m_Nodes[v];
			Ogre::Vector2 vertTexCoord = organface.GetTextureCoord(v);

			uint32 areaValue = GetPixelFromAreaTesture(vertTexCoord.x, vertTexCoord.y);
			if ((areaValue & AreaMark[AT_Focal_Zone_1]) != 0)
			{
				physface->m_Nodes[v]->m_Flag = physface->m_Nodes[v]->m_Flag | AreaMark[AT_Focal_Zone_1];
			}
		}
	}

	int counter = 0;
	GFPhysSoftBodyNode* node = pOO->m_physbody->GetNodeList();
	while(node!= NULL)
	{
		if ((node->m_Flag & AreaMark[AT_Focal_Zone_1])!= 0)
		{
			counter++;
		}
		node = node->m_Next;
	}
	return result;
}

std::string		CutStart[4] = {
	"CutStart0",
	"CutStart1",
	"CutStart2",
	"CutStart3"
};
std::string		CutNotStart[4] = {
	"CutNotStart0",
	"CutNotStart1",
	"CutNotStart2",
	"CutNotStart3"
};

void CAcessoriesCutTraining_1st::receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif )
{
	m_bNeedCheck = true;
	int matchPointNum = 0;
	int missPointNum = 0;
	memset(areaMatchNum,0,sizeof(areaMatchNum));
	m_mistakeCounterMax = 0;
	CAcessoriesCutTraining::receiveCheckPointList(type, texCord, TFUV , tool , oif);
	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	GFPhysSoftBodyShape &shape = pOO->m_physbody->GetSoftBodyShape();
	GFPhysSoftBodyEdge * edge = NULL;

	m_bNeedCheck = true;
	if (MisNewTraining::OCPT_Burn == type)
	{
		int PointNum = TFUV.size();
		for (int i = 0; i < PointNum; ++i)
		{
			if (TFUV[i].x > 0.2f && TFUV[i].x < 0.8f && TFUV[i].y > 0.2f && TFUV[i].y < 0.8f)
			{
				uint32 areaValue = GetPixelFromAreaTesture(texCord[i].x, texCord[i].y);
				uint32 markValue = 0x00ffffff & areaValue;
				if(markValue != 0)
				{
					if ((markValue & AreaMark[AT_Determine_Area_Burn_1])!= 0)
					{
						areaMatchNum[AT_Determine_Area_Burn_1] ++;
					}
					else if ((markValue & AreaMark[AT_Determine_Area_Burn_2])!= 0)
					{
						areaMatchNum[AT_Determine_Area_Burn_2] ++;
					}
					else
					{
						testMistake(markValue);
					}
					// checked;
					matchPointNum++;
				}
				else
				{
					missPointNum++;
				}
			}
		}
		{
			m_foundMistake = false;

			if (0 == m_mistakeCounterMax)
			{
				m_foundMistake = false;
			}
			else if (areaMatchNum[AT_Uterus] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Uterus");
				CScoreMgr::Instance()->Grade("Damage_Uterus");
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Left");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Right");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Right");
			}
			else if (areaMatchNum[AT_Ovary_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Left");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ovary_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Right");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Right");
			}
			else if (areaMatchNum[AT_Ligament_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Left");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ligament_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Right");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Right");
			}
		}

		if (m_foundMistake != true)
		{
			if (areaMatchNum[AT_Determine_Area_Burn_1] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Accurate_Burn");
			}
		}
	
		//replase
		//if (pOO->m_realDynamicBleedFacesIndex.empty() && generateBleedPointForFinish)
		if (pOO->GetNumOfDynamicBlood() == 0 && generateBleedPointForFinish)
		{
			CTipMgr::Instance()->ShowTip("BurnAfterCut");
			CScoreMgr::Instance()->Grade("BurnAfterCut");
		}
	}
	else if (MisNewTraining::OCPT_Cut == type)
	{
		AreaCutDetermationSave();
		BYTE whiteMark = 0;
		int PointNum = texCord.size();
		for (int i = 0; i < PointNum; ++i)
		{
			uint32 areaValue = GetPixelFromAreaTesture(texCord[i].x, texCord[i].y);
			{
				int tcx = texCord[i].x*(m_BurnMark_W - 1);

				int tcy = texCord[i].y*(m_BurnMark_H - 1);

				// 				whiteMark = m_BurnMarkArea[tcy * m_BurnMark_H + tcx];
			}

			uint32 markValue = 0x00ffffff & areaValue;
			if(markValue != 0)
			{
				if ((markValue & AreaMark[AT_Determine_Area_Cut_1])!= 0)
				{
					areaMatchNum[AT_Determine_Area_Cut_1] ++;
				}
				else if ((markValue & AreaMark[AT_Determine_Area_Cut_2])!= 0)
				{
					areaMatchNum[AT_Determine_Area_Cut_2] ++;
				}
				else
				{
					testMistake(markValue);
				}
				// checked;
				matchPointNum++;
			}
			else
			{
				missPointNum++;
			}
		}
		{
			m_foundMistake = false;

			if (0 == m_mistakeCounterMax)
			{
				m_foundMistake = false;
			}
			else if (areaMatchNum[AT_Uterus] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Uterus");
				if (m_ScoreTimes[AT_Focal_Zone_1]< 10)					CScoreMgr::Instance()->Grade("Damage_Uterus");
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Left");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Right");
				if (m_ScoreTimes[AT_Focal_Zone_1]< 10)					CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Right");
			}
			else if (areaMatchNum[AT_Ovary_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Left");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ovary_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Right");
				if (m_ScoreTimes[AT_Focal_Zone_1]< 10)					CScoreMgr::Instance()->Grade("Damage_Ovary_Right");
			}
			else if (areaMatchNum[AT_Ligament_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Left");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ligament_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Right");
				if (m_ScoreTimes[AT_Focal_Zone_1]< 10)					CScoreMgr::Instance()->Grade("Damage_Ligament_Right");
			}
		}
		if (m_foundMistake != true)
		{
			if (areaMatchNum[AT_Determine_Area_Cut_1] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Accurate_Cut");
				m_ScoreTimes[AT_Determine_Area_Cut_1]++;
			}
			if (areaMatchNum[AT_Determine_Area_Cut_2] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Accurate_Cut");
				m_ScoreTimes[AT_Determine_Area_Cut_2]++;
			}
		}
		for(int lineIndex = 0; lineIndex < 4; lineIndex++)
		{
			if (nodeLineCuted[lineIndex])
			{
				continue;
			}
			else
			{
				int misslineCount = 0;
				for (int i = 0; i < nodeLine[lineIndex].size(); i+=2)
				{
					edge = shape.GetEdge(nodeLine[lineIndex][i], nodeLine[lineIndex][i+1]);
					if (edge == NULL)
					{
						misslineCount++;
					}
				}
				if (misslineCount > 0)
				{
					nodeLineCuted[lineIndex] = true;
					if (lineIndex == 0)
					{
						if (nodeLineCuted[1] == false && nodeLineCuted[2] == false && nodeLineCuted[3] == false )
						{
							CTipMgr::Instance()->ShowTip(CutStart[lineIndex]);
							CScoreMgr::Instance()->Grade(CutStart[lineIndex]);
						}
						else
						{
							CScoreMgr::Instance()->Grade(CutNotStart[lineIndex]);
						}
					}
					else
					{
						if (nodeLineCuted[lineIndex-1])
						{
							CTipMgr::Instance()->ShowTip(CutStart[lineIndex]);
							CScoreMgr::Instance()->Grade(CutStart[lineIndex]);
						}
						else
						{
							CScoreMgr::Instance()->Grade(CutNotStart[lineIndex]);
						}
						if (lineIndex == 3)
						{
							generateBleedPointForFinish = true;
						}
					}
				}

			}
		}
		if (generateBleedPointForFinish == true)
		{
#if(0)
			for (int i = 0; i < nodeLine[3].size(); i+=2)
			{
				edge = shape.GetEdge(nodeLine[3][i], nodeLine[3][i+1]);
				if (edge == NULL)
				{
					nodeLine_4_Influseced.insert(nodeLine[3][i]);
					nodeLine_4_Influseced.insert(nodeLine[3][i+1]);
				}
			}

			std::set<GFPhysSoftBodyNode*>::iterator itor;

			for(size_t f = 0 ; f < pOO->m_OriginFaces.size() ; f++)
			{
				MMO_Face & organface = pOO->m_OriginFaces[f];

				if (organface.m_physface)
				{
					for(int fn = 0; fn < 3 ; fn++)
					{
						itor = nodeLine_4_Influseced.find(organface.m_physface->m_Nodes[fn]);
						if (itor != nodeLine_4_Influseced.end())
						{
							pOO->m_realDynamicBleedFacesIndex.push_back(f);
							nodeLine_4_Influseced.erase(itor);
						}
					}
				}
			}
#endif
		}

		float testresult = 0;
		std::vector<Ogre::Vector2> reginInfo;
		pOO->TestLinkingArea(AreaMark[AT_Focal_Zone_1] , reginInfo);

		int nodeNum = 0;
		int nodeNum_mark = 0;
		int maxIndex;
		int maxNum = 0;
		for(int i = 0; i < reginInfo.size(); i++)
		{
			nodeNum+= reginInfo[i].x;
			nodeNum_mark += reginInfo[i].y;
			if (maxNum < reginInfo[i].x)
			{
				maxNum = reginInfo[i].x;
				maxIndex = i;
			}
		}
		float cutPercent = reginInfo[maxIndex].y / nodeNum_mark;
		if(cutPercent < 0.1f)
		{
			TrainingFinish();
			if(m_ScoreSys)
			{
			   m_ScoreSys->SetTrainSucced();
			}
			//CTipMgr::Instance()->ShowTip("Tip_Cut_Percent_0");
			CScoreMgr::Instance()->Grade("CutClean");
		}
		else 	if(cutPercent < 0.2f)
		{
			CTipMgr::Instance()->ShowTip("Tip_Cut_Percent_1");
		}
		else 	if(cutPercent < 0.5f)
		{
			CTipMgr::Instance()->ShowTip("Tip_Cut_Percent_4");
		}
		else
		{
			CTipMgr::Instance()->ShowTip("Tip_Cut_Percent_9");
		}
	}
}

#if 0
void CAcessoriesCutTraining_1st::loadNodeUVs()
{
	std::ifstream stream;
	stream.open("../Config/MultiPortConfig/Gynaecology/Tubalcut_Config.txt");
	if(stream.is_open())
	{
		char buffer[100];
		stream.getline(buffer,99);
		std::string str = buffer;
		int num = atoi(str.c_str());
		std::string	sstr[2];
		int dotIndex;
		for(int i = 0; i< num; i++)
		{
			stream.getline(buffer,99);
			str = buffer;
			dotIndex = str.find(",");
			sstr[0] = str.substr(0, dotIndex);
			sstr[1] = str.substr(dotIndex + 1);
			nodeUVs[i].x = atoi(sstr[0].c_str());
			nodeUVs[i].y = atoi(sstr[1].c_str());
		}

	}

}

void CAcessoriesCutTraining_1st::matchNodes()
{
	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	GFPhysSoftBodyNode* nearestNode;
	int		nearestNode_Index;
	float	minDis = 10000;

	for (int nodeIndex = 0 ; nodeIndex< 8; nodeIndex++)
	{
		nodeUVs[nodeIndex] = nodeUVs[nodeIndex] / 2048;
		for(size_t f = 0 ; f < pOO->m_OriginFaces.size() ; f++)
		{
			MMO_Face & organface = pOO->m_OriginFaces[f];

			for(int i = 0; i < 3; i ++)
			{
				Ogre::Vector2 dict = organface.m_TextureCoord[i] - nodeUVs[nodeIndex];
				float dis = dict.length();
				if (minDis > dis)
				{
					minDis = dis;
					nearestNode = organface.m_physface->m_Nodes[i];
					nearestNode_Index = organface.vi[i];
				}
			}
		}
		nodes[nodeIndex] = nearestNode;
		nodes_ID[nodeIndex] = nearestNode_Index;
		minDis = 10000;
	}
}
#endif

void CAcessoriesCutTraining_1st::searchRoad( int index )
{
	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().load("texPointBS.png" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 0);
	Ogre::Image img;
	tex->convertToImage(img);
// 	img.save("c://ss_1st.bmp");

	int	col = img.getColourAt(0, 0, 0).getAsARGB();

	int colArea[4] = { 0xffff00ff, 0xff00ff00, 0xff0000ff, 0xffff0000};

//	for (int nodeIndex = 0 ; nodeIndex< 8; nodeIndex++)
	{
		for(size_t f = 0 ; f < pOO->m_OriginFaces.size() ; f++)
		{
			MMO_Face & organface = pOO->m_OriginFaces[f];
			int matchArea = -1;
			for(int Vi= 0; Vi < 3; Vi++)
			{
				Ogre::Vector2 vertTexCoord = organface.GetTextureCoord(Vi);

				col = img.getColourAt(vertTexCoord.x * 2048, vertTexCoord.y * 2048, 0).getAsARGB();

				if(col == colArea[0])
				{
					matchArea=0;
				}
				else if(col == colArea[1])
				{
					matchArea=1;
				}
				else if(col == colArea[2])
				{
					matchArea=2;
				}
				else if(col == colArea[3])
				{
					matchArea=3;
				}
			}

			if(matchArea != -1)
			{
				nodeLine[matchArea].push_back(organface.m_physface->m_Nodes[0]);
				nodeLine[matchArea].push_back(organface.m_physface->m_Nodes[1]);
				nodeLine[matchArea].push_back(organface.m_physface->m_Nodes[0]);
				nodeLine[matchArea].push_back(organface.m_physface->m_Nodes[2]);
				nodeLine[matchArea].push_back(organface.m_physface->m_Nodes[1]);
				nodeLine[matchArea].push_back(organface.m_physface->m_Nodes[2]);
			}
		}
	}


}

void CAcessoriesCutTraining_1st::doFinishCount()
{
	m_ScoreTimes[AT_Focal_Zone_3]++;
	if (m_ScoreTimes[AT_Focal_Zone_3] > 200)
	{
		return;
	}
	else if (m_ScoreTimes[AT_Focal_Zone_3] == 200)
	{
		CTipMgr::Instance()->ShowTip("TrainingEnded");
	}
}

CAcessoriesCutTraining_2nd::CAcessoriesCutTraining_2nd( const Ogre::String & strName )
:	CAcessoriesCutTraining(strName)
{

}

void CAcessoriesCutTraining_2nd::receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif )
{
	int matchPointNum = 0;
	int missPointNum = 0;
	memset(areaMatchNum,0,sizeof(areaMatchNum));
	m_mistakeCounterMax = 0;
	m_bNeedCheck = true;

	if (MisNewTraining::OCPT_Burn == type)
	{
		int PointNum = TFUV.size();
		for (int i = 0; i < PointNum; ++i)
		{
			if (TFUV[i].x > 0.2f && TFUV[i].x < 0.8f && TFUV[i].y > 0.2f && TFUV[i].y < 0.8f)
			{
				uint32 areaValue = GetPixelFromAreaTesture(texCord[i].x, texCord[i].y);
				uint32 markValue = 0x00ffffff & areaValue;
				if(markValue != 0)
				{
					if ((markValue & AreaMark[AT_Determine_Area_Burn_1])!= 0)
					{
						areaMatchNum[AT_Determine_Area_Burn_1] ++;
					}
					else if ((markValue & AreaMark[AT_Determine_Area_Burn_2])!= 0)
					{
						areaMatchNum[AT_Determine_Area_Burn_2] ++;
					}
					else
					{
						testMistake(markValue);
					}
					// checked;
					matchPointNum++;
				}
				else
				{
					missPointNum++;
				}
			}
		}
		bool misFound = false;
		{
			int mistakeTimes = 1;
			m_foundMistake = false;

			if (0 == m_mistakeCounterMax)
			{
				m_foundMistake = false;
			}
			else if (areaMatchNum[AT_Uterus] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Uterus");
				if (m_ScoreTimes[AT_Uterus] < 10)
				{
					CScoreMgr::Instance()->Grade("Damage_Uterus");
					m_ScoreTimes[AT_Uterus]++;
				}
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Left");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Right");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Right");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ovary_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Left");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ovary_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Right");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Right");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ligament_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Left");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ligament_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Right");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Right");
				m_bFinished = true;
			}
		}
		if (m_foundMistake != true)
		{
			if (m_ScoreTimes[AT_Determine_Area_Burn_1] == 0 && areaMatchNum[AT_Determine_Area_Burn_1] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Accurate_Burn_1");
				CScoreMgr::Instance()->Grade("TrainingBurnLeft");
				m_ScoreTimes[AT_Determine_Area_Burn_1]++;
			}
			else if (areaMatchNum[AT_Determine_Area_Burn_2] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Accurate_Burn_2");
				CScoreMgr::Instance()->Grade("TrainingBurnRight");
				m_ScoreTimes[AT_Determine_Area_Burn_2]++;
			}
		}
	}
	else if (MisNewTraining::OCPT_Cut == type)
	{
		AreaCutDetermationSave();
		int PointNum = texCord.size();
		BYTE whiteMark = 0;
		int burnPoint = 0;
		int unburnPoint = 0;
		for (int i = 0; i < PointNum; ++i)
		{
			uint32 areaValue = GetPixelFromAreaTesture(texCord[i].x, texCord[i].y);
			uint32 markValue = 0x00ffffff & areaValue;
			Ogre::ColourValue color = GetColorFromImage(texCord[i].x, texCord[i].y);

			{
				int tcx = texCord[i].x*(m_BurnMark_W - 1);

				int tcy = texCord[i].y*(m_BurnMark_H - 1);

// 				whiteMark = m_BurnMarkArea[tcy * m_BurnMark_H + tcx];
			}

			if(markValue != 0)
			{
				if ((markValue & AreaMark[AT_Determine_Area_Cut_1])!= 0)
				{
					areaMatchNum[AT_Determine_Area_Cut_1] ++;
				}
				else if ((markValue & AreaMark[AT_Determine_Area_Cut_2])!= 0)
				{
					areaMatchNum[AT_Determine_Area_Cut_2] ++;
				}
				else
				{
					testMistake(markValue);
				}
				// checked;
				matchPointNum++;
			}
			else
			{
				missPointNum++;
			}
			if (color.g > 0x40)
			{
				burnPoint++;
			}
			else
			{
				unburnPoint++;
			}
		}

		{
			int mistakeTimes = 1;
			m_foundMistake = false;

			if (0 == m_mistakeCounterMax)
			{
				m_foundMistake = false;
			}
			else if (areaMatchNum[AT_Uterus] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Uterus");
				CScoreMgr::Instance()->Grade("Damage_Uterus");
//				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Left");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Left");
//				m_ScoreTimes[AT_Fallopian_Tube_Left] += mistakeTimes;
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Right");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Right");
//				m_ScoreTimes[AT_Fallopian_Tube_Right] += mistakeTimes ;
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ovary_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Left");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Left");
// 				m_ScoreTimes[AT_Ovary_Left] += mistakeTimes ;
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ovary_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Right");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Right");
// 				m_ScoreTimes[AT_Ovary_Right] += mistakeTimes ;
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ligament_Left]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Left");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Left");
// 				m_ScoreTimes[AT_Ligament_Left] += mistakeTimes ;
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ligament_Right]  == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Right");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Right");
// 				m_ScoreTimes[AT_Ligament_Right] += mistakeTimes ;
				m_bFinished = true;
			}
		}
		if (m_foundMistake != true)
		{
			if (areaMatchNum[AT_Determine_Area_Cut_1] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Accurate_Cut_1");
				CScoreMgr::Instance()->Grade("TrainingCutLeft");
				m_ScoreTimes[AT_Determine_Area_Cut_1]++;
			}
			if (areaMatchNum[AT_Determine_Area_Cut_2] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Accurate_Cut_2");
				CScoreMgr::Instance()->Grade("TrainingCutRight");
				m_ScoreTimes[AT_Determine_Area_Cut_2]++;
			}
			if (burnPoint < unburnPoint * 5)
			{
				if (areaMatchNum[AT_Determine_Area_Cut_1] > 0)
				{
					CTipMgr::Instance()->ShowTip("Tip_Cut_Unburn_Area");
					if (m_ScoreTimes[AT_Focal_Zone_1] == 0)
					{
						CScoreMgr::Instance()->Grade("Miss_Burn_Area_1");
						m_ScoreTimes[AT_Focal_Zone_1]++;
					}
				}
				if (areaMatchNum[AT_Determine_Area_Cut_2] > 0)
				{
					CTipMgr::Instance()->ShowTip("Tip_Cut_Unburn_Area");
					if (m_ScoreTimes[AT_Focal_Zone_2] == 0)
					{
						CScoreMgr::Instance()->Grade("Miss_Burn_Area_2");
						m_ScoreTimes[AT_Focal_Zone_2]++;
					}
				}
			}
		}
	}
}

void CAcessoriesCutTraining_2nd::doFinishCount()
{
	m_ScoreTimes[AT_Focal_Zone_3]++;
	if (m_ScoreTimes[AT_Focal_Zone_3] > 200)
	{
		return;
	}
	else if (m_ScoreTimes[AT_Focal_Zone_3] == 200)
	{
		CTipMgr::Instance()->ShowTip("TrainingEnded");
		
	}
}
CAcessoriesCutTraining_3rd::CAcessoriesCutTraining_3rd( const Ogre::String & strName )
:	CAcessoriesCutTraining(strName)
{

}

void CAcessoriesCutTraining_3rd::receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUVss  , ITool * tool , MisMedicOrganInterface * oif  )
{
	int matchPointNum = 0;
	int missPointNum = 0;
	memset(areaMatchNum,0,sizeof(areaMatchNum));
	m_mistakeCounterMax = 0;
	m_bNeedCheck = true;
	if (MisNewTraining::OCPT_Clip == type)
	{
		int PointNum = texCord.size();
		for (int i = 0; i < PointNum; ++i)
		{
			if (texCord[i].x > 0.2f && texCord[i].x < 0.8f && texCord[i].y > 0.2f && texCord[i].y < 0.8f)
			{
				uint32 areaValue = GetPixelFromAreaTesture(texCord[i].x, texCord[i].y);
				uint32 markValue = 0x00ffffff & areaValue;
				if(markValue != 0)
				{
					if ((markValue & AreaMark[AT_Determine_Area_Cut_1])!= 0)
					{
						areaMatchNum[AT_Determine_Area_Cut_1] ++;
					}
					else if ((markValue & AreaMark[AT_Determine_Area_Cut_2])!= 0)
					{
						areaMatchNum[AT_Determine_Area_Cut_2] ++;
					}
					else
					{
						testMistake(markValue);
					}
					// checked;
					matchPointNum++;
				}
				else
				{
					missPointNum++;
				}
			}
		}
		bool misFound = false;
		RegisterMistake(1);

		if (m_foundMistake != true)
		{
			if (areaMatchNum[AT_Determine_Area_Cut_1] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Action_Determine_Area_1");
				if (m_ScoreTimes[AT_Determine_Area_Cut_1] == 0)
				{
					CScoreMgr::Instance()->Grade("ClipLeft");
				}
				else
					CScoreMgr::Instance()->Grade("ClipLeft_Unnecessary");
				m_ScoreTimes[AT_Determine_Area_Cut_1]++;
			}
			else if (areaMatchNum[AT_Determine_Area_Cut_2] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Action_Determine_Area_2");
				if (m_ScoreTimes[AT_Determine_Area_Cut_2] == 0)
				{
					CScoreMgr::Instance()->Grade("ClipRight");
				}
				else
					CScoreMgr::Instance()->Grade("ClipRight_Unnecessary");
				m_ScoreTimes[AT_Determine_Area_Cut_2]++;
			}
			else
				CTipMgr::Instance()->ShowTip("Tip_Accurate_Clip");
		}
		else
		{
			if (areaMatchNum[AT_Determine_Area_Cut_1] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Action_Determine_Area_1_Mistake");
				CScoreMgr::Instance()->Grade("TrainingCutLeft");
				m_ScoreTimes[AT_Determine_Area_Cut_1]++;
			}
			else if (areaMatchNum[AT_Determine_Area_Cut_2] > 0)
			{
				CTipMgr::Instance()->ShowTip("Tip_Action_Determine_Area_2_Mistake");
				CScoreMgr::Instance()->Grade("TrainingCutRight");
				m_ScoreTimes[AT_Determine_Area_Cut_2]++;
			}
			else
			{
				CTipMgr::Instance()->ShowTip("TrainingEndedByMistake");
				CScoreMgr::Instance()->Grade("TrainingNotFinished");
				m_bFinished = true;
			}
		}
	}
}

void CAcessoriesCutTraining_3rd::doFinishCount()
{
	m_ScoreTimes[AT_Focal_Zone_3]++;
	if (m_ScoreTimes[AT_Focal_Zone_3] > 200)
	{
		return;
	}
	else if (m_ScoreTimes[AT_Focal_Zone_3] == 200)
	{
		CTipMgr::Instance()->ShowTip("TrainingEnded");
	}
}

CAcessoriesCutTraining_4th::CAcessoriesCutTraining_4th(const Ogre::String & strName)
:	CAcessoriesCutTraining(strName),
m_BeginCut(false),
m_LastLocateResult(false),
m_NeedShowCutTip(true),
m_bRightDir(false),
m_bCompleteCut(false),
m_organ(NULL)
{
    TipcolArea[0] = 0xffff0000;//子宫
    TipcolArea[1] = 0xff00ff00;//卵巢
    TipcolArea[2] = 0xffffff00;//右侧输卵管
    TipcolArea[3] = 0xffff00ff;//圆韧带
    TipcolArea[4] = 0xff007f00;//左侧输卵管

    colArea[0] = 0xff0000ff;//切割顺序从伞端到宫角
    colArea[1] = 0xff00ffff;
    colArea[2] = 0xff000000;
    colArea[3] = 0xffffffff;

}


bool CAcessoriesCutTraining_4th::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
    bool result = CAcessoriesCutTraining::Initialize(pTrainingConfig , pToolConfig);

    MisMedicOrganInterface * tmp = m_DynObjMap[12];
    m_organ = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

    m_Area->convertToImage(m_TexImage);

    m_ElecCutColorCode.reserve(5);

    for (int i = 0 ;i <4;i++)
    {
        notexist[i] = true;
    }
    return result;
}

bool CAcessoriesCutTraining_4th::Checkfinish()
{
    int numOfAllNode = 0;
    int numOfNodeinGreatestSubpart = 0;
    bool cutsucess = false;

    if (m_organ->GetNumSubParts() == 1)
    {
        return false;
    }

    for(std::size_t s = 0 ; s < m_organ->GetNumSubParts();++s)
    {
        numOfAllNode += m_organ->GetSubPart(s)->m_Nodes.size();
    }

	for (std::size_t s = 0; s < m_organ->GetNumSubParts(); ++s)
    {
		int n = m_organ->GetSubPart(s)->m_Nodes.size();
        if (n >= numOfNodeinGreatestSubpart)
        {
            numOfNodeinGreatestSubpart = n;
        }
        float ratio = (float)numOfNodeinGreatestSubpart / numOfAllNode;

        if(ratio < 0.85f)
        {
            cutsucess = true;        
            break;
        }
    }    
    std::vector<int>::iterator it = m_ElecCutColorCode.end() - 1;  
    //最后一刀如果不在切割顺序里也让训练结束
	if (cutsucess && *it == 0/*&& (*it == 0||*it == 1||*it == 2||*it == 3)*/)
    {           
        m_bCompleteCut = true;
        Inception::Instance()->EmitShowMovie("End");
        TrainingFinish();
        if(m_ScoreSys)
        {
            m_ScoreSys->SetTrainSucced();
        }
        return true;
    }
    else
    {
        return false;
    }
}


bool CAcessoriesCutTraining_4th::Update(float dt)
{
	CAcessoriesCutTraining::Update(dt);
#if 1 
    CTool * rightTool = (CTool*)(m_pToolsMgr->GetRightTool());
    CTool * lefTool = (CTool*)(m_pToolsMgr->GetLeftTool());
    bool IsElecButtonLeft = false;
    bool IsElecButtonRight = false;
    if (rightTool )
    {
        IsElecButtonRight = (rightTool->m_bElectricLeftPad ||rightTool->m_bElectricRightPad);
    }
    if (lefTool)
    {
        IsElecButtonLeft = (lefTool->m_bElectricLeftPad ||lefTool->m_bElectricRightPad); 
    }
    
    if (IsElecButtonRight)
    {
        
        int col = m_TexImage.getColourAt(rightTool->m_TextureCoord.x * 2048, rightTool->m_TextureCoord.y * 2048, 0).getAsARGB();
        if (col == TipcolArea[0])
        {
            if (rightTool->m_CumTime  > 1.0f && rightTool->m_CumTime < 5.0f)
            {
                CTipMgr::Instance()->ShowTip("Tip_Damage_Uterus");
                ShowNextTip();
            }
            else if(rightTool->m_CumTime >= 5.0f)
            {
                TrainingErrorWithoutQuit("Tip_Destory_Uterus");
            }
            
        }
        else if (col == TipcolArea[1])
        {
            if (rightTool->m_CumTime  > 1.0f && rightTool->m_CumTime < 5.0f)
            {
                CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary");
                ShowNextTip();
            }
            else if(rightTool->m_CumTime >= 5.0f)
            {
                TrainingErrorWithoutQuit("Tip_Destory_Ovary");
            }
        }
        else if (col == TipcolArea[2])
        {
            if (rightTool->m_CumTime  > 1.0f && rightTool->m_CumTime < 5.0f)
            {
                CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Right");
                ShowNextTip();
            }
            else if(rightTool->m_CumTime >= 5.0f)
            {
                TrainingErrorWithoutQuit("Tip_Destory_Fallopian_Right");
            }
        }
        else if (col == TipcolArea[3])
        {
            if (rightTool->m_CumTime  > 1.0f && rightTool->m_CumTime < 5.0f)
            {
                CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament");
                ShowNextTip();
            }
            else if(rightTool->m_CumTime >= 5.0f)
            {
                TrainingErrorWithoutQuit("Tip_Destory_Ligament");
            }
        }

    }

    if (IsElecButtonLeft)
    {

        int col = m_TexImage.getColourAt(lefTool->m_TextureCoord.x * 2048, lefTool->m_TextureCoord.y * 2048, 0).getAsARGB();
        if (col == TipcolArea[0])
        {
            if (lefTool->m_CumTime  > 1.0f && lefTool->m_CumTime < 5.0f)
            {
                CTipMgr::Instance()->ShowTip("Tip_Damage_Uterus");
                ShowNextTip();
            }
            else if(lefTool->m_CumTime >= 5.0f)
            {
                TrainingErrorWithoutQuit("Tip_Destory_Uterus");
            }

        }
        else if (col == TipcolArea[1])
        {
            if (lefTool->m_CumTime  > 1.0f && lefTool->m_CumTime < 5.0f)
            {
                CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary");
                ShowNextTip();
            }
            else if(lefTool->m_CumTime >= 5.0f)
            {
                TrainingErrorWithoutQuit("Tip_Destory_Ovary");
            }
        }
        else if (col == TipcolArea[2])
        {
            if (lefTool->m_CumTime  > 1.0f && lefTool->m_CumTime < 5.0f)
            {
                CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Right");
                ShowNextTip();
            }
            else if(lefTool->m_CumTime >= 5.0f)
            {
                TrainingErrorWithoutQuit("Tip_Destory_Fallopian_Right");
            }
        }
        else if (col == TipcolArea[3])
        {
            if (lefTool->m_CumTime  > 1.0f && lefTool->m_CumTime < 5.0f)
            {
                CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament");
                ShowNextTip();
            }
            else if(lefTool->m_CumTime >= 5.0f)
            {
                TrainingErrorWithoutQuit("Tip_Destory_Ligament");
            }
        }

    }     

#endif
	if (m_bCompleteCut == false)
	{
		Checkfinish();
	}
	
	return true;
}

void CAcessoriesCutTraining_4th::CheckClampAreaRegion(const std::vector<Ogre::Vector2> & texCord , ITool * tool , MisMedicOrganInterface * oif)
{
    std::set<AreaType> regions;
    //if(tool->GetType() == )
    for(size_t t = 0 ; t < texCord.size() ; t++)
    {
        AreaType areaType = getAreaType(oif , texCord[t]);
        regions.insert(areaType);
    }
    std::set<AreaType>::iterator itor = regions.begin();
    for( ; itor != regions.end() ; ++itor)
    {
        AreaType regionType = *itor;
        if(regionType == AT_Uterus)
        {
            m_NumOfUterusBeClamped++;            
        }
        else if(regionType == AT_Fallopian_Tube_Right)
        {            
            m_NumOfFallopianTubeBeClamped++;
        }
        else if(regionType == AT_Ovary_Left || regionType == AT_Ovary_Right)
        {            
            m_NumOfOvaryBeClamped++;
        }
    }
}
void CAcessoriesCutTraining_4th::CheckCutAreaRegion(const std::vector<Ogre::Vector2> & texCord , ITool * tool , MisMedicOrganInterface * oif)
{
    std::set<AreaType> regions;
    //if(tool->GetType() == )
    for(size_t t = 0 ; t < texCord.size() ; t++)
    {
        AreaType areaType = getAreaType(oif , texCord[t]);
        regions.insert(areaType);
    }
    std::set<AreaType>::iterator itor = regions.begin();
    for( ; itor != regions.end() ; ++itor)
    {
        AreaType regionType = *itor;
        if(regionType == AT_Uterus)
        {
            m_NumOfUterusBeDamaged++;            
        }
        else if(regionType == AT_Fallopian_Tube_Right)
        {            
            m_NumOfFallopianTubeBeDamaged++;
        }
        else if(regionType == AT_Ovary_Left || regionType == AT_Ovary_Right)
        {            
            m_NumOfOvaryBeDamaged++;
        }
    }
}

void CAcessoriesCutTraining_4th::receiveCheckPointList(MisNewTraining::OperationCheckPointType type, const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV, ITool * tool, MisMedicOrganInterface * oif)
{

	if (type == MisNewTraining::OCPT_Clamp)
	{
		if (!texCord.empty())
		{
			CheckClampAreaRegion(texCord, tool, oif);
		}
	}
	else if (type == MisNewTraining::OCPT_Cut || type == MisNewTraining::OCPT_ElecCut)
	{
		if (!texCord.empty())
		{
			CheckCutAreaRegion(texCord, tool, oif);
		}
		if (!m_BeginCut)
		{
			m_BeginCut = true;
		}
	}
	bool b_cut_error_tip[5] = { false, false, false, false, false };
	if (MisNewTraining::OCPT_Cut == type)
	{
		if (!texCord.empty())
		{
			//如何把切割输卵管筋膜的顺序引入?
#if 0
			const Ogre::Vector2 & tex = texCord[0];
			int col = m_TexImage.getColourAt(tex.x * 2048, tex.y * 2048, 0).getAsARGB();
#else
			std::set<int> colorset;

			for (int i = 0; i < texCord.size(); i++)
			{
				const Ogre::Vector2 & tex = texCord[i];
				int col = m_TexImage.getColourAt(tex.x * 2048, tex.y * 2048, 0).getAsARGB();
				colorset.insert(col);
			}
#endif
			set<int>::iterator iter0;
			iter0 = colorset.find(colArea[0]);

			set<int>::iterator iter1;
			iter1 = colorset.find(colArea[1]);

			set<int>::iterator iter2;
			iter2 = colorset.find(colArea[2]);

			set<int>::iterator iter3;
			iter3 = colorset.find(colArea[3]);
			//////////////////////////////////////////////////////////////////////////
			set<int>::iterator Tipiter0;
			Tipiter0 = colorset.find(TipcolArea[0]);

			set<int>::iterator Tipiter1;
			Tipiter1 = colorset.find(TipcolArea[1]);

			set<int>::iterator Tipiter2;
			Tipiter2 = colorset.find(TipcolArea[2]);

			set<int>::iterator Tipiter3;
			Tipiter3 = colorset.find(TipcolArea[3]);

			set<int>::iterator Tipiter4;
			Tipiter4 = colorset.find(TipcolArea[4]);
			//////////////////////////////////////////////////////////////////////////
			if (iter0 != colorset.end())
			{
				if (notexist[3])
				{
					//只要回到伞端开始剪，重新记录
					for (int j = 0; j < m_ElecCutColorCode.size(); j++)
					{
						notexist[m_ElecCutColorCode[j]] = true;
					}
					m_ElecCutColorCode.clear();

					//////////////////////////////////////////////////////////////////////////
					m_ElecCutColorCode.push_back(3);
					notexist[3] = false;
				}
			}
			else if (iter1 != colorset.end())
			{
				if (notexist[2] /*&& notexist[3] == false*/)
				{
					m_ElecCutColorCode.push_back(2);
					notexist[2] = false;
				}
			}
			else if (iter2 != colorset.end())
			{
				if (notexist[1] /*&& notexist[2] == false*/)
				{
					m_ElecCutColorCode.push_back(1);
					notexist[1] = false;
				}
			}
			else if (iter3 != colorset.end())
			{
				if (notexist[0] /*&& notexist[1] == false*/)
				{
					m_ElecCutColorCode.push_back(0);
					notexist[0] = false;
				}
			}
			else if (Tipiter0 != colorset.end())
			{
				b_cut_error_tip[0] = true;
			}
			else if (Tipiter1 != colorset.end())
			{
				b_cut_error_tip[1] = true;
			}
			else if (Tipiter2 != colorset.end())
			{
				b_cut_error_tip[2] = true;
			}
			else if (Tipiter3 != colorset.end())
			{
				b_cut_error_tip[3] = true;
			}
			else if (Tipiter4 != colorset.end())
			{
				b_cut_error_tip[4] = true;
			}

			m_find3 = find(m_ElecCutColorCode.begin(), m_ElecCutColorCode.end(), 3);
			m_find2 = find(m_ElecCutColorCode.begin(), m_ElecCutColorCode.end(), 2);
			m_find1 = find(m_ElecCutColorCode.begin(), m_ElecCutColorCode.end(), 1);
			m_find0 = find(m_ElecCutColorCode.begin(), m_ElecCutColorCode.end(), 0);//consider reuse

			if (true)
			{
#if 1
				ProcessCut();
#endif                
			}

			if (b_cut_error_tip[0])
			{
				CTipMgr::Instance()->ShowTip("Tip_Damage_Uterus");
				ShowNextTip();
				b_cut_error_tip[0] = false;
				return;
			}
			if (b_cut_error_tip[1])
			{
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary");
				ShowNextTip();
				b_cut_error_tip[1] = false;
				return;
			}
			if (b_cut_error_tip[2])
			{
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Right");
				ShowNextTip();
				b_cut_error_tip[2] = false;
				return;
			}
			if (b_cut_error_tip[3])
			{
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament");
				ShowNextTip();
				b_cut_error_tip[3] = false;
				return;
			}
			if (b_cut_error_tip[4])
			{
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Left");
				ShowNextTip();
				b_cut_error_tip[4] = false;
				return;
			}

		}
	}
	else if (MisNewTraining::OCPT_Burn == type)
	{

	}
	else
	{

	}
}

void CAcessoriesCutTraining_4th::ShowNextTip()
{
    if (!m_ElecCutColorCode.empty())
    {
        if (m_ElecCutColorCode.end() == m_find3)
        {
            SetNextTip("CutNotStart0",2.5f);
            return;
        }
        else if (m_ElecCutColorCode.end() == m_find2)
        {
            SetNextTip("CutNotStart1",2.5f);
            return;
        }
        else if (m_ElecCutColorCode.end() == m_find1)
        {
            SetNextTip("CutNotStart2",2.5f);
            return;
        }
        else if (m_ElecCutColorCode.end() == m_find0)
        {
            SetNextTip("CutNotStart3",2.5f);
            return;
        }
    }   
}

void CAcessoriesCutTraining_4th::ProcessCut()
{
    //如果在第一次出现2后，检测之前是否有3。
    //如果在第一次出现1后，检测之前是否有2。
    //如果在第一次出现0后，检测之前是否有1。    
    std::vector<int>::iterator pos;

    bool IsExist012 = false;  
    bool IsExist3 = false;    
    bool IsExist2 = false;
    bool IsExist1 = false;

    if (m_ElecCutColorCode.end() == m_find3)
    {    
        CTipMgr::Instance()->ShowTip("CutNotStart0");
        return;
    }
    else
    {
        for (pos = m_ElecCutColorCode.begin();pos != m_find3; pos++)
        {
            if (0 == *pos || 1 == *pos || 2 == *pos)
            {
                IsExist012 = true;
                break;
            }
        }
        if (m_ElecCutColorCode.end() == m_find2)
        {
            if (!IsExist012)
            {
                CTipMgr::Instance()->ShowTip("CutStart0");
            
            }
            else
            {
                CTipMgr::Instance()->ShowTip("CutNotStart0");            
            }           
            return;
        } 
        else
        {
            for (pos = m_ElecCutColorCode.begin();pos != m_find2; pos++)
            {
                if (3 == *pos)
                {
                    IsExist3 = true;
                    break;
                }
            }
            if (m_ElecCutColorCode.end() == m_find1)
            {
                if (!IsExist3)
                {
                    CTipMgr::Instance()->ShowTip("CutNotStart1");                
                }
                else
                {
                    CTipMgr::Instance()->ShowTip("CutStart1");                
                }                
                return;
            } 
            else
            {
                for (pos = m_ElecCutColorCode.begin();pos != m_find1; pos++)
                {
                    if (2 == *pos)
                    {
                        IsExist2 = true;
                        break;
                    }
                }
                if (m_ElecCutColorCode.end() == m_find0)
                {
                    if (!IsExist2)
                    {
                        CTipMgr::Instance()->ShowTip("CutNotStart2");
                    }
                    else
                    {
                        CTipMgr::Instance()->ShowTip("CutStart2");
                    }                   
                    return;
                }
                else
                {
                    for (pos = m_ElecCutColorCode.begin();pos != m_find0; pos++)
                    {
                        if (1 == *pos)
                        {
                            IsExist1 = true;
                            break;
                        }
                    }
                    if (!IsExist1)
                    {
                        CTipMgr::Instance()->ShowTip("CutNotStart3");
                    }
                    else
                    {
                        m_bRightDir = true;
                        CTipMgr::Instance()->ShowTip("CutStart3");
                    }
                }
            }
        }
    }
    return;
}

CAcessoriesCutTraining::AreaType CAcessoriesCutTraining_4th::getAreaType( MisMedicOrganInterface * oif , const Ogre::Vector2 & texCoord )
{
    uint32 areaColor = GetPixelFromAreaTesture(texCoord.x ,texCoord.y);
    uint32 markValue = areaColor & 0x00ffffff;
    uint8 red = 0 , green = 0 , blue = 0;
    GetRGBComponent(markValue , red , green , blue);

    if(oif->m_OrganID == EODT_UTERUS)
    {
        if (red == 255 && green == 0 && blue == 0)
        {
            return AT_Uterus;
        }
        else if (red == 0 && green == 255 && blue == 0)
        {
            return AT_Ovary_Right;
        }
        else if (red == 255 && green == 255 && blue == 0)
        {
            return AT_Fallopian_Tube_Right;
        }
        else if (red == 0 && green == 127 && blue == 0)
        {
            return AT_Fallopian_Tube_Left;
        }
        else if (red == 255 && green == 0 && blue == 255)
        {
            return AT_Ligament_Left;
        }
    }
    return AT_NONE;
}

bool CAcessoriesCutTraining_4th::DetectExplore( float dt )
{
    //if(!m_ExploreResult && !m_BeginCut)
    //{
    //    m_ExploreResult = __super::DetectExplore(dt);

    //    if(m_ExploreResult)
    //    {
    //        CTipMgr::Instance()->ShowTip("Tip_Explore_Finish");            
    //        
    //        if (true)//
    //        {
    //            CTipMgr::Instance()->ShowTip("Tip_Aiming_Fallopian_After_Explore");
    //        }
    //        //SetNextTip("Tip_Aiming_Fallopian_After_Explore",0.5f);
    //    }
    //}
    //return m_ExploreResult;
    return true;
}

bool CAcessoriesCutTraining_4th::DetectLocation( float dt )
{
    //bool locateResult = __super::DetectLocation(dt);

    //if(locateResult != m_LastLocateResult && !m_BeginCut) 
    //{
    //    if(locateResult)
    //        CTipMgr::Instance()->ShowTip("Tip_Aiming_Fallopian");
    //    else
    //        CTipMgr::Instance()->ShowTip("Tip_Not_Aiming_Fallopian");

    //    m_LastLocateResult = locateResult;
    //}

    //if(locateResult && m_NeedShowCutTip && !m_BeginCut)
    //{
    //    CTipMgr::Instance()->ShowTip("Tip_CutDir_Tip");
    //    m_NeedShowCutTip = false;
    //}

    //return locateResult;

    return true;
}

//  void CAcessoriesCutTraining_4th::KeyPress(QKeyEvent * event)
//  {
// 	__super::KeyPress(event);
// 
//  	if (event->key() == Qt::Key_A)
//  	{
//  		DynObjMap::iterator itor = m_DynObjMap.begin();
//  		while (itor != m_DynObjMap.end())
//  		{
//  			MisMedicOrgan_Ordinary *oif = dynamic_cast<MisMedicOrgan_Ordinary *>(itor->second);
//  			if (oif)
//  			{
//  				for (size_t p = 0; p < m_WaterPools.size(); p++)
//  				{
//  					if (!m_WaterPools[p]->IsReject(oif->m_OrganID))
//  					{
//  						Ogre::Vector3 planeorigin = m_WaterPools[p]->GetCurOrigin();
//  						Ogre::Vector3 planenormal = m_WaterPools[p]->GetPlaneNormal();///只一个血池
// 						oif->DrawOrganTextureDipBlood(planeorigin, planenormal);
//  					}
//  				}
//  			}
//  			itor++;
//  		}
//  	}
//  }

void CAcessoriesCutTraining_4th::OnSaveTrainingReport()
{
    MxOperateItem *pOperateItem = NULL;

    AddOperateItem("Clamp_Uterus_Count" , float(m_NumOfUterusBeClamped));
    AddOperateItem("Clamp_Tube_Count" , float(m_NumOfFallopianTubeBeClamped));
    AddOperateItem("Clamp_Ovary_Count" , float(m_NumOfOvaryBeClamped));

    AddOperateItem("Damage_Uterus_Count" , float(m_NumOfUterusBeDamaged));
    AddOperateItem("Damage_Tube_Count" , float(m_NumOfFallopianTubeBeDamaged));
    AddOperateItem("Damage_Ovary_Count" , float(m_NumOfOvaryBeDamaged));
    
    if (m_bRightDir)
    {
        AddOperateItem("Cut_Dir_Right",1,true);
    }   

    if (m_bCompleteCut)
    {
        AddOperateItem("Cut_Complete_Tube",1,true);
    } 

    __super::OnSaveTrainingReport();
}
//=================================================================================
#define UNCUTCODE -2
CAcessoriesCutTraining_5th::CAcessoriesCutTraining_5th(const Ogre::String & strName)
:	CAcessoriesCutTraining(strName) ,
/*m_EmbryoLocation(Ogre::Vector3(-2.96 , -5.2 , -5.5) , 0.9 , true) , */
m_BeginCut(false),
m_LastLocateResult(false) ,
m_IsTookOutSoftly(true),
m_CurErrorCount(0),
m_CutBiasedLevel(0),
m_CutDirErrorCount(0),
m_LastCutColorCode(UNCUTCODE),
m_GlobalMaxCutCode(UNCUTCODE),
m_GlobalMinCutCode(UNCUTCODE),
m_NeedShowCutTip(true),
m_IsEmbryoTookOut(false),
m_IsEmbryoPutIn(false),
m_IsEmbryoRemoved(false),
m_IsFlush(false),
m_IsSuck(false),
m_pEmbryo(NULL),
m_pUterus(NULL) 
,m_Text1(1 , 1, -640 , 100)
{
// 	m_ExploreLocations.push_back(new ViewDetection(Ogre::Vector3(2,-5.55,-3.7) , 0.95 , true));
// 	m_ExploreLocations.push_back(new ViewDetection(Ogre::Vector3(0,-5.55,-3.4) , 0.95 , true));
// 	m_ExploreLocations.push_back(new ViewDetection(Ogre::Vector3(-2.53,-6.61,-3.7) , 0.95 , true));
// 
// 	m_ExploreLocations.push_back(new ViewDetection(Ogre::Vector3(2.5,-6.82,-8.14) , 0.95 , true));
// 	m_ExploreLocations.push_back(new ViewDetection(Ogre::Vector3(-2.93,-6.62,-8.14) , 0.95 , true));
// 
// 	m_EmbryoLocation.SetDetectDist(3.5);
// 	m_EmbryoLocation.DetectDist(true);

	InitErrorTip();

// 	m_StartPosInTexSpace = Ogre::Vector2(0.354492 , 0.18994);
// 	m_EndPosInTexSpace = Ogre::Vector2(0.3427734 , 0.229492);
// 
// 	m_CutDir = m_EndPosInTexSpace - m_StartPosInTexSpace;
// 	m_TransverseCutMaxLength = m_CutDir.length() * 0.5;
// 	m_CutDir.normalise();
// 
// 	m_ErrorDir.x = -m_CutDir.y;
// 	m_ErrorDir.y = m_CutDir.x;
}

void CAcessoriesCutTraining_5th::InitErrorTip()
{
/*
	red 分量
	01 左韧带
	16 左输卵管
	32 左卵巢
	48 右韧带
	64 右输卵管
	80 右卵巢
	96 子宫
	112 左卵巢、输卵管间韧带
	128	右卵巢、输卵管间韧带
*/
	m_CutErrorTipAboutUteri[-1] = Ogre::String("Tip_Damage_Cavity");
	m_CutErrorTipAboutUteri[0] = Ogre::String("Tip_Damage_Ligament_Left");
	m_CutErrorTipAboutUteri[1] = Ogre::String("Tip_Damage_Fallopian_Tube_Left");
	m_CutErrorTipAboutUteri[2] = Ogre::String("Tip_Damage_Ovary_Left");
	m_CutErrorTipAboutUteri[3] = Ogre::String("Tip_Damage_Ligament_Right");
	m_CutErrorTipAboutUteri[4] = Ogre::String("Tip_Damage_Fallopian_Tube_Right");
	m_CutErrorTipAboutUteri[5] = Ogre::String("Tip_Damage_Ovary_Right");
	m_CutErrorTipAboutUteri[6] = Ogre::String("Tip_Damage_Uterus");
	m_CutErrorTipAboutUteri[7] = Ogre::String("Tip_Damage_Ligament_Left");
	m_CutErrorTipAboutUteri[8] = Ogre::String("Tip_Damage_Ligament_Right");

}

bool CAcessoriesCutTraining_5th::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = CAcessoriesCutTraining::Initialize(pTrainingConfig , pToolConfig);

	MisMedicOrganInterface * oif = GetOrgan(EODT_UTERUS);
	MisMedicOrgan_Ordinary * pUterus = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);
	if(pUterus)
	{
		m_pUterus = pUterus;
		m_pUterus->SetMinPunctureDist(0.5f);
	}
	oif = NULL;
	oif = GetOrgan(EODT_PEITAI);
	MisMedicOrgan_Ordinary * pEmbryo = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);
	if (pEmbryo)
	{
		m_pEmbryo = pEmbryo;
		m_pEmbryo->SetMinPunctureDist(0.05f);
		//m_pEmbryo->GetOwnerMaterialPtr()->getTechnique(0)->getPass(0)->setDepthBias(-40);
	}

	return true;
}
MisMedicOrgan_Ordinary * CAcessoriesCutTraining_5th::FilterClampedOrgan(std::vector<MisMedicOrgan_Ordinary *> & organs)
{
    for(size_t c = 0 ; c < organs.size() ; c++)
	{
        MisMedicOrgan_Ordinary * organ = organs[c];
        if(organ->GetOrganType() == EODT_PEITAI)
		{
		   MisMedicOrgan_Ordinary * filterorgan = dynamic_cast<MisMedicOrgan_Ordinary*>(GetOrgan(EODT_UTERUS));
		   return filterorgan;
           break;
		}
	}
	return 0;
}

bool CAcessoriesCutTraining_5th::Update(float dt)
{
	CAcessoriesCutTraining::Update(dt);

	//TestExplore(dt);

	//TestLocation(dt);

	TestTakeOutEmbryo(dt);

	//temp!!
	TestFlushBloodAndSuction(dt);
/*
	if(m_pUterus->m_IsGrasped)
		m_Text1.AddText("movespeed:  " + Ogre::StringConverter::toString(m_pUterus->m_FaceMoveSpeedInClamp));
	else if(m_pUterus->m_IsSucked)
		m_Text1.AddText("movespeed:  " + Ogre::StringConverter::toString(m_pUterus->m_FaceMoveSpeedInSuction));
*/
	return true;
}

void CAcessoriesCutTraining_5th::receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif)
{
	if(type == MisNewTraining::OCPT_Clamp)
	{
		if(!texCord.empty())
		{
			CheckClampAreaRegion(texCord , tool , oif);
		}
	}
	else if(type == MisNewTraining::OCPT_ElecCut)
	{
		if(!texCord.empty())
		{
			const Ogre::Vector2 & tex = texCord[0];
			uint32 areaColor = GetPixelFromAreaTesture(tex.x ,tex.y);
			ProcessElecCut(oif , areaColor);
		}
	}
	else if(type == MisNewTraining::OCPT_Cut)
	{
		if(!texCord.empty())
		{
			std::vector<uint32> areaValues;
			for(size_t t = 0 ; t < texCord.size() ; t++)
			{
				const Ogre::Vector2 & tex = texCord[t];
				uint32 areaColor = GetPixelFromAreaTesture(tex.x ,tex.y);
				areaValues.push_back(areaColor);
			}
			ProcessCut(oif , areaValues);
		}
	}
	else if(type == MisNewTraining::OCPT_Burn_Cut_Face)
	{
		
	}

	//当m_BeginCut为true时不再检测探查和提示定位
	if(type == MisNewTraining::OCPT_Cut || type == MisNewTraining::OCPT_ElecCut)
	{
		if(!m_BeginCut) 
		{
			m_BeginCut = true;
			// 			if(m_NeedShowCutTip)
			// 			{
			// 				CTipMgr::Instance()->ShowTip("Tip_CutDir_Tip");
			// 				m_NeedShowCutTip = false;
			// 			}
		}
	}

}

CAcessoriesCutTraining::AreaType CAcessoriesCutTraining_5th::getAreaType(MisMedicOrganInterface * oif , const Ogre::Vector2 & texCoord)
{
	uint32 areaColor = GetPixelFromAreaTesture(texCoord.x ,texCoord.y);
	uint32 markValue = areaColor & 0x00ffffff;
	uint8 red = 0 , green = 0 , blue = 0;
	GetRGBComponent(markValue , red , green , blue);

	if(oif->m_OrganID == EODT_UTERUS)
	{
		if(blue < 1)					
		{
			//cut the wrong area
			if(red != 0)
			{
				int colorCode = red;//GetColorCode(red);
				if(colorCode == (int)CC_Fallopian_Tube_Left)
					return AT_Fallopian_Tube_Left;
				else if(colorCode == (int)CC_Fallopian_Tube_Right)
					return AT_Fallopian_Tube_Right;
				else if(colorCode == (int)CC_Ovary_Left)
					return AT_Ovary_Left;
				else if(colorCode == (int)CC_Ovary_Right)
					return AT_Ovary_Right;
				else if(colorCode == (int)CC_Uterus)
					return AT_Uterus;
			}
			return AT_NONE;
		}
		else if(blue > 0)
		{
			return AT_Fallopian_Tube_Right;
		}
	}
	return AT_NONE;
}

void CAcessoriesCutTraining_5th::ElecCutStart()
{
	if(!m_CutColorCode.empty()){
		if(m_LastCutColorCode != UNCUTCODE){
			int currFirstCode = m_CutColorCode[0];
			if((currFirstCode - m_LastCutColorCode) >= 2){
				CTipMgr::Instance()->ShowTip("Tip_CutDir_Error");
			}
		}
	}
}

void CAcessoriesCutTraining_5th::ElecCutEnd()
{
	//檢驗是否有橫向切
// 	if(m_CutColorCode.size() >= 2)
// 	{
// 		int minLevel = m_CutColorCode[0];
// 		int maxLevel = minLevel;
// 
// 		for(size_t p = 0 ; p < m_CutColorCode.size(); p++)
// 		{
// 			minLevel = min(minLevel , m_CutColorCode[p]);
// 			maxLevel = max(maxLevel , m_CutColorCode[p]);
// 		}
// 
// 		m_GlobalMaxCutCode = max(maxLevel , m_GlobalMaxCutCode); 
// 		m_GlobalMinCutCode  = min(minLevel ,  m_GlobalMinCutCode);
// 
// 		if((maxLevel - minLevel) >= 2) { 
// 			m_CutDirErrorCount++;
// 			CTipMgr::Instance()->ShowTip("Tip_CutDir_Error");
// 		}
// 
// 		m_LastCutColorCode = m_CutColorCode.back();
// 	}
// 
// 	m_CutColorCode.clear();
	
	//記錄切錯地方次数
	//organ 1
	m_CurErrorCount += m_CutErrorCodeAboutUteri.size();
	RecordCutErrorScore(m_CutErrorCodeAboutUteri);	
	m_CutErrorCodeAboutUteri.clear();
	//organ 2
	//不能切，没有处理
}

void CAcessoriesCutTraining_5th::ProcessElecCut(MisMedicOrganInterface * oif , uint32 areaValue)
{
	uint32 markValue = areaValue & 0x00ffffff;
	uint8 red = 0 , green = 0 , blue = 0;
	GetRGBComponent(markValue , red , green , blue);

	if(oif->m_OrganID == 17) 
	{
		//cut the TubalCutA_DomeActive
		CTipMgr::Instance()->ShowTip("Tip_Cut_Wrong_Organ");
		return;
	}
	else if(oif->m_OrganID == EODT_UTERUS)
	{
		if(blue < 1)					
		{
			//cut the wrong area
			if(red != 0)
			{
				int colorCode = GetColorCode(red);
				
				m_CutErrorCodeAboutUteri.insert(colorCode);

				std::map<int , Ogre::String>::iterator itor = m_CutErrorTipAboutUteri.find(colorCode);
				if(itor != m_CutErrorTipAboutUteri.end())
					CTipMgr::Instance()->ShowTip(itor->second);
				else
					CTipMgr::Instance()->ShowTip("Tip_Cut_Wrong_Organ");

				m_CutColorCode.push_back(-1);
			}
			else 
			{
				m_CutErrorCodeAboutUteri.insert(-1);
				CTipMgr::Instance()->ShowTip("Tip_Damage_Cavity");
			}
		
		}
		else if(blue > 0)
		{
			int colorLevel = GetColorCode(blue);
			m_CutColorCode.push_back(colorLevel);
			
			if(colorLevel == 4 || colorLevel == 5 || colorLevel == 6)
			{
				//在范围内
				CTipMgr::Instance()->ShowTip("Tip_Cut_Right");
			}
			else if(colorLevel == 3 || colorLevel == 7) 
			{
				//范围偏差
				CTipMgr::Instance()->ShowTip("Tip_Cut_Bias_1CM");
				m_CutBiasedLevel = 1;
			}
			else if(colorLevel == 1 || colorLevel == 2 || colorLevel == 8) 
			{
				//范围有较大偏差
				CTipMgr::Instance()->ShowTip("Tip_Cut_Bias_2CM");
				m_CutBiasedLevel = 2;
			}
			else
			{
				//范围有很大偏差,没分
				CTipMgr::Instance()->ShowTip("Tip_Cut_Bias_2+CM");
				m_CutBiasedLevel = 3;
			}


			//测试是否横着切
			if(!m_IsEmbryoTookOut)
			{
				if(m_GlobalMaxCutCode == UNCUTCODE || m_GlobalMinCutCode == UNCUTCODE)
				{
					m_GlobalMinCutCode = colorLevel;
					m_GlobalMaxCutCode = colorLevel;
				}
				else
				{
					m_GlobalMinCutCode = min(colorLevel , m_GlobalMinCutCode);
					m_GlobalMaxCutCode = max(colorLevel , m_GlobalMaxCutCode);
					if((m_GlobalMaxCutCode - m_GlobalMinCutCode) >= 3)
					{
						CTipMgr::Instance()->ShowTip("Tip_CutDir_Error");
						CScoreMgr::Instance()->Grade("Cut_Dir_Error");
						m_CutDirErrorCount++;

						m_GlobalMinCutCode = colorLevel;
						m_GlobalMaxCutCode = colorLevel;
					}
				}
			}	
		}
		//测试是否横着切
// 		if(m_CutColorCode.size() > 1)
// 		{
// 			int lastLevel = m_CutColorCode[m_CutColorCode.size() - 1];
// 			int lastButOneLevel = m_CutColorCode[m_CutColorCode.size() - 2];
// 			if(abs(lastLevel - lastButOneLevel) > 1)
// 				CTipMgr::Instance()->ShowTip("Tip_CutDir_Error");
// 		}

	}
}

void CAcessoriesCutTraining_5th::ProcessCut(MisMedicOrganInterface * oif , std::vector<uint32> & areaValues)
{
	if(oif->m_OrganID == 17) 
	{
		CTipMgr::Instance()->ShowTip("Tip_Cut_Wrong_Organ");
		return;
	}
	else if(oif->m_OrganID == EODT_UTERUS)
	{
		for(size_t t = 0 ;t < areaValues.size() ; t++)
		{
			uint32 markValue = areaValues[t] & 0x00ffffff;
			uint8 red = 0 , green = 0 , blue = 0;
			GetRGBComponent(markValue , red , green , blue);

			if(blue < 1)					
			{
				//cut the wrong area
				if(red != 0)
				{
					int colorCode = GetColorCode(red);

					m_CutErrorCodeAboutUteri.insert(colorCode);

					std::map<int , Ogre::String>::iterator itor = m_CutErrorTipAboutUteri.find(colorCode);
					if(itor != m_CutErrorTipAboutUteri.end())
						CTipMgr::Instance()->ShowTip(m_CutErrorTipAboutUteri[colorCode]);
					else
						CTipMgr::Instance()->ShowTip("Tip_Cut_Wrong_Organ");
					//m_CutColorCode.push_back(-1);
				}
				else 
				{
					m_CutErrorCodeAboutUteri.insert(-1);
					CTipMgr::Instance()->ShowTip("Tip_Damage_Cavity");
				}
			}
		}
		//記錄切錯地方次数
		//organ 1
		m_CurErrorCount += m_CutErrorCodeAboutUteri.size();
		RecordCutErrorScore(m_CutErrorCodeAboutUteri);	
		m_CutErrorCodeAboutUteri.clear();
		//organ 2
		//不能切，没有处理
	}
}

void CAcessoriesCutTraining_5th::OnTakeOutSomething(int OrganID)
{
	if(OrganID == m_pEmbryo->m_OrganID)
	{
		RemoveOrganFromWorld(m_pEmbryo);
		m_pEmbryo = NULL;
		Inception::Instance()->EmitShowMovie("TakeOutEmbryo");
		CTipMgr::Instance()->ShowTip("Tip_TakeOut_Embryo_Finish");		
		CScoreMgr::Instance()->Grade("Put_Embryo");
		m_IsEmbryoRemoved = true;
	}
}

int CAcessoriesCutTraining_5th::GetColorCode(uint8 component , int exp /* = 4 */)
{
	int result = component;
	result = result >> exp;
	return result;
}

bool CAcessoriesCutTraining_5th::DetectExplore(float dt)
{	 
	 if(!m_ExploreResult && !m_BeginCut)
	 {
		 __super::DetectExplore(dt);

		 if(m_ExploreResult)
		 {
			 CTipMgr::Instance()->ShowTip("Tip_Explore_Finish");
			 CScoreMgr::Instance()->Grade("Explore_Finish");
			 if(!m_IsEmbryoTookOut)
				 CTipMgr::Instance()->ShowTip("Tip_Aiming_Embryo_After_Explore");
		 }
	 }
	 return m_ExploreResult;
}

bool CAcessoriesCutTraining_5th::DetectLocation(float dt)
{
	bool locateResult = __super::DetectLocation(dt);
	
	if(locateResult != m_LastLocateResult && !m_BeginCut) 
	{
		if(locateResult)
			CTipMgr::Instance()->ShowTip("Tip_Aiming_Embryo");
		else
			CTipMgr::Instance()->ShowTip("Tip_Not_Aiming_Embryo");

		m_LastLocateResult = locateResult;
	}

	if(locateResult && m_NeedShowCutTip && !m_BeginCut)
	{
		CTipMgr::Instance()->ShowTip("Tip_CutDir_Tip");
		m_NeedShowCutTip = false;
	}

	return locateResult;
}

void CAcessoriesCutTraining_5th::TestExplore(float dt)
{
	//探查相关 begin
	if(!m_ExploreResult && !m_BeginCut)
	{
		std::list<ViewDetection*>::iterator itor = m_ExploreLocations.begin();
		while(itor != m_ExploreLocations.end())
		{
			ViewDetection * pVd = *itor;
			bool result = pVd->Update(dt , m_pLargeCamera);
			int colorIndex = result ? 1 : 0;
			//pVd->Draw(m_pLargeCamera , colorIndex);
			if(result)
			{
				delete pVd;
				itor = m_ExploreLocations.erase(itor);
			}
			else
				++itor;
		}
		if(m_ExploreLocations.empty())
		{
			m_ExploreResult = true;
			CTipMgr::Instance()->ShowTip("Tip_Explore_Finish");
			CScoreMgr::Instance()->Grade("Explore_Finish");
			if(!m_IsEmbryoTookOut)
				CTipMgr::Instance()->ShowTip("Tip_Aiming_Embryo_After_Explore");
		}
	}
	//探查相关 end
}

void CAcessoriesCutTraining_5th::TestLocation(float dt)
{
	//定位begin
	bool locateResult = m_EmbryoLocation.Update(dt , m_pLargeCamera);

	int colorIndex = locateResult ? 1 : 0;
	//m_EmbryoLocation.Draw(m_pLargeCamera , colorIndex);

	if(locateResult != m_LastLocateResult && !m_BeginCut) 
	{
		if(locateResult)
			CTipMgr::Instance()->ShowTip("Tip_Aiming_Embryo");
		else
			CTipMgr::Instance()->ShowTip("Tip_Not_Aiming_Embryo");

		m_LastLocateResult = locateResult;
	}

	if(locateResult && m_NeedShowCutTip && !m_BeginCut)
	{
		CTipMgr::Instance()->ShowTip("Tip_CutDir_Tip");
		m_NeedShowCutTip = false;
	}
	//定位end
}

void CAcessoriesCutTraining_5th::TestTakeOutEmbryo(float dt)
{
	MisMedicObjectEnvelop *pEnvelop = m_pEnvelopEmbryoAndUterus;

	if(pEnvelop == NULL)
		return;
#if 0
	m_painting.ClearEdges();
	for(size_t c = 0 ; c < pEnvelop->m_NodeDistConstraints.size() ; c++){
		if(pEnvelop->m_NodeDistConstraints[c].IsValid()){
			GFPhysSoftBodyNodeDistConstraint * pConstraint = pEnvelop->m_NodeDistConstraints[c].m_pConstraint;
			if(pConstraint){
				m_painting.PushBackEdge(CustomEdgeWithNode(pConstraint->m_Nodes[0] , pConstraint->m_Nodes[1] , Ogre::ColourValue::White ));
			}
		}
	}
	m_painting.Update(dt , m_pLargeCamera);
#endif

	if(!m_IsEmbryoTookOut)
	{
		//取出胚 begin
		if(pEnvelop->GetNumOfConsRemovedWithFace() >= 8) {
			CTipMgr::Instance()->ShowTip("Tip_Can_qupei");
		}

		if((m_pEmbryo != NULL) && 
		   ((m_pEmbryo->IsClamped() && m_pEmbryo->m_FaceMoveIncrementInClamp <= 0) ||
				(m_pEmbryo->m_IsSucked && m_pEmbryo->m_FaceMoveIncrementInSuction <= 0)))
		{
			float maxForce = pEnvelop->CheckTearConnect(0.005);
			float total = pEnvelop->GetNumOfConstraint();
			float removed = pEnvelop->GetNumOfConstraintRemoved();

			//m_Text1.AddText("maxDist:  " + Ogre::StringConverter::toString(maxDist));

			//是否温柔取出
			if(m_pEmbryo->m_FaceMoveSpeedInClamp > 12.0 || m_pEmbryo->m_FaceMoveSpeedInSuction > 12.0)
				m_IsTookOutSoftly = false;

			if((removed / total )  >  0.5) 
			{
				m_IsEmbryoTookOut = true;

				pEnvelop->RemoveConstraints();

				((GFPhysSoftBodyShape*)m_pEmbryo->m_physbody->GetCollisionShape())->SetSSCollideTag(GFPhysSoftBodyShape::SSCT_FACEVSNODE);

				CTipMgr::Instance()->ShowTip("Tip_Put_Embryo");

				//是否温柔取出
				if(m_IsTookOutSoftly)
					CScoreMgr::Instance()->Grade("Qupei_Softly");
				else
					CScoreMgr::Instance()->Grade("Qupei_Roughly");


				//所用器械是否正确
				if(m_pEmbryo->IsClamped() && m_pEmbryo->m_ClampInstrumentType != TT_BIPOLARELECFORCEPS)
				{
					CScoreMgr::Instance()->Grade("Qupei_Instrument_Right");
				}

				//
				if(m_CutDirErrorCount == 0)
				{
					CScoreMgr::Instance()->Grade("Cut_Dir_Right");
                    AddOperateItem("Cut_Dir_Right", 1.0f, true);                
				}
			}
		}
		//取出胚 end
	}
	else
	{
		if(!m_IsEmbryoPutIn && m_pEmbryo && m_pEmbryo->m_IsInContainer)
		{
			m_IsEmbryoPutIn = true;
			CTipMgr::Instance()->ShowTip("Tip_Put_Embryo_Finish");
		}
		else if(m_IsEmbryoPutIn && m_pEmbryo && !m_pEmbryo->m_IsInContainer)
		{
			m_IsEmbryoPutIn = false;
			CTipMgr::Instance()->ShowTip("Tip_Put_Embryo");
		}
	}
}
#define TEST 0
void CAcessoriesCutTraining_5th::TestFlushBloodAndSuction(float dt)
{
#if TEST
	m_IsEmbryoRemoved = true;
#endif
	//temp!!
	if(m_IsEmbryoRemoved)
	{
		WaterManager * pWaterMgr = EffectManager::Instance()->GetWaterManager();
		if(pWaterMgr && pWaterMgr->IsWaterColumnExsit())
			m_IsFlush = true;

		if(m_IsFlush)
		{
			int num = GetNumOfWaterPools();
			for(size_t p = 0 ; p < num ; p++)
			{
				WaterPool * pPool = GetWaterPools(p);
				Ogre::Vector2 planePos;
				if(pPool->GetCurrHeight() < 0.001)
				{
					m_IsSuck = true;
				}
			}
		}

		if(m_IsFlush && m_IsSuck)
		{
			TrainingFinish();
		}
	}
}

void CAcessoriesCutTraining_5th::RecordCutErrorScore(std::set<int> & codes)
{
	std::set<int>::iterator codeItor = codes.begin();
	for( ; codeItor != codes.end(); ++codeItor)
	{
		std::map<int , Ogre::String>::iterator tipItor = m_CutErrorTipAboutUteri.find(*codeItor);
		if(tipItor != m_CutErrorTipAboutUteri.end())
			CScoreMgr::Instance()->Grade(tipItor->second);
	}
}

void CAcessoriesCutTraining_5th::OnSaveTrainingReport()
{
	MxOperateItem *pOperateItem = NULL;

	AddOperateItem("Clamp_Uterus_Count" , float(m_NumOfUterusBeClamped));
	AddOperateItem("Clamp_Tube_Count" , float(m_NumOfFallopianTubeBeClamped));
	AddOperateItem("Clamp_Ovary_Count" , float(m_NumOfOvaryBeClamped));

    if (m_ExploreResult)//探查结果
    {
        AddOperateItem("Explore_Finish", 1.0f, true);
    }

    if (m_LastLocateResult)//定位结果
    {
        AddOperateItem("Locate_Success", 1.0f, true);
    } 
	
    if (m_IsEmbryoTookOut && m_IsEmbryoPutIn)//取出胚胎放入标本袋
    {
        AddOperateItem("TakeOutEmbryo_Embryo", 1.0f, true);
    } 

    if (m_IsFlush)//冲洗
    {
        AddOperateItem("Flush_Finish", 1.0f, true);
    }

    if (m_IsSuck)//处理创面
    {
        AddOperateItem("Process_Wound", 1.0f, true);
    }

    __super::OnSaveTrainingReport();
}

