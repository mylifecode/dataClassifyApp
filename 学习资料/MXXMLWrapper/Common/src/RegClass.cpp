/**Author:zx**/
#pragma once
#include "stdafx.h"
#include "RegClass.h"
#include "XMLWrapperLight.h"
#include "XMLWrapperMeshNode.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperSceneNode.h"
#include "XMLWrapperStaticScene.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperTraining.h"
#include "XMLWrapperGlobalTraining.h"
#include "XMLWrapperToolConfig.h"
#include "XMLWrapperHardwareConfig.h"
#include "XMLWrapperConnect.h"
#include "XMLWrapperAdhere.h"
#include "XMLWrapperAdhesion.h"
#include "XMLWrapperCollision.h"
#include "XMLWrapperMark.h"
#include "XMLWrapperLiquid.h"
#include "XMLWrapperParticle.h"
#include "XMLWrapperParticles.h"
#include "XMLWrapperPart.h"
#include "XMLWrapperPursue.h"
#include "XMLWrapperToolPlace.h"
#include "XMLWrapperDataForDeviceCandidate.h"
#include "XMLWrapperToolForTask.h"
#include "XMLWrapperMovie.h"
#include "XMLWrapperScore.h"
#include "XMLWrapperOnLineGrade.h"
#include "XMLWrapperTip.h"
#include "XMLWrapperShadow.h"
#include "XMLWrapperSphere.h"
#include "XMLWrapperMucous.h"
#include "XMLWrapperDetector.h"
#include "XMLWrapperOrganTranslation.h"
#include "XMLWrapperWaterPool.h"
#include "XMLWrapperViewDetection.h"
#include "XMLWrapperGlobalConfig.h"
#include "XMLWrapperOperateItem.h"

MXXMLWRAPPER_API void RegXMLClass()
{
	CXMLWrapperTraining::RegisterClass();
	CXMLWrapperMeshNode::RegisterClass();
	CXMLWrapperOrgan::RegisterClass();
	CXMLWrapperLight::RegisterClass();
	CXMLWrapperSceneNode::RegisterClass();
	CXMLWrapperStaticScene::RegisterClass();
	CXMLWrapperTool::RegisterClass();
	CXMLWrapperGlobalTraining::RegisterClass();
	CXMLWrapperToolConfig::RegisterClass();
    CXMLWrapperHardwareConfig::RegisterClass();
	CXMLWrapperConnect::RegisterClass();
    CXMLWrapperAdhere::RegisterClass();
	CXMLWrapperAdhesionCluster::RegisterClass();
    CXMLWrapperCollision::RegisterClass();
	CXMLWrapperMark::RegisterClass();
	CXMLWrapperLiquid::RegisterClass();
	CXMLWrapperParticles::RegisterClass();
	CXMLWrapperParticle::RegisterClass();
	CXMLWrapperPart::RegisterClass();
	CXMLWrapperPursue::RegisterClass();
	CXMLWrapperMovie::RegisterClass();
	CXMLWrapperToolPlace::RegisterClass();
    CXMLWrapperDataForDeviceCandidate::RegisterClass();
	CXMLWrapperToolForTask::RegisterClass();
	CXMLWrapperScore::RegisterClass();
	CXMLWrapperOnLineGrade::RegisterClass();
	CXMLWrapperTip::RegisterClass();
	CXMLWrapperShadow::RegisterClass();
	CXMLWrapperSphere::RegisterClass();
	CXMLWrapperDetector::RegisterClass();
	CXMLWrapperMucous::RegisterClass();
	CXMLWrapperOrganTranslation::RegisterClass();
	CXMLWrapperWaterPool::RegisterClass();
	CXMLWrapperOperateItem::RegisterClass();
	CXMLWrapperViewDetection::RegisterClass();

	CXMLWrapperGlobalConfig::RegisterClass();

}