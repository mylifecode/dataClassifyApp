#ifndef _ADHESIONRESECTIONTRAIN_
#define _ADHESIONRESECTIONTRAIN_

#include "AcessoriesCutTrain.h"
#include "CustomCollision.h"

#include "Painting.h"

//粘连分离
class CAdhesionResectionTrain : public CAcessoriesCutTraining
{
public:
	CAdhesionResectionTrain(const Ogre::String & strName);
	bool	 Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual	bool Update(float dt);
	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif );
	virtual void ElecCutStart();
	virtual void ElecCutEnd();
	void ProcessElecCut(MisMedicOrganInterface * oif , uint32 areaValue);


	//探查位置
	std::list<ViewDetection *> m_ExploreLocations;
private:
	bool m_ExploreResult;


	//錯誤提示
	std::map<int , Ogre::String> m_CutErrorTipAboutUteri;
//	std::map<int , Ogre::String> m_CutErrorTipAboutOther;	//unused now
	
	//一次電切操作切錯的範圍
	std::set<int> m_CutErrorCodeAboutUteri;
//	std::set<int> m_CutErrorCodeAboutOther;

	MisMedicOrgan_Ordinary * m_pUterus;

	PaintingTool m_painting;
};

#endif