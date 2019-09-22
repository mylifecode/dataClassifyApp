#include "Mxdefine.h"
#include "RbPointToolWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QTextedit>
#include "MXApplication.h"

#include "../MXRobot/Common/LightMgr.h"
#include "../MXRobot/Include/BasicTraining.h"
#include "../MXRobot/Include/stdafx.h"
#include "../MXRobot/Include/PathputTool.h"
#include "MouseInput.h"
#include "InputSystem.h"

#include "TrainingMgr.h"
#include "../MXRobot/NewTrain/MisMedicOrganOrdinary.h"
#include "../MXRobot/NewTrain/MisNewTraining.h"
#include "EditorTool/EditCamera.h"

extern EditorCamera* gEditCamera;

#define SLIDER_INIT_SET(slider, spinBox, min, max, value, signalStep) 	slider->setMinimum(min);  slider->setMaximum(max);   slider->setValue(value*100);\
    slider->setSingleStep(signalStep); spinBox->setText(QString::number(value));

#define SLIDER_INIT_SET1(slider, spinBox, min, max, value, signalStep) 	slider->setMinimum(min);  slider->setMaximum(max);   slider->setValue(value*10000);\
	slider->setSingleStep(signalStep); spinBox->setText(QString::number(value));

#define LIMIT_ZERO_RANGE(f) f = (f < 0.00001 && f > -0.00001) ? 0.0 : f;

class  ObjListChangeListener: public ObjectLoadReleaseListener
{
public:
	void onDynamicObjectChanged();
	RbPointToolWidget * m_parent;
};

class ToolEventListenr: public PathPutToolEventListener
{
public:
	ToolEventListenr(RbPointToolWidget * parent):m_parent(parent){}
	~ToolEventListenr(){}
	virtual void OnConnectPairChanged();
	virtual void OnConnectLoaded();
	RbPointToolWidget * m_parent;
};

void ObjListChangeListener::onDynamicObjectChanged()
{
	m_parent->updateObjectList();
}

void ToolEventListenr::OnConnectPairChanged()
{
	m_parent->updateConnectionList();
}

void ToolEventListenr::OnConnectLoaded()
{
	m_parent->updateConnectionList();
}
QString GetOrganNameByType(TEDOT type)
{
    switch (type)
    {
    case EDOT_NO_TYPE:
        return QString::fromLocal8Bit("Î´Öª");
        break;
    case EDOT_MESOCOLON:
        return QString::fromLocal8Bit("´óÍøÄ¤");
        break;
    case EDOT_SPLEEN:
        return QString::fromLocal8Bit("Æ¢Ôà");
        break;
    case EDOT_GEROTAS:
        return QString::fromLocal8Bit("ÉöÖÜ");
        break;
    case EDOT_KIDNEY_VESSELS:
        return QString::fromLocal8Bit("Éö¼°¸½Êô");
        break;
    case EDOT_KIDNEYCONNECT_STRONG:
        return QString::fromLocal8Bit("Éö½îÄ¤-Ç¿");
        break;
    case EDOT_KIDNEYCONNECT_WEAK:
        return QString::fromLocal8Bit("Éö½îÄ¤-Èõ");
        break;
    case EDOT_BRAVERY_ARTERY:
        return QString::fromLocal8Bit("µ¨ÄÒ¶¯Âö");
        break;
    case EDOT_COMMON_BILE_DUCT:
        return QString::fromLocal8Bit("¸Î×Ü¹Ü");
        break;
    case EDOT_CYSTIC_DUCT:
        return QString::fromLocal8Bit("µ¨ÄÒ¹Ü");
        break;
    case EDOT_GALLBLADDER:
        return QString::fromLocal8Bit("µ¨ÄÒ");
        break;
    case EDOT_HEPATIC_ARTERY:
        return QString::fromLocal8Bit("¸Î¶¯Âö");
        break;
    case EDOT_LIVER:
        return QString::fromLocal8Bit("¸Î");
        break;
    case EDOT_VEIN:
        return QString::fromLocal8Bit("µ¨ÄÒÈý½Ç");
        break;
    case EDOT_RENAL_ARTERY:
        return QString::fromLocal8Bit("µ×²¿Èý½Ç");
        break;
    case EDOT_HELPER_OBJECT0:
        return QString::fromLocal8Bit("¸¨ÖúÌå");
        break;
    case EODT_VEINCONNECT:
        return QString::fromLocal8Bit("µ¨ÄÒÈý½ÇÁ¬½Ó");
        break;
    case EODT_VEINBOTTOMCONNECT:
        return QString::fromLocal8Bit("µ¨ÄÒ´²Á¬½Ó");
        break;
    case EDOT_IMA:
        return QString::fromLocal8Bit("³¦ÏµÄ¤ÏÂ¶¯Âö");
        break;
    case EDOT_IMV:
        return QString::fromLocal8Bit("³¦ÏµÄ¤ÏÂ¾²Âö");
        break;
    case EODT_URETER:
        return QString::fromLocal8Bit("ÊäÄò¹Ü");
        break;
    case EDOT_GONADAL:
        return QString::fromLocal8Bit("ÐÔÏÙ");
        break;
    case EDOT_SIGMOIDCUTPART:
        return QString::fromLocal8Bit("ÇÐ¸î²¿·Ö");
        break;
    case EDOT_SIGMOIDNOCUTPART:
        return QString::fromLocal8Bit("Î´ÇÐ¸î²¿·Ö");
        break;
    case EODT_DOME_ACTIVE:
        return QString::fromLocal8Bit("¸¹±Ú");
        break;
    case EDOT_SIGMOIDCONNECT:
        return QString::fromLocal8Bit("ÒÒ×´½á³¦Á¬½Ó");
        break;
    default:
        return QString::fromLocal8Bit("Î´Öª");
        break;
    }
}
RbPointToolWidget::RbPointToolWidget(QWidget *parent /* = NULL */)
:QDialog(parent , Qt::WindowCloseButtonHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);

    m_pCamera = NULL;

	m_bg = new QFrame(this);
	m_bg->setObjectName("RbAboutBackgroundFrame");
	m_bg->setWindowFlags(Qt::Widget | Qt::WindowStaysOnBottomHint);	

	ui.objectlist->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.objectlist->setAutoFillBackground(true);

	//grabKeyboard();
	//m_dynobjsel = 0;
	m_objlistmodel = 0;
	
	m_pathlistmodel = new QStandardItemModel();
	//ui.pointpathlist->setModel(m_pathlistmodel);

	m_connectlistmodel = new QStandardItemModel();

	m_registerdtrain = 0;

    setWindowTitle(QString::fromLocal8Bit("²ÎÊýÉèÖÃ"));
    ui.tabWidget->setTabText(0,QString::fromLocal8Bit("Èí×éÖ¯"));
    ui.tabWidget->setTabText(1,QString::fromLocal8Bit("µÆ¹â"));
    ui.tabWidget->setTabText(2,QString::fromLocal8Bit("Ïà»ú"));
	
	connect(ui.autobuildconnect, SIGNAL(clicked()), this, SLOT(on_autoRebuildConnect()));

    connect(ui.AutoGenConnect, SIGNAL(clicked()), this, SLOT(on_AutoGenConnect()));

	connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(on_saveClicked()));

	connect(ui.editselconnectbt, SIGNAL(clicked()), this, SLOT(on_EditSelButtonClicked()));

	connect(ui.saveveinconnect, SIGNAL(clicked()), this, SLOT(seralizestrip()));

	connect(ui.savetofiletexmap, SIGNAL(clicked()), this, SLOT(genTexmap()));
	
	connect(ui.delselconnect, SIGNAL(clicked()), this, SLOT(on_delselconnect()));
	
	//connect(ui.refreshpath, SIGNAL(clicked()), this, SLOT(on_refreshpathClicked()));

	//connect(ui.deletepath, SIGNAL(clicked()), this, SLOT(on_deletepathClicked()));
	
	connect(ui.objectlist,SIGNAL( doubleClicked(const QModelIndex &)),this,SLOT(Objlistclicked(const QModelIndex &)));

	//connect(ui.pointpathlist,SIGNAL( clicked(const QModelIndex &)),this,SLOT(PointPathlistclicked(const QModelIndex &)));

	connect(ui.connectlist,SIGNAL( clicked(const QModelIndex &)),this,SLOT(ConnectListclicked(const QModelIndex &)));

	connect(ui.reset , SIGNAL( clicked()),this,SLOT(reset()));

	//connect(ui.putdensity , SIGNAL( textChanged( const QString &)),this,SLOT(on_putdensityTextChanged(const QString &)));

	connect(ui.putdensityslider,SIGNAL( sliderMoved(int)),this,SLOT(on_valueChanged(int)));

	connect(ui.adhheightslider , SIGNAL( sliderMoved(int)),this,SLOT(on_changeAdheHeight(int)));

	connect(ui.adhheightslidera , SIGNAL( sliderMoved(int)),this,SLOT(on_changeAdheHeightA(int)));

	connect(ui.adhheightsliderb , SIGNAL( sliderMoved(int)),this,SLOT(on_changeAdheHeightB(int)));

	connect(ui.cboxhidsel , SIGNAL( stateChanged(int)),this,SLOT(cb_selstateChanged(int)));

	connect(ui.cboxhidunsel , SIGNAL( stateChanged(int)),this,SLOT(cb_unselstateChanged(int)));

//yincg add light setting
	connect(ui.horizontalSlider_LightPos_X, SIGNAL(valueChanged(int)), this, SLOT(onSetPosX(int))); 
 	connect(ui.horizontalSlider_LightPos_Y, SIGNAL(valueChanged(int)), this, SLOT(onSetPosY(int))); 
 	connect(ui.horizontalSlider_LightPos_Z, SIGNAL(valueChanged(int)), this, SLOT(onSetPosZ(int))); 
 
 	connect(ui.horizontalSlider_LightDir_X, SIGNAL(valueChanged(int)), this, SLOT(onSetDirX(int)));  
 	connect(ui.horizontalSlider_LightDir_Y, SIGNAL(valueChanged(int)), this, SLOT(onSetDirY(int)));  
 	connect(ui.horizontalSlider_LightDir_Z, SIGNAL(valueChanged(int)), this, SLOT(onSetDirZ(int))); 
 
 	connect(ui.horizontalSlider_Diffuse_R, SIGNAL(valueChanged(int)), this, SLOT(onSetDiffuseR(int))); 
 	connect(ui.horizontalSlider_Diffuse_G, SIGNAL(valueChanged(int)), this, SLOT(onSetDiffuseG(int))); 
 	connect(ui.horizontalSlider_Diffuse_B, SIGNAL(valueChanged(int)), this, SLOT(onSetDiffuseB(int))); 
 	
 	connect(ui.horizontalSlider_Specular_R, SIGNAL(valueChanged(int)), this, SLOT(onSetSpecularR(int))); 
 	connect(ui.horizontalSlider_Specular_G, SIGNAL(valueChanged(int)), this, SLOT(onSetSpecularG(int)));
 	connect(ui.horizontalSlider_Specular_B, SIGNAL(valueChanged(int)), this, SLOT(onSetSpecularB(int)));

	connect(ui.horizontalSlider_Atte_X, SIGNAL(valueChanged(int)), this, SLOT(onSetAtteRange(int)));
	connect(ui.horizontalSlider_Atte_Y, SIGNAL(valueChanged(int)), this, SLOT(onSetAtteConstant(int)));
	connect(ui.horizontalSlider_Atte_Z, SIGNAL(valueChanged(int)), this, SLOT(onSetAtteLinear(int)));
	connect(ui.horizontalSlider_Atte_W, SIGNAL(valueChanged(int)), this, SLOT(onSetAttequadraticui(int)));

	connect(ui.horizontalSlider_InnerAngle, SIGNAL(valueChanged(int)), this, SLOT(onSetInnerAngle(int)));
	connect(ui.horizontalSlider_OuterAngle, SIGNAL(valueChanged(int)), this, SLOT(onSetOuterAngle(int)));
	connect(ui.horizontalSlider_FallOff, SIGNAL(valueChanged(int)), this, SLOT(onSetFallOff(int)));

	connect(ui.horizontalSlider_Ambient_R, SIGNAL(valueChanged(int)), this, SLOT(onSetAmbientR(int)));
	connect(ui.horizontalSlider_Ambient_G, SIGNAL(valueChanged(int)), this, SLOT(onSetAmbientG(int)));
	connect(ui.horizontalSlider_Ambient_B, SIGNAL(valueChanged(int)), this, SLOT(onSetAmbientB(int)));

	connect(ui.comboBox_LightType, SIGNAL(currentIndexChanged(int)), this, SLOT(onLightTypeChanged(int)));

	connect(ui.Execute, SIGNAL(clicked()), this, SLOT(SetLight()));

//camera setting
    connect(ui.hor_cam_pos_x, SIGNAL(valueChanged(int)), this, SLOT(onSetCamPosX(int)));
    connect(ui.hor_cam_pos_y, SIGNAL(valueChanged(int)), this, SLOT(onSetCamPosY(int)));
    connect(ui.hor_cam_pos_z, SIGNAL(valueChanged(int)), this, SLOT(onSetCamPosZ(int)));

    connect(ui.hor_cam_dir_x, SIGNAL(valueChanged(int)), this, SLOT(onSetCamDirX(int)));
    connect(ui.hor_cam_dir_y, SIGNAL(valueChanged(int)), this, SLOT(onSetCamDirY(int)));
    connect(ui.hor_cam_dir_z, SIGNAL(valueChanged(int)), this, SLOT(onSetCamDirZ(int)));

    connect(ui.hor_cam_fovy, SIGNAL(valueChanged(int)), this, SLOT(onSetFovy(int)));

    connect(ui.pushButton_3, SIGNAL(clicked()), this, SLOT(onRestoreButtonClicked()));

    connect(ui.hor_cam_ortho_w, SIGNAL(valueChanged(int)), this, SLOT(onSetOrthoWindow_w(int)));
    connect(ui.hor_cam_ortho_h, SIGNAL(valueChanged(int)), this, SLOT(onSetOrthoWindow_h(int)));

    connect(ui.hor_cam_asp, SIGNAL(valueChanged(int)), this, SLOT(onSetAspectRatio(int)));
	
	connect(ui.le_cam_pos_x, SIGNAL(editingFinished()), this, SLOT(onEditCamPosX()));
	
	m_inreadlight = false;

	m_listener = new ObjListChangeListener();
	((ObjListChangeListener*)m_listener)->m_parent = this;
	//updateObjectList();
	m_tooleventlistener = new ToolEventListenr(this);
}

void RbPointToolWidget::onSetPosX(int value){ui.lineEdit_pos_x->setText(QString::number((float)value/10.0f));}
void RbPointToolWidget::onSetPosY(int value){ui.lineEdit_pos_y->setText(QString::number((float)value/10.0f));}
void RbPointToolWidget::onSetPosZ(int value){ui.lineEdit_pos_z->setText(QString::number((float)value/10.0f));}
void RbPointToolWidget::onSetDirX(int value){ui.lineEdit_dir_x->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetDirY(int value){ui.lineEdit_dir_y->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetDirZ(int value){ui.lineEdit_dir_z->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetDiffuseR(int value){ui.lineEdit_diff_r->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetDiffuseG(int value){ui.lineEdit_diff_g->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetDiffuseB(int value){ui.lineEdit_diff_b->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetSpecularR(int value){ui.lineEdit_spe_r->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetSpecularG(int value){ui.lineEdit_spe_g->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetSpecularB(int value){ui.lineEdit_spe_b->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetAtteRange(int value){ui.lineEdit_Atte_X->setText(QString::number((float)value));}
void RbPointToolWidget::onSetAtteConstant(int value){ui.lineEdit_Atte_Y->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetAtteLinear(int value){ui.lineEdit_Atte_Z->setText(QString::number((float)value/10000.0f));}
void RbPointToolWidget::onSetAttequadraticui(int value){ui.lineEdit_Atte_W->setText(QString::number((float)value/10000.0f));}
void RbPointToolWidget::onSetInnerAngle(int value){ui.lineEdit_InnerAngle->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetOuterAngle(int value){ui.lineEdit_OuterAngle->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetFallOff(int value){ui.lineEdit_FallOff->setText(QString::number((float)value/100.0f));}

void RbPointToolWidget::onSetAmbientR(int value){ui.lineEdit_Ambient_R->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetAmbientG(int value){ui.lineEdit_Ambient_G->setText(QString::number((float)value/100.0f));}
void RbPointToolWidget::onSetAmbientB(int value){ui.lineEdit_Ambient_B->setText(QString::number((float)value/100.0f));}

//camera pos
void RbPointToolWidget::onSetCamPosX(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_pos_x->setText(QString::number((float)value/100.0f));
    Ogre::Vector3 currPos = m_pCamera->getParentNode()->getPosition();
	Ogre::Quaternion currOri = m_pCamera->getParentNode()->getOrientation();
	Ogre::Vector3 temp_currPos((float)value/100.0f, currPos.y, currPos.z);

	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(temp_currPos);
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(currOri);
}

void RbPointToolWidget::onEditCamPosX()
{
	if(!m_pCamera) return; 
	Ogre::Vector3 currPos = m_pCamera->getParentNode()->getPosition();
	Ogre::Quaternion currOri = m_pCamera->getParentNode()->getOrientation();
	float currPosX =(float)ui.le_cam_pos_x->text().toInt();
	Ogre::Vector3 temp_currPos(currPosX, currPos.y, currPos.z);

	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(temp_currPos);
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(currOri);
}

void RbPointToolWidget::onSetCamPosY(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_pos_y->setText(QString::number((float)value/100.0f));
	Ogre::Vector3 currPos = m_pCamera->getParentNode()->getPosition();
	Ogre::Quaternion currOri = m_pCamera->getParentNode()->getOrientation();
	Ogre::Vector3 temp_currPos(currPos.x, (float)value/100.0f,  currPos.z);

	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(temp_currPos);
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(currOri);
}
void RbPointToolWidget::onSetCamPosZ(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_pos_z->setText(QString::number((float)value/100.0f));
	Ogre::Vector3 currPos = m_pCamera->getParentNode()->getPosition();
	Ogre::Quaternion currOri = m_pCamera->getParentNode()->getOrientation();
	Ogre::Vector3 temp_currPos(currPos.x, currPos.y, (float)value/100.0f);

	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(temp_currPos);
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(currOri);
}

//camera dir
void RbPointToolWidget::onSetCamDirX(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_dir_x->setText(QString::number((float)value/10000.0f));
	Ogre::Vector3 currPos = m_pCamera->getParentNode()->getPosition();
	Ogre::Quaternion currOri = m_pCamera->getParentNode()->getOrientation();
	Ogre::Quaternion temp_currOri(currOri.w, (float)value/10000.0f, currOri.y, currOri.z);

	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(currPos);
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(temp_currOri);
}
void RbPointToolWidget::onSetCamDirY(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_dir_y->setText(QString::number((float)value/10000.0f));
	Ogre::Vector3 currPos = m_pCamera->getParentNode()->getPosition();
	Ogre::Quaternion currOri = m_pCamera->getParentNode()->getOrientation();
	Ogre::Quaternion temp_currOri(currOri.w, currOri.x, (float)value/10000.0f, currOri.z);

	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(currPos);
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(temp_currOri);
}
void RbPointToolWidget::onSetCamDirZ(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_dir_z->setText(QString::number((float)value/10000.0f));
	Ogre::Vector3 currPos = m_pCamera->getParentNode()->getPosition();
	Ogre::Quaternion currOri = m_pCamera->getParentNode()->getOrientation();
	Ogre::Quaternion temp_currOri(currOri.w, currOri.x, currOri.y, (float)value/10000.0f);

	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(currPos);
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(temp_currOri);
}

void RbPointToolWidget::onSetFovy(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_fovy->setText(QString::number((float)value/100.0f));
    m_pCamera->setFOVy(Ogre::Radian(Ogre::Degree((float)value/100.0f)));
}

void RbPointToolWidget::onSetOrthoWindow_w(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_ortho_w->setText(QString::number((float)value));
    m_pCamera->setOrthoWindowWidth((float)value);
}
void RbPointToolWidget::onSetOrthoWindow_h(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_ortho_h->setText(QString::number((float)value));
    m_pCamera->setOrthoWindowHeight((float)value);
}

void RbPointToolWidget::onSetAspectRatio(int value)
{
    if(!m_pCamera) return; 
    ui.le_cam_asp->setText(QString::number((float)value / 100.0f));
    m_pCamera->setAspectRatio((float)value / 100.0f);
}



void RbPointToolWidget::onLightTypeChanged(int value)
{
	if(ui.comboBox_LightType->currentIndex() != Ogre::Light::LightTypes::LT_SPOTLIGHT)
	{
		ui.horizontalSlider_InnerAngle->setEnabled(false);
		ui.lineEdit_InnerAngle->setEnabled(false);
		ui.horizontalSlider_OuterAngle->setEnabled(false);
		ui.lineEdit_OuterAngle->setEnabled(false);
		ui.horizontalSlider_FallOff->setEnabled(false);
		ui.lineEdit_FallOff->setEnabled(false);
	}
	else
	{
		ui.horizontalSlider_InnerAngle->setEnabled(true);
		ui.lineEdit_InnerAngle->setEnabled(true);
		ui.horizontalSlider_OuterAngle->setEnabled(true);
		ui.lineEdit_OuterAngle->setEnabled(true);
		ui.horizontalSlider_FallOff->setEnabled(true);
		ui.lineEdit_FallOff->setEnabled(true);
	}
}


RbPointToolWidget::~RbPointToolWidget()
{
	if( m_objlistmodel )
	{
		delete m_objlistmodel;
		m_objlistmodel = 0;
	}	

	if( m_pathlistmodel )
	{
		delete m_pathlistmodel;
		m_pathlistmodel = 0;
	}	

	if( m_connectlistmodel )
	{
		delete m_connectlistmodel;
		m_connectlistmodel = 0;
	}	

	if( m_registerdtrain )
	{
		CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();

		SY_ASSERT(pTraining == m_registerdtrain || pTraining==0);
		if(pTraining)
		{
			pTraining->removeDynObjLoadReleaseListener(m_listener);
			pTraining = 0;
		}
	}

	if( m_listener )
	{
		delete m_listener;
		m_listener = 0;
	}

	if( m_tooleventlistener )
	{
		//PathPutTool::GetCurrentTool()->RemoveEventListener(m_tooleventlistener);
		delete m_tooleventlistener;
		m_tooleventlistener = 0;
	}
}
void RbPointToolWidget::on_autoRebuildConnect()
{
	ITraining * itrain = CTrainingMgr::Instance()->GetCurTraining();

	MisNewTraining * misnewtrain = dynamic_cast<MisNewTraining*>(itrain);

	DynObjMap objmap = misnewtrain->GetOrganObjectMap();

	PathPutTool::GetCurrentTool()->AutoDetectConnectAdhersion("veinnormal_test.sdf" , objmap);
}
void RbPointToolWidget::on_saveClicked()
{
#if(0)
		QString textstr = ui.connectpatha->text();
		int pathida = textstr.toInt();

		textstr = ui.connectpathb->text();
		int pathidb = textstr.toInt();

		PathPointsInOrgan * patha = PathPutTool::GetCurrentTool()->findPathById(pathida);
		PathPointsInOrgan * pathb = PathPutTool::GetCurrentTool()->findPathById(pathidb);

		if(patha != 0 && pathb !=0 && patha != pathb)
		{
			CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();

			CGallbladderTraining * gallbatrain = dynamic_cast<CGallbladderTraining*>(pTraining);

			if(gallbatrain){
					
					gallbatrain->m_dynamicstrips->AddOneConnection(*patha , *pathb);
					
					QString strshow = "PathA: " + QString::number(pathida) + "  PathB: " + QString::number(pathidb);

					QStandardItem* item = new QStandardItem(strshow);

					item->setData(QVariant::fromValue(pathida),Qt::UserRole + 1);

					item->setData(QVariant::fromValue(pathidb),Qt::UserRole + 2);

					m_connectlistmodel->appendRow(item);

					ui.connectlist->setModel(m_connectlistmodel);
			}
		}
#else
		CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();

		if(pTraining)
		{
			pTraining->SetEditorMode(true);

			
			MouseInput::GetInstance()->SetProcessMouseEvent(true);
			//MouseInput::GetInstance()->SetDefaultPosition(Ogre::Vector3(8.0540066f , -4.7864556f , -4.2675395f));

			//MouseInput::GetInstance()->SetDefaultOrientation(Ogre::Quaternion(-0.50260752f , 0.82407808f , 0.19477108f , 0  ));

		}
	
		/*CGallbladderTraining * gallbatrain = dynamic_cast<CGallbladderTraining*>(pTraining);

		if(gallbatrain)
		{
				std::vector<StripInfo> strips;
				std::vector<PathPointsInOrgan*> pathinadd  = PathPutTool::GetCurrentTool()->findAllPathsInOrgan(EDOT_GALLBLADDER);
				std::vector<PathPointsInOrgan*> pathinliver  = PathPutTool::GetCurrentTool()->findAllPathsInOrgan(EDOT_HELPER_OBJECT0);
				if(pathinadd.size() > 0 && pathinliver.size() > 0)
				{
					gallbatrain->m_dynamicstrips->AddOneConnection(*pathinadd[pathinadd.size()-1] , *pathinliver[pathinliver.size()-1]);
				}
		}*/
#endif
}

void RbPointToolWidget::on_EditSelButtonClicked()
{
		

}
void RbPointToolWidget::genTexmap()
{
	PathPutTool::GetCurrentTool()->mapobjfiletoConnection("c:\\veinconnect\\veinconnect.map" , "c:\\veinconnect\\veinconnectrefined.obj" , "c:\\veinconnect\\veinconnect.ctmap");
}
void RbPointToolWidget::seralizestrip()
{
#if(0)
		CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();

		CGallbladderTraining * gallbatrain = dynamic_cast<CGallbladderTraining*>(pTraining);

		if(gallbatrain)
		{
				DynamicObjUtility::serializeDynamicStripObject(gallbatrain->m_dynamicstrips , "c:\\teststrip.sdf");
		}
#else
	   Ogre::String savedfile = "c:\\veinconnect\\teststrip.sdf";

	   if(PathPutTool::GetCurrentTool())
		  PathPutTool::GetCurrentTool()->serialize(savedfile.c_str());
	
		QMessageBox::question(NULL,"save succeed", QString(savedfile.c_str()), QMessageBox::Yes, QMessageBox::Yes);  
#endif
}

void RbPointToolWidget::reset()
{
	PathPutTool::GetCurrentTool()->Reset();
}
void RbPointToolWidget::on_refreshpathClicked()
{
		/*m_pathlistmodel->clear();
		std::vector<PathPointsInOrgan*> & allpath = PathPutTool::GetCurrentTool()-> m_pathpoints;
		
		for(size_t p = 0 ; p < allpath.size() ; p++)
		{
				PathPointsInOrgan * organpath = allpath[p];
				int pahtid = organpath->m_pathid;
				
				QString strshow = "Path Id = " + QString::number(pahtid);
				
				QStandardItem* item = new QStandardItem(strshow);
				
				item->setData(QVariant::fromValue(pahtid),Qt::UserRole + 1);
				
				m_pathlistmodel->appendRow(item);*/
		//}

		///ui.pointpathlist->setModel(m_pathlistmodel);
}

void RbPointToolWidget::on_deletepathClicked()
{
		/*QModelIndexList indexlist = ui.pointpathlist->selectionModel()->selectedIndexes();

		for (int i = 0 ; i < indexlist.size() ; i++)
		{
				QModelIndex & index = indexlist[i];

				int pathid = index.data(Qt::UserRole + 1).value<int>();
					
				if (pathid > 0)
				{
						CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();


						CGallbladderTraining * gallbatrain = dynamic_cast<CGallbladderTraining*>(pTraining);

						if(gallbatrain)
						{
								gallbatrain->m_dynamicstrips->DeleteOneOgranPath(pathid);

						}
				}
		}*/
				
}
void RbPointToolWidget::onCurrentTrainChanged(CBasicTraining * ptrain)
{
	m_registerdtrain = ptrain;
	if(m_registerdtrain == nullptr)
		return;

	if (PathPutTool::GetCurrentTool() == 0)
	{
		PathPutTool * pathtool = new PathPutTool();
		pathtool->Construct(m_registerdtrain->m_pOms->GetSceneManager(), m_registerdtrain);
	}

	ptrain->addDynObjLoadReleaseListener(m_listener);
	updateObjectList();

	PathPutTool::GetCurrentTool()->AddEventListener(m_tooleventlistener);

	InitLightSet();
    InitCameraSet();
	updateConnectionList();
}

void RbPointToolWidget::keyPressEvent(QKeyEvent *event)
{

}
void RbPointToolWidget::onDynamicObjectChanged()
{
	updateObjectList();
}
void RbPointToolWidget::updateObjectList()
{
	if(m_objlistmodel == 0)
	{
		m_objlistmodel = new QStandardItemModel();
	}

	m_objlistmodel->clear();
	
	CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();

	if(pTraining)
	{
		std::vector<MisMedicOrganInterface*> OrgansInTrain;
		
		pTraining->GetAllOrgan(OrgansInTrain);

		for(size_t c = 0 ; c < OrgansInTrain.size() ; c++)
		{
			int OrganId = OrgansInTrain[c]->m_OrganID;
			QString objname = GetOrganNameByType((TEDOT)OrganId);

			QStandardItem * item = new QStandardItem(objname);
	
			item->setData(QVariant::fromValue(OrganId),Qt::UserRole + 1);
			
			m_objlistmodel->appendRow(item);
		}
	}	
	ui.objectlist->setModel(m_objlistmodel);
}

void RbPointToolWidget::updateConnectionList()
{
	std::vector<int> & clusters = PathPutTool::GetCurrentTool()->m_ConnectPairs;
		
	m_connectlistmodel->clear();

	int paircount = 0;
		
	for (size_t i = 0 ; i < clusters.size() ; i++)
	{	
		int connectid = (int)clusters[i];
				
		QString strshow = "Connection: " + QString::number(connectid);

		QStandardItem* item = new QStandardItem(strshow);

		item->setData(QVariant::fromValue(connectid),Qt::UserRole + 1);

		m_connectlistmodel->appendRow(item);

		ui.connectlist->setModel(m_connectlistmodel);
	
		paircount ++;
	}

		//ui.putdensity->setText(QString::number(paircount)) ;
}

void RbPointToolWidget::Objlistclicked(const QModelIndex &index)
{
#if(0)
	QStandardItem * item = m_objlistmodel->itemFromIndex(index);
	if (item > 0)
	{
		//QVariant & var = itemmap.begin().value();
		unsigned int objid = index.data(Qt::UserRole + 1).value<unsigned int>();//(unsigned int)var.data();
		
		CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();

		if(pTraining)
		{
			CBasicTraining::MAP_I_OBJ & dynobjmap = pTraining->m_mapIDDynObj;

			CBasicTraining::MAP_I_OBJ::iterator itor = dynobjmap.find(objid);

			if(itor != dynobjmap.end())
			{
					CDynamicObject * dynobj = itor->second;

					m_dynobjsel = dynobj;
	
					PathPutTool::GetCurrentTool()->SetObjectInEdit(dynobj);
					
					bool visible = m_dynobjsel->GetVisible();
					m_dynobjsel->SetVisible(!visible);
			}
		}
	}
#else
	CBasicTraining * pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();

	pTraining->SetEditorMode(true);

	MouseInput::GetInstance()->SetProcessMouseEvent(true);

// 	MouseInput::GetInstance()->SetDefaultPosition(Ogre::Vector3(8.0540066f , -4.7864556f , -4.2675395f));
// 
// 	MouseInput::GetInstance()->SetDefaultOrientation(Ogre::Quaternion(-0.50260752f , 0.82407808f , 0.19477108f , 0  ));

	QStandardItem * item = m_objlistmodel->itemFromIndex(index);
	if (item > 0)
	{
			unsigned int objid = index.data(Qt::UserRole + 1).value<unsigned int>();

			if(pTraining)
			{
				VeinConnectObject * veinconn = dynamic_cast<VeinConnectObject*>(pTraining->GetOrgan(objid));
					
				if(veinconn)
				{
					PathPutTool::GetCurrentTool()->AddConnectionFromVeinObject(veinconn);
				}
				/*CBasicTraining::MAP_I_OBJ & dynobjmap = pTraining->m_mapIDDynObj;


					CBasicTraining::MAP_I_OBJ::iterator itor = dynobjmap.find(objid);

					if(itor != dynobjmap.end())
					{
							CDynamicObject * dynobj = itor->second;
							
							DynamicStripsObject* dynstrip = dynobj->GetDynamicStrip();

							PathPutTool::GetCurrentTool()->addconnectionFromStrips(dynstrip);
		
							pTraining->RemoveDynamicObject(dynobj->GetEnmDynamicObjType());
					}
				*/
			}
		}
#endif
}

void RbPointToolWidget::PointPathlistclicked(const QModelIndex &index)
{
		QStandardItem * item = m_pathlistmodel->itemFromIndex(index);
		if (item > 0)
		{
			int pathid = index.data(Qt::UserRole + 1).value<unsigned int>();
			PathPutTool::GetCurrentTool()->SetPathToEdit(pathid);
		}
}


void RbPointToolWidget::ConnectListclicked(const QModelIndex &index)
{
	QStandardItem * item = m_connectlistmodel->itemFromIndex(index);
	if (item > 0)
	{
		int connectid = index.data(Qt::UserRole + 1).value< int>();
		PathPutTool::GetCurrentTool()->SetCurrentSelectedConnect(connectid);
	}
		
}
//==========================================================================================
void RbPointToolWidget::on_putdensityTextChanged(const QString & str)
{
	float density = str.toFloat();
	if(density < FLT_EPSILON)
	   density = 0.01f;
}
//==========================================================================================
void RbPointToolWidget::on_delselconnect()
{
	PathPutTool::GetCurrentTool()->RemoveSelectedCluster();
}
//==========================================================================================
void  RbPointToolWidget::cb_unselstateChanged(int value)
{
		PathPutTool::GetCurrentTool()->m_hiddenunselected = (value > 0 ? true:false);
}
void  RbPointToolWidget::cb_selstateChanged(int value)
{
		PathPutTool::GetCurrentTool()->m_hiddenselected = (value > 0 ? true:false);
}
void RbPointToolWidget::on_valueChanged(int value)
{
		float minimum = ui.putdensityslider->minimum();

		float maximum = ui.putdensityslider->maximum();

		float percent = (value-minimum) / (maximum-minimum);

		if (percent < 0)
			percent = 0;

		if (percent > 1)
			percent = 1;

		float min = 0.05f;
		float max = 0.8f;

		PathPutTool::s_putdensity = min*(1-percent)+max*percent;
}

void RbPointToolWidget::on_changeAdheHeight(int value)
{
		float minimum = ui.putdensityslider->minimum();

		float maximum = ui.putdensityslider->maximum();

		float percent = (value-minimum) / (maximum-minimum);

		if (percent < 0)
			percent = 0;

		if (percent > 1)
			percent = 1;

		float min = 0.0f;
		float max = 1.0f;

		float v = min*(1-percent)+max*percent;

		PathPutTool::GetCurrentTool()->ChangeSelectedPairSuspendDistInPartA(v);
		PathPutTool::GetCurrentTool()->ChangeSelectedPairSuspendDistInPartB(v);
}

void RbPointToolWidget::on_changeAdheHeightA(int value)
{
	float minimum = ui.putdensityslider->minimum();

		float maximum = ui.putdensityslider->maximum();

		float percent = (value-minimum) / (maximum-minimum);

		if (percent < 0)
			percent = 0;

		if (percent > 1)
			percent = 1;

		float min = 0.0f;
		float max = 1.0f;

		float v = min*(1-percent)+max*percent;

		PathPutTool::GetCurrentTool()->ChangeSelectedPairSuspendDistInPartA(v);
}
//=======================================================================================
void RbPointToolWidget::on_changeAdheHeightB(int value)
{
		float minimum = ui.putdensityslider->minimum();

		float maximum = ui.putdensityslider->maximum();

		float percent = (value-minimum) / (maximum-minimum);

		if (percent < 0)
			percent = 0;

		if (percent > 1)
			percent = 1;

		float min = 0.0f;
		float max = 1.0f;

		float v = min*(1-percent)+max*percent;

		PathPutTool::GetCurrentTool()->ChangeSelectedPairSuspendDistInPartB(v);
}
//=======================================================================================
void RbPointToolWidget::onRestoreButtonClicked()
{
    if (!m_pCamera) return;

    static const Ogre::Vector3 pos = m_initCamPos;
    static const Ogre::Vector3 dir = m_initCamDir.normalisedCopy();
    static const Ogre::Radian fovy = m_camFovy_radian;
    static const float asp = m_fInitAspectRatio;
//     m_pCamera->getParentSceneNode()->setPosition(Ogre::Vector3(1.51796, -12.1484, 5.965401));
//     m_pCamera->getParentSceneNode()->setDirection(Ogre::Vector3(0,1,0));
    m_pCamera->setFOVy(Ogre::Degree(fovy));
    m_pCamera->setAspectRatio(asp);
    InitCameraSet();
}

//init light set
    

void RbPointToolWidget::InitCameraSet()
{
    m_pCamera = CTrainingMgr::Instance()->GetCurTraining()->m_pLargeCamera;
    //Ogre::Vector3 camPos = m_pCamera->getRealPosition();
	Ogre::Vector3 camPos = m_pCamera->getParentNode()->getPosition();
    //Ogre::Vector3 camdir = m_pCamera->getRealDirection().normalisedCopy();
	Ogre::Quaternion camdir = m_pCamera->getParentNode()->getOrientation();
    Ogre::Real fovy = m_pCamera->getFOVy().valueDegrees();
    float orthoWindow_w = m_pCamera->getOrthoWindowWidth();
    float orthoWindow_h = m_pCamera->getOrthoWindowHeight();
    float AspectRatio = m_pCamera->getAspectRatio();


//     m_initCamPos = camPos;
//     m_initCamDir = camdir;
    m_camFovy_radian = m_pCamera->getFOVy();
    m_fInitAspectRatio = AspectRatio;

    float pos_x = camPos.x; LIMIT_ZERO_RANGE(pos_x);
    float pos_y = camPos.y; LIMIT_ZERO_RANGE(pos_y);
    float pos_z = camPos.z; LIMIT_ZERO_RANGE(pos_z);

    float dir_x = camdir.x; LIMIT_ZERO_RANGE(dir_x);
    float dir_y = camdir.y; LIMIT_ZERO_RANGE(dir_y);
    float dir_z = camdir.z; LIMIT_ZERO_RANGE(dir_z);

    float f_fovy = fovy;    LIMIT_ZERO_RANGE(f_fovy);

    SLIDER_INIT_SET(ui.hor_cam_pos_x, ui.le_cam_pos_x, -10000, 10000, pos_x, 1) ui.le_cam_pos_x->setText(QString::number(pos_x));
    SLIDER_INIT_SET(ui.hor_cam_pos_y, ui.le_cam_pos_y, -10000, 10000, pos_y, 1) ui.le_cam_pos_y->setText(QString::number(pos_y));
    SLIDER_INIT_SET(ui.hor_cam_pos_z, ui.le_cam_pos_z, -10000, 10000, pos_z, 1) ui.le_cam_pos_z->setText(QString::number(pos_z)); 

    SLIDER_INIT_SET1(ui.hor_cam_dir_x, ui.le_cam_dir_x, -10000, 10000, dir_x, 1) ui.le_cam_dir_x->setText(QString::number(dir_x));
    SLIDER_INIT_SET1(ui.hor_cam_dir_y, ui.le_cam_dir_y, -10000, 10000, dir_y, 1) ui.le_cam_dir_y->setText(QString::number(dir_y));
    SLIDER_INIT_SET1(ui.hor_cam_dir_z, ui.le_cam_dir_z, -10000, 10000, dir_z, 1) ui.le_cam_dir_z->setText(QString::number(dir_z));

    SLIDER_INIT_SET(ui.hor_cam_ortho_w, ui.le_cam_ortho_w, 0, 2000, orthoWindow_w/100.0f, 1) ui.le_cam_ortho_w->setText(QString::number(orthoWindow_w));
    SLIDER_INIT_SET(ui.hor_cam_ortho_h, ui.le_cam_ortho_h, 0, 2000, orthoWindow_h/100.0f, 1) ui.le_cam_ortho_h->setText(QString::number(orthoWindow_h));


    SLIDER_INIT_SET(ui.hor_cam_fovy, ui.le_cam_fovy, 0, 10000, f_fovy, 1) ui.le_cam_fovy->setText(QString::number(f_fovy));
    SLIDER_INIT_SET(ui.hor_cam_asp, ui.le_cam_asp, 0, 1000, AspectRatio, 1) ui.le_cam_asp->setText(QString::number(AspectRatio));

}

void RbPointToolWidget::InitLightSet()
{
	std::map<Ogre::String,Ogre::Light*>::iterator iter;
	std::map<Ogre::String,Ogre::Light*> LightMap = CLightMgr::Instance()->GetLightMAP();
	if(LightMap.empty())
		return;
	iter=LightMap.begin();
	for(; iter!=LightMap.end(); iter++)
	{
		Ogre::String str = static_cast<Ogre::String>(iter->first);
		ui.comboBox_Name->addItem(QWidget::tr(str.c_str()));  
	}
	ui.comboBox_Name->setCurrentIndex(0);
	
	Ogre::Light* light = CLightMgr::Instance()->GetLight((Ogre::String)(ui.comboBox_Name->currentText().toStdString()));

	SLIDER_INIT_SET(ui.horizontalSlider_LightPos_X, ui.lineEdit_pos_x, -100, 100, light->getPosition().x/100.0f, 1) ui.lineEdit_pos_x->setText(QString::number(light->getPosition().x));
	SLIDER_INIT_SET(ui.horizontalSlider_LightPos_Y, ui.lineEdit_pos_y, -100, 100, light->getPosition().y/100.0f, 1) ui.lineEdit_pos_y->setText(QString::number(light->getPosition().y));
	SLIDER_INIT_SET(ui.horizontalSlider_LightPos_Z, ui.lineEdit_pos_z, -100, 100, light->getPosition().z/100.0f, 1) ui.lineEdit_pos_z->setText(QString::number(light->getPosition().z));

	SLIDER_INIT_SET(ui.horizontalSlider_LightDir_X, ui.lineEdit_dir_x, -100.0f, 100.0f, light->getDirection().x, 1)
	SLIDER_INIT_SET(ui.horizontalSlider_LightDir_Y, ui.lineEdit_dir_y, -100.0f, 100.0f, light->getDirection().y, 1)
	SLIDER_INIT_SET(ui.horizontalSlider_LightDir_Z, ui.lineEdit_dir_z, -100.0f, 100.0f, light->getDirection().z, 1)

	SLIDER_INIT_SET(ui.horizontalSlider_Diffuse_R, ui.lineEdit_diff_r, 0.0f, 100.0f, light->getDiffuseColour().r, 1)
	SLIDER_INIT_SET(ui.horizontalSlider_Diffuse_G, ui.lineEdit_diff_g, 0.0f, 100.0f, light->getDiffuseColour().g, 1)
	SLIDER_INIT_SET(ui.horizontalSlider_Diffuse_B, ui.lineEdit_diff_b, 0.0f, 100.0f, light->getDiffuseColour().b, 1)

	SLIDER_INIT_SET(ui.horizontalSlider_Specular_R, ui.lineEdit_spe_r, 0.0f, 100.0f, light->getSpecularColour().r, 1)
	SLIDER_INIT_SET(ui.horizontalSlider_Specular_G, ui.lineEdit_spe_g, 0.0f, 100.0f, light->getSpecularColour().g, 1)
	SLIDER_INIT_SET(ui.horizontalSlider_Specular_B, ui.lineEdit_spe_b, 0.0f, 100.0f, light->getSpecularColour().b, 1)

	SLIDER_INIT_SET(ui.horizontalSlider_Atte_X, ui.lineEdit_Atte_X, 0.0f, 1000.0f, light->getAttenuationRange()/100.0f, 1)	 ui.lineEdit_Atte_X->setText(QString::number(light->getAttenuationRange()));
	SLIDER_INIT_SET(ui.horizontalSlider_Atte_Y, ui.lineEdit_Atte_Y, 0.0f, 100.0f, light->getAttenuationConstant(), 1) ui.lineEdit_Atte_Y->setText(QString::number(light->getAttenuationConstant()));
	SLIDER_INIT_SET(ui.horizontalSlider_Atte_Z, ui.lineEdit_Atte_Z, 0.0f, 100.0f, light->getAttenuationLinear()*100, 1)	 ui.lineEdit_Atte_Z->setText(QString::number(light->getAttenuationLinear()));
	SLIDER_INIT_SET(ui.horizontalSlider_Atte_W, ui.lineEdit_Atte_W, 0.0f, 100.0f, light->getAttenuationQuadric()*100, 1)	 ui.lineEdit_Atte_W->setText(QString::number(light->getAttenuationQuadric()));

	Ogre::ColourValue cv = MXOgreWrapper::Instance()->GetDefaultSceneManger()->getAmbientLight();

	SLIDER_INIT_SET(ui.horizontalSlider_Ambient_R, ui.lineEdit_Ambient_R, 0.0f, 100.0f, cv.r, 1)
	SLIDER_INIT_SET(ui.horizontalSlider_Ambient_G, ui.lineEdit_Ambient_G, 0.0f, 100.0f, cv.g, 1)
	SLIDER_INIT_SET(ui.horizontalSlider_Ambient_B, ui.lineEdit_Ambient_B, 0.0f, 100.0f, cv.b, 1)
 	
	ui.comboBox_LightType->addItem(QWidget::tr("LT_POINT"));
 	ui.comboBox_LightType->addItem(QWidget::tr("LT_DIRECTIONAL "));
 	ui.comboBox_LightType->addItem(QWidget::tr("LT_SPOTLIGHT"));
 
 	ui.comboBox_LightType->setCurrentIndex(light->getType());


	if(light->getType() != Ogre::Light::LightTypes::LT_SPOTLIGHT)
	{
		ui.horizontalSlider_InnerAngle->setEnabled(false);
		ui.lineEdit_InnerAngle->setEnabled(false);
		ui.horizontalSlider_OuterAngle->setEnabled(false);
		ui.lineEdit_OuterAngle->setEnabled(false);
		ui.horizontalSlider_FallOff->setEnabled(false);
		ui.lineEdit_FallOff->setEnabled(false);
	}
	else
	{
		SLIDER_INIT_SET(ui.horizontalSlider_InnerAngle, ui.lineEdit_InnerAngle, 0.0f, 314.0f, light->getSpotlightInnerAngle().valueDegrees()*0.017354, 1)ui.lineEdit_InnerAngle->setText(QString::number(light->getSpotlightInnerAngle().valueDegrees()*0.017354));
		SLIDER_INIT_SET(ui.horizontalSlider_OuterAngle, ui.lineEdit_OuterAngle, 0.0f, 314.0f, light->getSpotlightOuterAngle().valueDegrees()*0.017354, 1)ui.lineEdit_OuterAngle->setText(QString::number(light->getSpotlightOuterAngle().valueDegrees()*0.017354));
		SLIDER_INIT_SET(ui.horizontalSlider_FallOff, ui.lineEdit_FallOff, 0.0f, 1000.0f, light->getSpotlightFalloff(), 1)
	}
}

void RbPointToolWidget::SetLight()
{
	std::map<Ogre::String,Ogre::Light*> LightMap = CLightMgr::Instance()->GetLightMAP();
	if(LightMap.empty())
		return;
 	Ogre::String lightName = ui.comboBox_Name->currentText().toStdString();
 	Ogre::Light* light = CLightMgr::Instance()->GetLight(lightName);
	switch(ui.comboBox_LightType->currentIndex())
	{
	case 0:light->setType(Ogre::Light::LightTypes::LT_POINT);break;
	case 1:light->setType(Ogre::Light::LightTypes::LT_DIRECTIONAL);break;
	case 2:light->setType(Ogre::Light::LightTypes::LT_SPOTLIGHT);break;
	}
	Ogre::Vector3 poslight = Ogre::Vector3(ui.horizontalSlider_LightPos_X->value()/10.0f, ui.horizontalSlider_LightPos_Y->value()/10.0f, ui.horizontalSlider_LightPos_Z->value()/10.0f);
	Ogre::Vector3 posdir = Ogre::Vector3(ui.horizontalSlider_LightDir_X->value()/100.0f,ui.horizontalSlider_LightDir_Y->value()/100.0f, ui.horizontalSlider_LightDir_Z->value()/100.0f);
  	light->setPosition(poslight);
  	light->setDirection(posdir);
	light->setDiffuseColour(Ogre::ColourValue(ui.horizontalSlider_Diffuse_R->value()/100.0f,ui.horizontalSlider_Diffuse_G->value()/100.0f, ui.horizontalSlider_Diffuse_B->value()/100.0f));
   	light->setSpecularColour(Ogre::ColourValue(ui.horizontalSlider_Specular_R->value()/100.0f, ui.horizontalSlider_Specular_G->value()/100.0f, ui.horizontalSlider_Specular_B->value()/100.0f));
	light->setAttenuation(ui.horizontalSlider_Atte_X->value(), ui.horizontalSlider_Atte_Y->value()/100.0f, ui.horizontalSlider_Atte_Z->value()/10000.0f, ui.horizontalSlider_Atte_W->value()/10000.0f);

	Ogre::SceneManager* mSceneMgr = MXOgreWrapper::Instance()->GetDefaultSceneManger();
	mSceneMgr->setAmbientLight(Ogre::ColourValue(ui.horizontalSlider_Ambient_R->value()/100.0f, ui.horizontalSlider_Ambient_G->value()/100.0f, ui.horizontalSlider_Ambient_B->value()/100.0f));

 	if(ui.comboBox_LightType->currentIndex() == Ogre::Light::LightTypes::LT_SPOTLIGHT)
 	{
		light->setSpotlightInnerAngle(Ogre::Radian(ui.horizontalSlider_InnerAngle->value()/100.0f));
 		light->setSpotlightOuterAngle(Ogre::Radian(ui.horizontalSlider_OuterAngle->value()/100.0f));
 		light->setSpotlightFalloff(ui.horizontalSlider_FallOff->value()/100.0f);
 	}
}



void RbPointToolWidget::showEvent(QShowEvent *)
{
	gEditCamera->InitGlobalEditorCamera();
}

void RbPointToolWidget::on_AutoGenConnect()
{
    PathPutTool::GetCurrentTool()->AutoGenAdhersion();    
}