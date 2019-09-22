#ifndef _ACESSORIESCUTRAIN_SUB_
#define _ACESSORIESCUTRAIN_SUB_

#include "AcessoriesCutTrain.h"
#include "CustomCollision.h"
#include "Painting.h"

class CAcessoriesCutTraining_1st :public CAcessoriesCutTraining			// TubalCutTrain
{
public:
	CAcessoriesCutTraining_1st( const Ogre::String & strName);
	bool	Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif );
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


};

class CAcessoriesCutTraining_2nd :public CAcessoriesCutTraining			//	JueyuTrain
{
public:
	CAcessoriesCutTraining_2nd( const Ogre::String & strName);
	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif );
	virtual void	doFinishCount();
};

class CAcessoriesCutTraining_3rd :public CAcessoriesCutTraining			//	SilverClipTrain
{
public:
	CAcessoriesCutTraining_3rd( const Ogre::String & strName);
	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif );
	virtual void	doFinishCount();

// protected:
// 	virtual void RegisterMistake(int	mistakeType);

};

class CAcessoriesCutTraining_4th : public CAcessoriesCutTraining
{
public:    
    int TipcolArea[5]; 
    int colArea[4];//切割顺序从伞端到宫角

	CAcessoriesCutTraining_4th(const Ogre::String & strName);
    bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
    virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif );    
    bool Update(float dt);
    CAcessoriesCutTraining::AreaType getAreaType(MisMedicOrganInterface * oif , const Ogre::Vector2 & texCoord);
    void OnSaveTrainingReport();
	//virtual void KeyPress(QKeyEvent * event);
protected:
    virtual bool DetectExplore(float dt);
    virtual bool DetectLocation(float dt);    
private:
    void ProcessCut();    
    void ShowNextTip();
    bool Checkfinish();    
    void CheckClampAreaRegion(const std::vector<Ogre::Vector2> & texCord , ITool * tool , MisMedicOrganInterface * oif);
    void CheckCutAreaRegion(const std::vector<Ogre::Vector2> & texCord , ITool * tool , MisMedicOrganInterface * oif);
    bool m_BeginCut;
    bool m_LastLocateResult;
    bool m_NeedShowCutTip;

    MisMedicOrgan_Ordinary * m_organ;
    
    std::vector<int> m_ElecCutColorCode;
    bool notexist[4];

    std::vector<int>::iterator m_find3;
    std::vector<int>::iterator m_find2;
    std::vector<int>::iterator m_find1;
    std::vector<int>::iterator m_find0;
    bool m_bRightDir;
    bool m_bCompleteCut;
};

class CAcessoriesCutTraining_5th : public CAcessoriesCutTraining
{
public:
	enum ColorCode
	{
		CC_Ligament_Left						= 1,
		CC_Fallopian_Tube_Left			        = 16,
		CC_Ovary_Left							= 32,
		CC_Ligament_Right					    = 48,
		CC_Fallopian_Tube_Right			        = 64,
		CC_Ovary_Right							= 80,
		CC_Uterus								= 96,
		CC_Ligament_Left_TO				        = 112,		//卵巢、输卵管间韧带
		CC_Ligament_Right_TO			        = 128,		//卵巢、输卵管间韧带
	};

	CAcessoriesCutTraining_5th(const Ogre::String & strName);
	
	void	 InitErrorTip();
	
	bool	 Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	
	virtual MisMedicOrgan_Ordinary * FilterClampedOrgan(std::vector<MisMedicOrgan_Ordinary *> & organs);
	virtual	bool Update(float dt);
	
	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif );
	
	virtual AreaType getAreaType(MisMedicOrganInterface * oif , const Ogre::Vector2 & texCoord);
	
	virtual void ElecCutStart();
	
	virtual void ElecCutEnd();
	
	void ProcessElecCut(MisMedicOrganInterface * oif , uint32 areaValue);
	
	void ProcessCut(MisMedicOrganInterface * oif , std::vector<uint32> & areaValues);
	
	virtual void OnTakeOutSomething(int OrganID);
	
	int GetColorCode(uint8 component , int exp = 4);

	
	void OnSaveTrainingReport();

	//胚胎定位
	ViewDetection m_EmbryoLocation;
	//探查位置
	std::list<ViewDetection *> m_ExploreLocations;
protected:
	virtual bool DetectExplore(float dt);

	virtual bool DetectLocation(float dt);
private:
	void TestExplore(float dt);

	void TestLocation(float dt);

	void TestTakeOutEmbryo(float dt);

	void TestFlushBloodAndSuction(float dt);

	void RecordCutErrorScore(std::set<int> & codes);

	//錯誤提示
	std::map<int , Ogre::String> m_CutErrorTipAboutUteri;
//	std::map<int , Ogre::String> m_CutErrorTipAboutOther;	//unused now
	
	//一次電切操作切錯的範圍
	std::set<int> m_CutErrorCodeAboutUteri;
//	std::set<int> m_CutErrorCodeAboutOther;

	//評分相關
	bool m_IsTookOutSoftly; 
	int m_CurErrorCount;		//
	int m_CutBiasedLevel;		//0-無偏差 编号相差3及以上無分
	int m_CutDirErrorCount;	//电/剪切方向错误次数

	//电/剪切范围颜色编号
	std::vector<int> m_CutColorCode;
	int m_LastCutColorCode;
	int m_GlobalMaxCutCode;
	int m_GlobalMinCutCode;

	//操作记录
	bool m_BeginCut;
	//bool m_ExploreResult;
	bool m_LastLocateResult;
	bool m_NeedShowCutTip;
	bool m_IsEmbryoTookOut;
	bool m_IsEmbryoPutIn;
	bool m_IsEmbryoRemoved;

	bool m_IsFlush;
	bool m_IsSuck;

	bool m_IsBurnCutFace;
	float m_ContinuousBurnCutFaceTime;

	MisMedicOrgan_Ordinary * m_pEmbryo;
	MisMedicOrgan_Ordinary * m_pUterus;

	PaintingTool m_painting;
	TextDisplay m_Text1;
};

#endif