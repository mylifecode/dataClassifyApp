/**Author:zx**/
#pragma once
#include "IMXDefine.h"
#include "MXEvent.h"
#include "Singleton.h"
#include "ITool.h"
#include "MXToolEvent.h"

class ITraining;

typedef void (* HandleEventCB)(MxEvent *, ITraining * pTraining);

class CMXEventsDump : public CSingleT<CMXEventsDump> 
{
public:
	CMXEventsDump();
	~CMXEventsDump();
	
	MxToolEvent * CreateEventNew(MxEvent::EventType enmEventType, ITool * pTool, void * pUserPoint);
	
	void PushEvent(MxEvent * pEvent , bool immediately = false);
	
	void DumpEvents();
	void ClearEvent();
	//1:ͨ��ע���Զ�����¼��������������¼�
	//bHandleOldEvents:��ע���µĴ�����ʱ������������оɵ��¼����ò����������Ƿ�������Щ������Ĭ�ϲ���Ϊ���
	void RegisterHandleEventsFunc(HandleEventCB pFuncHandleEvent, ITraining * pTraining,bool bHandleOldEvents = false);
	
	//2:��ITraining���HandleEvent�����������¼�
	void RegisterHandleEventsFunc(ITraining * pTraining,bool bHandleOldEvents = false);

	//ɾ����ǰע����¼�������	
	void UnRegisterHandleEventsFunc();

private:
	//�ڲ��¼�������
	void DumpEvent(MxEvent * pEvent);

	std::queue<MxEvent *> m_queEvents;


	HandleEventCB m_pFuncHandleEvent;
	ITraining * m_pCurTraining;

	//�¼�������������
	enum EventHandleType
	{
		Ignore,				//û���¼�������(Ĭ�ϴ���ʽ)
		UseTraining,		//��ITraining���HandelEvent����������
		Customed			//ʹ���Զ���ĺ��������¼�
	};

	EventHandleType m_eventHandleType;
};