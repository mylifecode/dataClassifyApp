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
		Ϊĳ��ѡ��һ����е�����Ѿ��̶�����е��ѡ��ʧ�ܷ���NULL
	*/
	ITool * ChooseOneFixedTool(const Ogre::String& toolName,bool isLeftHand = true);

	/**
		�Ƴ����б��̶�����е
	*/
	void RemoveAllFixedTools();
	void AllFixedToolsRelease();
    void RemoveLeftFixedTool();
    void RemoveRightFixedTool();

	/**
		�̶���ǰĳ�����е���Ա���������µ���е��������
	*/
	ITool* FixTool(bool isLeftHand = true);

	/**
		����һ����е
	*/
	ITool* CreateTool(const Ogre::String & strType, const bool bReuse, const Ogre::String & strSubType,bool bLeft);
	/**
		�Ƴ���ǰʹ�õ�������е
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

	/// ��ʹ�õ�������е��ͨ����ʱ�䣬��������ǰ��е
	float m_totalElectricTime;
	/// ��ʹ�õ�������е��ͨ����Чʱ�䣬��������ǰ��е
	float m_totalValidElectricTime;
	float m_maxKeeppingElectricBeginTime;
	float m_maxKeeppingElectricTime;
	/// ���������ʱ��
	float m_toolSuctionTime;
	/// ��������������
	float m_toolIrrigationTime;

	/// ��ʹ�õ�������е���ͷŵ��Ѽ�������������ǰ��е
	int m_nReleasedTitanicClip;
	/// ��е�бմ���
	int m_leftToolClosedTimes;
	int m_rightToolClosedTimes;
	/// �ƶ��ܳ���
	float m_leftToolMovedDistance;
	float m_rightToolMovedDistance;
	/// �ƶ�ʱ��
	float m_leftToolMovedTime;
	float m_rightToolMovedTime;
	/// ͨ��Ӱ��ʱ��
	float m_electricAffectTimeForHemoClip;
	float m_electricAffectTimeForOrdinaryOrgan;
	/// ���е���е�Ƿ�պϲ���
	bool m_toolIsClosedInsertion;

	/// ��е�����׹ܴ���
	int m_toolInAndOutTimes;
	/// ��е�ڷǲ���������Ƿ��ڱպ�״̬
	bool m_toolIsClosedInSeparteTime;

	/// ��ǰ���̶�����е
	std::map<Ogre::String, ITool*> m_fixedTools;
};
