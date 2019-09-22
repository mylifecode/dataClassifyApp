/**Supplement:zx**/
#pragma once
#include "ITool.h"
#include "Singleton.h"
#include "ITraining.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "GoPhysContactListener.h"

class CXMLContentManager;
class CXMLWrapperTraining;
class CXMLWrapperToolConfig;
class CXMLWrapperHardwareConfig;

using namespace GoPhys;

class CToolsMgr// : public GFPhysSoftRigidContactPointListener
{
public:
	CToolsMgr(void);
	~CToolsMgr(void);

public:
	bool Initialize(Ogre::SceneManager * pSceneManager, CXMLWrapperTraining * pTrainingConfig, ITraining * pTraining);

	void Terminate();

	void OnCurrentTrainDeleted();

	void AddTool(const Ogre::String& strToolName, ITool *pTool);

	void RemoveTool(const Ogre::String& strToolName);

	void Update(float dt);

	inline ITool * GetLeftTool() const
	{
		return m_pLeftTool;
	}

	inline ITool * GetRightTool() const
	{
		return m_pRightTool;
	}

	void GetFixTools(std::vector<ITool*> & result)
	{
		std::map<Ogre::String, ITool*>::iterator itor = m_fixedTools.begin();
		while (itor != m_fixedTools.end())
		{
			result.push_back(itor->second);
			itor++;
		}
	}

	inline ITraining * GetOwnerTraining() const {return m_pTraining;}
	inline void SetOwnerTraining(ITraining * pTraining) { m_pTraining = pTraining;}

	ITool * SetLeftTool(ITool * pTool); // set left tool in use, zx
	ITool * SetRightTool(ITool * pTool); // set right tool in use, zx

	ITool * SwitchTool(bool bLeft, Ogre::String strToolType, Ogre::String strSubString = ""); // switch tool, zx

	/**
		为某侧选择一个器械，从已经固定的器械中选择，失败返回NULL
	*/
	ITool * ChooseOneFixedTool(const Ogre::String& toolName,bool isLeftHand = true);

	/**
		移除所有被固定的器械
	*/
	void RemoveAllFixedTools();
	void AllFixedToolsRelease();
    void RemoveLeftFixedTool();
    void RemoveRightFixedTool();

	/**
		固定当前某侧的器械，以便继续加入新的器械到场景中
	*/
	ITool* FixTool(bool isLeftHand = true);

	/**
		创建一把器械
	*/
	ITool* CreateTool(const Ogre::String & strType, const bool bReuse, const Ogre::String & strSubType,bool bLeft);
	/**
		移除当前使用的左右器械
	*/

	void SetVisibleForAllTools(bool vis);

	bool RemoveAllCurrentTool(bool includefixedtool = true);
	bool RemoveLeftCurrentTool();
	bool RemoveRightCurrentTool();

	void StatisticToolData(ITool* pTool);

	float GetTotalElectricTime();
	float GetValidElectricTime();
	float GetMaxKeeppingElectricBeginTime();
	float GetMaxKeeppingElectricTime();

	float GetToolSuctionTime();
	float GetToolIrrigationTime();

	int GetNumberOfReleasedTitanicClip();

	int GetLeftToolClosedTimes();
	int GetRightToolClosedTimes();

	float GetLeftToolMovedTime();
	float GetRightToolMovedTime();

	float GetLeftToolMovedDistance();
	float GetRightToolMovedDistance();

	float GetLeftToolMovedSpeed();
	float GetRightToolMovedSpeed();
	
    unsigned int GetLeftToolFastestSpeedTimes();
	unsigned int GetRightToolFastestSpeedTimes();

	float GetElectricTimeForHemoClip();
	float GetElectricTimeForOrdinaryOrgan();        

	bool ToolIsClosedInsertion();

	int GetToolInAndOutTimes() {return m_toolInAndOutTimes;}

	bool ToolIsClosedInSeparateTime();
	

	void LoadHardwareConfig();
	CXMLWrapperHardwareConfig * GetHardwareConfigPointer() const { return m_pHardwareConfig; }

	inline void SetToolPlaceName(Ogre::String strToolPlaceName) { m_strToolPlaceName = strToolPlaceName; }      
  
private:
	
	typedef std::map<Ogre::String, ITool*> MAP_NAME_TOOL;
	MAP_NAME_TOOL map_NameTool;

	static int s_nCount;

	ITool * m_pLeftTool;
	ITool * m_pRightTool;
	CXMLWrapperTraining * m_pTrainingConfig;
	ITraining * m_pTraining;
    CXMLWrapperHardwareConfig * m_pHardwareConfig;
    CXMLContentManager * m_pXMLContentManager;
	int m_nLeftDeviceLabelID;
	int m_nRightDeviceLabelID;
	Ogre::String m_strToolPlaceName;

	/// 已使用的所有器械的通电总时间，不包括当前器械
	float m_totalElectricTime;
	/// 已使用的所有器械的通电有效时间，不包括当前器械
	float m_totalValidElectricTime;
	float m_maxKeeppingElectricBeginTime;
	float m_maxKeeppingElectricTime;
	/// 冲吸器冲的时间
	float m_toolSuctionTime;
	/// 冲吸器吸的世间
	float m_toolIrrigationTime;

	/// 已使用的所有器械所释放的钛夹数，不包括当前器械
	int m_nReleasedTitanicClip;
	/// 器械夹闭次数
	int m_leftToolClosedTimes;
	int m_rightToolClosedTimes;
	/// 移动总长度
	float m_leftToolMovedDistance;
	float m_rightToolMovedDistance;
	/// 移动时间
	float m_leftToolMovedTime;
	float m_rightToolMovedTime;
	/// 通电影响时间
	float m_electricAffectTimeForHemoClip;
	float m_electricAffectTimeForOrdinaryOrgan;
	/// 所有的器械是否闭合插入
	bool m_toolIsClosedInsertion;

	/// 器械进出套管次数
	int m_toolInAndOutTimes;
	/// 器械在非操作情况下是否处于闭合状态
	bool m_toolIsClosedInSeparteTime;

	/// 当前被固定的器械
	std::map<Ogre::String, ITool*> m_fixedTools;
};
