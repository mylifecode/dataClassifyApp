#ifndef _ACESSORIECUTRAIN2_
#define _ACESSORIECUTRAIN2_

#include "AcessoriesCutTrain.h"
#include "CustomCollision.h"
#include "Painting.h"

class CAccessorieCutTrain2 :public CAcessoriesCutTraining			// TubalCutTrain
{
public:
	CAccessorieCutTrain2(const Ogre::String & strName);
	bool	Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type, const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV, ITool * tool, MisMedicOrganInterface * oif);
	int		cutLevel;
	int		currentLevel;

#if 0
	void	loadNodeUVs();
	void	matchNodes();
#endif
	void	searchRoad(int index);

	Ogre::TexturePtr	m_tex;


#if 0
	Ogre::Vector2		nodeUVs[8];
	GFPhysSoftBodyNode*		nodes[8];
	int					nodes_ID[8];
	short*				nodeEdges;
#endif
	std::vector<GFPhysSoftBodyNode*>	nodeLine[4];
	std::set<GFPhysSoftBodyNode*>	nodeLine_4_Influseced;
	bool	nodeLineCuted[4];
	bool	generateBleedPointForFinish;
	virtual void	doFinishCount();
	virtual bool BeginRendOneFrame(float timeelapsed);
	virtual bool OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight);

	uint32* markMap;

};

#endif