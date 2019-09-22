#include "MxDefine.h"
#include "RbAdjustShaderParametersWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTextedit>
#include "MXApplication.h"
#include "MXOgreGraphic.h"

#include "../MXRobot/Common/LightMgr.h"
#include "../MXRobot/Include/BasicTraining.h"
#include "../MXRobot/Include/stdafx.h"
#include "../MXRobot/Include/PathputTool.h"
#include "MouseInput.h"

#include "TrainingMgr.h"
#include "../MXRobot/NewTrain/MisNewTraining.h"
#include "XMLWrapperTraining.h"

// #include "../MXRobot/NewTrain/MisMedicOrganOrdinary.h"
// #include "../MXRobot/NewTrain/MisNewTraining.h"

#define RGBDenominator 1000.0f

#define GammaDenominator 50.0f

#define OriginalRhosDenominator 1000.0f

//ward
#define WardRhosDenominator 50.0f
#define WardAlphaXDenominator 100.0f
#define WardAlphaYDenominator 100.0f
//cook torrance
#define CTRhosDenominator 40.0f
#define CTRoughnessDenominator 1000.0f
#define RefAtNormalDenominator 100.0f


#define RhotsDenominator 1.0f

#define HdrEnableDenominator 100.0f
#define KeyValueModeDenominator 100.0f
#define KeyValueDenominator 1000.0f
#define BloomScaleDenominator 1000.0f




float GetSliderValue(QSlider *pSlider , int value , float denominator)
{
	float min = pSlider->minimum();
	float max = pSlider->maximum();

	float result = ((float)value /*- min*/) / denominator;// / (max - min);
	//result /= denominator;
	return result;
}

void SetSliderValue(QSlider *pSlider , float value , float denominator)
{
	int pos =  value * denominator + 0.5;
	pSlider->setValue(pos);	
}

static QString GetOrganNameByType(TEDOT type)
{
	switch(type)
	{
	case EDOT_NO_TYPE:
		return QString::fromLocal8Bit("未知");
		break;
	case EDOT_BRAVERY_ARTERY:
		return QString::fromLocal8Bit("胆囊动脉");
		break;
	case EDOT_COMMON_BILE_DUCT:
		return QString::fromLocal8Bit("肝总管");
		break;
	case EDOT_CYSTIC_DUCT:
		return QString::fromLocal8Bit("胆囊管");
		break;
	case EDOT_GALLBLADDER:
		return QString::fromLocal8Bit("胆囊");
		break;
	case EDOT_HEPATIC_ARTERY:
		return QString::fromLocal8Bit("肝动脉");
		break;
	case EDOT_LIVER:
		return QString::fromLocal8Bit("肝");
		break;
	case EDOT_VEIN:
		return QString::fromLocal8Bit("胆囊三角");
		break;
	case EDOT_RENAL_ARTERY:
		return QString::fromLocal8Bit("底部三角");
		break;
	case EDOT_HELPER_OBJECT0:
		return QString::fromLocal8Bit("辅助体");
		break;
	case EODT_VEINCONNECT:
		return QString::fromLocal8Bit("胆囊连接");
		break;
	case EODT_VEINBOTTOMCONNECT:
		return QString::fromLocal8Bit("胆囊底部连接");
		break;
	default:
		return QString::fromLocal8Bit("未知");
		break;
	}
}




RbAdjustShaderParametersWidget::RbAdjustShaderParametersWidget(QWidget *parent /* = NULL */)
:QDialog(parent , Qt::WindowCloseButtonHint | Qt::WindowTitleHint) ,
	m_SelectedOrganID(-1),
	m_pManualForSelectedOrgan(NULL),
	m_pSelectedOrgan(NULL),
	m_Timer(NULL),
	m_IsShowOrganFrame(true),
	m_IsGetMat(false) , 
	m_CommonScale(1.0)
{
	ui.setupUi(this);
	
	ui.DynObjList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.DynObjList->setAutoFillBackground(true);

	m_dynobj_list_model = new QStandardItemModel();

	m_registerd_train = 0;

	connect(ui.DynObjList,SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OnDynObjListClicked(const QModelIndex &)));

	m_Timer = new QTimer(this);
	connect(m_Timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	m_Timer->start(50);

 	connect(ui.FrameVisibleBtn , SIGNAL(clicked()), this, SLOT(OnFrameVisibleBtnClicked()));


	connect(ui.KaRSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KaGSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KaBSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KdRSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KdGSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KdBSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KsRSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KsGSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KsBSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	
	connect(ui.RhotsSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));

	connect(ui.OriginalRhosSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));

	connect(ui.GammaSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.Ward_AlphaxSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.Ward_AlphaySlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.WardRhosSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.CTRhosSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.CTRoughnessSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.RefAtNormSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));

	//ssss
	connect(ui.sssWidthSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.maxOffsetMmSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.SSSSstrengthSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));


	connect(ui.CommonScaleFactorLE, SIGNAL(textChanged(const QString &)),this,SLOT(OnLineEditTextChanged(const QString &)));



	

	connect(ui.IsUseHDRSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KeyValueModeSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.KeyValueSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));
	connect(ui.BloomScaleSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnParamsValueChanged(int)));

}

RbAdjustShaderParametersWidget::~RbAdjustShaderParametersWidget()
{
	if(m_dynobj_list_model)
	{
		delete m_dynobj_list_model;
		m_dynobj_list_model = 0 ;
	}
	
	if(m_registerd_train)
	{
		CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
		SY_ASSERT(pTraining == m_registerd_train || pTraining==0);
		if(pTraining)
		{

		}
	}
	
	if(m_pManualForSelectedOrgan)
	{
		m_pManualForSelectedOrgan->detachFromParent();
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pManualForSelectedOrgan);
		m_pManualForSelectedOrgan = NULL;
	}
	
	if(m_Timer)
	{
		m_Timer->stop();
		delete m_Timer;
	}

	m_pSelectedOrgan = NULL;
	m_IsShowOrganFrame =false;

	
}

void RbAdjustShaderParametersWidget::UpdateDynObjList()
{
	if(m_dynobj_list_model == 0)
	{
		m_dynobj_list_model = new QStandardItemModel();
	}
	m_dynobj_list_model->clear();

	CBasicTraining * pTraining = m_registerd_train;

	if(pTraining)
	{
		std::vector<MisMedicOrganInterface*> OrgansInTrain;

		pTraining->GetAllOrgan(OrgansInTrain);

		for(size_t c = 0 ; c < OrgansInTrain.size() ; c++)
		{
			MisMedicOrganInterface* pOrganInterface = OrgansInTrain[c];
			int OrganId = pOrganInterface->m_OrganID;
			QString objname = QString(pOrganInterface->getName().c_str());//GetOrganNameByType((TEDOT)OrganId);

			QStandardItem * item = new QStandardItem(objname);

			item->setData(QVariant::fromValue(OrganId),Qt::UserRole + 1);

			m_dynobj_list_model->appendRow(item);
		}
	}	
	ui.DynObjList->setModel(m_dynobj_list_model);
}


void RbAdjustShaderParametersWidget::OnCurrentTrainChanged(CBasicTraining * ptrain)
{
	m_registerd_train = ptrain;
	if(m_registerd_train == nullptr)
		return;

	UpdateDynObjList();

}

void RbAdjustShaderParametersWidget::OnDynObjListClicked(const QModelIndex &index)
{
	QStandardItem * item = m_dynobj_list_model->itemFromIndex(index);

	if (item > 0)
	{
		m_SelectedOrganID = index.data(Qt::UserRole + 1).value<unsigned int>();
		m_pSelectedOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(m_registerd_train->GetOrgan(m_SelectedOrganID));
		
		if(m_pSelectedOrgan == NULL)
			return;

//		std::map<int , Ogre::MaterialPtr>::iterator itor = m_OrganMaterialPtrs.find(m_SelectedOrganID);
// 		if(itor == m_OrganMaterialPtrs.end())
// 		{
// 			Ogre::String originMatName = m_pSelectedOrgan->getMaterialName();
// 			ui.MatNameLabel->setText(QString(originMatName.c_str()));
// 			Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(originMatName);
// 
// 			m_OrganMaterialPtrs.insert(make_pair(m_SelectedOrganID , material));

		Ogre::String originMatName = m_pSelectedOrgan->getMaterialName();
		ui.MatNameLabel->setText(QString(originMatName.c_str()));
		m_CurrMatPtr = Ogre::MaterialManager::getSingleton().getByName(originMatName);
		//设置数值
		GetShaderParams(m_CurrMatPtr);
		ShowShaderParams();
// 		}
// 		else
// 		{
// 			Ogre::MaterialPtr material = itor->second;
// 			
// 		}
	}
}




void RbAdjustShaderParametersWidget::OnFrameVisibleBtnClicked()
{
	m_IsShowOrganFrame = !m_IsShowOrganFrame;
}

void RbAdjustShaderParametersWidget::OnParamsValueChanged(int value)
{
	QSlider *pSlider =  qobject_cast<QSlider*>(sender());
	if(!pSlider)
		return;

	if(m_registerd_train->m_pTrainingConfig->m_HDR)
	{
		if(!m_IsGetMat)
		{
			Ogre::CompositorManager& compMgr = Ogre::CompositorManager::getSingleton();
			Ogre::CompositorLogic* comlogic =  compMgr.getCompositorLogic("HDR");
			//HDRLogic * hdrlogic = dynamic_cast<HDRLogic*>(comlogic);
			
			//m_SSSSBlurXMatPtr = hdrlogic->GetSSSSBlurXMatPtr();
			//m_SSSSBlurYMatPtr = hdrlogic->GetSSSSBlurYMatPtr();

// 			m_BloomMatPtr =hdrlogic->GetBrightpassPtr();
// 			m_FinalToneMappingMatPtr =hdrlogic->GetFinalToneMatPtr();
			m_IsGetMat = true;
		}
		//about sss
		if(pSlider == ui.sssWidthSlider) {
			float fvalue = GetSliderValue(pSlider , value , 100.0f) * m_CommonScale;
			QString vstr = QString::number(fvalue);
			ui.sssWidthLabel->setText(vstr);
			SetFpParameterValue(m_SSSSBlurXMatPtr ,"sssWidthX", fvalue);
			SetFpParameterValue(m_SSSSBlurYMatPtr ,"sssWidthY", fvalue);
		}
		else if(pSlider == ui.maxOffsetMmSlider) {
			float fvalue = GetSliderValue(pSlider , value , 100.0f) * m_CommonScale;
			QString vstr = QString::number(fvalue);
			ui.maxOffsetMmLabel->setText(vstr);
			SetFpParameterValue(m_SSSSBlurXMatPtr ,"maxOffsetMmX", fvalue);
			SetFpParameterValue(m_SSSSBlurYMatPtr ,"maxOffsetMmY", fvalue);
		}
		else if(pSlider == ui.SSSSstrengthSlider) {
			float fvalue = GetSliderValue(pSlider , value , 100.0f) * m_CommonScale;
			QString vstr = QString::number(fvalue);
			ui.SSSSstrengthLable->setText(vstr);
			SetFpParameterValue(m_SSSSBlurXMatPtr ,"strengthX", fvalue);
			SetFpParameterValue(m_SSSSBlurYMatPtr ,"strengthY", fvalue);
		}

		//about hdr
		if(pSlider == ui.IsUseHDRSlider) {
			float fvalue = GetSliderValue(pSlider , value , HdrEnableDenominator);
			if(fvalue < 0.5)
				ui.IsUseHDRLabel->setText("enable");
			else
				ui.IsUseHDRLabel->setText("disable");
			SetFpParameterValue(m_FinalToneMappingMatPtr ,"IsUseHDR", fvalue);
		}
		else if(pSlider == ui.KeyValueModeSlider) {
			float fvalue = GetSliderValue(pSlider , value , KeyValueModeDenominator);
			if(fvalue < 0.5)
				ui.KeyValueModeLabel->setText("manual");
			else
				ui.KeyValueModeLabel->setText("auto");
			SetFpParameterValue(m_FinalToneMappingMatPtr ,"HDRKeyValueMode", fvalue);
			SetFpParameterValue(m_BloomMatPtr ,"BloomKeyValueMode", fvalue);
		}
		else if(pSlider == ui.BloomScaleSlider) {
			float fvalue = GetSliderValue(pSlider , value , BloomScaleDenominator);
			QString vstr = QString::number(fvalue);
			ui.BloomScaleLabel->setText(vstr);
			SetFpParameterValue(m_FinalToneMappingMatPtr ,"BloomScale", fvalue);
		}
		else if(pSlider == ui.KeyValueSlider) {
			float fvalue = GetSliderValue(pSlider , value , KeyValueDenominator);
			QString vstr = QString::number(fvalue);
			ui.KeyValueLabel->setText(vstr);
			SetFpParameterValue(m_FinalToneMappingMatPtr ,"HDRKeyValue", fvalue);
			SetFpParameterValue(m_BloomMatPtr ,"BloomKeyValue", fvalue);
		}
	}

	if(!m_pSelectedOrgan)
		return;

	if(pSlider == ui.KaRSlider)
	{
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KaRLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.AmbientColor.r = fvalue;
		m_CurrMatPtr->setAmbient(m_CurrShaderParams.AmbientColor);

	} else if(pSlider == ui.KaGSlider) {
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KaGLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.AmbientColor.g = fvalue;
		m_CurrMatPtr->setAmbient(m_CurrShaderParams.AmbientColor);

	} else if(pSlider == ui.KaBSlider) {
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KaBLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.AmbientColor.b = fvalue;
		m_CurrMatPtr->setAmbient(m_CurrShaderParams.AmbientColor);

	} else if(pSlider == ui.KdRSlider) {
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KdRLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.DiffuseColor.r = fvalue;
		m_CurrMatPtr->setDiffuse(m_CurrShaderParams.DiffuseColor);

	} else if(pSlider == ui.KdGSlider) {
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KdGLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.DiffuseColor.g = fvalue;
		m_CurrMatPtr->setDiffuse(m_CurrShaderParams.DiffuseColor);

	} else if(pSlider == ui.KdBSlider) {
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KdBLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.DiffuseColor.b = fvalue;
		m_CurrMatPtr->setDiffuse(m_CurrShaderParams.DiffuseColor);

	} else if(pSlider == ui.KsRSlider) {
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KsRLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.SpecularColor.r = fvalue;
		m_CurrMatPtr->setSpecular(m_CurrShaderParams.SpecularColor);

	} else if(pSlider == ui.KsGSlider) {
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KsGLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.SpecularColor.g = fvalue;
		m_CurrMatPtr->setSpecular(m_CurrShaderParams.SpecularColor);

	} else if(pSlider == ui.KsBSlider) {
		float fvalue = GetSliderValue(pSlider , value , RGBDenominator);
		ui.KsBLabel->setText(QString::number(fvalue));
		m_CurrShaderParams.SpecularColor.b = fvalue;
		m_CurrMatPtr->setSpecular(m_CurrShaderParams.SpecularColor);
	} 	else if(pSlider == ui.RhotsSlider) {		 //other constant
		float fvalue = GetSliderValue(pSlider , value , RhotsDenominator);
		QString vstr = QString::number(fvalue);
		ui.RhotsLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"rhots", fvalue / 1000);
	}
	else if(pSlider == ui.GammaSlider) {		 //other constant
		float fvalue = GetSliderValue(pSlider , value , GammaDenominator);
		QString vstr = QString::number(fvalue);
		ui.GammaLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"Gamma", fvalue);
	}
	else if(pSlider == ui.WardRhosSlider) {		 //other constant
		float fvalue = GetSliderValue(pSlider , value , WardRhosDenominator);
		QString vstr = QString::number(fvalue);
		ui.WardRhosLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"WardRhos", fvalue);
	}
	else if(pSlider == ui.Ward_AlphaxSlider) {		 //other constant
		float fvalue = GetSliderValue(pSlider , value , WardAlphaXDenominator);
		QString vstr = QString::number(fvalue);
		ui.Ward_AlphaxLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"WardAlphaX", fvalue);
	}
	else if(pSlider == ui.Ward_AlphaySlider) {		 //other constant
		float fvalue = GetSliderValue(pSlider , value , WardAlphaYDenominator);
		QString vstr = QString::number(fvalue);
		ui.Ward_AlphayLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"WardAlphaY", fvalue);
	}
	else if(pSlider == ui.CTRhosSlider) {		 //other constant
		float fvalue = GetSliderValue(pSlider , value , CTRhosDenominator);
		QString vstr = QString::number(fvalue);
		ui.CTRhosLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"CTRhos", fvalue);
	}
	else if(pSlider == ui.CTRoughnessSlider) {		 //other constant
		float fvalue = GetSliderValue(pSlider , value , CTRoughnessDenominator);
		QString vstr = QString::number(fvalue);
		ui.CTRoughnessLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"CTRoughness", fvalue);
	}
	else if(pSlider == ui.RefAtNormSlider) {		 //other constant
		float fvalue = GetSliderValue(pSlider , value , RefAtNormalDenominator);
		QString vstr = QString::number(fvalue);
		ui.RefAtNormalLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"RefAtNormal", fvalue);
	}
	else if(pSlider == ui.OriginalRhosSlider) {
		float fvalue = GetSliderValue(pSlider , value , OriginalRhosDenominator);
		QString vstr = QString::number(fvalue);
		ui.OriginalRhosLabel->setText(vstr);
		SetFpParameterValue(m_CurrMatPtr ,"OriginalRhos", fvalue);
	}
}

void RbAdjustShaderParametersWidget::OnLineEditTextChanged(const QString &text)
{
	QLineEdit *pLE = qobject_cast<QLineEdit*>(sender());
	if(!pLE)
		return;

	bool ok;
	if(pLE == ui.CommonScaleFactorLE)
	{
		m_CommonScale = pLE->text().toFloat(&ok);
	}
}

void RbAdjustShaderParametersWidget::SetFpParameterValue(Ogre::MaterialPtr matPtr , const char * paramName, float paramValue)
{
	try
	{
		Ogre::Pass * pass = matPtr->getTechnique(0)->getPass(0);
		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		params->setNamedConstant(paramName, paramValue);
	}
	catch(Ogre::Exception &ex)// (Ogre::InvalidParametersException & ex)
	{
		ShowDebugInfo(ex.getDescription().c_str());
	}

}

void RbAdjustShaderParametersWidget::timerUpdate()
{
	DrawSelectedOrgan();
}

void RbAdjustShaderParametersWidget::ShowDebugInfo(const QString& str)
{
	ui.DebugText->append(str);	
}

void RbAdjustShaderParametersWidget::GetShaderParams(Ogre::MaterialPtr matPtr)
{
	if(matPtr.isNull() == false)
	{
		Ogre::Technique * tech = matPtr->getTechnique(0);

		if(tech->getNumPasses() > 0)
		{
			Ogre::Pass * pass = tech->getPass(0);
			m_CurrShaderParams.AmbientColor =  pass->getAmbient();
			m_CurrShaderParams.DiffuseColor =  pass->getDiffuse();
			m_CurrShaderParams.SpecularColor =  pass->getSpecular();

			//fragment shader
			Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
			const Ogre::GpuNamedConstants & fsNamedConstants = params->getConstantDefinitions();
			Ogre::GpuConstantDefinitionMap fsMap = fsNamedConstants.map;
			ShowDebugInfo("fragment shader named constants:");
			for(Ogre::GpuConstantDefinitionMap::iterator itor = fsMap.begin() ; itor != fsMap.end() ; itor++)
			{
				Ogre::String nameStr = itor->first;
				ShowDebugInfo(nameStr.c_str());
				if(nameStr == "Gamma")
				{
					size_t index = itor->second.physicalIndex;
					const float *  pvalue =  params->getFloatPointer(index);
					SetSliderValue(ui.GammaSlider , *pvalue , GammaDenominator);
					ui.GammaLabel->setText(QString::number(*pvalue));
				}
				else if(nameStr == "WardRhos")
				{
					size_t index = itor->second.physicalIndex;
					const float *  pvalue =  params->getFloatPointer(index);
					SetSliderValue(ui.WardRhosSlider , *pvalue , WardRhosDenominator);
					ui.WardRhosLabel->setText(QString::number(*pvalue));

				}
				else if(nameStr == "WardAlphaX")
				{
					size_t index = itor->second.physicalIndex;
					const float *  pvalue =  params->getFloatPointer(index);
					SetSliderValue(ui.Ward_AlphaxSlider , *pvalue , WardAlphaXDenominator);
					ui.Ward_AlphaxLabel->setText(QString::number(*pvalue));			
				}
				else if(nameStr == "WardAlphaY")
				{
					size_t index = itor->second.physicalIndex;
					const float *  pvalue =  params->getFloatPointer(index);
					SetSliderValue(ui.Ward_AlphaySlider , *pvalue , WardAlphaYDenominator);
					ui.Ward_AlphayLabel->setText(QString::number(*pvalue));			

				}
				else if(nameStr == "CTRhos")
				{
					size_t index = itor->second.physicalIndex;
					const float *  pvalue =  params->getFloatPointer(index);
					SetSliderValue(ui.CTRhosSlider , *pvalue , CTRhosDenominator);
					ui.CTRhosLabel->setText(QString::number(*pvalue));			

				}
				else if(nameStr == "CTRoughness")
				{
					size_t index = itor->second.physicalIndex;
					const float *  pvalue =  params->getFloatPointer(index);
					SetSliderValue(ui.CTRoughnessSlider , *pvalue , CTRhosDenominator);
					ui.CTRoughnessLabel->setText(QString::number(*pvalue));			

				}
				else if(nameStr == "RefAtNormal")
				{
					size_t index = itor->second.physicalIndex;
					const float *  pvalue =  params->getFloatPointer(index);
					SetSliderValue(ui.RefAtNormSlider , *pvalue , RefAtNormalDenominator);
					ui.RefAtNormalLabel->setText(QString::number(*pvalue));			
				}
				else if(nameStr == "OriginalRhos")
				{
					size_t index = itor->second.physicalIndex;
					const float *  pvalue =  params->getFloatPointer(index);
					SetSliderValue(ui.OriginalRhosSlider , *pvalue , OriginalRhosDenominator);
					ui.OriginalRhosLabel->setText(QString::number(*pvalue));			
				}
			}
		}
	}
}

void RbAdjustShaderParametersWidget::ShowShaderParams()
{
	//ambient diffuse specular
	ui.KaRLabel->setText(QString::number(m_CurrShaderParams.AmbientColor.r));
	ui.KaGLabel->setText(QString::number(m_CurrShaderParams.AmbientColor.g));
	ui.KaBLabel->setText(QString::number(m_CurrShaderParams.AmbientColor.b));

	ui.KdRLabel->setText(QString::number(m_CurrShaderParams.DiffuseColor.r));
	ui.KdGLabel->setText(QString::number(m_CurrShaderParams.DiffuseColor.g));
	ui.KdBLabel->setText(QString::number(m_CurrShaderParams.DiffuseColor.b));

	ui.KsRLabel->setText(QString::number(m_CurrShaderParams.SpecularColor.r));
	ui.KsGLabel->setText(QString::number(m_CurrShaderParams.SpecularColor.g));
	ui.KsBLabel->setText(QString::number(m_CurrShaderParams.SpecularColor.b));

	SetSliderValue(ui.KaRSlider , m_CurrShaderParams.AmbientColor.r , RGBDenominator);
	SetSliderValue(ui.KaGSlider , m_CurrShaderParams.AmbientColor.g , RGBDenominator);
	SetSliderValue(ui.KaBSlider , m_CurrShaderParams.AmbientColor.b , RGBDenominator);

	SetSliderValue(ui.KdRSlider , m_CurrShaderParams.DiffuseColor.r , RGBDenominator);
	SetSliderValue(ui.KdGSlider , m_CurrShaderParams.DiffuseColor.g , RGBDenominator);
	SetSliderValue(ui.KdBSlider , m_CurrShaderParams.DiffuseColor.b , RGBDenominator);

	SetSliderValue(ui.KsRSlider , m_CurrShaderParams.SpecularColor.r , RGBDenominator);
	SetSliderValue(ui.KsGSlider , m_CurrShaderParams.SpecularColor.g , RGBDenominator);
	SetSliderValue(ui.KsBSlider , m_CurrShaderParams.SpecularColor.b , RGBDenominator);

	//ohter constant



	
}

void RbAdjustShaderParametersWidget::DrawSelectedOrgan()
{
	if(m_pManualForSelectedOrgan)
		m_pManualForSelectedOrgan->clear();
	int hehe = 0;
	if(m_pSelectedOrgan && m_IsShowOrganFrame)
	{
		if(!m_pSelectedOrgan->m_Visible)
			return;
		if(!m_pManualForSelectedOrgan)
		{
			m_pManualForSelectedOrgan = MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
			MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_pManualForSelectedOrgan);
			m_pManualForSelectedOrgan->setDynamic(true);
		}
		
		Ogre::Vector3 center = Ogre::Vector3::ZERO;
		m_pManualForSelectedOrgan->begin("VeinEditor/BezierWireFrame");

		for(size_t v = 0 ; v  < m_pSelectedOrgan->m_OrganRendNodes.size() ; v++)
		{
			m_pManualForSelectedOrgan->position(m_pSelectedOrgan->m_OrganRendNodes[v].m_CurrPos + m_pSelectedOrgan->m_OrganRendNodes[v].m_Normal * 0.05);
			m_pManualForSelectedOrgan->colour(0,1,1,0.5);
			center += m_pSelectedOrgan->m_OrganRendNodes[v].m_CurrPos;
		}
		
		center /= m_pSelectedOrgan->m_OrganRendNodes.size();

		for (size_t f = 0 ; f < m_pSelectedOrgan->m_OriginFaces.size(); f++)
		{
			int vid[3];
			vid[0] = m_pSelectedOrgan->m_OriginFaces[f].vi[0];
			vid[1] = m_pSelectedOrgan->m_OriginFaces[f].vi[1];
			vid[2] = m_pSelectedOrgan->m_OriginFaces[f].vi[2];
			if(m_pSelectedOrgan->m_OriginFaces[f].m_physface && m_pSelectedOrgan->m_OriginFaces[f].m_NeedRend)
				m_pManualForSelectedOrgan->triangle(vid[0] , vid[1] , vid[2]);
		}
		m_pManualForSelectedOrgan->end();

// 		m_pManualForSelectedOrgan->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
// 		
// 		m_pManualForSelectedOrgan->position(center);
// 		m_pManualForSelectedOrgan->colour(1, 0, 0);
// 		m_pManualForSelectedOrgan->position(center + 3 * Ogre::Vector3::UNIT_X);
// 		m_pManualForSelectedOrgan->colour(1, 0, 0);
// 
// 		m_pManualForSelectedOrgan->position(center);
// 		m_pManualForSelectedOrgan->colour(0, 1, 0);
// 		m_pManualForSelectedOrgan->position(center + 3 * Ogre::Vector3::UNIT_Y);
// 		m_pManualForSelectedOrgan->colour(0, 1, 0);
// 
// 		m_pManualForSelectedOrgan->position(center);
// 		m_pManualForSelectedOrgan->colour(0, 0, 1);
// 		m_pManualForSelectedOrgan->position(center + 3 * Ogre::Vector3::UNIT_Z);
// 		m_pManualForSelectedOrgan->colour(0, 0, 1);
// 
// 		m_pManualForSelectedOrgan->end();
	}
}
