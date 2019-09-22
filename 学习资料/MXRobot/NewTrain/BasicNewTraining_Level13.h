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


//���������ѵ��
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
		//ɾ��ָ����pair���ɹ�����true������false
		//pPair����ɾ����pair
		bool ErasePair(const VeinConnectPair * pPair);
		bool Showed();
		Position Pos() {return pos;}
		int GetCurRatio() {return curRatio;}

	private:
		MisMedicOrgan_Ordinary * pOrgan;
		Ogre::Vector3 endPoint1;					//�˵�1
		Ogre::Vector3 endPoint2;					//�˵�2
		float radius;								//���Ұ뾶
		std::set<const VeinConnectPair*> pairs;		//�����ǵ�VeinConnectPair����
		std::size_t initNumPair;					//��ʼpair����
		int curRatio;								//��ǰpair�ĸ�����				
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

	int m_lastPunctureOrganTime;			//���һ�δ������ٵ�ʱ��
	int m_lastPunctureVesselTime;			//���һ�δ���Ѫ�ܵ�ʱ��
	int m_lastElecOrganTime;				//���һ�ε絽���ٵ�ʱ��
	int m_lastElecVesselTime;				//���һ�ε絽Ѫ�ܵ�ʱ��
	int m_deltaTime;						//ʱ����(ms)

	DrawObject m_drawObject;

	bool  m_bOffVessel;///�Ƿ񳶶�Ѫ��
	float m_trainbegintime;
	bool  m_bElecOff;
};