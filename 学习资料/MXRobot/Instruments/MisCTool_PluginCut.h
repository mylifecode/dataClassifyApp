#ifndef _MISCTOOL_PLUGINCUT_
#define _MISCTOOL_PLUGINCUT_
#include "MisMedicCToolPluginInterface.h"

class VeinConnectObject;

class MisCTool_PluginCut : public MisMedicCToolPluginInterface
{
public:
    MisCTool_PluginCut(CTool * tool, Real checkCutStartShaft = 3.0f);

	~MisCTool_PluginCut();

	void onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair);


	//when a physics update frame end
	virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt);

	//when a ordinary frame update start(one ordinary frame may include zero-many physics frame)
	virtual void OneFrameUpdateStarted(float timeelapsed);

	//when a ordinary frame update end
//	virtual void OneFrameUpdateEnded();

	struct VeinConnInCutRegion
	{
		VeinConnInCutRegion(VeinConnectObject * veinobj ,  int clustterId , int pairId) : m_pVeinObj(veinobj) , m_ClusterId(clustterId) ,  m_PairId(pairId){}
		//int m_VeinOrganId;
		VeinConnectObject *m_pVeinObj;
		int m_ClusterId;
		int m_PairId;
	};

    float m_CutBeginCheckShaft;

	bool  m_CanPerformCutOperaion;

	float m_LastClipShaft;//��һ�γɹ�����ʱ��shaftֵ

	float m_MaxShaftSinceLastClip;//��һ�γɹ�

	float m_TimeElapsedSinceLastClip;

    bool  m_IsClampingConnect;//��ֱ�������������е��ʹ��cut��������ڳ�ʼ�����ֶ���Ϊtrue
	
	bool  m_IsClampingOrgans;

	int   m_ClampingClockTimes;
private:
	void ClearVeinConnInCutRegion() { m_VeinConnInCutRegion.clear();}
	void TestVeinConnInCutReions(VeinConnectObject *veinobj);
	
	std::vector<MisCTool_PluginCut::VeinConnInCutRegion> m_VeinConnInCutRegion;
};

#endif