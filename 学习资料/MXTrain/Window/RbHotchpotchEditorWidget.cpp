#include "MxDefine.h"
#include "RbHotchpotchEditorWidget.h"
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

#include "MxDefine.h"

// #include "../MXRobot/NewTrain/MisMedicOrganOrdinary.h"
// #include "../MXRobot/NewTrain/MisNewTraining.h"

using namespace EditorTool;


float GetSliderValue(QSlider *pSlider , int value)
{
	float min = pSlider->minimum();
	float max = pSlider->maximum();

	float result = ((float)value /*- min*/) / (max - min);
	return result;
}

void SetSliderValue(QSlider *pSlider , float value)
{
// 	int pos =  value  + 0.5;
// 	pSlider->setValue(pos);	
}

// static QString GetOrganNameByType(TEDOT type)
// {
// 	switch(type)
// 	{
// 	case EDOT_NO_TYPE:
// 		return QString::fromLocal8Bit("未知");
// 		break;
// 	case EDOT_BRAVERY_ARTERY:
// 		return QString::fromLocal8Bit("胆囊动脉");
// 		break;
// 	case EDOT_COMMON_BILE_DUCT:
// 		return QString::fromLocal8Bit("肝总管");
// 		break;
// 	case EDOT_CYSTIC_DUCT:
// 		return QString::fromLocal8Bit("胆囊管");
// 		break;
// 	case EDOT_GALLBLADDER:
// 		return QString::fromLocal8Bit("胆囊");
// 		break;
// 	case EDOT_HEPATIC_ARTERY:
// 		return QString::fromLocal8Bit("肝动脉");
// 		break;
// 	case EDOT_LIVER:
// 		return QString::fromLocal8Bit("肝");
// 		break;
// 	case EDOT_VEIN:
// 		return QString::fromLocal8Bit("胆囊三角");
// 		break;
// 	case EDOT_RENAL_ARTERY:
// 		return QString::fromLocal8Bit("底部三角");
// 		break;
// 	case EDOT_HELPER_OBJECT0:
// 		return QString::fromLocal8Bit("辅助体");
// 		break;
// 	case EODT_VEINCONNECT:
// 		return QString::fromLocal8Bit("胆囊连接");
// 		break;
// 	case EODT_VEINBOTTOMCONNECT:
// 		return QString::fromLocal8Bit("胆囊底部连接");
// 		break;
// 	default:
// 		return QString::fromLocal8Bit("未知");
// 		break;
// 	}
// }

uint32 GetARGB(uint32 r , uint32 g , uint32 b , uint32 a)
{
	uint32 argb = 0;
	argb |= a;
	argb = argb << 8;
	argb |= r;
	argb = argb << 8;
	argb |= g;
	argb = argb << 8;
	argb |= b;
	return argb;
}

RbHotchpotchEditorWidget::RbHotchpotchEditorWidget(QWidget *parent /* = NULL */)
:QDialog(parent , Qt::WindowCloseButtonHint | Qt::WindowTitleHint) ,
	m_SelectedOrganID(-1),
	m_VdObjIndex(-1),
	m_CommonStep(0.0f),
	m_CommonPos(Ogre::Vector3::ZERO) ,
	m_StepBasic(0.0f) , 
	m_StepScale(1.0f) ,
	m_pSelectedOrgan(NULL),
	m_Timer(NULL) 
{
//	setAttribute( Qt::WA_DeleteOnClose, true );
// 	setAttribute( Qt::WA_NativeWindow, true );
// 	setAttribute( Qt::WA_StyleSheet, false);
// 	setAttribute( Qt::WA_SetStyle, false);
	ui.setupUi(this);

// 	QWindowsVistaStyle * pstyle = new QWindowsVistaStyle;
// 	pstyle->polish(this);
// 	setStyle(pstyle);
	
	ui.DynObjList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.DynObjList->setAutoFillBackground(true);

	m_DynObjListModel = new QStandardItemModel();

	m_ViewDetectionObjListModel = new QStandardItemModel();

	m_pRegisterdTraining = NULL;
//list
	connect(ui.DynObjList,SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OnDynObjListClicked(const QModelIndex &)));
	connect(ui.ViewDetectionObjList,SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OnViewDetectionObjListClicked(const QModelIndex &)));

//btn
	connect(ui.OpenBtn, SIGNAL(clicked()), this, SLOT(OnOpenEditorBtnClicked()));
	connect(ui.TrainingEditModeBtn, SIGNAL(clicked()), this, SLOT(OnOpenTrainingEditModeBtnClicked()));
	
	//organ
	connect(ui.ExportOrganObjBtn, SIGNAL(clicked()), this, SLOT(OnExportOrganObjBtnClicked()));

	connect(ui.FrameVisibleBtn , SIGNAL(clicked()), this, SLOT(OnFrameVisibleBtnClicked()));


	//vd
	connect(ui.AddVDBtn , SIGNAL(clicked()), this, SLOT(OnAddVdBtnClicked()));

	connect(ui.VDXABtn, SIGNAL(clicked()), this, SLOT(OnChangeVdBtnClicked()));
	connect(ui.VDXMBtn, SIGNAL(clicked()), this, SLOT(OnChangeVdBtnClicked()));
	connect(ui.VDYABtn, SIGNAL(clicked()), this, SLOT(OnChangeVdBtnClicked()));
	connect(ui.VDYMBtn, SIGNAL(clicked()), this, SLOT(OnChangeVdBtnClicked()));
	connect(ui.VDZABtn, SIGNAL(clicked()), this, SLOT(OnChangeVdBtnClicked()));
	connect(ui.VDZMBtn, SIGNAL(clicked()), this, SLOT(OnChangeVdBtnClicked()));
	connect(ui.VDChangeMinCosBtn, SIGNAL(clicked()), this, SLOT(OnChangeVdBtnClicked()));
	connect(ui.VDChangeDistBtn, SIGNAL(clicked()), this, SLOT(OnChangeVdBtnClicked()));
	connect(ui.VDSetPosBtn, SIGNAL(clicked()), this, SLOT(OnSetPosBtnClicked()));

	//doodle
	connect(ui.AddDoodleBtn, SIGNAL(clicked()), this, SLOT(OnAddDoodleBtnClicked()));
	connect(ui.SaveDoodleResultBtn , SIGNAL(clicked()), this, SLOT(OnSaveDoodleBtnClicked()));
	connect(ui.LoadImageIntoDoodleBtn , SIGNAL(clicked()), this, SLOT(OnLoadImageIntoDoodleBtnClicked()));
	connect(ui.ToggleShowEdgeBtn , SIGNAL(clicked()), this, SLOT(OnToggleShowEdgeBtnClicked()));



// 	m_Timer = new QTimer(this);
// 	connect(m_Timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
// 	m_Timer->start(50);


//slider
	connect(ui.StepSlider , SIGNAL(sliderMoved(int)),this,SLOT(OnStepSliderValueChanged(int)));

//lineedit
	connect(ui.StepScaleLineEdit , SIGNAL(textChanged(const QString &)),this,SLOT(OnStepScaleLineEditTextChanged(const QString &)));

	connect(ui.PosXLineEdit, SIGNAL(textChanged(const QString &)),this,SLOT(OnPosLineEditTextChanged(const QString &)));
	connect(ui.PosYLineEdit, SIGNAL(textChanged(const QString &)),this,SLOT(OnPosLineEditTextChanged(const QString &)));
	connect(ui.PosZLineEdit , SIGNAL(textChanged(const QString &)),this,SLOT(OnPosLineEditTextChanged(const QString &)));

	connect(ui.BrushColorRedLE , SIGNAL(textChanged(const QString &)),this,SLOT(OnPosLineEditTextChanged(const QString &)));
	connect(ui.BrushColorGreenLE , SIGNAL(textChanged(const QString &)),this,SLOT(OnPosLineEditTextChanged(const QString &)));
	connect(ui.BrushColorBlueLE , SIGNAL(textChanged(const QString &)),this,SLOT(OnPosLineEditTextChanged(const QString &)));
	connect(ui.BrushColorAlphaLE , SIGNAL(textChanged(const QString &)),this,SLOT(OnPosLineEditTextChanged(const QString &)));
	connect(ui.BrushSizeLe , SIGNAL(textChanged(const QString &)),this,SLOT(OnPosLineEditTextChanged(const QString &)));
	connect(ui.BrushSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnBrushRadiusSliderChanged(int)));


	m_RComponent = ui.BrushColorRedLE->text().toUInt();
	m_GComponent = ui.BrushColorGreenLE->text().toUInt();
	m_BComponent = ui.BrushColorBlueLE->text().toUInt();
	m_AComponent= ui.BrushColorAlphaLE->text().toUInt();
	uint32 argb = GetARGB(m_RComponent , m_GComponent , m_BComponent , m_AComponent);
	m_BrushColor.setAsARGB(argb);

}

RbHotchpotchEditorWidget::~RbHotchpotchEditorWidget()
{
	if(m_DynObjListModel)
	{
		delete m_DynObjListModel;
		m_DynObjListModel = 0 ;
	}
	
	if(m_ViewDetectionObjListModel)
	{
		delete m_ViewDetectionObjListModel;
		m_ViewDetectionObjListModel = 0;
	}

	if(m_pRegisterdTraining)
	{
		CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
		SY_ASSERT(pTraining == m_pRegisterdTraining || pTraining==0);
		if(pTraining)
		{

		}
	}
	
	if(m_Timer)
	{
		m_Timer->stop();
		delete m_Timer;
	}

}

void RbHotchpotchEditorWidget::UpdateDynObjList()
{
	if(m_DynObjListModel == 0)
	{
		m_DynObjListModel = new QStandardItemModel();
	}
	m_DynObjListModel->clear();

	CBasicTraining * pTraining = m_pRegisterdTraining;

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

			m_DynObjListModel->appendRow(item);
		}
	}	
	ui.DynObjList->setModel(m_DynObjListModel);
}

void RbHotchpotchEditorWidget::UpdateViewDetectionObjList()
{
	if(m_ViewDetectionObjListModel == 0)
	{
		m_ViewDetectionObjListModel = new QStandardItemModel();
	}
	m_ViewDetectionObjListModel->clear();

	HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
	if(pEditor)
	{
		std::vector<ViewDetectionForEditor*>  & viewDs = pEditor->m_ViewDetections;

		for(size_t c = 0 ; c < viewDs.size() ; c++)
		{
			ViewDetectionForEditor *pVD = viewDs[c];
			QString strName = QString(pVD->GetName().c_str());
			QStandardItem * pItem = new QStandardItem(strName);
			pItem->setData(QVariant::fromValue(c),Qt::UserRole + 1);
			m_ViewDetectionObjListModel->appendRow(pItem);
		}

		ui.ViewDetectionObjList->setModel(m_ViewDetectionObjListModel);
	}
}


void RbHotchpotchEditorWidget::OnCurrentTrainChanged(CBasicTraining * ptrain)
{
	m_pRegisterdTraining = ptrain;

	UpdateDynObjList();

	UpdateViewDetectionObjList();
}

void RbHotchpotchEditorWidget::OnDynObjListClicked(const QModelIndex &index)
{
	QStandardItem * item = m_DynObjListModel->itemFromIndex(index);

	if (item > 0)
	{
		m_SelectedOrganID = index.data(Qt::UserRole + 1).value<unsigned int>();
		m_pSelectedOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(m_pRegisterdTraining->GetOrgan(m_SelectedOrganID));

		if(m_pSelectedOrgan == NULL)
			return;

		ui.OrganSelectedLabel->setText(QString(m_pSelectedOrgan->getName().c_str()));
	}
}

void RbHotchpotchEditorWidget::OnViewDetectionObjListClicked(const QModelIndex &index)
{
	QStandardItem * item = m_ViewDetectionObjListModel->itemFromIndex(index);

	if (item)
	{
		m_VdObjIndex = index.data(Qt::UserRole + 1).value<unsigned int>();

		ViewDetectionForEditor * pVDObj = HotchpotchEditor::GetCurrentEditor()->m_ViewDetections[m_VdObjIndex];

		const Ogre::Vector3 & pos = pVDObj->GetPosition();
		std::stringstream ss;
		ss << pos.x << "," << pos.y << "," << pos.z ;
		ui.VDPosLabel->setText(QString(ss.str().c_str()));

		float detectDist = pVDObj->GetDetectDist();
		ui.VDDetectDistLabel->setText(QString::number(detectDist));

		float minCos = pVDObj->GetMinCos();
		ui.VDMinCosLabel->setText(QString::number(minCos));

	}
}

void RbHotchpotchEditorWidget::OnFrameVisibleBtnClicked()
{

}

void RbHotchpotchEditorWidget::OnOpenEditorBtnClicked()
{
	HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
	if(!pEditor)
	{
		pEditor = new HotchpotchEditor;
		pEditor->Construct(m_pRegisterdTraining->m_pOms->GetSceneManager() , m_pRegisterdTraining);
	}

	if(pEditor)
	{
		ui.OpenLabel->setText("On");
	}
}

void RbHotchpotchEditorWidget::OnOpenTrainingEditModeBtnClicked()
{
	m_pRegisterdTraining->SetEditorMode(true);

	MouseInput::GetInstance()->SetProcessMouseEvent(true);

	MouseInput::GetInstance()->SetDefaultPosition(Ogre::Vector3(8.0540066f , -4.7864556f , -4.2675395f));

	MouseInput::GetInstance()->SetDefaultOrientation(Ogre::Quaternion(-0.50260752f , 0.82407808f , 0.19477108f , 0  ));
}

void RbHotchpotchEditorWidget::OnExportOrganObjBtnClicked()
{
	if(m_pSelectedOrgan)
	{
		HotchpotchEditor *pEditor = HotchpotchEditor::GetCurrentEditor();
		if(pEditor)
		{
			if(pEditor->ExportOrganToObjFile(m_pSelectedOrgan))
				ShowDebugInfo("export finish./n");
			else
				ShowDebugInfo("export failed./n");
		}
	}
}

void RbHotchpotchEditorWidget::OnAddVdBtnClicked()
{
	HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
	if(pEditor)
	{
		pEditor->AddViewDetectionObject();

		UpdateViewDetectionObjList();
	}
}

void RbHotchpotchEditorWidget::OnChangeVdBtnClicked()
{
	QPushButton *pt = qobject_cast<QPushButton*>(sender());
	if(!pt)
		return;

	if(m_VdObjIndex < 0)
		return;

	ViewDetectionForEditor * pVDObj = HotchpotchEditor::GetCurrentEditor()->m_ViewDetections[m_VdObjIndex];

	if(pt == ui.VDXABtn)
	{
		pVDObj->Translate(Ogre::Vector3(m_CommonStep , 0 , 0));
	}
	else if(pt == ui.VDXMBtn)
	{
		pVDObj->Translate(Ogre::Vector3(-m_CommonStep , 0 , 0));
	}
	else if(pt == ui.VDYABtn)
	{
		pVDObj->Translate(Ogre::Vector3(0 , m_CommonStep, 0));
	}
	else if(pt == ui.VDYMBtn)
	{
		pVDObj->Translate(Ogre::Vector3(0 , -m_CommonStep , 0));
	}
	else if(pt == ui.VDZABtn)
	{
		pVDObj->Translate(Ogre::Vector3(0 , 0 , m_CommonStep));
	}
	else if(pt == ui.VDZMBtn)
	{
		pVDObj->Translate(Ogre::Vector3(0 , 0 , -m_CommonStep));
	}
	else if(pt == ui.VDChangeMinCosBtn)
	{
		pVDObj->SetMinCos(m_CommonStep);
	}
	else if(pt == ui.VDChangeDistBtn)
	{
		pVDObj->DetectDist(true);
		pVDObj->SetDetectDist(m_CommonStep);
	}
	
	const Ogre::Vector3 & pos = pVDObj->GetPosition();
	
	std::stringstream ss;
	ss << pos.x << "," << pos.y << "," << pos.z ;
	ui.VDPosLabel->setText(QString(ss.str().c_str()));

	float detectDist = pVDObj->GetDetectDist();
	ui.VDDetectDistLabel->setText(QString::number(detectDist));

	float minCos = pVDObj->GetMinCos();
	ui.VDMinCosLabel->setText(QString::number(minCos));
}

void RbHotchpotchEditorWidget::OnSetPosBtnClicked()
{
	if(m_VdObjIndex < 0)
		return;

	ViewDetectionForEditor * pVDObj = HotchpotchEditor::GetCurrentEditor()->m_ViewDetections[m_VdObjIndex];
	
	pVDObj->SetPosition(m_CommonPos);

	const Ogre::Vector3 & pos = pVDObj->GetPosition();

	std::stringstream ss;
	ss << pos.x << "," << pos.y << "," << pos.z ;
	ui.VDPosLabel->setText(QString(ss.str().c_str()));
	
}

void RbHotchpotchEditorWidget::OnAddDoodleBtnClicked()
{
	HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
	if(!pEditor)
		return;

	if(m_pSelectedOrgan)
	{
		pEditor->AddOrganPainter(m_pSelectedOrgan);
	}
}

void RbHotchpotchEditorWidget::OnSaveDoodleBtnClicked()
{
	HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
	if(!pEditor)
		return;

	pEditor->SaveOrganPainterResult();

}

void RbHotchpotchEditorWidget::OnLoadImageIntoDoodleBtnClicked()
{
	HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
	if(!pEditor)
		return;
	Ogre::String imageName("OrganMarkTemp.png");
	pEditor->LoadImageIntoOrganPainter(imageName);

}


void RbHotchpotchEditorWidget::OnToggleShowEdgeBtnClicked()
{
	HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
	if(!pEditor)
		return;

	pEditor->OrganPainterToggleShowEdge();

}


void RbHotchpotchEditorWidget::OnStepSliderValueChanged(int value)
{
	m_StepBasic = GetSliderValue(ui.StepSlider , value);
	m_CommonStep = m_StepBasic * m_StepScale;
	ui.StepLabel->setText(QString::number(m_CommonStep));
}

void RbHotchpotchEditorWidget::OnStepScaleLineEditTextChanged(const QString &text)
{
	bool ok;
	m_StepScale= ui.StepScaleLineEdit->text().toFloat(&ok);
	if(ok)
		m_CommonStep = m_StepBasic * m_StepScale;
	else
		m_CommonStep = m_StepBasic;
	ui.StepLabel->setText(QString::number(m_CommonStep));
}

void RbHotchpotchEditorWidget::OnPosLineEditTextChanged(const QString &text)
{
	QLineEdit *pLE = qobject_cast<QLineEdit*>(sender());
	if(!pLE)
		return;

	bool ok;
	if(pLE == ui.PosXLineEdit)
	{
		m_CommonPos.x = pLE->text().toFloat(&ok);
	}
	else if(pLE == ui.PosYLineEdit)
	{
		m_CommonPos.y = pLE->text().toFloat(&ok);
	}
	else if(pLE == ui.PosZLineEdit)
	{
		m_CommonPos.z = pLE->text().toFloat(&ok);
	}
	else if(pLE == ui.BrushColorRedLE)
	{
		m_RComponent = ui.BrushColorRedLE->text().toUInt(&ok);
		uint32 argb = GetARGB(m_RComponent , m_GComponent , m_BComponent , m_AComponent);
		m_BrushColor.setAsARGB(argb);

		HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
		if(pEditor)
			pEditor->SetOrganPainterBrushColor(m_BrushColor);
	}
	else if(pLE == ui.BrushColorGreenLE)
	{
		m_GComponent = ui.BrushColorGreenLE->text().toUInt(&ok);
		uint32 argb = GetARGB(m_RComponent , m_GComponent , m_BComponent , m_AComponent);
		m_BrushColor.setAsARGB(argb);

		HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
		if(pEditor)
			pEditor->SetOrganPainterBrushColor(m_BrushColor);
	}
	else if(pLE == ui.BrushColorBlueLE)
	{
		m_BComponent = ui.BrushColorBlueLE->text().toUInt(&ok);
		uint32 argb = GetARGB(m_RComponent , m_GComponent , m_BComponent , m_AComponent);
		m_BrushColor.setAsARGB(argb);

		HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
		if(pEditor)
			pEditor->SetOrganPainterBrushColor(m_BrushColor);
	}
	else if(pLE == ui.BrushColorAlphaLE)
	{
		m_AComponent= ui.BrushColorAlphaLE->text().toUInt(&ok);
		uint32 argb = GetARGB(m_RComponent , m_GComponent , m_BComponent , m_AComponent);
		m_BrushColor.setAsARGB(argb);

		HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
		if(pEditor)
			pEditor->SetOrganPainterBrushColor(m_BrushColor);
	}
	else if(pLE == ui.BrushSizeLe)
	{
		float size = pLE->text().toFloat(&ok);
		if (size > 0.05f)
			size = 0.05f;
		float percent = size / 0.05f;
		ui.BrushSizeSlider->setValue(percent * 100);

		HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
		if(pEditor)
			pEditor->SetOrganPainterBrushSize(size);
	}
}

void RbHotchpotchEditorWidget::OnBrushRadiusSliderChanged(int value)
{
	float percent = value / 100.0f;
	float brushsize = percent * 0.05f;
	QString text = QString::number(brushsize);
	ui.BrushSizeLe->setText(text);
	HotchpotchEditor * pEditor = HotchpotchEditor::GetCurrentEditor();
	if (pEditor)
		pEditor->SetOrganPainterBrushSize(brushsize);
}
void RbHotchpotchEditorWidget::timerUpdate()
{
}

void RbHotchpotchEditorWidget::ShowDebugInfo(const QString& str)
{
	ui.InfoText->append(str);	
}
