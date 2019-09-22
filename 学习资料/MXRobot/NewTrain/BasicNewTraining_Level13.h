#pragma once
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"

#include "TrainScoreSystem.h"
#include <vector>
#include <map>
#include <set>
#include "TrainingCommon.h"

class MisMedicOrgan_Ordinary;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class MisCTool_PluginClamp;
class CTool;


//电凝钩离断训练
class CBasicNewTraining_Level13 : public MisNewTraining
{
public:
	CBasicNewTraining_Level13(void);
	~CBasicNewTraining_Level13(void);

	bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	bool Update(float dt);
	void OnHandleEvent(MxEvent* pEvent);
	void OnOrganCutByTool(MisMedicOrganInterface * pOrgan , bool iselectriccut);
	void OnSaveTrainingReport();
private:

	enum Position{
		TopLeft,				
		TopCenter,
		BottomLeft
	};

	struct VesselInfo
	{
		VesselInfo(MisMedicOrgan_Ordinary * organ,const Ogre::Vector3 & p1,const Ogre::Vector3 & p2,Position position,float r = 0.f)
			:pOrgan(organ),
			endPoint1(p1),
			endPoint2(p2),
			pos(position)
		{
			initNumPair = 0;
			curRatio = 100;
			bShowed = false;
			radius = r;
		}
		const std::set<const VeinConnectPair*> & GetPairs() {return pairs;}
		void SetPairInfo(const VeinConnectObject * pVeinConnectObject,Ogre::Camera * pCamera);
		//删除指定的pair，成功返回true，否则false
		//pPair：被删除的pair
		bool ErasePair(const VeinConnectPair * pPair);
		bool Showed();
		Position Pos() {return pos;}
		int GetCurRatio() {return curRatio;}

	private:
		MisMedicOrgan_Ordinary * pOrgan;
		Ogre::Vector3 endPoint1;					//端点1
		Ogre::Vector3 endPoint2;					//端点2
		float radius;								//胶囊半径
		std::set<const VeinConnectPair*> pairs;		//被覆盖的VeinConnectPair对象
		std::size_t initNumPair;					//初始pair个数
		int curRatio;								//当前pair的覆盖率				
		bool bShowed;
		
		const Position pos;
	};

	void init();


private:

	typedef std::map<int,std::vector<int>> OrganNodesMap;
	OrganNodesMap m_organNodesMap;
	std::vector<VesselInfo*> m_vecVesselInfo;
	std::set<MisMedicOrgan_Ordinary*> m_allOrgans;
	int m_numShowedVessel                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  ;
	bool m_bFinish;
	bool m_bInit;

	int m_lastPunctureOrganTime;			//最后一次戳伤器官的时刻
	int m_lastPunctureVesselTime;			//最后一次戳伤血管的时刻
	int m_lastElecOrganTime;				//最后一次电到器官的时刻
	int m_lastElecVesselTime;				//最后一次电到血管的时刻
	int m_deltaTime;						//时间间隔(ms)

	DrawObject m_drawObject;

	bool  m_bOffVessel;///是否扯断血管
	float m_trainbegintime;
	bool  m_bElecOff;
};