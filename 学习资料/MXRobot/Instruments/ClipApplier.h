
#pragma once
#include "Forceps.h"

class ITool;
class CXMLWrapperTool;
/*�Ѽ�ǯ*/


//����Ϊ�Ѽ�ǯ��������״̬0������ȫ�򿪣�1������ȫ�պ�
//���ҽ�����ǰ״̬Ϊ1������һ��״̬Ϊ0ʱ�����������ϼ��Ѽ�
#define CLIPAPLLIER_COMPLETELY_OPEN_STATUS		0
#define CLIPAPLLIER_COMPLETELY_CLOSE_STATUS		1

class MisCTool_PluginClamp;
class MisCTool_PluginClipTitanic;
class CClipApplier : public CForceps
{
public:
	CClipApplier();
	
	CClipApplier(CXMLWrapperTool * pToolConfig);
	
	virtual ~CClipApplier(void);

	void InternalSimulationStart(int currStep , int TotalStep , float dt);

	inline void SetOpenCloseStatus(int status){m_nStatus = status;}
	inline int GetOpenCloseStatus(){return m_nStatus;}

	void CreateEmptyClip();

	virtual bool Update(float dt);

	virtual void Updates2m();

	virtual bool Initialize(CXMLWrapperTraining * pTraining);

	std::string GetCollisionConfigEntryName();

	//overridden for new train
	//virtual void OnCutBladeClampedTube(MisMedicOrgan_Tube & tubeclamp , int segment , int localsection , Real sectionWeight);

	//���Ӽ�ס����ʱ�Ƿ������崹ֱ
	bool IsClampedVertical();
	//�Ƿ��°벿�ֿɼ�
	bool IsLowerHalfPartVisiable();

protected:
	//ÿһƬ��еͷ�����ƶ��ľ���
	const float m_fHalfHeadMoveDistance;

	std::vector< ITool* > m_pEmptyClips;
	//int m_nLastShaftAsideForEmptyClip;

	int m_nStatus;										//�Ѽ�ǯ״̬

	Ogre::Vector3 m_vDirection;

	bool m_bClampBeforeRealse;

	MisCTool_PluginClipTitanic * m_TitanicPlugin;
};