#include "AccessorieCutTrain2.h"
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
#include "InputSystem.h"

#include "VolumeBlood.h"

#include "MisMedicEffectRender.h"


extern UINT32 AreaMark[];

CAccessorieCutTrain2::CAccessorieCutTrain2(const Ogre::String & strName)
	: CAcessoriesCutTraining(strName)
	, cutLevel(0)
	, currentLevel(0)
	, generateBleedPointForFinish(false)
{
	memset(nodeLineCuted, false, sizeof(nodeLineCuted));
	markMap = NULL;

}


bool CAccessorieCutTrain2::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
    bool result = CAcessoriesCutTraining::Initialize(pTrainingConfig, pToolConfig);
    searchRoad(0);


    MisMedicOrganInterface * tmp = m_DynObjMap[12];
    MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);
	pOO->SetBleedRadius(0.01f);

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
		// 		img.save("c://CAccessorieCutTrain2.bmp");
	}

	for (size_t f = 0; f < pOO->m_OriginFaces.size(); f++)
	{
		MMO_Face & organface = pOO->m_OriginFaces[f];

		GFPhysSoftBodyFace * physface = organface.m_physface;

		if (physface == 0)
			continue;
		for (int v = 0; v < 3; v++)
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
	while (node != NULL)
	{
		if ((node->m_Flag & AreaMark[AT_Focal_Zone_1]) != 0)
		{
			counter++;
		}
		node = node->m_Next;
	}


	if (m_DynObjMap.find(21) != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[21]);
		pOO->m_CutWidthScale = 2.0f;
	}
	if (m_DynObjMap.find(22) != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[22]);
		pOO->m_CutWidthScale = 2.0f;
	}
	if (m_DynObjMap.find(23) != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[23]);
		pOO->m_CutWidthScale = 2.0f;
	}
	return result;
}

std::string		CutStartOfAccessorieCutTrain2[4] = {
	"CutStart0",
	"CutStart1",
	"CutStart2",
	"CutStart3"
};
std::string		CutNotStartOfAccessorieCutTrain2[4] = {
	"CutNotStart0",
	"CutNotStart1",
	"CutNotStart2",
	"CutNotStart3"
};

void CAccessorieCutTrain2::receiveCheckPointList(MisNewTraining::OperationCheckPointType type, const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV, ITool * tool, MisMedicOrganInterface * oif)
{
	m_bNeedCheck = true;
	int matchPointNum = 0;
	int missPointNum = 0;
	memset(areaMatchNum, 0, sizeof(areaMatchNum));
	m_mistakeCounterMax = 0;
	CAcessoriesCutTraining::receiveCheckPointList(type, texCord, TFUV, tool, oif);
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
				if (markValue != 0)
				{
					if ((markValue & AreaMark[AT_Determine_Area_Burn_1]) != 0)
					{
						areaMatchNum[AT_Determine_Area_Burn_1] ++;
					}
					else if ((markValue & AreaMark[AT_Determine_Area_Burn_2]) != 0)
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
			else if (areaMatchNum[AT_Fallopian_Tube_Left] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Left");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Right] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Right");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Right");
			}
			else if (areaMatchNum[AT_Ovary_Left] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Left");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ovary_Right] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Right");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Right");
			}
			else if (areaMatchNum[AT_Ligament_Left] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Left");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ligament_Right] == m_mistakeCounterMax)
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
			if (markValue != 0)
			{
				if ((markValue & AreaMark[AT_Determine_Area_Cut_1]) != 0)
				{
					areaMatchNum[AT_Determine_Area_Cut_1] ++;
				}
				else if ((markValue & AreaMark[AT_Determine_Area_Cut_2]) != 0)
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
				if (m_ScoreTimes[AT_Focal_Zone_1] < 10)					CScoreMgr::Instance()->Grade("Damage_Uterus");
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Left] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Left");
				CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Fallopian_Tube_Right] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Right");
				if (m_ScoreTimes[AT_Focal_Zone_1] < 10)					CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Right");
			}
			else if (areaMatchNum[AT_Ovary_Left] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Left");
				CScoreMgr::Instance()->Grade("Damage_Ovary_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ovary_Right] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Right");
				if (m_ScoreTimes[AT_Focal_Zone_1] < 10)					CScoreMgr::Instance()->Grade("Damage_Ovary_Right");
			}
			else if (areaMatchNum[AT_Ligament_Left] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Left");
				CScoreMgr::Instance()->Grade("Damage_Ligament_Left");
				m_bFinished = true;
			}
			else if (areaMatchNum[AT_Ligament_Right] == m_mistakeCounterMax)
			{
				m_foundMistake = true;
				CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Right");
				if (m_ScoreTimes[AT_Focal_Zone_1] < 10)					CScoreMgr::Instance()->Grade("Damage_Ligament_Right");
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
		for (int lineIndex = 0; lineIndex < 4; lineIndex++)
		{
			if (nodeLineCuted[lineIndex])
			{
				continue;
			}
			else
			{
				int misslineCount = 0;
				for (int i = 0; i < nodeLine[lineIndex].size(); i += 2)
				{
					edge = shape.GetEdge(nodeLine[lineIndex][i], nodeLine[lineIndex][i + 1]);
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
						if (nodeLineCuted[1] == false && nodeLineCuted[2] == false && nodeLineCuted[3] == false)
						{
							CTipMgr::Instance()->ShowTip(CutStartOfAccessorieCutTrain2[lineIndex]);
							CScoreMgr::Instance()->Grade(CutStartOfAccessorieCutTrain2[lineIndex]);
						}
						else
						{
							CScoreMgr::Instance()->Grade(CutStartOfAccessorieCutTrain2[lineIndex]);
						}
					}
					else
					{
						if (nodeLineCuted[lineIndex - 1])
						{
							CTipMgr::Instance()->ShowTip(CutStartOfAccessorieCutTrain2[lineIndex]);
							CScoreMgr::Instance()->Grade(CutStartOfAccessorieCutTrain2[lineIndex]);
						}
						else
						{
							CScoreMgr::Instance()->Grade(CutStartOfAccessorieCutTrain2[lineIndex]);
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
			for (int i = 0; i < nodeLine[3].size(); i += 2)
			{
				edge = shape.GetEdge(nodeLine[3][i], nodeLine[3][i + 1]);
				if (edge == NULL)
				{
					nodeLine_4_Influseced.insert(nodeLine[3][i]);
					nodeLine_4_Influseced.insert(nodeLine[3][i + 1]);
				}
			}

			std::set<GFPhysSoftBodyNode*>::iterator itor;

			for (size_t f = 0; f < pOO->m_OriginFaces.size(); f++)
			{
				MMO_Face & organface = pOO->m_OriginFaces[f];

				if (organface.m_physface)
				{
					for (int fn = 0; fn < 3; fn++)
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
		pOO->TestLinkingArea(AreaMark[AT_Focal_Zone_1], reginInfo);

		int nodeNum = 0;
		int nodeNum_mark = 0;
		int maxIndex;
		int maxNum = 0;
		for (int i = 0; i < reginInfo.size(); i++)
		{
			nodeNum += reginInfo[i].x;
			nodeNum_mark += reginInfo[i].y;
			if (maxNum < reginInfo[i].x)
			{
				maxNum = reginInfo[i].x;
				maxIndex = i;
			}
		}
		float cutPercent = reginInfo[maxIndex].y / nodeNum_mark;
		if (cutPercent < 0.1f)
		{
			TrainingFinish();
			if (m_ScoreSys)
			{
				m_ScoreSys->SetTrainSucced();
			}
			//CTipMgr::Instance()->ShowTip("Tip_Cut_Percent_0");
			CScoreMgr::Instance()->Grade("CutClean");
		}
		else 	if (cutPercent < 0.2f)
		{
			CTipMgr::Instance()->ShowTip("Tip_Cut_Percent_1");
		}
		else 	if (cutPercent < 0.5f)
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
void CAccessorieCutTrain2::loadNodeUVs()
{
	std::ifstream stream;
	stream.open("../Config/MultiPortConfig/Gynaecology/Tubalcut_Config.txt");
	if (stream.is_open())
	{
		char buffer[100];
		stream.getline(buffer, 99);
		std::string str = buffer;
		int num = atoi(str.c_str());
		std::string	sstr[2];
		int dotIndex;
		for (int i = 0; i < num; i++)
		{
			stream.getline(buffer, 99);
			str = buffer;
			dotIndex = str.find(",");
			sstr[0] = str.substr(0, dotIndex);
			sstr[1] = str.substr(dotIndex + 1);
			nodeUVs[i].x = atoi(sstr[0].c_str());
			nodeUVs[i].y = atoi(sstr[1].c_str());
		}

	}

}

void CAccessorieCutTrain2::matchNodes()
{
	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	GFPhysSoftBodyNode* nearestNode;
	int		nearestNode_Index;
	float	minDis = 10000;

	for (int nodeIndex = 0; nodeIndex < 8; nodeIndex++)
	{
		nodeUVs[nodeIndex] = nodeUVs[nodeIndex] / 2048;
		for (size_t f = 0; f < pOO->m_OriginFaces.size(); f++)
		{
			MMO_Face & organface = pOO->m_OriginFaces[f];

			for (int i = 0; i < 3; i++)
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

void CAccessorieCutTrain2::searchRoad(int index)
{
	/*
	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().load("texPointBS.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 0);
	Ogre::Image img;
	tex->convertToImage(img);
	// 	img.save("c://ss_1st.bmp");

	int	col = img.getColourAt(0, 0, 0).getAsARGB();

	int colArea[4] = { 0xffff00ff, 0xff00ff00, 0xff0000ff, 0xffff0000 };

	//	for (int nodeIndex = 0 ; nodeIndex< 8; nodeIndex++)
	{
		for (size_t f = 0; f < pOO->m_OriginFaces.size(); f++)
		{
			MMO_Face & organface = pOO->m_OriginFaces[f];
			int matchArea = -1;
			for (int Vi = 0; Vi < 3; Vi++)
			{
				Ogre::Vector2 vertTexCoord = organface.GetTextureCoord(Vi);

				col = img.getColourAt(vertTexCoord.x * 2048, vertTexCoord.y * 2048, 0).getAsARGB();

				if (col == colArea[0])
				{
					matchArea = 0;
				}
				else if (col == colArea[1])
				{
					matchArea = 1;
				}
				else if (col == colArea[2])
				{
					matchArea = 2;
				}
				else if (col == colArea[3])
				{
					matchArea = 3;
				}
			}

			if (matchArea != -1)
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
	*/

}

void CAccessorieCutTrain2::doFinishCount()
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

bool CAccessorieCutTrain2::OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight)
{	
	//if (m_Area->) return false;

	if (!markMap) {
		Ogre::TexturePtr pTex;
		Ogre::HardwarePixelBufferSharedPtr pixelBuffer;
		Ogre::uint* pDest;
		int w, h;

		pTex = m_Area;
		w = m_AreaWidth;
		h = m_AreaHeight;

		markMap = (uint32*)malloc(w*h*sizeof(uint32));

		pixelBuffer = pTex->getBuffer();
		pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
		pDest = static_cast<Ogre::uint*>(pixelBuffer->getCurrentLock().data);
		memcpy(markMap, pDest, w*h * 4);
		pixelBuffer->unlock();

	}

	GFPhysSoftBodyFace* pFace = organ->m_physbody->GetFaceByUID(faceID);
	if (!pFace) return false;
	
	float tx[2];

	tx[0] = pFace->m_TexCoordU[0] * pFaceWeight[0] + pFace->m_TexCoordU[1] * pFaceWeight[1] + pFace->m_TexCoordU[2] * pFaceWeight[2];
	tx[1] = pFace->m_TexCoordV[0] * pFaceWeight[0] + pFace->m_TexCoordV[1] * pFaceWeight[1] + pFace->m_TexCoordV[2] * pFaceWeight[2];

	uint32 v;
	v = markMap[cw_clamp((int)(tx[1] * m_AreaHeight), 0, m_AreaHeight - 1) * m_AreaWidth + cw_clamp((int)(tx[0] * m_AreaWidth), 0, m_AreaWidth - 1)];

	if (v==0xFF000000) return true;

	return false;
}


int ccc1, ccc2 = 0;
VolumeBlood* volumeBlood;
int lastBleedFrame=-1000, currentFrame=0;
unsigned int* texBuffer=NULL;

bool CAccessorieCutTrain2::BeginRendOneFrame(float timeelapsed){

	char status[100], organName[100], line[1000];
	int faceID, vtxID[3];
	float w[3], texCoord[2], weight[3], vtxCoord1[3], vtxCoord2[3], vtxCoord3[3], norm[3], normLen, offset[3], vel[3], vtx[3], p12[3], p13[3];
	MisMedicOrgan_Ordinary* pOrgan;
	double t1, t2, t3;

	currentFrame++;
	//Sleep(1000);

	bool result = MisNewTraining::BeginRendOneFrame(timeelapsed);
	
	if (ccc1 == 0) {

		pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap.begin()->second);

		weight[0] = 0.33;
		weight[1] = 0.34;
		weight[2] = 0.33;

		//pOrgan->ApplyEffect_VolumeBlood(148, weight);


		ccc1 = 1;

		
	}


	
		
	
	
	return result;
}


