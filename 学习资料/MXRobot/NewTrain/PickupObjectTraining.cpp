#include "PickupObjectTraining.h"
#include "MisMedicOrganOrdinary.h"
#include "OgreAxisAlignedBox.h"
#include "Instruments/Tool.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "ACTubeShapeObject.h"
#include <algorithm>
#include <SYScoreTableManager.h>
#include <SYScoreTable.h>
#include <OgreMovablePlane.h>


//=============================================================================================
CPickupObjectTraining::CPickupObjectTraining(float limitTime)
	:m_nextConstructInfoIndex(0),
	m_curOrganObject(nullptr),
	m_hasUpdatedCollideData(false),
	m_aabbForTriangle(Ogre::Vector3::ZERO,Ogre::Vector3::ZERO),
	m_aabbForQuat(Ogre::Vector3::ZERO,Ogre::Vector3::ZERO),
	m_aabbForHexagon(Ogre::Vector3::ZERO,Ogre::Vector3::ZERO),
	m_hasFloorPlane(false),
	m_floorPlane("floorPlane"),
	m_limitTime(limitTime),
	m_hasLimitTime(limitTime > 0.f),
	m_curObjectPosition(OP_AboveFloor),
	m_lastToolPositionContactWithObject(OP_ContactFloor),
	m_lastToolClampStateContactWithObject(false),
	m_contactWithTool(false),
	m_hasClampedAndAboveFloor(false),
	m_dropTimes(0)
{
	for(auto& s : m_objectStageStates){
		s.contactWithTool = false;
		s.isClamped = false;
	}
}
//=============================================================================================
CPickupObjectTraining::~CPickupObjectTraining(void)
{
	
}
//======================================================================================================================
bool CPickupObjectTraining::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	InitCode();

	GenObjectConstructInfoSequence();
	
	return result; 
}
//======================================================================================================================
void CPickupObjectTraining::InitCode()
{
	m_stepCodes.resize(5);

	//step1
	m_stepCodes[0].successPost = "0040100110";
	m_stepCodes[0].positionError = "0040100111";
	m_stepCodes[0].methodError = "0040100112";
	m_stepCodes[0].clampObject = "0040200610";
	m_stepCodes[0].unclampObject = "0040200619";
	m_stepCodes[0].undrop = "0040200710";
	m_stepCodes[0].drop = "0040200711";
	m_stepCodes[0].multiDrop = "0040200712";

	//step2
	m_stepCodes[1].successPost = "0040300110";
	m_stepCodes[1].positionError = "0040300111";
	m_stepCodes[1].methodError = "0040300112";
	m_stepCodes[1].clampObject = "0040400610";
	m_stepCodes[1].unclampObject = "0040400619";
	m_stepCodes[1].undrop = "0040400710";
	m_stepCodes[1].drop = "0040400711";
	m_stepCodes[1].multiDrop = "0040400712";

	//step3
	m_stepCodes[2].successPost = "0040500110";
	m_stepCodes[2].positionError = "0040500111";
	m_stepCodes[2].methodError = "0040500112";
	m_stepCodes[2].clampObject = "0040600610";
	m_stepCodes[2].unclampObject = "0040600619";
	m_stepCodes[2].undrop = "0040600710";
	m_stepCodes[2].drop = "0040600711";
	m_stepCodes[2].multiDrop = "0040600712";

	//step4
	m_stepCodes[3].successPost = "0040700110";
	m_stepCodes[3].positionError = "0040700111";
	m_stepCodes[3].methodError = "0040700112";
	m_stepCodes[3].clampObject = "0040800610";
	m_stepCodes[3].unclampObject = "0040800619";
	m_stepCodes[3].undrop = "0040800710";
	m_stepCodes[3].drop = "0040800711";
	m_stepCodes[3].multiDrop = "0040800712";

	//step5
	m_stepCodes[4].successPost = "0040900110";
	m_stepCodes[4].positionError = "0040900111";
	m_stepCodes[4].methodError = "0040900112";
	m_stepCodes[4].clampObject = "0041000610";
	m_stepCodes[4].unclampObject = "0041000619";
	m_stepCodes[4].undrop = "0041000710";
	m_stepCodes[4].drop = "0041000711";
	m_stepCodes[4].multiDrop = "0041000712";
}

//======================================================================================================================
SYScoreTable* CPickupObjectTraining::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01100401");
}
//======================================================================================================================
MisCTool_PluginClamp* ToolIsClampState(CTool* tool)
{
	if(tool == nullptr)
		return nullptr;

	for(size_t p = 0 ; p <tool->m_ToolPlugins.size() ; p++)
	{
		MisCTool_PluginClamp * clampPlugin = dynamic_cast<MisCTool_PluginClamp*>(tool->m_ToolPlugins[p]);		
		if(clampPlugin && clampPlugin->isInClampState())
			return clampPlugin;
	}

	return nullptr;
}
//======================================================================================================================
void CPickupObjectTraining::onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{
	MisNewTraining::onRSFaceContactsSolveEnded(RSContactConstraints);

	m_contactWithTool = false;

	CTool* leftTool = GetLeftTool();
	CTool* rightTool = GetRightTool();

	for(std::size_t i = 0; i < RSContactConstraints.size(); ++i){
		const GFPhysSoftFaceRigidContact& rs = RSContactConstraints[i];
		if(rs.m_SoftBody == m_curOrganObject->m_physbody){
			GFPhysRigidBody* rb = rs.m_Rigid;
			if(leftTool && leftTool->GetRigidBodyPart(rb) != -1){
				m_contactWithTool = true;
				break;
			}
			else if(rightTool && rightTool->GetRigidBodyPart(rb) != -1){
				m_contactWithTool = true;
				break;
			}
		}
	}
}
//======================================================================================================================
bool CPickupObjectTraining::Update( float dt )
{
	bool result = MisNewTraining::Update(dt);

	if(m_curOrganObject && m_hasUpdatedCollideData == false){
		UpdateCollideData();
		m_hasUpdatedCollideData = true;
	}

	if(m_curOrganObject){
		bool isClamped = ObjectIsClamped();
		ObjectPosition oldPos = m_curObjectPosition;
		m_curObjectPosition = GetObjectPosition();

		ObjectStageState& curStageState = m_objectStageStates[m_curObjectPosition];
		curStageState.contactWithTool = m_contactWithTool;
		curStageState.isClamped = isClamped;

		//夹起过
		if(oldPos == OP_ContactFloor && m_curObjectPosition == OP_AboveFloor && isClamped){
			m_hasClampedAndAboveFloor = true;
		}

		GFPhysVector3 minAABB, maxAABB;
		m_curOrganObject->m_physbody->GetWorldAabb(minAABB, maxAABB);
		
		bool canNextStep = false;
		bool postPositionError = true;
		Ogre::AxisAlignedBox organAABB(GPVec3ToOgre(minAABB), GPVec3ToOgre(maxAABB));
		const MisMedicDynObjConstructInfo& curConstructInfo = m_objectConstructInfoSequence[m_nextConstructInfoIndex - 1];
		
		if(organAABB.intersects(m_aabbForTriangle)){
			if(curConstructInfo.m_OrganType == 102)
				postPositionError = false;
			canNextStep = true;
		}
		else if(organAABB.intersects(m_aabbForQuat)){
			if(curConstructInfo.m_OrganType == 100)
				postPositionError = false;
			canNextStep = true;
		}
		else if(organAABB.intersects(m_aabbForHexagon)){
			if(curConstructInfo.m_OrganType == 101)
				postPositionError = false;
			canNextStep = true;
		}

		if(canNextStep){
			int time = GetElapsedTime();
			bool postMethodError = false;
			int curObjectIndex = m_nextConstructInfoIndex - 1;
			const StepCode& stepCode = m_stepCodes[curObjectIndex];

			if(m_lastToolPositionContactWithObject != OP_AboveFloor || m_lastToolClampStateContactWithObject == false)
				postMethodError = true;

			//post code
			if(postMethodError)
				AddScoreItemDetail(stepCode.methodError,time);
			else if(postPositionError)
				AddScoreItemDetail(stepCode.positionError, time);
			else
				AddScoreItemDetail(stepCode.successPost, time);

			//clamp code
			if(m_hasClampedAndAboveFloor){
				AddScoreItemDetail(stepCode.clampObject, time);

				//drop code
				if(m_dropTimes == 0)
					AddScoreItemDetail(stepCode.undrop, time);
				else if(m_dropTimes == 1)
					AddScoreItemDetail(stepCode.drop, time);
				else
					AddScoreItemDetail(stepCode.multiDrop, time);
			}
			else
				AddScoreItemDetail(stepCode.unclampObject, time);

			//tip
			if(postPositionError){
				CTipMgr::Instance()->ShowTip("PostArea");
				if(m_nextConstructInfoIndex == m_objectConstructInfoSequence.size())
					StartTimer(2.f, "Finish");
			}
			else{
				if(m_nextConstructInfoIndex == m_objectConstructInfoSequence.size()){
					CTipMgr::Instance()->ShowTip("PostFinish");
					TrainingFinish();
				}
				else{
					CTipMgr::Instance()->ShowTip("PostCorrect");
				}
			}

			//next step
			if(m_nextConstructInfoIndex < m_objectConstructInfoSequence.size()){
				ResetState();
				CreateOneObject();
			}
			else if(m_nextConstructInfoIndex == m_objectConstructInfoSequence.size()){
				RemoveCurrentObject();

				//整体操作
				if(time < 40)
					AddScoreItemDetail("0041100300", time);
				else if(time < 60)
					AddScoreItemDetail("0041100301", time);

				//器械移动
				float leftVelocity = m_pToolsMgr->GetLeftToolMovedSpeed();
				float rightVelocity = m_pToolsMgr->GetRightToolMovedSpeed();
				float v = (leftVelocity + rightVelocity) / 2;
				if(v <= 5)
					AddScoreItemDetail("0041200800", time);
				else if(v <= 10)
					AddScoreItemDetail("0041200801", time);
				else
					AddScoreItemDetail("0041200802", time);

				//器械移动距离
				float leftDis = m_pToolsMgr->GetLeftToolMovedDistance();
				float rightDis = m_pToolsMgr->GetRightToolMovedDistance();
				float dis = (leftDis + rightDis) / 2;
				if(dis < 80)
					AddScoreItemDetail("0041301010", time);
				else
					AddScoreItemDetail("0041301011", time);

				//器械夹闭次数
				int leftTimes = m_pToolsMgr->GetLeftToolClosedTimes();
				int rightTimes = m_pToolsMgr->GetRightToolClosedTimes();
				int times = leftTimes + rightTimes;
				if(times < 8)
					AddScoreItemDetail("0041401100", time);
				else
					AddScoreItemDetail("0041401101", time);

				//镜头方向感
				if(time < 60)
					AddScoreItemDetail("0041500610", time);
				else if(time < 90)
					AddScoreItemDetail("0041500611", time);

				//操作时间
				if(time < 60)
					AddScoreItemDetail("0041600500", time);
				else if(time < 90)
					AddScoreItemDetail("0041600501", time);
				else
					AddScoreItemDetail("0041600502", time);
			}
			
			return result;
		}

		if(m_contactWithTool){
			if(m_lastToolPositionContactWithObject != m_curObjectPosition){
				//掉落次数
				if(m_lastToolPositionContactWithObject == OP_AboveFloor && !isClamped)
					++m_dropTimes;
				m_lastToolPositionContactWithObject = m_curObjectPosition;
				m_lastToolClampStateContactWithObject = isClamped;
			}

			if(isClamped)
				m_lastToolClampStateContactWithObject = true;
		}
	}

	return result;
}
//======================================================================================================================
void CPickupObjectTraining::InternalSimulateStart(int currStep, int TotalStep, Real dt)
{
	MisNewTraining::InternalSimulateStart(currStep, TotalStep, dt);
}
//======================================================================================================================
void CPickupObjectTraining::InternalSimulateEnd(int currStep, int TotalStep, Real dt)
{
	MisNewTraining::InternalSimulateEnd(currStep, TotalStep, dt);
}
//=============================================================================================
bool CPickupObjectTraining::BeginRendOneFrame(float timeelpsed)
{
	MisNewTraining::BeginRendOneFrame(timeelpsed);

	return true;
}
//======================================================================================================================
void CPickupObjectTraining::OnSaveTrainingReport()
{
	MisNewTraining::OnSaveTrainingReport();
}

void CPickupObjectTraining::GenObjectConstructInfoSequence()
{
	srand(time(nullptr));
	m_objectConstructInfoSequence = m_reservedConstructInfos;

	if(m_reservedConstructInfos.size() == 0)
		throw "no construct info";

	const int numReservedConstructInfo = static_cast<int>(m_objectConstructInfoSequence.size());
	const int numAddedInfo = 5 - numReservedConstructInfo;

	for(int i = 0; i < numAddedInfo; ++i){
		int index = rand() % numReservedConstructInfo;
		m_objectConstructInfoSequence.push_back(m_reservedConstructInfos[index]);
	}

	std::random_shuffle(m_objectConstructInfoSequence.begin(), m_objectConstructInfoSequence.end());

	//modify organ id
	int minId = m_objectConstructInfoSequence[0].m_OrganId;
	for(int i = 1; i < m_objectConstructInfoSequence.size(); ++i)
		m_objectConstructInfoSequence[i].m_OrganId = minId + i;
}

void CPickupObjectTraining::UpdateCollideData()
{
	Ogre::SceneManager* sceneManager = MXOgre_SCENEMANAGER;
	Ogre::SceneNode* rootNode = sceneManager->getRootSceneNode();
	Ogre::SceneNode* planeNode = nullptr;
	auto childItr = rootNode->getChildIterator();

	int counter = 0;
	while(childItr.hasMoreElements())
	{
		Ogre::SceneNode* node = static_cast<Ogre::SceneNode*>(childItr.getNext());
		const Ogre::String& name = node->getName();
		node->_updateBounds();
		if(Ogre::StringUtil::startsWith(name, "PickupTraining_PlaneForQuat")){
			m_aabbForQuat = node->_getWorldAABB();
			++counter;
		}
		else if(Ogre::StringUtil::startsWith(name, "PickupTraining_PlaneForTriangle")){
			m_aabbForTriangle = node->_getWorldAABB();
			++counter;
		}
		else if(Ogre::StringUtil::startsWith(name, "PickupTraining_PlaneForCylinder")){
			m_aabbForHexagon = node->_getWorldAABB();
			++counter;
		}
		else if(Ogre::StringUtil::startsWith(name, "FloorPlane")){
			planeNode = node;
			Ogre::SceneNode::ObjectIterator itr = node->getAttachedObjectIterator();
			while(itr.hasMoreElements()){
				auto plane = dynamic_cast<Ogre::MovablePlane*>(itr.getNext());
				if(plane){
					m_floorPlane = plane->_getDerivedPlane();
					m_hasFloorPlane = true;
					break;
				}
				
			}
			++counter;
		}

		if(counter == 4)
			break;
	}
}

void CPickupObjectTraining::CreateOneObject()
{
	if(m_curOrganObject)
		RemoveCurrentObject();

	if(m_nextConstructInfoIndex < m_objectConstructInfoSequence.size()){
		MisMedicDynObjConstructInfo& constructInfo = m_objectConstructInfoSequence[m_nextConstructInfoIndex];
		m_nextConstructInfoIndex++;

		m_curOrganObject = new MisMedicOrgan_Ordinary(constructInfo.m_OrganType, constructInfo.m_OrganId, this);
		//m_curOrganObject->ReadOrganObjectFile(constructInfo);
		m_curOrganObject->Create(constructInfo);
		m_DynObjMap.insert(std::make_pair(constructInfo.m_OrganId, m_curOrganObject));
	}
}

void CPickupObjectTraining::RemoveCurrentObject()
{
	if(m_curOrganObject){
		RemoveOrganFromWorld(m_curOrganObject);
		m_curOrganObject = nullptr;
	}
}

void CPickupObjectTraining::OnTrainingIlluminated()
{
	MisNewTraining::OnTrainingIlluminated();
	CreateOneObject();
	CTipMgr::Instance()->ShowTip("OperationBegin");

	if(m_hasLimitTime)
		StartTimer(m_limitTime);
}

void CPickupObjectTraining::OnTimerTimeout(int id, float dt, void* userData)
{
	//CTipMgr::Instance()->ShowTip("OperationTimeout");
	if(userData){
		Ogre::String data = static_cast<const char*>(userData);
		if (Ogre::StringUtil::match(data, "Finish"))
		{
			TrainingFinish();
		}
	}
}

CPickupObjectTraining::ObjectPosition CPickupObjectTraining::GetObjectPosition()
{
	if(m_curOrganObject == nullptr || !m_hasFloorPlane)
		throw "error";

	static bool flag = true;
	static float sign = 1.0f;
	//GFPhysVector3 planeNormal(m_floorPlane.normal.x, m_floorPlane.normal.z, -m_floorPlane.normal.y);
	GFPhysVector3 planeNormal(m_floorPlane.normal.x, m_floorPlane.normal.y, m_floorPlane.normal.z);
	
	GFPhysSoftBodyNode* node = m_curOrganObject->m_physbody->GetNodeList();
	GFPhysTransform transform = m_curOrganObject->m_physbody->GetWorldTransform();
	GFPhysVector3 worldPos;
	bool isUp = false;
	bool isDown = false;
	while(node){
		worldPos = transform * node->m_CurrPosition;
		float dis = planeNormal.Dot(node->m_CurrPosition) + m_floorPlane.d;
		if(flag){
			if(dis < 0.f)
				sign = -1;
			flag = false;
		}

		dis *= sign;
			
		if(dis > 0.00001f)
			isUp = true;
		else if(dis < 0.00001f)
			isDown = true;

		node = node->m_Next;
	}

	if(isUp && isDown == false)
		return OP_AboveFloor;
	else if(isUp == false && isDown)
		return OP_UnderFloor;
	else
		return OP_ContactFloor;
}

bool CPickupObjectTraining::ObjectIsClamped()
{
	bool isClamped = false;
	CTool* leftTool = GetLeftTool();
	CTool* rightTool = GetRightTool();

	if(ToolIsClampState(leftTool))
		isClamped = true;

	if(isClamped == false && ToolIsClampState(rightTool))
		isClamped = true;

	return isClamped;
}

void CPickupObjectTraining::ResetState()
{
	m_curObjectPosition = OP_AboveFloor;
	m_lastToolPositionContactWithObject = OP_ContactFloor;
	m_lastToolClampStateContactWithObject = false;
	m_contactWithTool = false;
	m_hasClampedAndAboveFloor = false;
	m_dropTimes = 0;

	for(auto& state : m_objectStageStates){
		state.contactWithTool = false;
		state.isClamped = false;
	}
}