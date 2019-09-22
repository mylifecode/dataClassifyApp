#include "Inception.h"
#include "RbPhantomBoxTest.h"
#include <QResizeEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QTextStream>
#include <QFile>
//////////////////////////////////////////////////////////////////////////
#include "InputSystem.h"
#include "MisRobotInput.h"
#include "EngineCore.h"
#include "BasicTraining.h"
#include "MXApplication.h"
#include "XMLWrapperTraining.h"
#include "MxDefine.h"

RbPhantomBoxTest::RbPhantomBoxTest(QWidget *parent /* = NULL */)
:QDialog( parent)
{
	ui.setupUi(this);
	setOtherGroupPos(1, 1, 1);
	setBoxGroupPos(1, 1, 1);
	setBoxGroupRot(1, 1, 1);
	setBoxGroupSize(1, 1, 1);
	setInstrumentGroupPos(1, 1, 1);
	//ui.SpinBox_other_pos_x->isRead();
	Mx::setWidgetStyle(this,"qss:RbPhantomBoxTest.qss");
}

RbPhantomBoxTest::~RbPhantomBoxTest()
{

}

void RbPhantomBoxTest::initWidgetValue()
{
	setOtherGroupPos(1, 1, 1);
	setBoxGroupPos(1, 1, 1);
	setBoxGroupRot(1, 1, 1);
	setBoxGroupSize(1, 1, 1);
	setInstrumentGroupPos(1, 1, 1);
}

void RbPhantomBoxTest::initWidgetValue(Ogre::String strData)
{
	std::stringstream ssData(strData);
	
	Ogre::Vector3 otherpos(1, 1,1);
	Ogre::Vector3 boxpos(1, 1,1);
	Ogre::Vector3 boxrot(1, 1,1);
	Ogre::Vector3 boxsize(1, 1,1);
	Ogre::Vector3 Instrumentpos(1, 1,1);

	ssData >> otherpos.x >> otherpos.y >> otherpos.z;
	ssData >> boxpos.x >> boxpos.y >> boxpos.z;
	ssData >> boxsize.x >> boxsize.y >> boxsize.z;
	ssData >> boxrot.x >> boxrot.y >> boxrot.z;
	ssData >> Instrumentpos.x >> Instrumentpos.y >> Instrumentpos.z;

	setOtherGroupPos(otherpos.x, otherpos.y, otherpos.z);
	setBoxGroupPos(boxpos.x, boxpos.y, boxpos.z);
	setBoxGroupRot(boxrot.x, boxrot.y, boxrot.z);
	setBoxGroupSize(boxsize.x, boxsize.y, boxsize.z);
	setInstrumentGroupPos(Instrumentpos.x, Instrumentpos.y, Instrumentpos.z);
}

Ogre::String RbPhantomBoxTest::getWidgetValueString()
{
	Ogre::String ssData = "";

	Ogre::Vector3 otherpos(1, 1,1);
	Ogre::Vector3 boxpos(1, 1,1);
	Ogre::Vector3 boxrot(1, 1,1);
	Ogre::Vector3 boxsize(1, 1,1);
	Ogre::Vector3 instrumentpos(1, 1, 1);
	Ogre::Vector3 invalidpos(0, 0, 0);
	
	getOtherGroupPos(otherpos);
	getBoxGroupPos(boxpos);
	getBoxGroupRot(boxrot);
	getBoxGroupSize(boxsize);
	getInstrumentGroupPos(instrumentpos);

	ssData = Ogre::StringConverter::toString(otherpos) +" "+ Ogre::StringConverter::toString(boxpos)+" "+
			 Ogre::StringConverter::toString(boxsize) +" "+ Ogre::StringConverter::toString(boxrot)+" "+
			Ogre::StringConverter::toString(instrumentpos) + " " +Ogre::StringConverter::toString(invalidpos);
	
	return ssData;

}

bool RbPhantomBoxTest::event( QEvent *e )
{
    bool rtv = __super::event( e );
    switch (e->type())
    {
    case QEvent::Resize:
        {
            QResizeEvent *_e = static_cast<QResizeEvent*>(e);
        }
        //else cont.
        break;
    }
    return rtv;
}

void RbPhantomBoxTest::mousePressEvent(QMouseEvent *event)
{
    return;
    if(event->button() == Qt::LeftButton)
    {
        oldPos = event->globalPos();
        press = true;
    }
}

void RbPhantomBoxTest::mouseMoveEvent(QMouseEvent *event)
{
    return;
    if (press)
    {
        this->move(this->pos() + event->globalPos() - oldPos);

        oldPos = event->globalPos();
    }
}

void RbPhantomBoxTest::mouseReleaseEvent(QMouseEvent *event)
{
    return;
    press = false;
}

void RbPhantomBoxTest::closeEvent(QCloseEvent * e)
{
	if(e->Close)
	{
		__super::closeEvent(e);
		emit closeExit();
	}
}

void RbPhantomBoxTest::on_mButtonReset_clicked()
{
	Inception::Instance()->EmitShowPhantomBoxDebug();
}

void RbPhantomBoxTest::saveWidgetValueToFile(Ogre::String filename, Ogre::String strData)
{
	QFile file( QString::fromStdString(filename));  
	file.open(QIODevice::WriteOnly);  
	file.close();  
	file.open(QIODevice::ReadWrite);  
	if (file.isOpen())  
	{  
		QTextStream txtout(&file);
		txtout << QString::fromStdString(strData) << endl;
 		txtout.flush();
 		file.close();
	}  
}

void RbPhantomBoxTest::on_mButtonSave_clicked()
{
	//////////////////////////////////////////////////////////////////////////
	EngineCore* enginecore = (EngineCore *)(static_cast<MXApplication*>(qApp))->GetEngineCore();
	CBasicTraining *   pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	if (!pTraining)  return; 
	if (!pTraining->m_pTrainingConfig)  return; 

	if (pTraining->m_pTrainingConfig->m_DebugForDeviceWorkspaceLeft)
	{
		saveWidgetValueToFile("auto_config_left.txt", getWidgetValueString());

	}else if (pTraining->m_pTrainingConfig->m_DebugForDeviceWorkspaceRight)
	{
		saveWidgetValueToFile("auto_config_right.txt", getWidgetValueString());
	}
	
};

void RbPhantomBoxTest::resetWidgetValue()
{
	//////////////////////////////////////////////////////////////////////////
	EngineCore* enginecore = (EngineCore *)(static_cast<MXApplication*>(qApp))->GetEngineCore();
	CBasicTraining *   pTraining = (CBasicTraining *)(static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	if (!pTraining)  return; 
	if (!pTraining->m_pTrainingConfig)  return; 
	if (pTraining->m_pTrainingConfig->m_DebugForDeviceWorkspaceLeft)
	{
		initWidgetValue(pTraining->m_pTrainingConfig->m_DataForDeviceWorkspaceLeft);
	}else if (pTraining->m_pTrainingConfig->m_DebugForDeviceWorkspaceRight)
	{
		initWidgetValue(pTraining->m_pTrainingConfig->m_DataForDeviceWorkspaceRight);
	}
}

void RbPhantomBoxTest::setOtherGroupPos(double x, double y , double z)
{
	ui.SpinBox_other_pos_x->setValue(x);
	ui.SpinBox_other_pos_y->setValue(y);
	ui.SpinBox_other_pos_z->setValue(z);
}

void RbPhantomBoxTest::getOtherGroupPos(Ogre::Vector3& pos)
{
	pos.x =ui.SpinBox_other_pos_x->value();
	pos.y =ui.SpinBox_other_pos_y->value();
	pos.z =ui.SpinBox_other_pos_z->value();
}

void RbPhantomBoxTest::setBoxGroupPos(double x, double y , double z)
{
 	ui.SpinBox_box_pos_x->setValue(x);
 	ui.SpinBox_box_pos_y->setValue(y);
 	ui.SpinBox_box_pos_z->setValue(z);
}

void RbPhantomBoxTest::getBoxGroupPos(Ogre::Vector3& pos)
{
	pos.x =ui.SpinBox_box_pos_x->value();
	pos.y =ui.SpinBox_box_pos_y->value();
	pos.z =ui.SpinBox_box_pos_z->value();
}

void RbPhantomBoxTest::setBoxGroupRot(double x, double y , double z)
{
	ui.SpinBox_box_rot_x->setValue(x);
	ui.SpinBox_box_rot_y->setValue(y);
	ui.SpinBox_box_rot_z->setValue(z);
}

void RbPhantomBoxTest::getBoxGroupRot(Ogre::Vector3& rot)
{
	rot.x =ui.SpinBox_box_rot_x->value();
	rot.y =ui.SpinBox_box_rot_y->value();
	rot.z =ui.SpinBox_box_rot_z->value();
}

void RbPhantomBoxTest::setBoxGroupSize(double x, double y , double z)
{
	ui.SpinBox_box_size_x->setValue(x);
	ui.SpinBox_box_size_y->setValue(y);
	ui.SpinBox_box_size_z->setValue(z);
}

void RbPhantomBoxTest::getBoxGroupSize(Ogre::Vector3& size)
{
	size.x =ui.SpinBox_box_size_x->value();
	size.y =ui.SpinBox_box_size_y->value();
	size.z =ui.SpinBox_box_size_z->value();
}

void RbPhantomBoxTest::setInstrumentGroupPos(double x, double y , double z)
{
	ui.SpinBox_ins_pos_x->setValue(x);
	ui.SpinBox_ins_pos_y->setValue(y);
	ui.SpinBox_ins_pos_z->setValue(z);
}

void RbPhantomBoxTest::getInstrumentGroupPos(Ogre::Vector3& pos)
{
	pos.x =ui.SpinBox_ins_pos_x->value();
	pos.y =ui.SpinBox_ins_pos_y->value();
	pos.z =ui.SpinBox_ins_pos_z->value();
}

